/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkStream
// Multiple Glk streams
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkDll.h"
#include "GlkFileRef.h"
#include "GlkStream.h"
#include "GlkWindow.h"

#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Base class for Glk streams
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkStream,CObject);

CWinGlkStream::CWinGlkStream(glui32 Rock) : CObject()
{
  m_Rock = Rock;
  m_iReadCount = 0;
  m_iWriteCount = 0;

  // Add to the global map of all streams
  StreamMap[this] = 0;

  if (RegisterObjFn)
    SetDispRock((*RegisterObjFn)(this,gidisp_Class_Stream));
  else
    m_DispRock.num = 0;
}

CWinGlkStream::~CWinGlkStream()
{
  // If this is an echo stream, clear the echo
  CMap<CWinGlkWnd*,CWinGlkWnd*,int,int>& WindowMap = CWinGlkWnd::GetWindowMap();
  CWinGlkWnd* pWnd;
  int i;

  POSITION MapPos = WindowMap.GetStartPosition();
  while (MapPos)
  {
    WindowMap.GetNextAssoc(MapPos,pWnd,i);
    if (pWnd->GetEchoStream() == this)
      pWnd->SetEchoStream(NULL);
  }

  if (UnregisterObjFn)
    (*UnregisterObjFn)(this,gidisp_Class_Stream,GetDispRock());

  // Remove from the global map
  StreamMap.RemoveKey(this);

  // If this is the current stream, set the current stream to NULL
  if (m_pCurrStream == this)
    m_pCurrStream = NULL;
}

glui32 CWinGlkStream::GetRock(void)
{
  return m_Rock;
}

void CWinGlkStream::PutCharacter(glui32 c)
{
  m_iWriteCount++;
}

glsi32 CWinGlkStream::GetCharacter(void)
{
  return -1;
}

glui32 CWinGlkStream::GetLine(char *pBuffer, glui32 Length)
{
  bool bGetLine = true;
  glsi32 Character = -1;
  int i = 0;

  if (Length == 0)
    return 0;

  while ((i < (int)Length-1) && bGetLine)
  {
    Character = GetCharacter();
    if (Character == -1)
      bGetLine = false;
    else
    {
      if (Character >= 0x00 && Character <= 0xFF)
        pBuffer[i++] = (char)Character;
      else
        pBuffer[i++] = '?';

      if (Character == '\n')
        bGetLine = false;
    }
  }

  pBuffer[i++] = '\0';
  return i-1;
}

glui32 CWinGlkStream::GetLine(glui32 *pBuffer, glui32 Length)
{
  bool bGetLine = true;
  glsi32 Character = -1;
  int i = 0;

  if (Length == 0)
    return 0;

  while ((i < (int)Length-1) && bGetLine)
  {
    Character = GetCharacter();
    if (Character == -1)
      bGetLine = false;
    else
    {
      pBuffer[i++] = (glui32)Character;
      if (Character == '\n')
        bGetLine = false;
    }
  }

  pBuffer[i++] = 0;
  return i-1;
}

glui32 CWinGlkStream::GetBuffer(char *pBuffer, glui32 Length)
{
  bool bGetBuffer = true;
  glsi32 Character = -1;
  int i = 0;

  if (Length == 0)
    return 0;

  while ((i < (int)Length) && bGetBuffer)
  {
    Character = GetCharacter();
    if (Character == -1)
      bGetBuffer = false;
    else
    {
      if (Character >= 0x00 && Character <= 0xFF)
        pBuffer[i++] = (char)Character;
      else
        pBuffer[i++] = '?';
    }
  }

  return i;
}

glui32 CWinGlkStream::GetBuffer(glui32 *pBuffer, glui32 Length)
{
  bool bGetBuffer = true;
  glsi32 Character = -1;
  int i = 0;

  if (Length == 0)
    return 0;

  while ((i < (int)Length) && bGetBuffer)
  {
    Character = GetCharacter();
    if (Character == -1)
      bGetBuffer = false;
    else
      pBuffer[i++] = (glui32)Character;
  }

  return i;
}

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

