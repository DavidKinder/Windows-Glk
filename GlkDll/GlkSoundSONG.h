/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC GLK Libraries
//
// GlkSoundSONG
// GLK SONG module sounds
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_SOUND_SONG_H_
#define WINGLK_SOUND_SONG_H_

#include "GlkSoundMOD.h"

/////////////////////////////////////////////////////////////////////////////
// Class for SONG sound loader
/////////////////////////////////////////////////////////////////////////////

class CWinGlkSONGSoundLoader : public CWinGlkSoundLoader
{
public:
  CWinGlkSONGSoundLoader() {}
  virtual ~CWinGlkSONGSoundLoader() {}

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
// Class for SONG sounds
/////////////////////////////////////////////////////////////////////////////

class CWinGlkSONGSound : public CWinGlkMODSound
{
  DECLARE_DYNAMIC(CWinGlkSONGSound)

public:
  CWinGlkSONGSound(BYTE* pData, int iLength);
  CWinGlkSONGSound(LPCTSTR pszFileName);
  virtual ~CWinGlkSONGSound();

protected:
  virtual void GetMODData(BYTE*& pData, int& iLength);

  void CreateMOD(void);
  void WriteHeaderValue(int sample, int offset, unsigned long value);

protected:
  BYTE* m_pMODData;
  int m_iMODLength;
};

#endif // WINGLK_SOUND_SONG_H_
