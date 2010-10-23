/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC GLK Libraries
//
// GlkSoundOGG
// GLK OGG sounds
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_SOUND_OGG_H_
#define WINGLK_SOUND_OGG_H_

#include "GlkSound.h"
#include "DSoundEngine.h"

#pragma warning(push)
#pragma warning(disable : 4244)
#include "vorbis/vorbisfile.h"
#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////////
// Class for OGG sound loader
/////////////////////////////////////////////////////////////////////////////

class CWinGlkOGGSoundLoader : public CWinGlkSoundLoader
{
public:
  CWinGlkOGGSoundLoader() {}
  virtual ~CWinGlkOGGSoundLoader() {}

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
// Class for OGG sounds
/////////////////////////////////////////////////////////////////////////////

class CWinGlkOGGSound : public CWinGlkSound, public CDSound
{
  DECLARE_DYNAMIC(CWinGlkOGGSound)

public:
  CWinGlkOGGSound(BYTE* pData, int iLength);
  CWinGlkOGGSound(LPCTSTR pszFileName);
  virtual ~CWinGlkOGGSound();

  virtual bool Play(int iRepeat, int iVolume);
  virtual bool IsPlaying(void);
  virtual void SetVolume(int iVolume);

  virtual void WriteSampleData(unsigned char* pSample, int iSampleLen);
  virtual bool IsSoundOver(DWORD Tick);
  virtual int GetType(void);

protected:
  static size_t VorbisRead(void*, size_t, size_t, void*);
  static int VorbisSeek(void*, ogg_int64_t, int);
  static int VorbisClose(void*);
  static long VorbisTell(void*);

  // Descriptor for the audio stream
  OggVorbis_File m_Stream;
  // Whether the stream has been opened
  bool m_StreamOpen;

  // The current point in the data
  BYTE* m_pReadPtr;
  // The duration of the sample
  int m_Duration;
};

#endif // WINGLK_SOUND_OGG_H_