CWinGlkStream* CWinGlkStream::m_pCurrStream = NULL;
CMap<CWinGlkStream*,CWinGlkStream*,int,int> CWinGlkStream::StreamMap;

CWinGlkStream* CWinGlkStream::GetCurrentStream(void)
{
  return m_pCurrStream;
}

void CWinGlkStream::SetCurrentStream(CWinGlkStream* pStream)
{
  m_pCurrStream = pStream;
}

void CWinGlkStream::CloseAllStreams(void)
{
  CWinGlkStream* pStream;
  CArray<CWinGlkStream*,CWinGlkStream*> StreamArray;
  int i;

  POSITION MapPos = StreamMap.GetStartPosition();
  while (MapPos)
  {
    StreamMap.GetNextAssoc(MapPos,pStream,i);
    StreamArray.Add(pStream);
  }

  for (i = 0; i < StreamArray.GetSize(); i++)
    delete StreamArray[i];
}

bool CWinGlkStream::IsValidStream(CWinGlkStream* pStream)
{
  int iDummy;
  if (StreamMap.Lookup(pStream,iDummy))
    return true;
  return false;
}

CWinGlkStream* CWinGlkStream::IterateStreams(CWinGlkStream* pStream, glui32* pRockPtr)
{
  CWinGlkStream* pMapStream = NULL;
  CWinGlkStream* pPrevStream = NULL;
  int iDummy;

  POSITION MapPos = StreamMap.GetStartPosition();
  while (MapPos)
  {
    StreamMap.GetNextAssoc(MapPos,pMapStream,iDummy);
    if (pStream == pPrevStream)
    {
      if (pRockPtr)
        *pRockPtr = pMapStream->GetRock();
      return pMapStream;
    }
    pPrevStream = pMapStream;
  }
  return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Glk window streams
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkStreamWnd,CWinGlkStream);

CWinGlkStreamWnd::CWinGlkStreamWnd(CWinGlkWnd* pWnd, glui32 Rock) : CWinGlkStream(Rock)
{
  m_pWindow = pWnd;
}

void CWinGlkStreamWnd::PutCharacter(glui32 c)
{
  CWinGlkStream::PutCharacter(c);
  if (m_pWindow)
    m_pWindow->PutCharacter(c);
}

void CWinGlkStreamWnd::SetStyle(int iStyle)
{
  if (m_pWindow)
    m_pWindow->SetStyle(iStyle);
}

void CWinGlkStreamWnd::SetHyperlink(int iLink)
{
  if (m_pWindow)
    m_pWindow->SetHyperlink(iLink);
}

CWinGlkWnd* CWinGlkStreamWnd::GetWindow(void)
{
  return m_pWindow;
}

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

CWinGlkStreamWnd* CWinGlkStreamWnd::FindWindowStream(CWinGlkWnd* pWnd)
{
  CWinGlkStream* pStream;
  int iDummy;

  POSITION MapPos = StreamMap.GetStartPosition();
  while (MapPos)
  {
    StreamMap.GetNextAssoc(MapPos,pStream,iDummy);
    if (pStream->IsKindOf(RUNTIME_CLASS(CWinGlkStreamWnd)))
    {
      if (((CWinGlkStreamWnd*)pStream)->GetWindow() == pWnd)
        return (CWinGlkStreamWnd*)pStream;
    }
  }
  return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Glk file streams
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkStreamFile,CWinGlkStream);

CWinGlkStreamFile::CWinGlkStreamFile(glui32 Rock) : CWinGlkStream(Rock)
{
  m_pHandle = NULL;
  m_bText = false;
  m_LastOper = 0;
}

CWinGlkStreamFile::~CWinGlkStreamFile()
{
  if (m_pHandle)
    fclose(m_pHandle);
}

void CWinGlkStreamFile::PutCharacter(glui32 c)
{
  CWinGlkStream::PutCharacter(c);
  if (m_pHandle)
  {
    SetNextOperation(filemode_Write);
    if (c <= 0xFF)
      fputc(c,m_pHandle);
    else
      fputc('?',m_pHandle);
  }
}

