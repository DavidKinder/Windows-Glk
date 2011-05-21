/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC GLK Libraries
//
// GlkStream
// Multiple GLK streams
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_STREAM_H_
#define WINGLK_STREAM_H_

extern "C"
{
#include "glk.h"
#include "gi_dispa.h"
}

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_WINDOW_H_
class CWinGlkWnd;
#endif

#ifndef WINGLK_FILEREF_H_
class CWinGlkFileRef;
#endif

/////////////////////////////////////////////////////////////////////////////
// Base class for Glk streams
/////////////////////////////////////////////////////////////////////////////

class CWinGlkStream : public CObject
{
  DECLARE_DYNAMIC(CWinGlkStream)

public:
  CWinGlkStream(glui32 Rock);
  virtual ~CWinGlkStream();

  glui32 GetRock(void);

  void SetDispRock(const gidispatch_rock_t& Rock) { m_DispRock = Rock; }
  gidispatch_rock_t& GetDispRock(void) { return m_DispRock; }

  void SetArrayRock(const gidispatch_rock_t& Rock) { m_ArrayRock = Rock; }
  gidispatch_rock_t& GetArrayRock(void) { return m_ArrayRock; }

  virtual void PutCharacter(glui32 c);

  virtual glsi32 GetCharacter(void);
  glui32 GetLine(char *pBuffer, glui32 Length);
  glui32 GetLine(glui32 *pBuffer, glui32 Length);
  glui32 GetBuffer(char *pBuffer, glui32 Length);
  glui32 GetBuffer(glui32 *pBuffer, glui32 Length);

  virtual void SetPosition(glsi32 Pos, glui32 Mode) {}
  virtual glui32 GetPosition(void) { return 0; }
  virtual void SetStyle(int iStyle) {}
  virtual void SetHyperlink(int iLink) {}

  int GetReadCount(void) { return m_iReadCount; }
  int GetWriteCount(void) { return m_iWriteCount; }

protected:
  glui32 m_Rock;
  gidispatch_rock_t m_DispRock;
  gidispatch_rock_t m_ArrayRock;
  int m_iReadCount;
  int m_iWriteCount;

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

public:
  static CWinGlkStream* GetCurrentStream(void);
  static void SetCurrentStream(CWinGlkStream* pStream);
  static void CloseAllStreams(void);

  static bool IsValidStream(CWinGlkStream* pStream);
  static CWinGlkStream* IterateStreams(CWinGlkStream* pStream, glui32* pRockPtr);

protected:
  static CWinGlkStream* m_pCurrStream;
  static CMap<CWinGlkStream*,CWinGlkStream*,int,int> StreamMap;
};

/////////////////////////////////////////////////////////////////////////////
// Glk window streams
/////////////////////////////////////////////////////////////////////////////

class CWinGlkStreamWnd : public CWinGlkStream
{
  DECLARE_DYNAMIC(CWinGlkStreamWnd)

public:
  CWinGlkStreamWnd(CWinGlkWnd* pWnd, glui32 Rock);
  virtual ~CWinGlkStreamWnd() {}

  virtual void PutCharacter(glui32 c);

  virtual void SetStyle(int iStyle);
  virtual void SetHyperlink(int iLink);

  CWinGlkWnd* GetWindow(void);

protected:
  CWinGlkWnd* m_pWindow;

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

public:
  static CWinGlkStreamWnd* FindWindowStream(CWinGlkWnd* pWnd);
};

/////////////////////////////////////////////////////////////////////////////
// Glk file streams
/////////////////////////////////////////////////////////////////////////////

class CWinGlkStreamFile : public CWinGlkStream
{
  DECLARE_DYNAMIC(CWinGlkStreamFile)

public:
  CWinGlkStreamFile(glui32 Rock);
  virtual ~CWinGlkStreamFile();

  virtual void PutCharacter(glui32 c);
  virtual glsi32 GetCharacter(void);

  virtual void SetPosition(glsi32 Pos, glui32 Mode);
  virtual glui32 GetPosition(void);

  bool OpenFile(CWinGlkFileRef* pFileRef, glui32 Mode);

protected:
  void SetNextOperation(glui32 oper);

protected:
  FILE* m_pHandle;
  bool m_bText;
  glui32 m_LastOper;
};

class CWinGlkStreamFileUni : public CWinGlkStreamFile
{
  DECLARE_DYNAMIC(CWinGlkStreamFileUni)

public:
  CWinGlkStreamFileUni(glui32 Rock);

  virtual void PutCharacter(glui32 c);
  virtual glsi32 GetCharacter(void);

  virtual void SetPosition(glsi32 Pos, glui32 Mode);
  virtual glui32 GetPosition(void);
};

/////////////////////////////////////////////////////////////////////////////
// Glk memory streams
/////////////////////////////////////////////////////////////////////////////

class CWinGlkStreamMem : public CWinGlkStream
{
  DECLARE_DYNAMIC(CWinGlkStreamMem)

public:
  CWinGlkStreamMem(char* pBuffer, glui32 Length, glui32 Rock);
  virtual ~CWinGlkStreamMem();

  virtual void PutCharacter(glui32 c);
  virtual glsi32 GetCharacter(void);

  virtual void SetPosition(glsi32 Pos, glui32 Mode);
  virtual glui32 GetPosition(void);

protected:
  char* m_pBuffer;
  glui32 m_Length;
  glui32 m_Position;
};

class CWinGlkStreamMemUni : public CWinGlkStream
{
  DECLARE_DYNAMIC(CWinGlkStreamMemUni)

public:
  CWinGlkStreamMemUni(glui32* pBuffer, glui32 Length, glui32 Rock);
  virtual ~CWinGlkStreamMemUni();

  virtual void PutCharacter(glui32 c);
  virtual glsi32 GetCharacter(void);

  virtual void SetPosition(glsi32 Pos, glui32 Mode);
  virtual glui32 GetPosition(void);

protected:
  glui32* m_pBuffer;
  glui32 m_Length;
  glui32 m_Position;
};

/////////////////////////////////////////////////////////////////////////////
// WinGlk resource streams
/////////////////////////////////////////////////////////////////////////////

class CWinGlkStreamRes : public CWinGlkStreamMem
{
  DECLARE_DYNAMIC(CWinGlkStreamRes)

public:
  CWinGlkStreamRes(LPCTSTR pszName, LPCTSTR pszType, glui32 Rock);
  virtual ~CWinGlkStreamRes();

protected:
  HGLOBAL m_hStream;
};

#endif // WINGLK_STREAM_H_
