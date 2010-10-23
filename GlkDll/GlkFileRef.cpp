/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkFileRef
// Glk file references
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkDll.h"
#include "GlkFileRef.h"
#include "Dialogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Class for Glk file references
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkFileRef,CObject);

CWinGlkFileRef::CWinGlkFileRef(glui32 Use, glui32 Rock, bool bTemp) : CObject()
{
  m_Use = Use;
  m_Rock = Rock;
  m_bTemporary = bTemp;

  // Add to the global map of all file references
  FileRefMap[this] = 0;

  if (RegisterObjFn)
    SetDispRock((*RegisterObjFn)(this,gidisp_Class_Fileref));
  else
    m_DispRock.num = 0;
}

CWinGlkFileRef::CWinGlkFileRef(const CWinGlkFileRef* pCopyRef, glui32 Use, glui32 Rock) : CObject()
{
  m_Use = Use;
  m_Rock = Rock;

  m_strFileName = pCopyRef->m_strFileName;
  m_bTemporary = pCopyRef->m_bTemporary;

  FileRefMap[this] = 0;
  if (RegisterObjFn)
    SetDispRock((*RegisterObjFn)(this,gidisp_Class_Fileref));
  else
    m_DispRock.num = 0;
}

CWinGlkFileRef::~CWinGlkFileRef()
{
  if (UnregisterObjFn)
    (*UnregisterObjFn)(this,gidisp_Class_Fileref,GetDispRock());

  // Remove from the global map
  FileRefMap.RemoveKey(this);
}

glui32 CWinGlkFileRef::GetRock(void)
{
  return m_Rock;
}

CString& CWinGlkFileRef::GetFileName(void)
{
  return m_strFileName;
}

void CWinGlkFileRef::SetFileName(LPCTSTR pszFileName, bool bValidate)
{
  if (pszFileName)
  {
    m_strFileName = pszFileName;
    if (bValidate)
    {
      m_strFileName.Replace('\\','-');
      m_strFileName.Replace('/','-');
      m_strFileName.Replace(':','-');
    }
  }
}

bool CWinGlkFileRef::GetIsText(void)
{
  return m_Use & fileusage_TextMode ? true : false;
}

bool CWinGlkFileRef::GetIsTemporary(void)
{
  return m_bTemporary;
}

void CWinGlkFileRef::DeleteFile(void)
{
  try
  {
    CFile::Remove(m_strFileName);
  }
  catch (CException* pEx)
  {
    pEx->Delete();
  }
}

bool CWinGlkFileRef::FileExists(void)
{
  return (::GetFileAttributes(m_strFileName) != 0xFFFFFFFF);
}

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

CMap<CWinGlkFileRef*,CWinGlkFileRef*,int,int> CWinGlkFileRef::FileRefMap;

CString CWinGlkFileRef::m_strData;
CString CWinGlkFileRef::m_strSaved;
CString CWinGlkFileRef::m_strTranscript;
CString CWinGlkFileRef::m_strRecord;

void CWinGlkFileRef::CloseAllFileRefs(void)
{
  CWinGlkFileRef* pFileRef;
  CArray<CWinGlkFileRef*,CWinGlkFileRef*> FileRefArray;
  int i;

  POSITION MapPos = FileRefMap.GetStartPosition();
  while (MapPos)
  {
    FileRefMap.GetNextAssoc(MapPos,pFileRef,i);
    FileRefArray.Add(pFileRef);
  }

  for (i = 0; i < FileRefArray.GetSize(); i++)
    delete FileRefArray[i];
}

bool CWinGlkFileRef::IsValidFileRef(CWinGlkFileRef* pFileRef)
{
  int iDummy;
  if (FileRefMap.Lookup(pFileRef,iDummy))
    return true;
  return false;
}

CWinGlkFileRef* CWinGlkFileRef::IterateFileRefs(CWinGlkFileRef* pFileRef, glui32* pRockPtr)
{
  CWinGlkFileRef* pMapFileRef = NULL;
  CWinGlkFileRef* pPrevFileRef = NULL;
  int iDummy;

  POSITION MapPos = FileRefMap.GetStartPosition();
  while (MapPos)
  {
    FileRefMap.GetNextAssoc(MapPos,pMapFileRef,iDummy);
    if (pFileRef == pPrevFileRef)
    {
      if (pRockPtr)
        *pRockPtr = pMapFileRef->GetRock();
      return pMapFileRef;
    }
    pPrevFileRef = pMapFileRef;
  }
  return NULL;
}

CWinGlkFileRef* CWinGlkFileRef::PromptForName(glui32 Usage, glui32 FileMode, glui32 Rock)
{
  CWinGlkFileRef* pFileRef = NULL;

  CString strTitle;
  strTitle.LoadString(FileMode == filemode_Read ?
    IDS_FREF_OPEN : IDS_FREF_SAVE);

  CString strDescription, strFileName;
  switch (Usage & fileusage_TypeMask)
  {
  case fileusage_Data:
    strDescription.LoadString(IDS_FREF_DATA);
    strFileName = m_strData;
    break;
  case fileusage_SavedGame:
    strDescription.LoadString(IDS_FREF_STATE);
    strFileName = m_strSaved;
    break;
  case fileusage_Transcript:
    strDescription.LoadString(IDS_FREF_SCRIPT);
    strFileName = m_strTranscript;
    break;
  case fileusage_InputRecord:
    strDescription.LoadString(IDS_FREF_INPUT);
    strFileName = m_strRecord;
    break;
  }

  if (strDescription.GetLength() > 0)
  {
    strTitle += ' ';
    strTitle += strDescription;
  }

  CString strFilter;
  strFilter.LoadString(IDS_FREF_FILTER);
  SimpleFileDialog FileDlg(FileMode == filemode_Read,NULL,strFileName,
    OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_ENABLESIZING,strFilter,NULL);
  FileDlg.m_ofn.lpstrTitle = strTitle;

  if (FileDlg.DoModal() == IDOK)
  {
    pFileRef = new CWinGlkFileRef(Usage,Rock);
    pFileRef->SetFileName(FileDlg.GetPathName());

    switch (Usage & fileusage_TypeMask)
    {
    case fileusage_Data:
      m_strData = FileDlg.GetPathName();
      break;
    case fileusage_SavedGame:
      m_strSaved = FileDlg.GetPathName();
      break;
    case fileusage_Transcript:
      m_strTranscript = FileDlg.GetPathName();
      break;
    case fileusage_InputRecord:
      m_strRecord = FileDlg.GetPathName();
      break;
    }
  }

  return pFileRef;
}

void CWinGlkFileRef::SetDefaultNames(LPCTSTR pszGameName)
{
  m_strData = pszGameName;
  m_strSaved = pszGameName;
  m_strTranscript = pszGameName;
  m_strRecord = pszGameName;

  if (m_strData.Find('.') < 0)
    m_strData += ".dat";
  if (m_strSaved.Find('.') < 0)
    m_strSaved += ".sav";
  if (m_strTranscript.Find('.') < 0)
    m_strTranscript += ".txt";
  if (m_strRecord.Find('.') < 0)
    m_strRecord += ".rec";
}
