/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC GLK Libraries
//
// GlkSoundAIFF
// GLK AIFF samples
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_SOUND_AIFF_H_
#define WINGLK_SOUND_AIFF_H_

#include "GlkSound.h"
#include "DSoundEngine.h"

/////////////////////////////////////////////////////////////////////////////
// Class for AIFF sound loader
/////////////////////////////////////////////////////////////////////////////

class CWinGlkAIFFSoundLoader : public CWinGlkSoundLoader
{
public:
  CWinGlkAIFFSoundLoader() {}
  virtual ~CWinGlkAIFFSoundLoader() {}

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
// Class for AIFF sounds
/////////////////////////////////////////////////////////////////////////////

class CWinGlkAIFFSound : public CWinGlkSound, public CDSound
{
  DECLARE_DYNAMIC(CWinGlkAIFFSound)

public:
  CWinGlkAIFFSound(BYTE* pData, int iLength);
  CWinGlkAIFFSound(LPCTSTR pszFileName);
  virtual ~CWinGlkAIFFSound();

  virtual bool Play(int iRepeat, int iVolume);
  virtual bool IsPlaying(void);
  virtual void SetVolume(int iVolume);

  virtual void WriteSampleData(unsigned char* pSample, int iSampleLen);
  virtual bool IsSoundOver(DWORD Tick);
  virtual int GetType(void);

  // Details of the sample
  struct SampleData
  {
    unsigned short channels;
    unsigned long samples;
    unsigned short bits;
    double rate;
    BYTE *data;

    unsigned long repeat1;
    unsigned long repeat2;
  };
  bool GetSampleData(SampleData& data);

protected:
  bool CheckRenderPtr(void);

  // Helper routines for reading AIFF data
  BYTE* FindChunk(LPCTSTR pszChunk);
  static unsigned short ReadShort(const unsigned char *bytes);
  static unsigned long ReadLong(const unsigned char *bytes);
  static double ReadExtended(const unsigned char *bytes);

protected:
  BYTE* m_pRenderPtr;
  BYTE* m_pRenderMin;
  BYTE* m_pRenderMax;

  // The duration of the sample
  int m_Duration;
};

#endif // WINGLK_SOUND_AIFF_H_