glsi32 CWinGlkStreamFile::GetCharacter(void)
{
  glsi32 Character = -1;

  if (m_pHandle)
  {
    SetNextOperation(filemode_Read);
    Character = fgetc(m_pHandle);
  }

  if (Character != -1)
    m_iReadCount++;
  return Character;
}

void CWinGlkStreamFile::SetPosition(glsi32 Pos, glui32 Mode)
{
  if (m_pHandle)
  {
    int iOrigin = SEEK_SET;
    if (Mode == seekmode_Current)
      iOrigin = SEEK_CUR;
    else if (Mode == seekmode_End)
      iOrigin = SEEK_END;

    fseek(m_pHandle,Pos,iOrigin);
    m_LastOper = 0;
  }
}

glui32 CWinGlkStreamFile::GetPosition(void)
{
  glui32 Position = 0;

  if (m_pHandle)
    Position = (glui32)ftell(m_pHandle);

  return Position;
}

bool CWinGlkStreamFile::OpenFile(CWinGlkFileRef* pFileRef, glui32 Mode)
{
  CString strMode;
  bool create = false;

  switch (Mode)
  {
  case filemode_Write:
    strMode = "w";
    break;
  case filemode_Read:
    strMode = "r";
    break;
  case filemode_ReadWrite:
    strMode = "r+";
    create = true;
    break;
  case filemode_WriteAppend:
    strMode = "r+";
    create = true;
    break;
  }

  if (pFileRef->GetIsText())
  {
    m_bText = true;
    strMode += "t";
  }
  else
  {
    m_bText = false;
    strMode += "b";
  }

  if (pFileRef->GetIsTemporary())
  {
    char pszTempDir[MAX_PATH];
    char pszTempFile[MAX_PATH];

    // Create the temporary file name here
    ::GetTempPath(MAX_PATH,pszTempDir);
    ::GetTempFileName(pszTempDir,"glk",0,pszTempFile);
    pFileRef->SetFileName(pszTempFile,fileusage_Data,true,false);
  }

  if (create)
  {
    // Create the file if it doesn't already exist
    if (::GetFileAttributes(pFileRef->GetFileName()) == 0xFFFFFFFF)
    {
      HANDLE hFile = ::CreateFile(pFileRef->GetFileName(),0,0,NULL,
        CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
      if (hFile != INVALID_HANDLE_VALUE)
        ::CloseHandle(hFile);
    }
  }

  // Open the file
  m_pHandle = fopen(pFileRef->GetFileName(),strMode);

  // If the file is opened in append mode, go to the end of it
  if (m_pHandle && (Mode == filemode_WriteAppend))
    fseek(m_pHandle,0,SEEK_END);

  return (m_pHandle != NULL);
}

void CWinGlkStreamFile::SetNextOperation(glui32 oper)
{
  /* If switching between reading and writing, force an fseek() */
  if ((m_LastOper != 0) && (m_LastOper != oper))
  {
    long pos = ftell(m_pHandle);
    fseek(m_pHandle,pos,SEEK_SET);
  }
  m_LastOper = oper;
}

IMPLEMENT_DYNAMIC(CWinGlkStreamFileUni,CWinGlkStreamFile);

CWinGlkStreamFileUni::CWinGlkStreamFileUni(glui32 Rock) : CWinGlkStreamFile(Rock)
{
}

void CWinGlkStreamFileUni::PutCharacter(glui32 c)
{
  CWinGlkStream::PutCharacter(c);
  if (m_pHandle)
  {
    SetNextOperation(filemode_Write);
    if (m_bText)
    {
      if (c > 0xFFFF)
        c = L'?';
      glui32 uc = (glui32)c;
      fputc(uc & 0xFF,m_pHandle);
      fputc((uc>>8) & 0xFF,m_pHandle);
    }
    else
    {
      glui32 uc = (glui32)c;
      fputc((uc>>24) & 0xFF,m_pHandle);
      fputc((uc>>16) & 0xFF,m_pHandle);
      fputc((uc>>8) & 0xFF,m_pHandle);
      fputc(uc & 0xFF,m_pHandle);
    }
  }
}

glsi32 CWinGlkStreamFileUni::GetCharacter(void)
{
  glsi32 Character = -1;

  if (m_pHandle)
  {
    SetNextOperation(filemode_Read);
    if (m_bText)
    {
      glui32 uc = 0;
      int c = fgetc(m_pHandle);
      if (c != -1)
      {
        uc |= (c & 0xFF);
        c = fgetc(m_pHandle);
        if (c != -1)
        {
          uc |= ((c & 0xFF) << 8);
          Character = uc;
        }
      }
    }
    else
    {
      glui32 uc = 0;
      for (int i = 0; i < 4; i++)
      {
        uc <<= 8;

        int c = fgetc(m_pHandle);
        if (c == -1)
          break;
        uc |= (c & 0xFF);

        if (i == 3)
          Character = uc;
      }
    }
  }

  if (Character != -1)
    m_iReadCount++;
  return Character;
}

void CWinGlkStreamFileUni::SetPosition(glsi32 Pos, glui32 Mode)
{
  if (m_pHandle)
  {
    int iOrigin = SEEK_SET;
    if (Mode == seekmode_Current)
      iOrigin = SEEK_CUR;
    else if (Mode == seekmode_End)
      iOrigin = SEEK_END;

    if (m_bText)
      fseek(m_pHandle,Pos*2,iOrigin);
    else
      fseek(m_pHandle,Pos*4,iOrigin);
    m_LastOper = 0;
  }
}

glui32 CWinGlkStreamFileUni::GetPosition(void)
{
  glui32 Position = 0;

  if (m_pHandle)
  {
    if (m_bText)
      Position = (glui32)ftell(m_pHandle)/2;
    else
      Position = (glui32)ftell(m_pHandle)/4;
  }

  return Position;
}

/////////////////////////////////////////////////////////////////////////////
// Glk memory streams
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkStreamMem,CWinGlkStream);

