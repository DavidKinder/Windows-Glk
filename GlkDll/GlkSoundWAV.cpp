/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkSoundWAV
// Glk WAV sounds
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkSoundWAV.h"
#include "GlkTime.h"
#include <math.h>
#include <mmreg.h>

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
// Class for WAV sound loader
/////////////////////////////////////////////////////////////////////////////

// Get file prefix for sounds supported by this loader
LPCTSTR CWinGlkWAVSoundLoader::GetFilePrefix(void)
{
  return "snd";
}

// Get the file extensions for sounds supported for this loader
int CWinGlkWAVSoundLoader::GetNumberFileExtensions(void)
{
  return 1;
}

LPCTSTR CWinGlkWAVSoundLoader::GetFileExtension(int iExtIndex)
{
  switch (iExtIndex)
  {
  case 0:
    return "wav";
  }
  return "";
}

// Get the identifier for sounds supported for this loader
glui32 CWinGlkWAVSoundLoader::GetIdentifier(void)
{
  return giblorb_make_id('W','A','V','E');
}

// Get a sound object
CWinGlkSound* CWinGlkWAVSoundLoader::GetSound(LPCTSTR pszFileName)
{
  return new CWinGlkWAVSound(pszFileName);
}

CWinGlkSound* CWinGlkWAVSoundLoader::GetSound(BYTE* pData, int iLength)
{
  return new CWinGlkWAVSound(pData,iLength);
}

/////////////////////////////////////////////////////////////////////////////
// Class for WAV sounds
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkWAVSound,CWinGlkSound);

CWinGlkWAVSound::CWinGlkWAVSound(BYTE* pData, int iLength) : CWinGlkSound(pData,iLength)
{
  m_pRenderPtr = NULL;
  m_pRenderMin = NULL;
  m_pRenderMax = NULL;
  m_Duration = 0;
}

CWinGlkWAVSound::CWinGlkWAVSound(LPCTSTR pszFileName) : CWinGlkSound(pszFileName)
{
  m_pRenderPtr = NULL;
  m_pRenderMin = NULL;
  m_pRenderMax = NULL;
  m_Duration = 0;
}

CWinGlkWAVSound::~CWinGlkWAVSound()
{
  RemoveFromList();
}

bool CWinGlkWAVSound::Play(int iRepeat, int iVolume, bool PauseState)
{
  SampleData data;
  if (GetSampleData(data) == false)
    return false;

  // Create a buffer
  if (CreateBuffer(data.channels,data.rate,data.bits) == false)
    return false;

  // Set the duration of the sample
  if (iRepeat > 0)
    m_Duration = (DWORD)ceil((data.samples * iRepeat * 1000.0) / data.rate);
  else
    m_Duration = -1;

  // Set up the current position for rendering wave data
  m_pRenderPtr = data.data;
  m_pRenderMin = m_pRenderPtr;
  m_pRenderMax = data.data + ((data.bits>>3)*data.samples*data.channels);

  // Fill the buffer with sample data
  m_iRepeat = (iRepeat < 0) ? -1 : iRepeat - 1;
  if (FillBuffer(GetBufferSize()) == false)
    return false;

  // Set the volume for the buffer
  SetVolume(iVolume);

  // Start the buffer playing
  return PlayBuffer(PauseState);
}

bool CWinGlkWAVSound::IsPlaying(void)
{
  return m_Active;
}

void CWinGlkWAVSound::Pause(bool PauseState)
{
  CDSound::Pause(PauseState);

  DWORD now = ::GetTickCount();
  CSingleLock Lock(CDSoundEngine::GetSoundLock(),TRUE);

  // If pausing, reduce the sound duration by the amount already played
  if (PauseState)
  {
    if (m_Duration > 0)
    {
      m_Duration -= TickCountDiff(now,m_StartTime);
      if (m_Duration < 0)
        m_Duration = 0;
    }
  }

  // Update the start time to now when pausing or unpausing
  m_StartTime = now;
}

void CWinGlkWAVSound::SetVolume(int iVolume)
{
  // The SetVolume() call to DirectSound requires a volume
  // in 100ths of a decibel.
  SetBufferVolume((LONG)DecibelVolume(iVolume) * 100L);
}

