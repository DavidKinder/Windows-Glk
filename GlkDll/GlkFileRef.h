/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkFileRef
// Glk file references
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_FILEREF_H_
#define WINGLK_FILEREF_H_

extern "C"
{
#include "glk.h"
#include "gi_dispa.h"
}

/////////////////////////////////////////////////////////////////////////////
// Class for Glk file references
/////////////////////////////////////////////////////////////////////////////

class CWinGlkFileRef : public CObject
{
  DECLARE_DYNAMIC(CWinGlkFileRef)

public:
  CWinGlkFileRef(glui32 Use, glui32 Rock, bool bTemp = false);
  CWinGlkFileRef(const CWinGlkFileRef* pCopyRef, glui32 Use, glui32 Rock);
  virtual ~CWinGlkFileRef();

  glui32 GetRock(void);

  void SetDispRock(const gidispatch_rock_t& Rock) { m_DispRock = Rock; }
  gidispatch_rock_t& GetDispRock(void) { return m_DispRock; }

  CString& GetFileName(void);
  void SetFileName(LPCTSTR pszFileName, glui32 Usage, bool bFullPath, bool bSetExt);

  bool GetIsText(void);
  bool GetIsTemporary(void);

  void DeleteFile(void);
  bool FileExists(void);

protected:
  glui32 m_Use;
  glui32 m_Rock;
  gidispatch_rock_t m_DispRock;
  CString m_strFileName;
  bool m_bTemporary;

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

public:
  static void CloseAllFileRefs(void);
  static bool IsValidFileRef(CWinGlkFileRef* pFileRef);
  static CWinGlkFileRef* IterateFileRefs(CWinGlkFileRef* pFileRef, glui32* pRockPtr);

  static CWinGlkFileRef* PromptForName(glui32 Usage, glui32 FileMode, glui32 Rock);

  static void SetDefaultNames(LPCTSTR pszGameName);

protected:
  static CMap<CWinGlkFileRef*,CWinGlkFileRef*,int,int> FileRefMap;

  static CString m_strData;
  static CString m_strSaved;
  static CString m_strTranscript;
  static CString m_strRecord;
};

#endif // WINGLK_FILEREF_H_
