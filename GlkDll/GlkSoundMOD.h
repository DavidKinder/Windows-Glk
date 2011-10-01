/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC GLK Libraries
//
// GlkSoundMOD
// GLK MOD module sounds
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_SOUND_MOD_H_
#define WINGLK_SOUND_MOD_H_

#include "GlkSound.h"
#include "DSoundEngine.h"

/////////////////////////////////////////////////////////////////////////////
// Class for MOD sound loader
/////////////////////////////////////////////////////////////////////////////

class CWinGlkMODSoundLoader : public CWinGlkSoundLoader
{
public:
  CWinGlkMODSoundLoader() {}
  virtual ~CWinGlkMODSoundLoader() {}

public:
  // Get file prefix for sounds supported by this loader
  virtual LPCTSTR GetFilePrefix(void);

  // Get the file extensions for sounds supported for this loader
  virtual int GetNumberFileExtensions(void);
  virtual LPCTSTR GetFileExtension(int iExtIndex);

  // Get the identifier for sounds supported for this loader
  virtual glui32 GetIdentifier(void);
  
  // Get a sound object
  virtual CWinGlkSound* GetSound(LPCTSTR pszFileName);
  virtual CWinGlkSound* GetSound(BYTE* pData, int iLength);
};

/////////////////////////////////////////////////////////////////////////////
// Class for MOD sounds
/////////////////////////////////////////////////////////////////////////////

class CSoundFile;

class CWinGlkMODSound : public CWinGlkSound, public CDSound
{
  DECLARE_DYNAMIC(CWinGlkMODSound)

public:
  CWinGlkMODSound(BYTE* pData, int iLength);
  CWinGlkMODSound(LPCTSTR pszFileName);
  virtual ~CWinGlkMODSound();

  virtual bool Play(int iRepeat, int iVolume, bool PauseState);
  virtual bool IsPlaying(void);
  virtual void Pause(bool PauseState);
  virtual void SetVolume(int iVolume);

  virtual void WriteSampleData(unsigned char* pSample, int iSampleLen);
  virtual bool IsSoundOver(DWORD Tick);
  virtual int GetType(void);

protected:
  virtual void GetMODData(BYTE*& pData, int& iLength);

  // The actual MODPlug player object
  CSoundFile* m_Player;

  // The duration of the sample
  int m_Duration;
};

#endif // WINGLK_SOUND_MOD_H_