CWinGlkStreamMem::CWinGlkStreamMem(char* pBuffer, glui32 Length, glui32 Rock) : CWinGlkStream(Rock)
{
  m_pBuffer = pBuffer;
  m_Length = Length;
  m_Position = 0;

  if (RegisterArrFn && pBuffer && (Length > 0))
    SetArrayRock((*RegisterArrFn)(pBuffer,Length,"&+#!Cn"));
  else
    m_ArrayRock.num = 0;
}

CWinGlkStreamMem::~CWinGlkStreamMem()
{
  if (UnregisterArrFn && m_pBuffer && (m_Length > 0))
    (*UnregisterArrFn)(m_pBuffer,m_Length,"&+#!Cn",GetArrayRock());
}

void CWinGlkStreamMem::PutCharacter(glui32 c)
{
  CWinGlkStream::PutCharacter(c);
  if (m_pBuffer)
  {
    if (m_Position < m_Length)
    {
      if (c <= 0xFF)
        m_pBuffer[m_Position++] = (unsigned char)c;
      else
        m_pBuffer[m_Position++] = '?';
    }
  }
}

glsi32 CWinGlkStreamMem::GetCharacter(void)
{
  glsi32 Character = -1;

  if (m_pBuffer)
  {
    if (m_Position < m_Length)
      Character = (unsigned char)m_pBuffer[m_Position++];
  }

  if (Character != -1)
    m_iReadCount++;
  return Character;
}

void CWinGlkStreamMem::SetPosition(glsi32 Pos, glui32 Mode)
{
  if (Mode == seekmode_Current)
    m_Position += Pos;
  else if (Mode == seekmode_End)
    m_Position = m_Length + Pos;
  else
    m_Position = Pos;

  if (m_Position < 0)
    m_Position = 0;
  if (m_Position > m_Length)
    m_Position = m_Length;
}

glui32 CWinGlkStreamMem::GetPosition(void)
{
  return m_Position;
}

IMPLEMENT_DYNAMIC(CWinGlkStreamMemUni,CWinGlkStream);

