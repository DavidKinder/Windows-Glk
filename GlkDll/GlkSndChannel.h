/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC GLK Libraries
//
// GlkSound
// GLK sound channels
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_SNDCHANNEL_H_
#define WINGLK_SNDCHANNEL_H_

#include "glk.h"
#include "gi_dispa.h"

class CWinGlkSound;

/////////////////////////////////////////////////////////////////////////////
// Class for Glk sound channels
/////////////////////////////////////////////////////////////////////////////

class CWinGlkSndChannel : public CObject
{
  DECLARE_DYNAMIC(CWinGlkSndChannel)

public:
  CWinGlkSndChannel(glui32 Rock);
  virtual ~CWinGlkSndChannel();

  glui32 GetRock(void);

  void SetDispRock(const gidispatch_rock_t& Rock) { m_DispRock = Rock; }
  gidispatch_rock_t& GetDispRock(void) { return m_DispRock; }

  void SetArrayRock(const gidispatch_rock_t& Rock) { m_ArrayRock = Rock; }
  gidispatch_rock_t& GetArrayRock(void) { return m_ArrayRock; }

  void Prepare(CWinGlkSound* pSound, glui32 iSound, int iNotify);
  bool Play(int iRepeat);
  void Stop(void);
  void SetVolume(int iVolume);
  void TimerPulse(void);

protected:
  glui32 m_Rock;
  gidispatch_rock_t m_DispRock;
  gidispatch_rock_t m_ArrayRock;

  CWinGlkSound* m_pSound;
  glui32 m_iSound;
  int m_iVolume;
  int m_iNotify;

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

public:
  static void CloseAllChannels(void);

  static bool IsValidChannel(CWinGlkSndChannel* pChannel);
  static CWinGlkSndChannel* IterateChannels(CWinGlkSndChannel* pChannel, glui32* pRockPtr);

protected:
  static CMap<CWinGlkSndChannel*,CWinGlkSndChannel*,int,int> ChannelMap;
};

#endif // WINGLK_SNDCHANNEL_H_
