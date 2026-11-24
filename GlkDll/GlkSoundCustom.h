/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC GLK Libraries
//
// GlkSoundCustom
// GLK Custom sounds
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_SOUND_CUSTOM_H_
#define WINGLK_SOUND_CUSTOM_H_

#include "GlkSound.h"
#include "DSoundEngine.h"
#include "WinGlk.h"

/////////////////////////////////////////////////////////////////////////////
// Class for custom sound loader
/////////////////////////////////////////////////////////////////////////////

class CWinGlkCustomSoundLoader : public CWinGlkSoundLoader
{
public:
  CWinGlkCustomSoundLoader(winglk_sound_format* pSoundFormat);
  virtual ~CWinGlkCustomSoundLoader() {}

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

protected:
  // Details of the custom format
  winglk_sound_format m_SoundFormat;
};

/////////////////////////////////////////////////////////////////////////////
// Class for custom sounds
/////////////////////////////////////////////////////////////////////////////

class CWinGlkCustomSound : public CWinGlkSound, public CDSound
{
  DECLARE_DYNAMIC(CWinGlkCustomSound)

public:
  CWinGlkCustomSound(winglk_sound_format& SoundFormat, BYTE* pData, int iLength);
  CWinGlkCustomSound(winglk_sound_format& SoundFormat, LPCTSTR pszFileName);
  virtual ~CWinGlkCustomSound();

  virtual bool Play(int iRepeat, int iVolume, bool PauseState);
  virtual bool IsPlaying(void);
  virtual void Pause(bool PauseState);
  virtual void SetVolume(int iVolume);

  virtual void WriteSampleData(unsigned char* pSample, int iSampleLen);
  virtual bool IsSoundOver(DWORD Tick);
  virtual int GetType(void);

protected:
  // The duration of the sample
  int m_Duration;

  // Details of the custom format
  winglk_sound_format m_SoundFormat;
  // Identifier for the custom sound
  DWORD_PTR m_Id;
};

#endif // WINGLK_SOUND_CUSTOM_H_
