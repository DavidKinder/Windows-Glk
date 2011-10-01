/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkSoundAIFF
// Glk AIFF sounds
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkSoundAIFF.h"
#include "GlkTime.h"
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
// Class for AIFF sound loader
/////////////////////////////////////////////////////////////////////////////

// Get file prefix for sounds supported by this loader
LPCTSTR CWinGlkAIFFSoundLoader::GetFilePrefix(void)
{
  return "snd";
}

// Get the file extensions for sounds supported for this loader
int CWinGlkAIFFSoundLoader::GetNumberFileExtensions(void)
{
  return 2;
}

LPCTSTR CWinGlkAIFFSoundLoader::GetFileExtension(int iExtIndex)
{
  switch (iExtIndex)
  {
  case 0:
    return "aif";
  case 1:
    return "aiff";
  }
  return "";
}

// Get the identifier for sounds supported for this loader
glui32 CWinGlkAIFFSoundLoader::GetIdentifier(void)
{
  return giblorb_make_id('F','O','R','M');
}

// Get a sound object
CWinGlkSound* CWinGlkAIFFSoundLoader::GetSound(LPCTSTR pszFileName)
{
  return new CWinGlkAIFFSound(pszFileName);
}

CWinGlkSound* CWinGlkAIFFSoundLoader::GetSound(BYTE* pData, int iLength)
{
  return new CWinGlkAIFFSound(pData,iLength);
}

/////////////////////////////////////////////////////////////////////////////
// Class for AIFF sounds
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkAIFFSound,CWinGlkSound);

CWinGlkAIFFSound::CWinGlkAIFFSound(BYTE* pData, int iLength) : CWinGlkSound(pData,iLength)
{
  m_pRenderPtr = NULL;
  m_pRenderMin = NULL;
  m_pRenderMax = NULL;
  m_Duration = 0;
}

CWinGlkAIFFSound::CWinGlkAIFFSound(LPCTSTR pszFileName) : CWinGlkSound(pszFileName)
{
  m_pRenderPtr = NULL;
  m_pRenderMin = NULL;
  m_pRenderMax = NULL;
  m_Duration = 0;
}

CWinGlkAIFFSound::~CWinGlkAIFFSound()
{
  RemoveFromList();
}

