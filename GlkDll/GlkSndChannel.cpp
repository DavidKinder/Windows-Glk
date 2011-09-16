/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkSound
// Glk sound channels
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkDll.h"
#include "GlkSndChannel.h"
#include "GlkTime.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Class for Glk sound channels
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkSndChannel,CObject);

CWinGlkSndChannel::CWinGlkSndChannel(glui32 Rock) : CObject()
{
  m_Rock = Rock;
  m_pSound = NULL;
  m_iSound = 0;
  m_iVolume = 0x10000;
  m_iNotify = 0;

  // Add to the global map of all sound channels
  ChannelMap[this] = 0;

  if (RegisterObjFn)
    SetDispRock((*RegisterObjFn)(this,gidisp_Class_Schannel));
  else
    m_DispRock.num = 0;

  // Make sure that the sound engine is set up
  ((CGlkApp*)AfxGetApp())->InitSoundEngine();
}

CWinGlkSndChannel::~CWinGlkSndChannel()
{
  Stop();

  if (UnregisterObjFn)
    (*UnregisterObjFn)(this,gidisp_Class_Schannel,GetDispRock());

  // Remove from the volume fade map
  {
    CSingleLock lock(&VolumeLock,TRUE);
    VolumeFadeMap.erase(this);
  }

  // Remove from the global map
  ChannelMap.RemoveKey(this);
}

glui32 CWinGlkSndChannel::GetRock(void)
{
  return m_Rock;
}

void CWinGlkSndChannel::Prepare(CWinGlkSound* pSound, glui32 iSound, int iNotify)
{
  Stop();

  // Store the new sound data
  m_pSound = pSound;
  m_iSound = iSound;
  m_iNotify = iNotify;
}

bool CWinGlkSndChannel::Play(int iRepeat)
{
  if (m_pSound == NULL)
    return false;
  if (m_pSound->IsPlaying())
    return false;

  if (m_pSound->Play(iRepeat,m_iVolume))
    return true;
  Stop();
  return false;
}

void CWinGlkSndChannel::Stop(void)
{
  if (m_pSound != NULL)
    delete m_pSound;
  m_pSound = NULL;

  m_iSound = 0;
  m_iNotify = 0;
}

void CWinGlkSndChannel::SetVolume(int iVolume, int iMillis, int iNotify)
{
  if (iMillis == 0)
  {
    // Remove any previous volume fade map entry
    {
      CSingleLock lock(&VolumeLock,TRUE);
      VolumeFadeMap.erase(this);
    }

    m_iVolume = iVolume;
    if (m_pSound != NULL)
      m_pSound->SetVolume(iVolume);

    if (iNotify != 0)
      ((CGlkApp*)AfxGetApp())->AddEvent(evtype_VolumeNotify,0,0,iNotify);
  }
  else if (iMillis > 0)
  {
    VolumeFade fade;
    fade.start = m_iVolume;
    fade.target = iVolume;
    fade.rate = (double)(iVolume - m_iVolume) / (double)iMillis;
    fade.startTime = ::GetTickCount();
    fade.notify = iNotify;

    // Add to the volume fade map
    {
      CSingleLock lock(&VolumeLock,TRUE);
      VolumeFadeMap[this] = fade;
    }
  }
}

void CWinGlkSndChannel::TimerPulse(void)
{
  if (m_pSound != NULL)
  {
    // Is the sound still playing?
    if (m_pSound->IsPlaying() == false)
    {
      // Post a sound notification event
      if (m_iNotify != 0)
        ((CGlkApp*)AfxGetApp())->AddEvent(evtype_SoundNotify,0,m_iSound,m_iNotify);
      Stop();
    }
  }

  VolumeFade fade;
  {
    CSingleLock lock(&VolumeLock,TRUE);
    std::map<CWinGlkSndChannel*,CWinGlkSndChannel::VolumeFade>::const_iterator it = VolumeFadeMap.find(this);
    if (it != VolumeFadeMap.end())
    {
      fade = it->second;
      if (fade.finished)
        VolumeFadeMap.erase(it);
    }
  }
  if (fade.finished && (fade.notify != 0))
  {
    // Post a volume notification event
    ((CGlkApp*)AfxGetApp())->AddEvent(evtype_VolumeNotify,0,0,fade.notify);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

CMap<CWinGlkSndChannel*,CWinGlkSndChannel*,int,int> CWinGlkSndChannel::ChannelMap;

void CWinGlkSndChannel::CloseAllChannels(void)
{
  CWinGlkSndChannel* pChannel;
  CArray<CWinGlkSndChannel*,CWinGlkSndChannel*> ChannelArray;
  int i;

  POSITION MapPos = ChannelMap.GetStartPosition();
  while (MapPos)
  {
    ChannelMap.GetNextAssoc(MapPos,pChannel,i);
    ChannelArray.Add(pChannel);
  }

  for (i = 0; i < ChannelArray.GetSize(); i++)
    delete ChannelArray[i];
}

bool CWinGlkSndChannel::IsValidChannel(CWinGlkSndChannel* pChannel)
{
  int iDummy;
  if (ChannelMap.Lookup(pChannel,iDummy))
    return true;
  return false;
}

CWinGlkSndChannel* CWinGlkSndChannel::IterateChannels(CWinGlkSndChannel* pChannel, glui32* pRockPtr)
{
  CWinGlkSndChannel* pMapChannel = NULL;
  CWinGlkSndChannel* pPrevChannel = NULL;
  int iDummy;

  POSITION MapPos = ChannelMap.GetStartPosition();
  while (MapPos)
  {
    ChannelMap.GetNextAssoc(MapPos,pMapChannel,iDummy);
    if (pChannel == pPrevChannel)
    {
      if (pRockPtr)
        *pRockPtr = pMapChannel->GetRock();
      return pMapChannel;
    }
    pPrevChannel = pMapChannel;
  }
  return NULL;
}

CCriticalSection CWinGlkSndChannel::VolumeLock;
std::map<CWinGlkSndChannel*,CWinGlkSndChannel::VolumeFade> CWinGlkSndChannel::VolumeFadeMap;

// Called from the sound engine thread
void CWinGlkSndChannel::VolumeFader(void)
{
  DWORD now = ::GetTickCount();
  CSingleLock lock(&VolumeLock,TRUE);

  std::map<CWinGlkSndChannel*,CWinGlkSndChannel::VolumeFade>::iterator it;
  for (it = VolumeFadeMap.begin(); it != VolumeFadeMap.end(); ++it)
  {
    VolumeFade& fade = it->second;
    if (!fade.finished)
    {
      // Work out the new volume
      double volume = fade.start+(TickCountDiff(now,fade.startTime)*fade.rate);

      // Don't let the new volume go beyond the target volume
      if (fade.rate >= 0.0)
      {
        if (volume >= fade.target)
        {
          volume = fade.target;
          fade.finished = true;
        }
      }
      else
      {
        if (volume <= fade.target)
        {
          volume = fade.target;
          fade.finished = true;
        }
      }

      // Use the new volume
      it->first->m_iVolume = (int)volume;
      if (it->first->m_pSound != NULL)
        it->first->m_pSound->SetVolume((int)volume);
    }
  }
}
