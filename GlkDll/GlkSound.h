/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC GLK Libraries
//
// GlkSound
// GLK sound objects
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_SOUND_H_
#define WINGLK_SOUND_H_

extern "C"
{
#include "glk.h"
}

/////////////////////////////////////////////////////////////////////////////
// Class for loaded sounds
/////////////////////////////////////////////////////////////////////////////

class CWinGlkSound : public CObject
{
  DECLARE_DYNAMIC(CWinGlkSound)

public:
  CWinGlkSound(BYTE* pData, int iLength);
  CWinGlkSound(LPCTSTR pszFileName);
  virtual ~CWinGlkSound();

  virtual bool Play(int iRepeat, int iVolume) = 0;
  virtual bool IsPlaying(void) = 0;
  virtual void SetVolume(int iVolume) = 0;

protected:
  double DecibelVolume(int iVolume);

protected:
  BYTE* m_pData;
  int m_iLength;
  bool m_bDelete;

  int m_iRepeat;
};

/////////////////////////////////////////////////////////////////////////////
// Base class for sound loaders
/////////////////////////////////////////////////////////////////////////////

class CWinGlkSoundLoader
{
public:
  CWinGlkSoundLoader() {}
  virtual ~CWinGlkSoundLoader() {}

public:
  // Get file prefix for sounds supported by this loader
  virtual LPCTSTR GetFilePrefix(void) = 0;

  // Get the file extensions for sounds supported for this loader
  virtual int GetNumberFileExtensions(void) = 0;
  virtual LPCTSTR GetFileExtension(int iExtIndex) = 0;

  // Get the identifier for sounds supported for this loader
  virtual glui32 GetIdentifier(void) = 0;
  
  // Get a sound object
  virtual CWinGlkSound* GetSound(LPCTSTR pszFileName) = 0;
  virtual CWinGlkSound* GetSound(BYTE* pData, int iLength) = 0;

  // Clean up after sound playback has been stopped
  virtual void SoundStopped(void) {}

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

public:
  // Set up the loaders
  static void InitLoaders(void);

  // Notify loaders to clean up after sound playback has been stopped
  static void AllSoundStopped(void);

  // Delete the loaders
  static void RemoveLoaders(void);

  // Get the number of loaders
  static int GetLoaderCount(void) { return (int)m_Loaders.GetSize(); }

  // Get a loader by index
  static CWinGlkSoundLoader* GetLoader(int iIndex) { return m_Loaders.GetAt(iIndex); }

  // Get a loader for a given sound identifier
  static CWinGlkSoundLoader* GetLoaderForID(glui32 id);

protected:
  static CArray<CWinGlkSoundLoader*,CWinGlkSoundLoader*> m_Loaders;
};

#endif // WINGLK_SOUND_H_