bool CWinGlkAIFFSound::Play(int iRepeat, int iVolume, bool PauseState)
{
  SampleData data;
  if (GetSampleData(data) == false)
    return false;

  // Create a buffer
  if (CreateBuffer(data.channels,(int)data.rate,data.bits) == false)
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

bool CWinGlkAIFFSound::IsPlaying(void)
{
  return m_Active;
}

void CWinGlkAIFFSound::Pause(bool PauseState)
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

void CWinGlkAIFFSound::SetVolume(int iVolume)
{
  // The SetVolume() call to DirectSound requires a volume
  // in 100ths of a decibel.
  SetBufferVolume((LONG)DecibelVolume(iVolume) * 100L);
}

// Write sample data into the supplied PCM sample buffers
void CWinGlkAIFFSound::WriteSampleData(unsigned char* pSample, int iSampleLen)
{
  switch (m_Format.wBitsPerSample)
  {
  case 8:
    {
      for (int i = 0; i < iSampleLen; i++)
      {
        if (CheckRenderPtr())
          *(pSample++) = (unsigned char)((*(m_pRenderPtr++)) ^ 0x80);
        else
          *(pSample++) = 0x80;
      }
    }
    break;
  case 16:
    {
      unsigned short* pWritePtr = (unsigned short*)pSample;
      for (int i = 0; i < iSampleLen; i += 2)
      {
        if (CheckRenderPtr())
        {
          *(pWritePtr++) = (unsigned short)
            (((*m_pRenderPtr)<<8) | *(m_pRenderPtr+1));
          m_pRenderPtr += 2;
        }
        else
          *(pWritePtr++) = 0;
      }
    }
    break;
  }
}

// Test that the current point into the AIFF buffer is valid
bool CWinGlkAIFFSound::CheckRenderPtr(void)
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
bool CWinGlkAIFFSound::IsSoundOver(DWORD Tick)
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
int CWinGlkAIFFSound::GetType(void)
{
  return (int)'A';
}

// Get details of the sample
bool CWinGlkAIFFSound::GetSampleData(SampleData& Data)
{
  if (m_pData == NULL)
    return false;

  // Check for AIFF header
  if (strncmp((char*)m_pData,"FORM",4) != 0)
    return false;
  if (strncmp((char*)(m_pData+8),"AIFF",4) != 0)
    return false;

  // Find the COMM chunk
  BYTE* chunk = FindChunk("COMM");
  if (chunk == NULL)
    return false;

  // Read in details of the sample
  Data.channels = ReadShort(chunk);
  Data.samples = ReadLong(chunk+2);
  Data.bits = ReadShort(chunk+6);
  Data.rate = ReadExtended(chunk+8);

  // Find the SSND chunk
  Data.data = FindChunk("SSND");
  if (Data.data == NULL)
    return false;

  // Set up default repeat information
  Data.repeat1 = 0;
  Data.repeat2 = 0;

  // Find the INST chunk
  chunk = FindChunk("INST");
  if (chunk != NULL)
  {
    // Look for a sustain loop
    if (ReadShort(chunk+8) != 0)
    {
      unsigned short mark1 = ReadShort(chunk+10);
      unsigned short mark2 = ReadShort(chunk+12);

      // Find the MARK chunk
      chunk = FindChunk("MARK");
      if (chunk != NULL)
      {
        unsigned short markers = ReadShort(chunk);

        BYTE* mark = chunk+2;
        for (int i = 0; i < markers; i++)
        {
          unsigned short id = ReadShort(mark);
          unsigned long pos = ReadLong(mark+2);

          if (id == mark1)
            Data.repeat1 = pos;
          else if (id == mark2)
            Data.repeat2 = pos;

          unsigned char nameLen = mark[6]+1;
          if ((nameLen % 2) == 1)
            nameLen++;
          mark += nameLen+6;
        }
      }
    }
  }
  return true;
}

// Find an AIFF chunk
BYTE* CWinGlkAIFFSound::FindChunk(LPCTSTR pszChunk)
{
  BYTE* pData = m_pData+12;
  while (true)
  {
    if (strncmp((char*)pData,pszChunk,4) == 0)
      return pData+8;

    // Move to the next chunk
    unsigned long size = ReadLong(pData+4)+8;
    if ((size % 2) == 1)
      size++;
    pData += size;

    if ((pData - m_pData) >= m_iLength)
      break;
  }
  return NULL;
}

unsigned short CWinGlkAIFFSound::ReadShort(const unsigned char *bytes)
{
  return (unsigned short)(
    ((unsigned short)(bytes[0] & 0xFF) << 8) |
    ((unsigned short)(bytes[1] & 0xFF)));
}

unsigned long CWinGlkAIFFSound::ReadLong(const unsigned char *bytes)
{
  return (unsigned long)(
    ((unsigned long)(bytes[0] & 0xFF) << 24) |
    ((unsigned long)(bytes[1] & 0xFF) << 16) |
    ((unsigned long)(bytes[2] & 0xFF) << 8) |
    ((unsigned long)(bytes[3] & 0xFF)));
}

/* 
 * Copyright (C) 1988-1991 Apple Computer, Inc.
 * All rights reserved.
 *
 * Machine-independent I/O routines for IEEE floating-point numbers.
 *
 * NaN's and infinities are converted to HUGE_VAL or HUGE, which
 * happens to be infinity on IEEE machines.  Unfortunately, it is
 * impossible to preserve NaN's in a machine-independent way.
 * Infinities are, however, preserved on IEEE machines.
 *
 * These routines have been tested on the following machines:
 *    Apple Macintosh, MPW 3.1 C compiler
 *    Apple Macintosh, THINK C compiler
 *    Silicon Graphics IRIS, MIPS compiler
 *    Cray X/MP and Y/MP
 *    Digital Equipment VAX
 *
 * Implemented by Malcolm Slaney and Ken Turkowski.
 *
 * Malcolm Slaney contributions during 1988-1990 include big- and little-
 * endian file I/O, conversion to and from Motorola's extended 80-bit
 * floating-point format, and conversions to and from IEEE single-
 * precision floating-point format.
 *
 * In 1991, Ken Turkowski implemented the conversions to and from
 * IEEE double-precision format, added more precision to the extended
 * conversions, and accommodated conversions involving +/- infinity,
 * NaN's, and denormalized numbers.
 */
#define UnsignedToFloat(u) (((double)((long)(u - 2147483647L - 1))) + 2147483648.0)
double CWinGlkAIFFSound::ReadExtended(const unsigned char *bytes)
{
  double f;
  int expon;
  unsigned long hiMant, loMant;

  expon = ((bytes[0] & 0x7F) << 8) | (bytes[1] & 0xFF);
  hiMant = ReadLong(bytes+2);
  loMant = ReadLong(bytes+6);

  if (expon == 0 && hiMant == 0 && loMant == 0)
    f = 0;
  else
  {
    if (expon == 0x7FFF) /* Infinity or NaN */
      f = -1;
    else
    {
      expon -= 16383;
      f = ldexp(UnsignedToFloat(hiMant),expon -= 31);
      f += ldexp(UnsignedToFloat(loMant),expon -= 32);
    }
  }

  if (bytes[0] & 0x80)
    return -f;
  return f;
}
