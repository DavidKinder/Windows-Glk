/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkDLL
// Glk dll entry and exit points
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_DLL_H_
#define WINGLK_DLL_H_

extern "C"
{
#include "glk.h"
#include "gi_blorb.h"
#include "gi_dispa.h"

extern void (*InterruptFn)(void);
extern gidispatch_rock_t (*RegisterObjFn)(void *obj, glui32 objclass);
extern void (*UnregisterObjFn)(void *obj, glui32 objclass, gidispatch_rock_t objrock);
extern gidispatch_rock_t (*RegisterArrFn)(void *array, glui32 len, char *typecode);
extern void (*UnregisterArrFn)(void *array, glui32 len, char *typecode, gidispatch_rock_t objrock);
}

#include "GlkGraphic.h"
#include "GlkSound.h"
#include "GlkStream.h"
#include "Resource.h"

#include <set>

class DarkMode;

/////////////////////////////////////////////////////////////////////////////
// CGlkApp
/////////////////////////////////////////////////////////////////////////////

class CGlkApp : public CWinApp
{
public:
  CGlkApp();

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CGlkApp)
public:
  virtual BOOL InitInstance();
  virtual int ExitInstance();
  //}}AFX_VIRTUAL

  //{{AFX_MSG(CGlkApp)
    // NOTE - the ClassWizard will add and remove member functions here.
    //    DO NOT EDIT what you see in these blocks of generated code !
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

public:
  CRect& GetWindowRect(void) { return m_WindowRect; }
  CRect& GetInnerRect(void) { return m_InnerRect; }
  int& GetWindowState(void) { return m_iWindowState; }

  void ReadSettings(void);
  void WriteSettings(void);
  void LoadConfigFile(const char* pszConfigName);
  void SetSaveOptions(bool bSave) { m_bSaveSettings = bSave; }
  CString GetRegistryPathForDarkMode(void);

  CString GetPropFontName(void) { return m_strPropFontName; }
  CString GetFixedFontName(void) { return m_strFixedFontName; }
  int GetFontPointSize(void) { return m_iFontPointSize; }
  bool SetPropFontName(LPCSTR fontName);
  bool SetFixedFontName(LPCSTR fontName);
  bool SetFontPointSize(int iFontPointSize);

  bool GetWindowBorders(void) { return m_bWindowBorders; }
  bool SetWindowBorders(bool bBorders);

  bool GetEnableGUI(void) { return m_bEnableGUI; }
  bool SetEnableGUI(bool bEnableGUI);

  bool GetStyleHints(void) { return m_bStyleHints; }
  void SetStyleHints(bool bHints) { m_bStyleHints = bHints; }

  bool GetCanSpeak(void) { return m_bSpeak; }
  void SetCanSpeak(bool bSpeak) { m_bSpeak = bSpeak; }

  CString GetSpeechVoice(void) { return m_strVoice; }
  void SetSpeechVoice(LPCSTR strVoice) { m_strVoice = strVoice; }

  int GetSpeechRate(void) { return m_iSpeakRate; }
  void SetSpeechRate(int iRate) { m_iSpeakRate = iRate; }

  int GetMaskID(void) { return m_iMaskID; }

  const std::set<unsigned long>& GetInputTerminators(void) { return m_InputTerminators; }
  void SetInputTerminators(const std::set<unsigned long>& term) { m_InputTerminators = term; }

  COLORREF GetLinkColour(void) { return m_LinkColour; }
  void SetLinkColour(COLORREF Colour) { m_LinkColour = Colour; }
  glsi32 GetTextColour(void) { return m_TextColour; }
  void SetTextColour(glsi32 Colour) { m_TextColour = Colour; }
  glsi32 GetBackColour(void) { return m_BackColour; }
  void SetBackColour(glsi32 Colour) { m_BackColour = Colour; }
  COLORREF GetSysOrDarkColour(int index, DarkMode* dark);

  CString& GetAppName(void) { return m_strAppName; }
  CString& GetAppTitle(void) { return m_strAppTitle; }
  CString& GetAppAboutText(void) { return m_strAppAboutText; }
  CString& GetMenuName(void) { return m_strMenuName; }
  CString& GetResourceDir(void) { return m_strResDir; }
  CString& GetInitialPath(void) { return m_strInitialPath; }
  void AddMenuName(CString& text);

  UINT GetUserGuiID(void) { return m_UserGuiID; }
  void SetUserGuiID(UINT id) { m_UserGuiID = id; }

  void SetHelpFile(const char* filename);
  bool HasHelpFile(void) { return m_bHasHelpFile; }

  bool GetNotifyFullScreen(void) { return m_bNotifyFull; }
  void SetNotifyFullScreen(bool notify) { m_bNotifyFull = notify; }

  bool GetStartFullScreen(void) { return m_bStartFull; }

  giblorb_map_t*& GetBlorbMap(void) { return m_pBlorbMap; }
  strid_t& GetBlorbFile(void) { return m_BlorbFile; }

  bool CreateMainWindow(void);
  HICON GetIcon(void);
  bool EventQueuesEmpty(void);
  void GetNextEvent(event_t* pEvent, bool bPoll);
  void AddEvent(glui32 Type, winid_t Win, glui32 Value1, glui32 Value2);
  void MessagePump(BOOL bWait);

  CWinGlkGraphic* LoadGraphic(int iNumber, BOOL bLoad, BOOL bApplyAlpha);
  CWinGlkSound* LoadSound(int iNumber);
  CWinGlkResource* LoadResource(int iNumber);

  bool CanSpeakChar(wchar_t c);
  void Speak(LPCSTR pszText);
  void Speak(LPCWSTR pszText);
  void InitSoundEngine(void);

  struct GameInfo
  {
    GameInfo() : cover(-1), showOptions(false)
    {
    }

    int cover;
    CString ifid;
    CString title;
    CString headline;
    CString description;
    CString author;
    CString year;
    CString series;
    CString seriesNumber;
    bool showOptions;
  };
  void LoadBabelMetadata(void);
  bool CheckGameId(void);
  GameInfo& GetGameInfo(void) { return m_GameInfo; }
  CRect GetScreenSize(bool full);

  enum Show_iFiction
  {
    Show_iF_Never = 0,
    Show_iF_First_Time = 1,
    Show_iF_Always = 2
  };
  Show_iFiction Get_iFiction(void) { return m_iFiction; }
  void Set_iFiction(Show_iFiction iF) { m_iFiction = iF; }

  bool CanOutputChar(glui32 c);

  void DebugOutput(const char* msg);
  char* DebugInput(bool wait);
  void DebugToFront(void);