CWinGlkStreamMemUni::CWinGlkStreamMemUni(glui32* pBuffer, glui32 Length, glui32 Rock) : CWinGlkStream(Rock)
{
  m_pBuffer = pBuffer;
  m_Length = Length;
  m_Position = 0;

  if (RegisterArrFn && pBuffer && (Length > 0))
    SetArrayRock((*RegisterArrFn)(pBuffer,Length,"&+#!Iu"));
  else
    m_ArrayRock.num = 0;
}

CWinGlkStreamMemUni::~CWinGlkStreamMemUni()
{
  if (UnregisterArrFn && m_pBuffer && (m_Length > 0))
    (*UnregisterArrFn)(m_pBuffer,m_Length,"&+#!Iu",GetArrayRock());
}

void CWinGlkStreamMemUni::PutCharacter(glui32 c)
{
  CWinGlkStream::PutCharacter(c);
  if (m_pBuffer)
  {
    if (m_Position < m_Length)
      m_pBuffer[m_Position++] = c;
  }
}

glsi32 CWinGlkStreamMemUni::GetCharacter(void)
{
  glsi32 Character = -1;

  if (m_pBuffer)
  {
    if (m_Position < m_Length)
      Character = m_pBuffer[m_Position++];
  }

  if (Character != -1)
    m_iReadCount++;
  return Character;
}

void CWinGlkStreamMemUni::SetPosition(glsi32 Pos, glui32 Mode)
{
  if (Mode == seekmode_Current)
    m_Position += Pos;
  else if (Mode == seekmode_End)
    m_Position = m_Length + Pos;
  else
    m_Position = Pos;

  if (m_Position < 0)
    m_Position = 0;
  if (m_Position > m_Length)
    m_Position = m_Length;
}

glui32 CWinGlkStreamMemUni::GetPosition(void)
{
  return m_Position;
}

/////////////////////////////////////////////////////////////////////////////
// Glk resource streams
/////////////////////////////////////////////////////////////////////////////

CWinGlkResource::CWinGlkResource(char* pData, glui32 Length, bool bText, bool bFreeData)
{
  m_pData = pData;
  m_Length = Length;
  m_bText = bText;
  m_bFreeData = bFreeData;
}

CWinGlkResource::~CWinGlkResource()
{
  if (m_bFreeData)
    delete[] m_pData;
}

IMPLEMENT_DYNAMIC(CWinGlkStreamResource,CWinGlkStream);

CWinGlkStreamResource::CWinGlkStreamResource(CWinGlkResource* pRes, glui32 Rock) : CWinGlkStream(Rock)
{
  m_pResource = pRes;
  m_Position = 0;
}

CWinGlkStreamResource::~CWinGlkStreamResource()
{
  delete m_pResource;
}

void CWinGlkStreamResource::PutCharacter(glui32 c)
{
}

glsi32 CWinGlkStreamResource::GetCharacter(void)
{
  glsi32 Character = -1;

  if (m_Position < m_pResource->m_Length)
    Character = (unsigned char)m_pResource->m_pData[m_Position++];

  if (Character != -1)
    m_iReadCount++;
  return Character;
}

void CWinGlkStreamResource::SetPosition(glsi32 Pos, glui32 Mode)
{
  if (Mode == seekmode_Current)
    m_Position += Pos;
  else if (Mode == seekmode_End)
    m_Position = m_pResource->m_Length + Pos;
  else
    m_Position = Pos;

  if (m_Position < 0)
    m_Position = 0;
  if (m_Position > m_pResource->m_Length)
    m_Position = m_pResource->m_Length;
}

glui32 CWinGlkStreamResource::GetPosition(void)
{
  return m_Position;
}

IMPLEMENT_DYNAMIC(CWinGlkStreamResourceUni,CWinGlkStream);

CWinGlkStreamResourceUni::CWinGlkStreamResourceUni(CWinGlkResource* pRes, glui32 Rock) : CWinGlkStream(Rock)
{
  m_pResource = pRes;
  m_Position = 0;
}

