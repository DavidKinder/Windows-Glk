/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkSoundCustom
// Glk custom sounds
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkSoundCustom.h"
#include "GlkTime.h"

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
// Class for custom sound loader
/////////////////////////////////////////////////////////////////////////////

CWinGlkCustomSoundLoader::CWinGlkCustomSoundLoader(winglk_sound_format* pSoundFormat)
{
  m_SoundFormat = *pSoundFormat;
}

// Get file prefix for sounds supported by this loader
LPCTSTR CWinGlkCustomSoundLoader::GetFilePrefix(void)
{
  return m_SoundFormat.prefix;
}

// Get the file extensions for sounds supported for this loader
int CWinGlkCustomSoundLoader::GetNumberFileExtensions(void)
{
  return 1;
}

LPCTSTR CWinGlkCustomSoundLoader::GetFileExtension(int iExtIndex)
{
  return m_SoundFormat.ext;
}

// Get the identifier for sounds supported for this loader
glui32 CWinGlkCustomSoundLoader::GetIdentifier(void)
{
  return m_SoundFormat.id;
}

// Get a sound object
CWinGlkSound* CWinGlkCustomSoundLoader::GetSound(LPCTSTR pszFileName)
{
  return new CWinGlkCustomSound(m_SoundFormat,pszFileName);
}

CWinGlkSound* CWinGlkCustomSoundLoader::GetSound(BYTE* pData, int iLength)
{
  return new CWinGlkCustomSound(m_SoundFormat,pData,iLength);
}

/////////////////////////////////////////////////////////////////////////////
// Class for custom sounds
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkCustomSound,CWinGlkSound);

CWinGlkCustomSound::CWinGlkCustomSound(winglk_sound_format& SoundFormat, BYTE* pData, int iLength)
  : CWinGlkSound(pData,iLength)
{
  m_Duration = 0;
  m_SoundFormat = SoundFormat;
  m_Cookie = 0;
}

CWinGlkCustomSound::CWinGlkCustomSound(winglk_sound_format& SoundFormat, LPCTSTR pszFileName)
  : CWinGlkSound(pszFileName)
{
  m_Duration = 0;
  m_SoundFormat = SoundFormat;
  m_Cookie = 0;
}

CWinGlkCustomSound::~CWinGlkCustomSound()
{
  RemoveFromList();
  if (m_Cookie)
    m_SoundFormat.stop(m_Cookie);
}

bool CWinGlkCustomSound::Play(int iRepeat, int iVolume, bool PauseState)
{
  if (m_Cookie)
  {
    m_SoundFormat.stop(m_Cookie);
    m_Cookie = 0;
  }

  // Get details of the sound
  struct winglk_sound Info {};
  m_Cookie = m_SoundFormat.start(m_pData,m_iLength,&Info);
  if (m_Cookie == 0)
    return false;

  // Create a buffer
  if (CreateBuffer(Info.channels,Info.freq,Info.bits) == false)
    return false;

  // Set the duration of the sound
  if (iRepeat > 0)
    m_Duration = (DWORD)(Info.duration * iRepeat);
  else
    m_Duration = -1;

  // Fill the buffer with sound data
  m_iRepeat = (iRepeat < 0) ? -1 : iRepeat - 1;
  if (FillBuffer(GetBufferSize()) == false)
    return false;

  // Set the volume for the buffer
  SetVolume(iVolume);

  // Start the buffer playing
  return PlayBuffer(PauseState);
}

bool CWinGlkCustomSound::IsPlaying(void)
{
  return m_Active;
}

void CWinGlkCustomSound::Pause(bool PauseState)
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

void CWinGlkCustomSound::SetVolume(int iVolume)
{
  // The SetVolume() call to DirectSound requires a volume
  // in 100ths of a decibel.
  SetBufferVolume((LONG)DecibelVolume(iVolume) * 100L);
}

// Write sound data into the supplied PCM sample buffers
void CWinGlkCustomSound::WriteSampleData(unsigned char* pSample, int iSampleLen)
{
  if (m_Cookie)
    m_SoundFormat.write(m_Cookie,pSample,iSampleLen,&m_iRepeat);
}

// Check if the sound has finished playing
bool CWinGlkCustomSound::IsSoundOver(DWORD Tick)
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
int CWinGlkCustomSound::GetType(void)
{
  return (int)m_SoundFormat.id;
}
