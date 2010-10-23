/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkSoundSONG
// Glk SONG module sounds
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkDll.h"
#include "GlkSoundAIFF.h"
#include "GlkSoundSONG.h"

extern "C"
{
#include "gi_blorb.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Class for SONG sound loader
/////////////////////////////////////////////////////////////////////////////

// Get file prefix for sounds supported by this loader
LPCTSTR CWinGlkSONGSoundLoader::GetFilePrefix(void)
{
  return "mus";
}

// Get the file extensions for sounds supported for this loader
int CWinGlkSONGSoundLoader::GetNumberFileExtensions(void)
{
  return 2;
}

LPCTSTR CWinGlkSONGSoundLoader::GetFileExtension(int iExtIndex)
{
  switch (iExtIndex)
  {
  case 0:
    return "sng";
  case 1:
    return "song";
  }
  return "";
}

// Get the identifier for sounds supported for this loader
glui32 CWinGlkSONGSoundLoader::GetIdentifier(void)
{
  return giblorb_make_id('S','O','N','G');
}

// Get a sound object
CWinGlkSound* CWinGlkSONGSoundLoader::GetSound(LPCTSTR pszFileName)
{
  return new CWinGlkSONGSound(pszFileName);
}

CWinGlkSound* CWinGlkSONGSoundLoader::GetSound(BYTE* pData, int iLength)
{
  return new CWinGlkSONGSound(pData,iLength);
}

/////////////////////////////////////////////////////////////////////////////
// Map of samples for transforming SONG into MOD
/////////////////////////////////////////////////////////////////////////////

class SampleMap : public CMap<int,int,CWinGlkAIFFSound*,CWinGlkAIFFSound*>
{
public:
  ~SampleMap()
  {
    // Discard all the loaded samples
    int i = 0;
    CWinGlkAIFFSound* pSample = NULL;
    POSITION pos = GetStartPosition();
    while (pos != NULL)
    {
      GetNextAssoc(pos,i,pSample);
      delete pSample;
    }
  }
};

/////////////////////////////////////////////////////////////////////////////
// Class for SONG sounds
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkSONGSound,CWinGlkMODSound);

CWinGlkSONGSound::CWinGlkSONGSound(BYTE* pData, int iLength) : CWinGlkMODSound(pData,iLength)
{
  m_pMODData = NULL;
  m_iMODLength = 0;
  CreateMOD();
}

CWinGlkSONGSound::CWinGlkSONGSound(LPCTSTR pszFileName) : CWinGlkMODSound(pszFileName)
{
  m_pMODData = NULL;
  m_iMODLength = 0;
  CreateMOD();
}

CWinGlkSONGSound::~CWinGlkSONGSound()
{
  delete[] m_pMODData;
}

// Get a pointer to the MOD data and the size of the data
void CWinGlkSONGSound::GetMODData(BYTE*& pData, int& iLength)
{
  pData = m_pMODData;
  iLength = m_iMODLength;
}

// Create a MOD in memory from the SONG
void CWinGlkSONGSound::CreateMOD(void)
{
  const int SAMPLE_NAME = 0;
  const int SAMPLE_LENGH = 22;
  const int SAMPLE_REPEAT_POS = 26;
  const int SAMPLE_REPEAT_LEN = 28;
  const int SAMPLE_SIZE = 30;

  m_iMODLength = m_iLength;

  // Loop over the samples
  SampleMap samples;
  for (int i = 0; i < 31; i++)
  {
    BYTE* sample = m_pData + 20 + i*SAMPLE_SIZE;
    if (*(sample + SAMPLE_NAME) != 0)
    {
      // Get the sample resource index
      int index = 0;
      if (sscanf((const char*)(sample + SAMPLE_NAME),"SND%d",&index) != 1)
        return;

      // Attempt to load the sample
      CWinGlkSound* snd = ((CGlkApp*)AfxGetApp())->LoadSound(index);
      if (snd == NULL)
        return;
      if (snd->IsKindOf(RUNTIME_CLASS(CWinGlkAIFFSound)) == FALSE)
        return;
      samples[index] = (CWinGlkAIFFSound*)snd;

      // Get details of the sample
      CWinGlkAIFFSound::SampleData data;
      if (((CWinGlkAIFFSound*)snd)->GetSampleData(data) == false)
        return;

      // Add to the total size of the MOD
      m_iMODLength += data.samples + 2;
    }
  }

  // Create a buffer for the MOD and put the SONG data at the start
  m_pMODData = new BYTE[m_iMODLength];
  ::CopyMemory(m_pMODData,m_pData,m_iLength);

  // Copy the samples into the buffer
  int writePos = m_iLength;
  for (int i = 0; i < 31; i++)
  {
    BYTE* sample = m_pData + 20 + i*SAMPLE_SIZE;
    if (*(sample + SAMPLE_NAME) != 0)
    {
      // Get the sample resource index
      int index = 0;
      sscanf((const char*)(sample + SAMPLE_NAME),"SND%d",&index);

      // Get the sample
      CWinGlkAIFFSound* aiff = samples[index];
      CWinGlkAIFFSound::SampleData data;
      aiff->GetSampleData(data);

      // Update the sample length
      WriteHeaderValue(i,SAMPLE_LENGH,data.samples+2);

      // Update repeat information
      if ((data.repeat1 != 0) && (data.repeat2 != 0))
      {
        WriteHeaderValue(i,SAMPLE_REPEAT_POS,data.repeat1+2);
        WriteHeaderValue(i,SAMPLE_REPEAT_LEN,data.repeat2-data.repeat1);
      }

      // Copy into the buffer, converting to 8-bit mono
      *(m_pMODData+(writePos++)) = 0;
      *(m_pMODData+(writePos++)) = 0;
      for (unsigned long j = 0; j < data.samples; j++)
        *(m_pMODData+writePos+j) = *(data.data+(j*data.channels*(data.bits>>3)));
      writePos += data.samples;
    }
  }
}

// Write a value into the MOD header
void CWinGlkSONGSound::WriteHeaderValue(int sample, int offset, unsigned long value)
{
  const int SAMPLE_SIZE = 30;
  unsigned short val = (unsigned short)(value>>1);

  BYTE* header = m_pMODData + 20 + (sample*SAMPLE_SIZE);
  header[offset] = (BYTE)(val >> 8);
  header[offset+1] = (BYTE)(val - (header[offset] << 8));
}