CWinGlkStreamResourceUni::~CWinGlkStreamResourceUni()
{
  delete m_pResource;
}

void CWinGlkStreamResourceUni::PutCharacter(glui32 c)
{
}

glsi32 CWinGlkStreamResourceUni::GetCharacter(void)
{
  glsi32 Character = -1;

  if (m_pResource->m_bText)
  {
    // Read a UTF-8 encoded character
    glui32 uc = 0;
    int c0, c1, c2, c3;

    c0 = GetNextChar();
    if (c0 == -1)
      return -1;
    if (c0 < 0x80)
      uc = c0;
    else
    {
      c1 = GetNextChar();
      if (c1 == -1)
        return -1;
      if ((c1 & 0xC0) != 0x80)
        return -1;
      if ((c0 & 0xE0) == 0xC0)
      {
        uc = (c0 & 0x1F) << 6;
        uc |= (c1 & 0x3F);
      }
      else
      {
        c2 = GetNextChar();
        if (c2 == -1)
          return -1;
        if ((c2 & 0xC0) != 0x80)
          return -1;
        if ((c0 & 0xF0) == 0xE0)
        {
          uc = (((c0 & 0xF)<<12)  & 0x0000F000);
          uc |= (((c1 & 0x3F)<<6) & 0x00000FC0);
          uc |= (((c2 & 0x3F))    & 0x0000003F);
        }
        else if ((c0 & 0xF0) == 0xF0)
        {
          c3 = GetNextChar();
          if (c3 == -1)
            return -1;
          if ((c3 & 0xC0) != 0x80)
            return -1;
          uc = (((c0 & 0x7)<<18)   & 0x1C0000);
          uc |= (((c1 & 0x3F)<<12) & 0x03F000);
          uc |= (((c2 & 0x3F)<<6)  & 0x000FC0);
          uc |= (((c3 & 0x3F))     & 0x00003F);
        }
        else
          return -1;
      }
    }
    Character = uc;
  }
  else
  {
    // Read a 32-bit binary value
    glui32 uc = 0;
    for (int i = 0; i < 4; i++)
    {
      uc <<= 8;

      int c = GetNextChar();
      if (c == -1)
        return -1;
      uc |= (c & 0xFF);
    }
    Character = uc;
  }

  if (Character != -1)
    m_iReadCount++;
  return Character;
}

void CWinGlkStreamResourceUni::SetPosition(glsi32 Pos, glui32 Mode)
{
  if (Mode == seekmode_Current)
    m_Position += Pos;
  else if (Mode == seekmode_End)
    m_Position = m_pResource->m_Length + Pos;
  else
    m_Position = Pos;

  if (m_Position < 0)
    m_Position = 0;
  if (m_Position > m_pResource->m_Length)
    m_Position = m_pResource->m_Length;
}

glui32 CWinGlkStreamResourceUni::GetPosition(void)
{
  return m_Position;
}

int CWinGlkStreamResourceUni::GetNextChar(void)
{
  int c = -1;
  if (m_Position < m_pResource->m_Length)
    c = (unsigned char)m_pResource->m_pData[m_Position++];
  return c;
}

/////////////////////////////////////////////////////////////////////////////
// WinGlk Windows resource streams (that is, resources embedded in
// Windows executables)
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkStreamWindowsResource,CWinGlkStreamMem);

CWinGlkStreamWindowsResource::CWinGlkStreamWindowsResource(LPCTSTR pszName,
  LPCTSTR pszType, glui32 Rock) : CWinGlkStreamMem(NULL,0,Rock)
{
  HRSRC rStream = ::FindResource(NULL,pszName,pszType);
  if (rStream)
  {
    m_hStream = ::LoadResource(NULL,rStream);
    if (m_hStream)
    {
      m_pBuffer = (char*)::LockResource(m_hStream);
      m_Length = ::SizeofResource(NULL,rStream);
    }
  }
}

CWinGlkStreamWindowsResource::~CWinGlkStreamWindowsResource()
{
  if (m_hStream)
    GlobalUnlock(m_hStream);
}
