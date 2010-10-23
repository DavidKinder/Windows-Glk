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

  // Remove from the global map
  ChannelMap.RemoveKey(this);
}

glui32 CWinGlkSndChannel::GetRock(void)
{
  return m_Rock;
}

bool CWinGlkSndChannel::Play(CWinGlkSound* pSound, glui32 iSound, int iRepeat, int iNotify)
{
  Stop();

  // Store the new sound data
  m_pSound = pSound;
  m_iSound = iSound;
  m_iNotify = iNotify;

  if (m_pSound->Play(iRepeat,m_iVolume) == false)
  {
    Stop();
    return false;
  }
  return true;
}

void CWinGlkSndChannel::Stop(void)
{
  if (m_pSound != NULL)
    delete m_pSound;
  m_pSound = NULL;

  m_iSound = 0;
  m_iNotify = 0;
}

void CWinGlkSndChannel::SetVolume(int iVolume)
{
  m_iVolume = iVolume;
  if (m_pSound != NULL)
    m_pSound->SetVolume(iVolume);
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
