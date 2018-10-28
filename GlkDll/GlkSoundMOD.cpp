/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkSoundMOD
// Glk MOD module sounds
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkSoundMOD.h"
#include "GlkTime.h"
#include "sndfile.h"

#ifndef I7GLK
#include "Resource.h"
#endif

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
// Class for MOD sound loader
/////////////////////////////////////////////////////////////////////////////

// Get file prefix for sounds supported by this loader
LPCTSTR CWinGlkMODSoundLoader::GetFilePrefix(void)
{
  return "mus";
}

// Get the file extensions for sounds supported for this loader
int CWinGlkMODSoundLoader::GetNumberFileExtensions(void)
{
  return 1;
}

LPCTSTR CWinGlkMODSoundLoader::GetFileExtension(int iExtIndex)
{
  return "mod";
}

// Get the identifier for sounds supported for this loader
glui32 CWinGlkMODSoundLoader::GetIdentifier(void)
{
  return giblorb_make_id('M','O','D',' ');
}

// Get a sound object
CWinGlkSound* CWinGlkMODSoundLoader::GetSound(LPCTSTR pszFileName)
{
  return new CWinGlkMODSound(pszFileName);
}

CWinGlkSound* CWinGlkMODSoundLoader::GetSound(BYTE* pData, int iLength)
{
  return new CWinGlkMODSound(pData,iLength);
}

/////////////////////////////////////////////////////////////////////////////
// Class for MOD sounds
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkMODSound,CWinGlkSound);

CWinGlkMODSound::CWinGlkMODSound(BYTE* pData, int iLength) : CWinGlkSound(pData,iLength)
{
  m_Player = new CSoundFile;
  m_Duration = 0;
}

CWinGlkMODSound::CWinGlkMODSound(LPCTSTR pszFileName) : CWinGlkSound(pszFileName)
{
  m_Player = new CSoundFile;
  m_Duration = 0;
}

CWinGlkMODSound::~CWinGlkMODSound()
{
  RemoveFromList();
  delete m_Player;
}

bool CWinGlkMODSound::Play(int iRepeat, int iVolume, bool PauseState)
{
  BYTE* pData = NULL;
  int iLength = 0;
  GetMODData(pData,iLength);

  if (pData == NULL)
    return false;

  // Only one MOD can be playing at any given time
  CDSoundEngine& Engine = CDSoundEngine::GetSoundEngine();
  if (Engine.CountSounds(GetType()) > 0)
    return false;

  WAVEFORMATEX& Format = Engine.GetPrimaryFormat();
  CSoundFile::SetWaveConfig(Format.nSamplesPerSec,Format.wBitsPerSample,Format.nChannels);
  CSoundFile::SetWaveConfigEx(TRUE,FALSE,FALSE,TRUE,TRUE,TRUE,FALSE);

  // Load the song into MODPlug
  if (!m_Player->Create(pData,iLength))
    return false;

  // Set the number of repeats and the duration
  if (iRepeat > 0)
  {
    m_Player->SetRepeatCount(iRepeat-1);
    m_Duration = m_Player->GetSongTime() * iRepeat;
  }
  else
  {
    m_Player->SetRepeatCount(-1);
    m_Duration = -1;
  }

  // Create a buffer
  if (CreateBuffer(Format.nChannels,Format.nSamplesPerSec,Format.wBitsPerSample) == false)
    return false;

  // Fill the buffer with sample data
  if (FillBuffer(GetBufferSize()) == false)
    return false;

  // Set the volume for the buffer
  SetVolume(iVolume);

  // Start the buffer playing
  return PlayBuffer(PauseState);
}

bool CWinGlkMODSound::IsPlaying(void)
{
  return m_Active;
}

void CWinGlkMODSound::Pause(bool PauseState)
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

void CWinGlkMODSound::SetVolume(int iVolume)
{
  // The SetVolume() call to DirectSound requires a volume
  // in 100ths of a decibel.
  SetBufferVolume((LONG)DecibelVolume(iVolume) * 100L);
}

void CWinGlkMODSound::WriteSampleData(unsigned char* pSample, int iSampleLen)
{
  WAVEFORMATEX& Format = CDSoundEngine::GetSoundEngine().GetPrimaryFormat();

  // Render audio
  int iBytes = m_Player->Read(pSample,iSampleLen) * Format.nBlockAlign;

  // Has the MOD been completely rendered?
  if (iBytes < iSampleLen)
  {
    for (int i = iBytes; i < iSampleLen; i++)
    {
      pSample[i] = (unsigned char)
        ((Format.wBitsPerSample == 8) ? 0x80 : 0x00);
    }
  }
}

// Check if the sound has finished playing
bool CWinGlkMODSound::IsSoundOver(DWORD Tick)
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
int CWinGlkMODSound::GetType(void)
{
  return (int)'M';
}

// Get a pointer to the MOD data and the size of the data
void CWinGlkMODSound::GetMODData(BYTE*& pData, int& iLength)
{
  pData = m_pData;
  iLength = m_iLength;
}
