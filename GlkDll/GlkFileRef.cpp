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

namespace {

int GetFilePart(const CString& path)
{
  int start1 = path.ReverseFind('\\');
  int start2 = path.ReverseFind('/');
  if (start1 < 0)
    start1 = 0;
  if (start2 < 0)
    start2 = 0;
  return (start1 > start2) ? start1 : start2;
}

void AddExt(CString& path, glui32 usage)
{
  if (path.Find('.',GetFilePart(path)) < 0)
  {
    switch (usage & fileusage_TypeMask)
    {
    case fileusage_Data:
      path.Append(".glkdata");
      break;
    case fileusage_SavedGame:
      path.Append(".glksave");
      break;
    case fileusage_Transcript:
    case fileusage_InputRecord:
      path.Append(".txt");
      break;
    }
  }
}

} // unnamed namespace

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

void CWinGlkFileRef::SetFileName(LPCTSTR pszFileName, glui32 Usage, bool bFullPath, bool bSetExt)
{
  if (pszFileName)
    m_strFileName = pszFileName;
  else
    m_strFileName.Empty();

  // Remove any illegal characters
  {
    const char* illegal = bFullPath ? "<>\"|?*" : "/\\<>:\"|?*";
    char find[2] = { '\0','\0' };

    for (const char* p = illegal; *p != '\0'; p++)
    {
      find[0] = *p;
      m_strFileName.Replace(find,"");
    }
  }

  // Save the file name as it would have been under the old algorithm
  CString oldAlgoName = m_strFileName;

  // If required, remove any existing filename extension
  if (bSetExt)
  {
    int i = m_strFileName.Find('.',GetFilePart(m_strFileName));
    if (i >= 0)
      m_strFileName.Truncate(i);
  }

  // If the filename is now empty, set it to 'null'
  if (m_strFileName.IsEmpty())
    m_strFileName = "null";

  // If there is no file extension, add an appropriate one
  AddExt(m_strFileName,Usage);

  // If the file name exists under the old algorithm, but not the new one, use the old name
  if (!FileExists() && (::GetFileAttributes(oldAlgoName) != 0xFFFFFFFF))
    m_strFileName = oldAlgoName;
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
  strTitle.LoadString((FileMode == filemode_Read)
    ? IDS_FREF_OPEN : IDS_FREF_SAVE);

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
    OFN_HIDEREADONLY|OFN_ENABLESIZING|((FileMode == filemode_Read)
      ? OFN_FILEMUSTEXIST : OFN_OVERWRITEPROMPT),
    strFilter,NULL);
  FileDlg.m_ofn.lpstrTitle = strTitle;

  if (FileDlg.DoModal() == IDOK)
  {
    pFileRef = new CWinGlkFileRef(Usage,Rock);
    pFileRef->SetFileName(FileDlg.GetPathName(),Usage,true,false);

    switch (Usage & fileusage_TypeMask)
    {
    case fileusage_Data:
      m_strData = pFileRef->GetFileName();
      break;
    case fileusage_SavedGame:
      m_strSaved = pFileRef->GetFileName();
      break;
    case fileusage_Transcript:
      m_strTranscript = pFileRef->GetFileName();
      break;
    case fileusage_InputRecord:
      m_strRecord = pFileRef->GetFileName();
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

  AddExt(m_strData,fileusage_Data);
  AddExt(m_strSaved,fileusage_SavedGame);
  AddExt(m_strTranscript,fileusage_Transcript);
  AddExt(m_strRecord,fileusage_InputRecord);
}
