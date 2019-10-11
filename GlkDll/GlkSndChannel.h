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

#include <map>

class CWinGlkSound;

#define WM_SOUND_NOTIFY WM_APP+1

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
  void Pause(bool PauseState);
  void SetVolume(int iVolume, int iMillis, int iNotify);
  void TimerPulse(void);

protected:
  glui32 m_Rock;
  gidispatch_rock_t m_DispRock;
  gidispatch_rock_t m_ArrayRock;

  CWinGlkSound* m_pSound;
  glui32 m_iSound;
  bool m_Paused;
  volatile int m_iVolume;
  int m_iNotify;

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

public:
  static void CloseAllChannels(void);

  static bool IsValidChannel(CWinGlkSndChannel* pChannel);
  static CWinGlkSndChannel* IterateChannels(CWinGlkSndChannel* pChannel, glui32* pRockPtr);

  // Called from the sound engine thread
  static void VolumeFader(void);

protected:
  static CMap<CWinGlkSndChannel*,CWinGlkSndChannel*,int,int> ChannelMap;

  struct VolumeFade
  {
    // The initial volume
    double start;
    // The final volume
    double target;
    // The change in the volume per millisecond
    double rate;
    // The start time of the volume change
    DWORD startTime;
    // If true, the volume fade has finished
    bool finished;
    // The notification value, or zero
    int notify;

    VolumeFade()
    {
      start = 0.0;
      target = 0.0;
      rate = 0.0;
      startTime = 0;
      finished = false;
      notify = 0;
    }
  };

  // Map of channels for which the volume is being faded, protected by a lock
  static CCriticalSection VolumeLock;
  static std::map<CWinGlkSndChannel*,VolumeFade> VolumeFadeMap;
};

#endif // WINGLK_SNDCHANNEL_H_
