/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkSoundOGG
// Glk OGG sounds
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkSoundOGG.h"
#include <math.h>

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
// Class for OGG sound loader
/////////////////////////////////////////////////////////////////////////////

// Get file prefix for sounds supported by this loader
LPCTSTR CWinGlkOGGSoundLoader::GetFilePrefix(void)
{
  return "mus";
}

// Get the file extensions for sounds supported for this loader
int CWinGlkOGGSoundLoader::GetNumberFileExtensions(void)
{
  return 1;
}

LPCTSTR CWinGlkOGGSoundLoader::GetFileExtension(int iExtIndex)
{
  return "ogg";
}

// Get the identifier for sounds supported for this loader
glui32 CWinGlkOGGSoundLoader::GetIdentifier(void)
{
  return giblorb_make_id('O','G','G','V');
}

// Get a sound object
CWinGlkSound* CWinGlkOGGSoundLoader::GetSound(LPCTSTR pszFileName)
{
  return new CWinGlkOGGSound(pszFileName);
}

CWinGlkSound* CWinGlkOGGSoundLoader::GetSound(BYTE* pData, int iLength)
{
  return new CWinGlkOGGSound(pData,iLength);
}

/////////////////////////////////////////////////////////////////////////////
// Class for OGG sounds
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkOGGSound,CWinGlkSound);

CWinGlkOGGSound::CWinGlkOGGSound(BYTE* pData, int iLength) : CWinGlkSound(pData,iLength)
{
  m_StreamOpen = false;
  m_pReadPtr = NULL;
  m_Duration = 0;
}

CWinGlkOGGSound::CWinGlkOGGSound(LPCTSTR pszFileName) : CWinGlkSound(pszFileName)
{
  m_StreamOpen = false;
  m_pReadPtr = NULL;
  m_Duration = 0;
}

CWinGlkOGGSound::~CWinGlkOGGSound()
{
  RemoveFromList();
  if (m_StreamOpen)
    ov_clear(&m_Stream);
}

bool CWinGlkOGGSound::Play(int iRepeat, int iVolume)
{
  m_pReadPtr = m_pData;
  if (m_pReadPtr == NULL)
    return false;

  // Open the stream
  ov_callbacks VorbisCBs;
  VorbisCBs.read_func = VorbisRead;
  VorbisCBs.close_func = VorbisClose;
  VorbisCBs.seek_func = VorbisSeek;
  VorbisCBs.tell_func = VorbisTell;
  if (ov_open_callbacks(this,&m_Stream,NULL,0,VorbisCBs) < 0)
    return false;
  m_StreamOpen = true;
  vorbis_info* Info = ov_info(&m_Stream,-1);

  // Create a buffer
  if (CreateBuffer(Info->channels,Info->rate,16) == false)
    return false;

  // Set the duration of the sample
  if (iRepeat > 0)
    m_Duration = (DWORD)ceil(1000.0 * iRepeat * ov_time_total(&m_Stream,-1));
  else
    m_Duration = -1;

  // Fill the buffer with sample data
  m_iRepeat = (iRepeat < 0) ? -1 : iRepeat - 1;
  if (FillBuffer(GetBufferSize()) == false)
    return false;

  // Set the volume for the buffer
  SetVolume(iVolume);

  // Start the buffer playing
  return PlayBuffer();
}

bool CWinGlkOGGSound::IsPlaying(void)
{
  return IsBufferPlaying();
}

void CWinGlkOGGSound::SetVolume(int iVolume)
{
  // The SetVolume() call to DirectSound requires a volume
  // in 100ths of a decibel.
  SetBufferVolume((LONG)DecibelVolume(iVolume) * 100L);
}

// Write sample data into the supplied PCM sample buffers
void CWinGlkOGGSound::WriteSampleData(unsigned char* pSample, int iSampleLen)
{
  int iStream, iCurrent = 0;
  while (iCurrent < iSampleLen)
  {
    long lRead =
      ov_read(&m_Stream,(char*)(pSample+iCurrent),(iSampleLen-iCurrent),0,2,1,&iStream);
    if (lRead > 0)
      iCurrent += lRead;
    else
    {
      if (m_iRepeat > 0)
      {
        ov_pcm_seek(&m_Stream,0);
        m_iRepeat--;
      }
      else if (m_iRepeat == -1)
        ov_pcm_seek(&m_Stream,0);
      else
      {
        while (iCurrent < iSampleLen)
          pSample[iCurrent++] = 0;
      }
    }
  }
}

// Check if the sound has finished playing
bool CWinGlkOGGSound::IsSoundOver(DWORD Tick)
{
  if (m_Playing == false)
    return true;

  // Check if sound is playing forever
  if (m_Duration < 0)
    return false;
  return (Tick > m_StartTime + m_Duration);
}

// Get a type identifier for the sound
int CWinGlkOGGSound::GetType(void)
{
  return (int)'O';
}

size_t CWinGlkOGGSound::VorbisRead(void* ptr, size_t byteSize, size_t sizeToRead, void* src)
{
  CWinGlkOGGSound* sound = (CWinGlkOGGSound*)src;

  int iRead = (int)sizeToRead;
  int iMaxRead = (int)((sound->m_pData + sound->m_iLength - sound->m_pReadPtr) / byteSize);
  if (iRead > iMaxRead)
    iRead = iMaxRead;

  memcpy(ptr,sound->m_pReadPtr,iRead * byteSize);
  sound->m_pReadPtr += iRead * byteSize;
  return iRead;
}

int CWinGlkOGGSound::VorbisSeek(void* src, ogg_int64_t offset, int whence)
{
  CWinGlkOGGSound* sound = (CWinGlkOGGSound*)src;

  switch (whence)
  {
  case SEEK_SET:
    sound->m_pReadPtr = sound->m_pData + offset;
    return 0;
  case SEEK_CUR:
    sound->m_pReadPtr += offset;
    return 0;
  case SEEK_END:
    sound->m_pReadPtr = sound->m_pData + sound->m_iLength;
    return 0;
  }
  return -1;
}

int CWinGlkOGGSound::VorbisClose(void*)
{
  return 0;
}

long CWinGlkOGGSound::VorbisTell(void* src)
{
  CWinGlkOGGSound* sound = (CWinGlkOGGSound*)src;

  return (long)(sound->m_pReadPtr - sound->m_pData);
}
