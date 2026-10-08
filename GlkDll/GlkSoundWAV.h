/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkSoundWAV
// Glk WAV sounds
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_SOUND_WAV_H_
#define WINGLK_SOUND_WAV_H_

#include "GlkSound.h"
#include "DSoundEngine.h"

/////////////////////////////////////////////////////////////////////////////
// Class for WAV sound loader
/////////////////////////////////////////////////////////////////////////////

class CWinGlkWAVSoundLoader : public CWinGlkSoundLoader
{
public:
  CWinGlkWAVSoundLoader() {}
  virtual ~CWinGlkWAVSoundLoader() {}

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
// Class for WAV sounds
/////////////////////////////////////////////////////////////////////////////

class CWinGlkWAVSound : public CWinGlkSound, public CDSound
{
  DECLARE_DYNAMIC(CWinGlkWAVSound)

public:
  CWinGlkWAVSound(BYTE* pData, int iLength);
  CWinGlkWAVSound(LPCTSTR pszFileName);
  virtual ~CWinGlkWAVSound();

  virtual bool Play(int iRepeat, int iVolume, bool PauseState);
  virtual bool IsPlaying(void);
  virtual void Pause(bool PauseState);
  virtual void SetVolume(int iVolume);

  virtual void WriteSampleData(unsigned char* pSample, int iSampleLen);
  virtual bool IsSoundOver(DWORD Tick);
  virtual int GetType(void);

protected:
  // Details of the sample
  struct SampleData
  {
    unsigned short channels;
    unsigned long rate;
    unsigned short bits;
    unsigned long samples;
    BYTE *data;
  };
  bool GetSampleData(SampleData& data);

  bool CheckRenderPtr(void);

  // Helper routines for reading WAV data
  BYTE* FindChunk(LPCTSTR pszChunk, unsigned long& Length);
  static unsigned short ReadShort(const unsigned char *bytes);
  static unsigned long ReadLong(const unsigned char *bytes);

protected:
  BYTE* m_pRenderPtr;
  BYTE* m_pRenderMin;
  BYTE* m_pRenderMax;

  // The duration of the sample
  int m_Duration;
};

#endif // WINGLK_SOUND_WAV_H_