// Write sample data into the supplied PCM sample buffers
void CWinGlkWAVSound::WriteSampleData(unsigned char* pSample, int iSampleLen)
{
  // WAV data is already in the format DirectSound expects: unsigned
  // 8-bit samples, or little-endian signed 16-bit samples.
  int bytes = m_Format.wBitsPerSample>>3;
  unsigned char silence = (bytes == 1) ? 0x80 : 0x00;

  for (int i = 0; i < iSampleLen; i += bytes)
  {
    if (CheckRenderPtr())
    {
      ::CopyMemory(pSample,m_pRenderPtr,bytes);
      m_pRenderPtr += bytes;
    }
    else
      ::FillMemory(pSample,bytes,silence);
    pSample += bytes;
  }
}

// Test that the current point into the WAV buffer is valid
bool CWinGlkWAVSound::CheckRenderPtr(void)
{
  if (m_pRenderPtr >= m_pRenderMax)
  {
    // Fail if this is the end of the last repeat
    if (m_iRepeat == 0)
      return false;

    // If not looping forever, decrement the repeat counter
    if (m_iRepeat > 0)
      m_iRepeat--;

    // Reset the pointer
    m_pRenderPtr = m_pRenderMin;
  }
  return true;
}

// Check if the sound has finished playing
bool CWinGlkWAVSound::IsSoundOver(DWORD Tick)
{
  if (m_Active == false)
    return true;

  // Check if sound is paused
  if ((GetStatus() & DSBSTATUS_PLAYING) == 0)
    return false;

  // Check if sound is playing forever
  if (m_Duration < 0)
    return false;

  return (Tick > m_StartTime + m_Duration);
}

// Get a type identifier for the sound
int CWinGlkWAVSound::GetType(void)
{
  return (int)'W';
}

// Get details of the sample
bool CWinGlkWAVSound::GetSampleData(SampleData& Data)
{
  if ((m_pData == NULL) || (m_iLength < 12))
    return false;

  // Check for WAV header
  if (strncmp((char*)m_pData,"RIFF",4) != 0)
    return false;
  if (strncmp((char*)(m_pData+8),"WAVE",4) != 0)
    return false;

  // Find the format chunk
  unsigned long length = 0;
  BYTE* chunk = FindChunk("fmt ",length);
  if ((chunk == NULL) || (length < 16))
    return false;

  // Only uncompressed PCM data is supported, either directly or
  // as the sub-format of WAVE_FORMAT_EXTENSIBLE
  unsigned short format = ReadShort(chunk);
  if ((format == WAVE_FORMAT_EXTENSIBLE) && (length >= 40))
    format = ReadShort(chunk+24);
  if (format != WAVE_FORMAT_PCM)
    return false;

  // Read in details of the sample
  Data.channels = ReadShort(chunk+2);
  Data.rate = ReadLong(chunk+4);
  unsigned short align = ReadShort(chunk+12);
  Data.bits = ReadShort(chunk+14);
  if ((Data.channels < 1) || (Data.channels > 2) || (Data.rate == 0))
    return false;
  if ((Data.bits != 8) && (Data.bits != 16))
    return false;
  if (align != Data.channels*(Data.bits>>3))
    return false;

  // Find the data chunk
  Data.data = FindChunk("data",length);
  if (Data.data == NULL)
    return false;
  Data.samples = length / align;
  return true;
}

// Find a WAV chunk, limiting its length to the data available
BYTE* CWinGlkWAVSound::FindChunk(LPCTSTR pszChunk, unsigned long& Length)
{
  BYTE* pData = m_pData+12;
  BYTE* pEnd = m_pData+m_iLength;
  while (pEnd - pData >= 8)
  {
    unsigned long size = ReadLong(pData+4);
    if (strncmp((char*)pData,pszChunk,4) == 0)
    {
      unsigned long avail = (unsigned long)(pEnd - (pData+8));
      Length = (size < avail) ? size : avail;
      return pData+8;
    }

    // Move to the next chunk
    if (size >= (unsigned long)(pEnd - pData))
      break;
    pData += size+8+(size % 2);
  }
  return NULL;
}

unsigned short CWinGlkWAVSound::ReadShort(const unsigned char *bytes)
{
  return (unsigned short)(
    ((unsigned short)(bytes[0] & 0xFF)) |
    ((unsigned short)(bytes[1] & 0xFF) << 8));
}

unsigned long CWinGlkWAVSound::ReadLong(const unsigned char *bytes)
{
  return (unsigned long)(
    ((unsigned long)(bytes[0] & 0xFF)) |
    ((unsigned long)(bytes[1] & 0xFF) << 8) |
    ((unsigned long)(bytes[2] & 0xFF) << 16) |
    ((unsigned long)(bytes[3] & 0xFF) << 24));
}