protected:
  CString FileName(LPCSTR pszPrefix, int iIndex, LPCSTR pszSuffix);
  void DeleteOldTempFiles(void);
  void LoadInternationalResources(void);
  CString GetDefaultFont(void);
  CString GetDefaultFixedFont(void);
  CString StrFromXML(IXMLDOMDocument* doc, LPCWSTR path);
  void InitDebugConsole(void);
  static UINT DebugInputThread(LPVOID data);

protected:
  bool m_bSettingsRead;
  bool m_bSaveSettings;

  CString m_strPropFontName;
  CString m_strFixedFontName;
  int m_iFontPointSize;
  CRect m_WindowRect;
  CRect m_InnerRect;
  int m_iWindowState;
  bool m_bWindowBorders;
  bool m_bWindowFrame;
  bool m_bEnableGUI;
  bool m_bStyleHints;
  bool m_bHasHelpFile;
  int m_iMaskID;
  bool m_bNotifyFull;
  bool m_bStartFull;

  std::set<unsigned long> m_InputTerminators;

  bool m_bSpeak;
  CString m_strVoice;
  int m_iSpeakRate;

  COLORREF m_LinkColour;
  glsi32 m_TextColour;
  glsi32 m_BackColour;

  CString m_strAppName;
  CString m_strAppTitle;
  CString m_strAppAboutText;
  CString m_strMenuName;
  CString m_strResDir;
  CString m_strInitialPath;

  UINT m_UserGuiID;

  CArray<event_t,event_t&> InputEvents;
  CArray<event_t,event_t&> TimerEvents;
  CArray<event_t,event_t&> ArrangeEvents;
  CArray<event_t,event_t&> SoundEvents;

  strid_t m_BlorbFile;
  giblorb_map_t* m_pBlorbMap;

  Show_iFiction m_iFiction;
  GameInfo m_GameInfo;

  struct Debug
  {
    CCriticalSection lock;
    CEvent notify;
    CArray<CString,CString&> cmds;
    char line[256];
    HWND console;

    Debug() : notify(FALSE, TRUE), console(0)
    {
    }
  };
  Debug* m_Debug;
};

#endif // WINGLK_DLL_H_
