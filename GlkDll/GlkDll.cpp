/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkDLL
// Glk DLL entry and exit points
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DSoundEngine.h"
#include "GlkDll.h"
#include "GlkDialogs.h"
#include "GlkFileRef.h"
#include "GlkMainWnd.h"
#include "GlkSndChannel.h"
#include "GlkTalk.h"
#include "GlkTime.h"
#include "GlkUnicode.h"
#include "GlkWindowGfx.h"
#include "GlkWindowTextBuffer.h"
#include "GlkWindowTextGrid.h"
#include "WinGlk.h"

#include <MultiMon.h>

extern "C"
{
#include "gi_debug.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGlkApp
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGlkApp, CWinApp)
  //{{AFX_MSG_MAP(CGlkApp)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGlkApp construction
/////////////////////////////////////////////////////////////////////////////

CGlkApp::CGlkApp()
{
  m_bSettingsRead = false;
  m_bSaveSettings = true;
  m_bWindowBorders = false;
  m_bWindowFrame = true;
  m_bEnableGUI = true;
  m_bStyleHints = true;
  m_bHasHelpFile = false;
  m_iMaskID = -1;
  m_bNotifyFull = true;
  m_bStartFull = false;

  m_bSpeak = false;
  m_iSpeakRate = 0;

  m_UserGuiID = 0;

  m_BlorbFile = 0;
  m_pBlorbMap = NULL;

  m_iFiction = Show_iF_First_Time;

  m_Debug = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGlkApp object
/////////////////////////////////////////////////////////////////////////////

CGlkApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CGlkApp message handlers
/////////////////////////////////////////////////////////////////////////////

BOOL CGlkApp::InitInstance() 
{
/*
  AfxMessageBox("Glk Test "__DATE__" "__TIME__,MB_OK|MB_ICONINFORMATION);
*/

  SetRegistryKey("Glk Applications");

  // Prepare resource loaders
  DeleteOldTempFiles();
  CWinGlkGraphicLoader::InitLoaders();
  CWinGlkSoundLoader::InitLoaders();

  // Enable COM
  ::CoInitialize(NULL);

  // Enable rich edit controls
  AfxInitRichEdit2();

  // Enable help
  EnableHtmlHelp();

  // Load international resources, if needed
  LoadInternationalResources();

  // Set data from resources
  m_strAppName.LoadString(IDS_STORY);
  m_strAppTitle.LoadString(IDS_STORY);
  m_strMenuName = "&Glk";

  // Mark the application as DPI aware
  HMODULE user = ::LoadLibrary("user32.dll");
  if (user != 0)
  {
    typedef BOOL(__stdcall *SETPROCESSDPIAWARE)(void);
    SETPROCESSDPIAWARE setProcessDPIAware = (SETPROCESSDPIAWARE)::GetProcAddress(
      user,"SetProcessDPIAware");
    if (setProcessDPIAware != NULL)
      (*setProcessDPIAware)();
    ::FreeLibrary(user);
  }

  return CWinApp::InitInstance();
}

int CGlkApp::ExitInstance() 
{
  CWinGlkGraphicLoader::RemoveLoaders();
  CWinGlkSoundLoader::RemoveLoaders();

  WriteSettings();
  ::CoUninitialize();
  return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// Support routines and variables for Windows Glk
/////////////////////////////////////////////////////////////////////////////

extern "C"
{
  void (*InterruptFn)(void) = NULL;
  gidispatch_rock_t (*RegisterObjFn)(void *obj, glui32 objclass) = NULL;
  void (*UnregisterObjFn)(void *obj, glui32 objclass, gidispatch_rock_t objrock) = NULL;
  gidispatch_rock_t (*RegisterArrFn)(void *array, glui32 len, char *typecode) = NULL;
  void (*UnregisterArrFn)(void *array, glui32 len, char *typecode, gidispatch_rock_t objrock) = NULL;
};

void CGlkApp::ReadSettings(void)
{
  if (m_bSettingsRead == false)
  {
    int iVersion = GetProfileInt("Glk Settings","Version",0);

    // Get a device context for the current display
    CWnd* pDesktop = CWnd::GetDesktopWindow();
    CDC* pDC = pDesktop->GetDC();

    ::ZeroMemory(&m_PropFont,sizeof(LOGFONT));
    m_PropFont.lfHeight = -MulDiv(GetProfileInt("Glk Settings","Proportional Font Size",10),
      pDC->GetDeviceCaps(LOGPIXELSY),72);
    m_PropFont.lfCharSet = ANSI_CHARSET;
    m_PropFont.lfOutPrecision = OUT_TT_PRECIS;
    m_PropFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    m_PropFont.lfQuality = PROOF_QUALITY;
    m_PropFont.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
    strncpy(m_PropFont.lfFaceName,
      GetProfileString("Glk Settings","Proportional Font Name",GetDefaultFont()),LF_FACESIZE);
    if ((iVersion < 131) && (strcmp(m_PropFont.lfFaceName,"Times New Roman") == 0))
      strncpy(m_PropFont.lfFaceName,GetDefaultFont(),LF_FACESIZE);

    ::ZeroMemory(&m_FixedFont,sizeof(LOGFONT));
    m_FixedFont.lfHeight = -MulDiv(GetProfileInt("Glk Settings","Fixed Font Size",10),
      pDC->GetDeviceCaps(LOGPIXELSY),72);
    m_FixedFont.lfCharSet = ANSI_CHARSET;
    m_FixedFont.lfOutPrecision = OUT_TT_PRECIS;
    m_FixedFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    m_FixedFont.lfQuality = PROOF_QUALITY;
    m_FixedFont.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
    strncpy(m_FixedFont.lfFaceName,
      GetProfileString("Glk Settings","Fixed Font Name",GetDefaultFixedFont()),LF_FACESIZE);
    if ((iVersion < 131) && (strcmp(m_FixedFont.lfFaceName,"Courier New") == 0))
      strncpy(m_FixedFont.lfFaceName,GetDefaultFixedFont(),LF_FACESIZE);

    m_WindowRect.left = GetProfileInt("Glk Settings","Window Left",0);
    m_WindowRect.top = GetProfileInt("Glk Settings","Window Top",0);
    m_WindowRect.right = GetProfileInt("Glk Settings","Window Right",0);
    m_WindowRect.bottom = GetProfileInt("Glk Settings","Window Bottom",0);
    m_iWindowState = GetProfileInt("Glk Settings","Window State",SW_SHOW);
    m_bNotifyFull = GetProfileInt("Glk Settings","Notify Full Screen",1) ? true : false;

    m_bWindowBorders = GetProfileInt("Glk Settings","Window Borders",0) ? true : false;
    if (iVersion < 131)
      m_bWindowBorders = false;

    m_bEnableGUI = GetProfileInt("Glk Settings","Enable GUI",1) ? true : false;
    m_bStyleHints = GetProfileInt("Glk Settings","Style Hints Active",1) ? true : false;
    m_bSpeak = GetProfileInt("Glk Settings","Speak Text",0) ? true : false;
    m_strVoice = GetProfileString("Glk Settings","Voice","");
    m_iSpeakRate = GetProfileInt("Glk Settings","Speech Rate",0);

    m_LinkColour = GetProfileInt("Glk Settings","Hyperlink Colour",RGB(0x00,0x00,0xFF));

    m_strInitialDir = GetProfileString("Glk Settings","Directory","");
    m_iFiction = (Show_iFiction)GetProfileInt("Glk Settings","Show iFiction Dialog",
      Show_iF_First_Time);

    pDesktop->ReleaseDC(pDC);

    CWinGlkWndTextBuffer::GetDefaultStyles()->ReadSettings("Glk Buffer Style %d",iVersion);
    CWinGlkWndTextGrid::GetDefaultStyles()->ReadSettings("Glk Grid Style %d",iVersion);
    m_TextColour = GetProfileInt("Glk Buffer Style 0","Text Colour",0xFFFFFFFF);
    m_BackColour = GetProfileInt("Glk Buffer Style 0","Back Colour",0xFFFFFFFE);

    m_bSettingsRead = true;
  }
}

void CGlkApp::WriteSettings(void)
{
  if (m_bSettingsRead && m_bSaveSettings)
  {
    WriteProfileInt("Glk Settings","Version",131);

    CWnd* pDesktop = CWnd::GetDesktopWindow();
    CDC* pDC = pDesktop->GetDC();

    WriteProfileString("Glk Settings","Proportional Font Name",CString(m_PropFont.lfFaceName));
    WriteProfileInt("Glk Settings","Proportional Font Size",
      -MulDiv(m_PropFont.lfHeight,72,pDC->GetDeviceCaps(LOGPIXELSY)));

    WriteProfileString("Glk Settings","Fixed Font Name",CString(m_FixedFont.lfFaceName));
    WriteProfileInt("Glk Settings","Fixed Font Size",
      -MulDiv(m_FixedFont.lfHeight,72,pDC->GetDeviceCaps(LOGPIXELSY)));

    WriteProfileInt("Glk Settings","Window Left",m_WindowRect.left);
    WriteProfileInt("Glk Settings","Window Top",m_WindowRect.top);
    WriteProfileInt("Glk Settings","Window Right",m_WindowRect.right);
    WriteProfileInt("Glk Settings","Window Bottom",m_WindowRect.bottom);
    WriteProfileInt("Glk Settings","Window State",m_iWindowState);
    WriteProfileInt("Glk Settings","Notify Full Screen",m_bNotifyFull ? 1 : 0);

    WriteProfileInt("Glk Settings","Window Borders",m_bWindowBorders ? 1 : 0);
    WriteProfileInt("Glk Settings","Enable GUI",m_bEnableGUI ? 1 : 0);
    WriteProfileInt("Glk Settings","Style Hints Active",m_bStyleHints ? 1 : 0);
    WriteProfileInt("Glk Settings","Speak Text",m_bSpeak ? 1 : 0);
    WriteProfileString("Glk Settings","Voice",m_strVoice);
    WriteProfileInt("Glk Settings","Speech Rate",m_iSpeakRate);

    WriteProfileInt("Glk Settings","Hyperlink Colour",m_LinkColour);

    WriteProfileString("Glk Settings","Directory",m_strInitialDir);
    WriteProfileInt("Glk Settings","Show iFiction Dialog",m_iFiction);

    pDesktop->ReleaseDC(pDC);

    CWinGlkWndTextBuffer::GetDefaultStyles()->WriteSettings("Glk Buffer Style %d");
    CWinGlkWndTextGrid::GetDefaultStyles()->WriteSettings("Glk Grid Style %d");
    WriteProfileInt("Glk Buffer Style 0","Text Colour",m_TextColour);
    WriteProfileInt("Glk Buffer Style 0","Back Colour",m_BackColour);
  }
}

void CGlkApp::LoadConfigFile(const char* pszConfigName)
{
  // Read user settings now, if this has not already been done
  ReadSettings();

  CWnd* pDesktop = CWnd::GetDesktopWindow();
  CDC* pDC = pDesktop->GetDC();

  CStdioFile configFile;
  CString configLine;
  if (configFile.Open(pszConfigName,CFile::modeRead|CFile::typeText))
  {
    m_bSaveSettings = false;
    while (configFile.ReadString(configLine))
    {
      int i = configLine.Find('=');
      if ((i >= 0) && (i < configLine.GetLength()-1))
      {
        CString key = configLine.Left(i);
        CString value = configLine.Right(configLine.GetLength()-i-1);

        // Turn borders between Glk windows on or off
        if (key.CompareNoCase("WindowBorders") == 0)
          m_bWindowBorders = (value.CompareNoCase("yes") == 0) ? true : false;

        // Turn the Glk application's window frame on or off
        if (key.CompareNoCase("WindowFrame") == 0)
        {
          m_bWindowFrame = (value.CompareNoCase("yes") == 0) ? true : false;
          if (m_bWindowFrame == false)
            m_bEnableGUI = false;
        }

        // Set the Blorb ID of the graphic to use as a mask
        // for the shape of the Glk application's window
        if (key.CompareNoCase("WindowMask") == 0)
        {
          int iMaskID = -1;
          if (sscanf(value,"%ld",&iMaskID) == 1)
            m_iMaskID = iMaskID;
        }

        // Set the width and height of the Glk application's window
        if (key.CompareNoCase("WindowWidth") == 0)
        {
          m_InnerRect.left = 0;
          sscanf(value,"%ld",&m_InnerRect.right);
          m_iWindowState = SW_SHOW;
        }
        if (key.CompareNoCase("WindowHeight") == 0)
        {
          m_InnerRect.top = 0;
          sscanf(value,"%ld",&m_InnerRect.bottom);
          m_iWindowState = SW_SHOW;
        }
        if (key.CompareNoCase("FullScreen") == 0)
        {
          if (value.CompareNoCase("yes") == 0)
          {
            m_InnerRect = GetScreenSize(true);
            m_iWindowState = SW_SHOW;
            m_bStartFull = true;
          }
        }

        // Set the name and point size of the proportional and
        // fixed width fonts
        int fontSize = 0;
        if (key.CompareNoCase("FontName") == 0)
          strncpy(m_PropFont.lfFaceName,value,LF_FACESIZE);
        if (key.CompareNoCase("FixedFontName") == 0)
          strncpy(m_FixedFont.lfFaceName,value,LF_FACESIZE);
        if (key.CompareNoCase("FontSize") == 0)
        {
          if (sscanf(value,"%d",&fontSize) == 1)
          {
            m_PropFont.lfHeight = -MulDiv(fontSize,
              pDC->GetDeviceCaps(LOGPIXELSY),72);
          }
        }
        if (key.CompareNoCase("FixedFontSize") == 0)
        {
          if (sscanf(value,"%d",&fontSize) == 1)
          {
            m_FixedFont.lfHeight = -MulDiv(fontSize,
              pDC->GetDeviceCaps(LOGPIXELSY),72);
          }
        }
        if (key.CompareNoCase("FontFile") == 0)
        {
          CString fontPath(pszConfigName);
          int iDir = fontPath.ReverseFind('\\');
          if (iDir >= 0)
            fontPath.Truncate(iDir+1);
          fontPath.Append(value);
          ::AddFontResource(fontPath);
        }
      }
    }
  }
  pDesktop->ReleaseDC(pDC);
}

void CGlkApp::LoadInternationalResources(void)
{
  const char* resDllName = NULL;
  switch (PRIMARYLANGID(::GetUserDefaultLangID()))
  {
  case LANG_FRENCH:
    resDllName = "GlkFrançais.dll";
    break;
  case LANG_GERMAN:
    resDllName = "GlkDeutsch.dll";
    break;
  case LANG_ITALIAN:
    resDllName = "GlkItaliano.dll";
    break;
  case LANG_SPANISH:
    resDllName = "GlkEspañol.dll";
    break;
  }

  if (resDllName != NULL)
  {
    HINSTANCE hDll = ::LoadLibrary(resDllName);
    if (hDll != NULL)
      AfxSetResourceHandle(hDll);
  }
}

bool CGlkApp::SetWindowBorders(bool bBorders)
{
  bool bBorderChanged = false;
  if (bBorders != m_bWindowBorders)
  {
    m_bWindowBorders = bBorders;

    CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetMainWnd();
    if (pMainWnd)
    {
      pMainWnd->SetBorders(bBorders);
      bBorderChanged = true;
    }
  }
  return bBorderChanged;
}

bool CGlkApp::SetEnableGUI(bool bEnableGUI)
{
  bool bGUIChanged = false;
  if (bEnableGUI != m_bEnableGUI)
  {
    m_bEnableGUI = bEnableGUI;

    CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetMainWnd();
    if (pMainWnd)
    {
      pMainWnd->SetGUI(bEnableGUI);
      bGUIChanged = true;
    }
  }
  return bGUIChanged;
}

void CGlkApp::AddMenuName(CString& text)
{
  CString name(m_strMenuName);
  name.Remove('&');
  text.Replace("Glk",(LPCTSTR)name);
}

void CGlkApp::SetHelpFile(const char* filename)
{
  free((void*)m_pszHelpFilePath);
  m_pszHelpFilePath = strdup(filename);
  m_bHasHelpFile = true;
}

bool CGlkApp::CreateMainWindow(void)
{
  if (m_pMainWnd == NULL)
  {
    CWinGlkMainWnd* pMainWindow = new CWinGlkMainWnd;
    if (pMainWindow->Create(m_bWindowFrame) == false)
      return false;

    // Use the created window as the main application window
    m_pMainWnd = pMainWindow;
    ::SetCursor(LoadCursor(IDC_ARROW));
  }
  return true;
}

HICON CGlkApp::GetIcon(void)
{
  HICON Icon = NULL;
  if (GetUserGuiID() != 0)
    Icon = ::LoadIcon(::GetModuleHandle(NULL),MAKEINTRESOURCE(GetUserGuiID()));
  if (Icon == NULL)
    Icon = LoadIcon(IDR_GLK);
  return Icon;
}

bool CGlkApp::EventQueuesEmpty(void)
{
  if (InputEvents.GetSize() > 0)
    return false;
  if (SoundEvents.GetSize() > 0)
    return false;
  if (TimerEvents.GetSize() > 0)
    return false;
  if (ArrangeEvents.GetSize() > 0)
    return false;
  return true;
}

void CGlkApp::GetNextEvent(event_t* pEvent, bool bPoll)
{
  if (bPoll == false)
  {
    if (InputEvents.GetSize() > 0)
    {
      memcpy(pEvent,&InputEvents[0],sizeof(event_t));
      InputEvents.RemoveAt(0);
      return;
    }
  }
  if (SoundEvents.GetSize() > 0)
  {
    memcpy(pEvent,&SoundEvents[0],sizeof(event_t));
    SoundEvents.RemoveAt(0);
    return;
  }
  if (TimerEvents.GetSize() > 0)
  {
    memcpy(pEvent,&TimerEvents[0],sizeof(event_t));
    TimerEvents.RemoveAt(0);
    return;
  }
  if (ArrangeEvents.GetSize() > 0)
  {
    memcpy(pEvent,&ArrangeEvents[0],sizeof(event_t));
    ArrangeEvents.RemoveAt(0);
    return;
  }
}

void CGlkApp::AddEvent(glui32 Type, winid_t Win, glui32 Value1, glui32 Value2)
{
  switch (Type)
  {
  case evtype_LineInput:
  case evtype_CharInput:
  case evtype_MouseInput:
  case evtype_Hyperlink:
  case winglk_evtype_GuiInput:
    {
      int iSize = InputEvents.GetSize();
      InputEvents.SetSize(iSize+1);

      InputEvents[iSize].type = Type;
      InputEvents[iSize].win = Win;
      InputEvents[iSize].val1 = Value1;
      InputEvents[iSize].val2 = Value2;
    }
    break;
  case evtype_Timer:
    if (TimerEvents.GetSize() == 0)
    {
      TimerEvents.SetSize(1);

      TimerEvents[0].type = Type;
      TimerEvents[0].win = Win;
      TimerEvents[0].val1 = Value1;
      TimerEvents[0].val2 = Value2;

      TRACE("Glk: Timer Event\n");
    }
    break;
  case evtype_Arrange:
    if (ArrangeEvents.GetSize() == 0)
    {
      ArrangeEvents.SetSize(1);

      ArrangeEvents[0].type = Type;
      ArrangeEvents[0].win = Win;
      ArrangeEvents[0].val1 = Value1;
      ArrangeEvents[0].val2 = Value2;

      TRACE("Glk: Arrange Event\n");
    }
    break;
  case evtype_SoundNotify:
  case evtype_VolumeNotify:
    {
      int iSize = SoundEvents.GetSize();
      SoundEvents.SetSize(iSize+1);

      SoundEvents[iSize].type = Type;
      SoundEvents[iSize].win = Win;
      SoundEvents[iSize].val1 = Value1;
      SoundEvents[iSize].val2 = Value2;

      TRACE("Glk: Sound Notification Event\n");
    }
    break;
  }
}

// Tick count when the message pump was last run
static DWORD LastTick = 0;

void CGlkApp::MessagePump(BOOL bWait)
{
  LastTick = ::GetTickCount();

  CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetMainWnd();
  if (pMainWnd)
  {
    if (pMainWnd->IsWindowVisible() == FALSE)
      pMainWnd->ShowWindow(GetWindowState());
  }

  bool bExit = false;
  MSG msg;
  if (bWait)
  {
    if (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
    { 
      while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
      {
        if (PumpMessage() == FALSE)
          bExit = true;
      }
    }
    else
    {
      LONG lIdle = 0;
      BOOL bIdle = TRUE;
      while (AfxGetMainWnd() && bIdle)
        bIdle = CWinApp::OnIdle(lIdle++);

      if (m_Debug)
      {
        HANDLE notify = m_Debug->notify;
        ::MsgWaitForMultipleObjects(1,&notify,FALSE,INFINITE,QS_ALLINPUT);
      }
      else
        ::WaitMessage();
    }
  }
  else
  {
    while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
    { 
      if (PumpMessage() == FALSE)
        bExit = true;
    }
  }

  if (bExit || (AfxGetMainWnd() == NULL))
  {
    if (InterruptFn)
      (*InterruptFn)();
    glk_exit();
  }
}

// Get the default font
CString CGlkApp::GetDefaultFont(void)
{
  // Get desktop settings
  NONCLIENTMETRICS ncm;
  ::ZeroMemory(&ncm,sizeof ncm);
  ncm.cbSize = sizeof ncm;
  ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof ncm,&ncm,0);
  CString fontName(ncm.lfMessageFont.lfFaceName);

  // Get a device context
  CWnd* wnd = CWnd::GetDesktopWindow();
  CDC* dc = wnd->GetDC();

  // Create the font
  CFont font;
  font.CreatePointFont(10,fontName);

  // Test if the font is TrueType
  CFont* oldFont = dc->SelectObject(&font);
  if (dc->GetOutlineTextMetrics(0,NULL) == 0)
    fontName = "Times New Roman";

  // Free the device context
  dc->SelectObject(oldFont);
  wnd->ReleaseDC(dc);

  return fontName;
}

static int CALLBACK EnumFontProc(ENUMLOGFONTEX*, NEWTEXTMETRICEX* ,DWORD, LPARAM found)
{
  *((bool*)found) = true;
  return 0;
}

// Get the default fixed width font
CString CGlkApp::GetDefaultFixedFont(void)
{
  CString fontName = "Courier";

  // Get a device context for the display
  CWnd* wnd = CWnd::GetDesktopWindow();
  CDC* dc = wnd->GetDC();

  // List of fixed width fonts to look for
  const char* fixedFonts[] =
  {
    "Consolas",
    "Lucida Console",
    "Courier New"
  };

  // Search the list of fixed width fonts for a match
  LOGFONT fontInfo;
  ::ZeroMemory(&fontInfo,sizeof fontInfo);
  fontInfo.lfCharSet = DEFAULT_CHARSET;
  bool found = false;
  for (int i = 0; i < sizeof fixedFonts / sizeof fixedFonts[0]; i++)
  {
    strcpy(fontInfo.lfFaceName,fixedFonts[i]);
    ::EnumFontFamiliesEx(dc->GetSafeHdc(),&fontInfo,(FONTENUMPROC)EnumFontProc,(LPARAM)&found,0);
    if (found)
    {
      fontName = fontInfo.lfFaceName;
      break;
    }
  }

  // Release the desktop device context
  wnd->ReleaseDC(dc);
  return fontName;
}

void CGlkApp::LoadBabelMetadata(void)
{
  if (m_pBlorbMap == NULL)
    return;

  giblorb_result_t result;
  unsigned int id_Fspc = giblorb_make_id('F','s','p','c');
  if (giblorb_load_chunk_by_type(m_pBlorbMap,giblorb_method_Memory,&result,id_Fspc,0) == giblorb_err_None)
  {
    unsigned char* data = (unsigned char*)result.data.ptr;
    m_GameInfo.cover = (data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3];
    giblorb_unload_chunk(m_pBlorbMap,result.chunknum);
  }

  unsigned int id_IFmd = giblorb_make_id('I','F','m','d');
  if (giblorb_load_chunk_by_type(m_pBlorbMap,giblorb_method_Memory,&result,id_IFmd,0) != giblorb_err_None)
    return;

  CString meta((const char*)result.data.ptr,result.length);
  giblorb_unload_chunk(m_pBlorbMap,result.chunknum);

  CComPtr<IXMLDOMDocument> doc;
  if (FAILED(doc.CoCreateInstance(CLSID_DOMDocument)))
    return;

  VARIANT_BOOL success = 0;
  CStreamOnCString metaStream(meta);
  if (doc->load(CComVariant(&metaStream),&success) != S_OK)
    return;

  m_GameInfo.ifid = StrFromXML(doc,L"/ifindex/story/identification/ifid");
  m_GameInfo.title = StrFromXML(doc,L"/ifindex/story/bibliographic/title");
  m_GameInfo.headline = StrFromXML(doc,L"/ifindex/story/bibliographic/headline");
  m_GameInfo.author = StrFromXML(doc,L"/ifindex/story/bibliographic/author");
  m_GameInfo.year = StrFromXML(doc,L"/ifindex/story/bibliographic/firstpublished");
  m_GameInfo.series = StrFromXML(doc,L"/ifindex/story/bibliographic/series");
  m_GameInfo.seriesNumber = StrFromXML(doc,L"/ifindex/story/bibliographic/seriesnumber");

  CComPtr<IXMLDOMNode> node;
  CComBSTR path(L"/ifindex/story/bibliographic/description");
  if (SUCCEEDED(doc->selectSingleNode(path,&node)) && (node != NULL))
  {
    CComPtr<IXMLDOMNodeList> childList;
    if (SUCCEEDED(node->get_childNodes(&childList)))
    {
      CComPtr<IXMLDOMNode> childNode;
      while (SUCCEEDED(childList->nextNode(&childNode)))
      {
        if (childNode == NULL)
          break;

        DOMNodeType type;
        if (SUCCEEDED(childNode->get_nodeType(&type)))
        {
          switch (type)
          {
          case NODE_TEXT:
            {
              CComBSTR text;
              childNode->get_text(&text);

              CString unformatted(text.m_str), formatted, token;
              int pos = 0;
              token = unformatted.Tokenize(" \t\r\n",pos);
              while (token.IsEmpty() == FALSE)
              {
                if (formatted.IsEmpty() == FALSE)
                  formatted.AppendChar(' ');
                formatted.Append(token);
                token = unformatted.Tokenize(" \t\r\n",pos);
              }

              m_GameInfo.description.Append(formatted);
            }
            break;
          case NODE_ELEMENT:
            {
              CComBSTR name;
              childNode->get_nodeName(&name);
              if (name == L"br")
                m_GameInfo.description.Append("\r\r");
            }
            break;
          }
        }

        childNode.Release();
      }
    }
  }
}

bool CGlkApp::CheckGameId(void)
{
  if (m_GameInfo.ifid.IsEmpty())
    return false;

  // Does a key exist for this game? If not, create it
  CRegKey key;
  DWORD disposition = 0;
  if (key.Create(GetSectionKey("Known Games"),m_GameInfo.ifid,REG_NONE,REG_OPTION_NON_VOLATILE,
    KEY_READ|KEY_WRITE,NULL,&disposition) != ERROR_SUCCESS)
  {
    return false;
  }

  switch (m_iFiction)
  {
  case Show_iF_Never:
    return false;
  case Show_iF_First_Time:
    return (disposition == REG_CREATED_NEW_KEY);
  case Show_iF_Always:
    return true;
  default:
    return false;
  }
}

CRect CGlkApp::GetScreenSize(bool full)
{
  MONITORINFO monInfo;
  ::ZeroMemory(&monInfo,sizeof monInfo);
  monInfo.cbSize = sizeof monInfo;

  HMONITOR mon = ::MonitorFromWindow(AfxGetMainWnd()->GetSafeHwnd(),MONITOR_DEFAULTTOPRIMARY);
  if (::GetMonitorInfo(mon,&monInfo))
    return full ? monInfo.rcMonitor : monInfo.rcWork;

  return CRect(0,0,::GetSystemMetrics(SM_CXSCREEN),::GetSystemMetrics(SM_CYSCREEN));
}

bool CGlkApp::CanOutputChar(glui32 c)
{
  CWinGlkBufferDC dc(NULL);

  CWnd* pDesktop = CWnd::GetDesktopWindow();
  CDC* pDC = pDesktop->GetDC();
  dc.CreateCompatibleDC(pDC);
  pDesktop->ReleaseDC(pDC);

  dc.SetStyle(style_Normal,0,NULL);

  CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetMainWnd();
  if (pMainWnd)
  {
    TextOutput& TextOut = pMainWnd->GetTextOut();
    return TextOut.CanOutput(dc.GetSafeHdc(),c);
  }
  else
  {
    TextOutput TextOut;
    return TextOut.CanOutput(dc.GetSafeHdc(),c);
  }

  return false;
}

void CGlkApp::DebugOutput(const char* msg)
{
  InitDebugConsole();

  HANDLE out = ::GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD written;
  ::WriteFile(out,msg,strlen(msg),&written,NULL);
  ::WriteFile(out,"\n",1,&written,NULL);
}

char* CGlkApp::DebugInput(bool wait)
{
  InitDebugConsole();

  while (true)
  {
    {
      CSingleLock guard(&(m_Debug->lock),TRUE);
      if (!m_Debug->cmds.IsEmpty())
      {
        CString cmd = m_Debug->cmds.GetAt(0);
        m_Debug->cmds.RemoveAt(0);
        if (m_Debug->cmds.IsEmpty())
          m_Debug->notify.ResetEvent();

        size_t max = sizeof m_Debug->line;
        memset(m_Debug->line,0,max);
        strncpy(m_Debug->line,cmd,max-1);
        return m_Debug->line;
      }
    }

    if (wait)
    {
      if (AfxGetMainWnd() != NULL)
        MessagePump(TRUE);
      else
        ::WaitForSingleObject(m_Debug->notify,INFINITE);
    }
    else
      return NULL;
  }
}

void CGlkApp::DebugToFront(void)
{
  InitDebugConsole();

  if (m_Debug->console != 0)
    ::SetForegroundWindow(m_Debug->console);
}

void CGlkApp::InitDebugConsole(void)
{
  if (m_Debug == NULL)
  {
    // Disconnect from any existing console and create ourselves a new one
    ::FreeConsole();
    ::AllocConsole();
    ::SetConsoleCtrlHandler(NULL,TRUE);
    m_Debug = new Debug();

    // Set the title for the console window
    CString title;
    title.Format("%s Debug",m_strAppName);
    ::SetConsoleTitle(title);

    // Get the console's window handle, if possible
    HMODULE kernel = ::LoadLibrary("kernel32.dll");
    if (kernel != 0)
    {
      typedef HWND(__stdcall *GETCONSOLEWINDOW)(void);
      GETCONSOLEWINDOW getConsoleWindow = (GETCONSOLEWINDOW)::GetProcAddress(kernel,"GetConsoleWindow");
      if (getConsoleWindow != NULL)
        m_Debug->console = (*getConsoleWindow)();
      ::FreeLibrary(kernel);
    }

    // Set the console window's icon, if possible
    if (m_Debug->console != 0)
      ::SendMessage(m_Debug->console,WM_SETICON,ICON_BIG,(LPARAM)GetIcon());

    // Start a thread to read from the console
    AfxBeginThread(DebugInputThread,m_Debug);
  }
}

UINT CGlkApp::DebugInputThread(LPVOID data)
{
  Debug* debug = (Debug*)data;
  HANDLE in = ::GetStdHandle(STD_INPUT_HANDLE);

  char line[256];
  while (true)
  {
    DWORD read;
    if (::ReadConsole(in,line,sizeof line,&read,NULL) == FALSE)
      break;

    line[read] = 0;
    for (size_t i = 0; i < read; i++)
    {
      if ((line[i] == '\n') || (line[i] == '\r'))
        line[i] = 0;
    }

    {
      CString strLine(line);
      CSingleLock guard(&(debug->lock),TRUE);
      debug->cmds.Add(strLine);
      debug->notify.SetEvent();
    }
  }
  return 0;
}

CString CGlkApp::StrFromXML(IXMLDOMDocument* doc, LPCWSTR path)
{
  CComPtr<IXMLDOMNode> node;
  if (SUCCEEDED(doc->selectSingleNode(CComBSTR(path),&node)) && (node != NULL))
  {
    CComBSTR text;
    node->get_text(&text);
    return text.m_str;
  }
  return "";
}

/////////////////////////////////////////////////////////////////////////////
// Support for graphics and sound resources
/////////////////////////////////////////////////////////////////////////////

CWinGlkGraphic* CGlkApp::LoadGraphic(int iNumber, BOOL bLoad, BOOL bApplyAlpha)
{
  glui32 id = 0;

  BYTE* pData = NULL;
  UINT iLength = 0;
  bool bFreeData = false;

  // First try to load the graphic from the Blorb resource map
  if (m_pBlorbMap)
  {
    giblorb_result_t Result;
    if (giblorb_load_resource(m_pBlorbMap,giblorb_method_Memory,&Result,giblorb_ID_Pict,iNumber) == giblorb_err_None)
    {
      pData = (BYTE*)Result.data.ptr;
      iLength = Result.length;
      id = Result.chunktype;
    }
  }

  // Now try to load the graphic from a file
  if (id == 0)
  {
    CFile ResFile;

    // For each loader, try to find a matching resource
    int i = 0;
    while ((id == 0) && (i < CWinGlkGraphicLoader::GetLoaderCount()))
    {
      CWinGlkGraphicLoader* pLoader = CWinGlkGraphicLoader::GetLoader(i);
      CString strFileExt = pLoader->GetFileExtension();
      if (ResFile.Open(FileName("pic",iNumber,strFileExt),CFile::modeRead))
        id = pLoader->GetIdentifier();
      i++;
    }

    // Get the graphics data from the file
    if (id != 0)
    {
      iLength = (UINT)ResFile.GetLength();
      pData = new BYTE[iLength];
      ResFile.Read(pData,iLength);
      bFreeData = true;
    }
  }

  // Has a graphic been found?
  CWinGlkGraphic* pGraphic = NULL;
  if (id != 0)
  {
    CWinGlkGraphicLoader* pLoader = CWinGlkGraphicLoader::GetLoaderForID(id);
    if (pLoader)
      pGraphic = pLoader->LoadGraphic(pData,iLength,bLoad,bApplyAlpha);
  }

  if (bFreeData)
    delete[] pData;
  return pGraphic;
}

CWinGlkSound* CGlkApp::LoadSound(int iNumber)
{
  glui32 id = 0;

  BYTE* pData = NULL;
  UINT iLength = 0;
  CString strSndFileName;

  // First try to load the sound from the Blorb resource map
  if (m_pBlorbMap)
  {
    giblorb_result_t Result;
    if (giblorb_load_resource(m_pBlorbMap,giblorb_method_Memory,&Result,giblorb_ID_Snd,iNumber) == giblorb_err_None)
    {
      pData = (BYTE*)Result.data.ptr;
      iLength = Result.length;
      id = Result.chunktype;
    }
  }

  // Now try to load the sound from a file
  if (id == 0)
  {
    // For each loader, try to find a matching resource
    int i = 0;
    while ((id == 0) && (i < CWinGlkSoundLoader::GetLoaderCount()))
    {
      CWinGlkSoundLoader* pLoader = CWinGlkSoundLoader::GetLoader(i);
      CString strFilePrefix = pLoader->GetFilePrefix();

      int j = 0;
      while ((id == 0) && (j < pLoader->GetNumberFileExtensions()))
      {
        CString strFileExt = pLoader->GetFileExtension(j);
        CString strFileName = FileName(strFilePrefix,iNumber,strFileExt);
        if (::GetFileAttributes(strFileName) != 0xFFFFFFFF)
        {
          id = pLoader->GetIdentifier();
          strSndFileName = strFileName;
        }
        j++;
      }
      i++;
    }
  }

  // Has a sound been found?
  CWinGlkSound* pSound = NULL;
  if (id != 0)
  {
    CWinGlkSoundLoader* pLoader = CWinGlkSoundLoader::GetLoaderForID(id);
    if (pLoader)
    {
      if (pData != NULL)
        pSound = pLoader->GetSound(pData,iLength);
      else if (strSndFileName.GetLength() > 0)
        pSound = pLoader->GetSound(strSndFileName);
    }
  }
  return pSound;
}

CWinGlkResource* CGlkApp::LoadResource(int iNumber)
{
  glui32 id = 0;

  char* pData = NULL;
  glui32 iLength = 0;
  bool bFreeData = false;

  // First try to load from the Blorb resource map
  if (m_pBlorbMap)
  {
    giblorb_result_t Result;
    if (giblorb_load_resource(m_pBlorbMap,giblorb_method_Memory,&Result,giblorb_ID_Data,iNumber) == giblorb_err_None)
    {
      pData = (char*)Result.data.ptr;
      iLength = Result.length;
      id = Result.chunktype;
    }
  }

  // Now try to load from a file
  if (id == 0)
  {
    CFile ResFile;
    if (ResFile.Open(FileName("data",iNumber,"txt"),CFile::modeRead))
    {
      iLength = (UINT)ResFile.GetLength();
      pData = new char[iLength];
      ResFile.Read(pData,iLength);
      id = giblorb_ID_TEXT;
      bFreeData = true;
    }
    else if (ResFile.Open(FileName("data",iNumber,"bin"),CFile::modeRead))
    {
      iLength = (UINT)ResFile.GetLength();
      pData = new char[iLength];
      ResFile.Read(pData,iLength);
      id = giblorb_ID_BINA;
      bFreeData = true;
    }
  }

  // Has a resource been found?
  if (id != 0)
  {
    if (id == giblorb_ID_TEXT)
      return new CWinGlkResource(pData,iLength,true,bFreeData);
    else if (id == giblorb_ID_BINA)
      return new CWinGlkResource(pData,iLength,false,bFreeData);
    else if (id == giblorb_make_id('F','O','R','M'))
      return new CWinGlkResource(pData,iLength,false,bFreeData);
  }
  return 0;
}

CString CGlkApp::FileName(LPCTSTR pszPrefix,int iIndex,LPCTSTR pszSuffix)
{
  CString strFileName;
  strFileName.Format("%s%s%d.%s",m_strResDir,pszPrefix,iIndex,pszSuffix);
  return strFileName;
}

// Delete any old Glk temporary files
void CGlkApp::DeleteOldTempFiles(void)
{
  // Get the temporary directory
  char TempFiles[MAX_PATH];
  ::GetTempPath(MAX_PATH,TempFiles);
  if (::GetFileAttributes(TempFiles) == 0xFFFFFFFF)
    strcpy(TempFiles,".\\");
  if (strlen(TempFiles) > 0)
  {
    if (TempFiles[strlen(TempFiles)-1] != '\\')
      strcat(TempFiles,"\\");
  }

  // Find any remaining temporary files
  strcat(TempFiles,"glk*.tmp");
  CFileFind TempFinder;
  BOOL bFinding = TempFinder.FindFile(TempFiles);
  while (bFinding)
  {
    bFinding = TempFinder.FindNextFile();
    CTime LastWrite;
    if (TempFinder.GetLastWriteTime(LastWrite))
    {
      CTimeSpan SinceLastWrite = CTime::GetCurrentTime() - LastWrite;
      if (SinceLastWrite.GetTotalHours() > 0)
      {
        try
        {
          CFile::Remove(TempFinder.GetFilePath());
        }
        catch (CException* pEx)
        {
          pEx->Delete();
        }
      }
    }
  }
}

// Determine if a character should be spoken
bool CGlkApp::CanSpeakChar(wchar_t c)
{
  switch (c)
  {
  case L'[':
  case L']':
  case L'{':
  case L'}':
  case L'<':
  case L'>':
    return false;
  }
  return true;
}

// Speak the given text
void CGlkApp::Speak(LPCSTR pszText)
{
  if (*pszText == '\0')
    return;

  if (GetCanSpeak() && TextToSpeech::GetSpeechEngine().IsAvailable())
  {
    // Make sure that the speech engine is set up
    TextToSpeech::GetSpeechEngine().Initialize(m_strVoice,m_iSpeakRate);

    // Send the text to the speech engine
    CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetMainWnd();
    TextToSpeech::GetSpeechEngine().Speak(pszText,pMainWnd->GetCodePage());
  }
}

void CGlkApp::Speak(LPCWSTR pszText)
{
  if (*pszText == L'\0')
    return;

  if (GetCanSpeak() && TextToSpeech::GetSpeechEngine().IsAvailable())
  {
    // Make sure that the speech engine is set up
    TextToSpeech::GetSpeechEngine().Initialize(m_strVoice,m_iSpeakRate);

    // Send the text to the speech engine
    TextToSpeech::GetSpeechEngine().Speak(pszText);
  }
}

// Initialize the sound engine
void CGlkApp::InitSoundEngine(void)
{
  CreateMainWindow();

  if (CDSoundEngine::GetSoundEngine().Initialize(CWinGlkSndChannel::VolumeFader) == false)
  {
    CString noDSound;
    noDSound.LoadString(IDS_NO_DSOUND);
    ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(),
      noDSound,"Windows Glk",MB_ICONWARNING|MB_OK);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Interface into Windows Glk
/////////////////////////////////////////////////////////////////////////////

extern "C" int InitGlk(unsigned int iVersion)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  // Check the required Glk version
  unsigned int iThisVersion = glk_gestalt(gestalt_Version,0);
  if (iVersion > iThisVersion)
  {
    CString NewerMsg;
    NewerMsg.LoadString(IDS_NEWER);
    MessageBox(0,NewerMsg,"Glk",MB_ICONINFORMATION|MB_OK);
    return 0;
  }

  return 1;
}

/////////////////////////////////////////////////////////////////////////////
// Generic Glk functions
/////////////////////////////////////////////////////////////////////////////

static bool invalidate = false;
static bool exiting = false;

extern "C" void glk_exit(void)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  // Don't re-enter this function
  if (exiting)
    ::ExitProcess(0);
  exiting = true;

  gidebug_announce_cycle(gidebug_cycle_End);

  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  if (CWinGlkWnd::GetFinalOutput())
  {
    CWinGlkWnd::SetExiting();

    CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetMainWnd();
    if (pMainWnd)
    {
      if (pMainWnd->IsWindowVisible() == FALSE)
        pMainWnd->ShowWindow(pApp->GetWindowState());

      CString strExitTitle = pApp->GetAppTitle();
      strExitTitle += ' ';

      CString strHitExit;
      strHitExit.LoadString(IDS_HITEXIT);
      strExitTitle += strHitExit;

      pMainWnd->SetWindowText(strExitTitle);
      pMainWnd->Invalidate();
    }

    bool bLoop = true;
    while (bLoop)
    {
      if (AfxGetMainWnd())
      {
        MSG msg;
        if (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
        { 
          if (pApp->PumpMessage() == FALSE)
            bLoop = false;
        }
        else
        {
          LONG lIdle = 0;
          while (pApp->CWinApp::OnIdle(lIdle++));
          ::WaitMessage();
        }
      }
      else
        bLoop = false;
    }
  }

  CWinGlkFileRef::CloseAllFileRefs();
  CWinGlkStream::CloseAllStreams();
  CWinGlkWnd::CloseAllWindows();

  // Stop the sound engine's background thread before
  // deleting the sound channels so that deleting sounds
  // doesn't lead to waiting on the sound mutex.
  CDSoundEngine::GetSoundEngine().StopThread();
  CWinGlkSndChannel::CloseAllChannels();
  CWinGlkSoundLoader::AllSoundStopped();
  TextToSpeech::GetSpeechEngine().Destroy();
  CDSoundEngine::GetSoundEngine().Destroy();

  if (pApp->GetBlorbMap())
  {
    giblorb_destroy_map(pApp->GetBlorbMap());
    pApp->GetBlorbMap() = NULL;
  }

  CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetMainWnd();
  if (pMainWnd)
    pMainWnd->DestroyWindow();

  ::ExitProcess(0);
}

extern "C" void glk_set_interrupt_handler(void (*func)(void))
{
  InterruptFn = func;
}

extern "C" void glk_tick(void)
{
  // Run the message pump if the last tick was over a second ago
  if (TickCountDiff(::GetTickCount(),LastTick) > 1000)
  {
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (AfxGetMainWnd())
      ((CGlkApp*)AfxGetApp())->MessagePump(FALSE);
  }
}

extern "C" glui32 glk_gestalt(glui32 sel, glui32 val)
{
  return glk_gestalt_ext(sel,val,NULL,0);
}

extern "C" glui32 glk_gestalt_ext(glui32 sel, glui32 val, glui32 *arr, glui32 arrlen)
{
  switch (sel)
  {
  case gestalt_Version:
    return 0x00000705; // Glk 0.7.5

  case gestalt_LineInput:
    if ((val >= 32 && val <= 126) || (val >= 160 && val <= 0xFFFF))
      return 1;
    return 0;

  case gestalt_CharInput:
    switch (val)
    {
    case keycode_Return:
    case keycode_Left:
    case keycode_Right:
    case keycode_Up:
    case keycode_Down:
    case keycode_Delete:
    case keycode_Escape:
    case keycode_PageUp:
    case keycode_PageDown:
    case keycode_Home:
    case keycode_End:
    case keycode_Func1:
    case keycode_Func2:
    case keycode_Func3:
    case keycode_Func4:
    case keycode_Func5:
    case keycode_Func6:
    case keycode_Func7:
    case keycode_Func8:
    case keycode_Func9:
    case keycode_Func10:
    case keycode_Func11:
    case keycode_Func12:
      return 1;
    default:
      if ((val >= 32 && val <= 126) || (val >= 160 && val <= 0xFFFF))
        return 1;
      break;
    }
    return 0;

  case gestalt_CharOutput:
    if (val == L'\n')
    {
      if (arr && (arrlen > 0))
        arr[0] = 0;
      return gestalt_CharOutput_CannotPrint;
    }
    else if ((val >= 32 && val <= 126) || (val >= 160 && val <= 0x10FFFF))
    {
      if (arr && (arrlen > 0))
        arr[0] = 1;
      return ((CGlkApp*)AfxGetApp())->CanOutputChar(val) ?
        gestalt_CharOutput_ExactPrint : gestalt_CharOutput_CannotPrint;
    }
    else
    {
      // Invalid characters are printed as e.g. [0x7F]
      if (arr && (arrlen > 0))
        arr[0] = (val <= 0xFF) ? 6 : 12;
      return gestalt_CharOutput_CannotPrint;
    }

  case gestalt_MouseInput:
    if ((val == wintype_Graphics) || (val == wintype_TextGrid))
      return 1;
    return 0;

  case gestalt_Timer:
    return 1;

  case gestalt_Graphics:
    return 1;
  case gestalt_GraphicsTransparency:
    return 1;

  case gestalt_DrawImage:
    if ((val == wintype_Graphics) || (val == wintype_TextBuffer))
      return 1;
    return 0;

  case gestalt_Sound:
    return 1;
  case gestalt_Sound2:
    return 1;
  case gestalt_SoundVolume:
    return 1;
  case gestalt_SoundNotify:
    return 1;
  case gestalt_SoundMusic:
    return 1;

  case gestalt_Hyperlinks:
    return 1;
  case gestalt_HyperlinkInput:
    if ((val == wintype_TextBuffer) || (val == wintype_TextGrid))
      return 1;
    return 0;

  case gestalt_Unicode:
    return 1;
  case gestalt_UnicodeNorm:
    return 1;

  case gestalt_LineInputEcho:
    return 1;

  case gestalt_LineTerminators:
    return 1;
  case gestalt_LineTerminatorKey:
    switch (val)
    {
    case keycode_Escape:
    case keycode_PageUp:
    case keycode_PageDown:
    case keycode_Func1:
    case keycode_Func2:
    case keycode_Func3:
    case keycode_Func4:
    case keycode_Func5:
    case keycode_Func6:
    case keycode_Func7:
    case keycode_Func8:
    case keycode_Func9:
    case keycode_Func10:
    case keycode_Func11:
    case keycode_Func12:
      return 1;
    default:
      return 0;
    }
    break;

  case gestalt_DateTime:
    return 1;

  case gestalt_ResourceStream:
    return 1;

  case gestalt_GraphicsCharInput:
    return 1;

  case gestalt_GarglkText:
    return 1;
  }
  return 0;
}

extern "C" unsigned char glk_char_to_lower(unsigned char ch)
{
  static const char* pszLoTable1 =
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@abcdefghijklmnopqrstuvwxyz[\\]^_"
    "`abcdefghijklmnopqrstuvwxyz{|}~ ";

  static const char* pszLoTable2 =
    " ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿"
    "àáâãäåæçèéêëìíîïðñòóôõö×øùúûüýþß"
    "àáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ";

  unsigned char new_ch = ch;

  if (ch >= 32 && ch <= 126)
    new_ch = pszLoTable1[ch-32];
  if (ch >= 160 && ch <= 255)
    new_ch = pszLoTable2[ch-160];

  return new_ch;
}

extern "C" unsigned char glk_char_to_upper(unsigned char ch)
{
  static const char* pszHiTable1 =
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    "`ABCDEFGHIJKLMNOPQRSTUVWXYZ{|}~ ";

  static const char* pszHiTable2 =
    " ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿"
    "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß"
    "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ÷ØÙÚÛÜÝÞÿ";

  unsigned char new_ch = ch;

  if (ch >= 32 && ch <= 126)
    new_ch = pszHiTable1[ch-32];
  if (ch >= 160 && ch <= 255)
    new_ch = pszHiTable2[ch-160];

  return new_ch;
}

extern "C" winid_t glk_window_get_root(void)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  winid_t root = 0;

  if (CWinGlkWnd::GetMainWindow())
    root = (winid_t)CWinGlkWnd::GetMainWindow();

  return root;
}

extern "C" winid_t glk_window_open(winid_t split, glui32 method, glui32 size, glui32 wintype, glui32 rock)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  CWinGlkWnd* pSplitWnd = NULL;

  // Read user settings now, if this has not already been done
  pApp->ReadSettings();

  // Does a main window already exist?
  if (CWinGlkWnd::GetMainWindow())
  {
    if (split == 0)
      return 0;
    pSplitWnd = (CWinGlkWnd*)split;
    if (CWinGlkWnd::IsValidWindow(pSplitWnd) == FALSE)
      return 0;
  }
  else
  {
    if (split != 0)
      return 0;

    // Create the main window
    if (pApp->CreateMainWindow() == false)
      return 0;
  }

  CWinGlkWnd* pNewWnd = NULL;
  switch (wintype)
  {
  case wintype_Blank:
    pNewWnd = new CWinGlkWnd(rock);
    break;
  case wintype_TextBuffer:
    pNewWnd = new CWinGlkWndTextBuffer(rock);
    break;
  case wintype_TextGrid:
    pNewWnd = new CWinGlkWndTextGrid(rock);
    break;
  case wintype_Graphics:
    pNewWnd = new CWinGlkWndGraphics(rock);
    break;
  }
  if (pNewWnd)
    pNewWnd = CWinGlkWnd::OpenWindow(pNewWnd,pSplitWnd,method,size);

  invalidate = true;
  return (winid_t)pNewWnd;
}

extern "C" void glk_window_close(winid_t win, stream_result_t *result)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    ((CWinGlkWnd*)win)->CloseWindow(result);
    invalidate = true;
  }
}

extern "C" void glk_window_get_size(winid_t win, glui32 *widthptr, glui32 *heightptr)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  int iWidth = 0;
  int iHeight = 0;

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
    ((CWinGlkWnd*)win)->GetSize(iWidth,iHeight);

  if (widthptr)
    *widthptr = (glui32)iWidth;
  if (heightptr)
    *heightptr = (glui32)iHeight;
}

extern "C" void glk_window_set_arrangement(winid_t win, glui32 method, glui32 size, winid_t keywin)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkWnd* pWnd = (CWinGlkWnd*)win;
  CWinGlkWnd* pKey = (CWinGlkWnd*)keywin;

  if (CWinGlkWnd::IsValidWindow(pWnd) == FALSE)
    return;
  if ((pKey != NULL) && (CWinGlkWnd::IsValidWindow(pKey) == FALSE))
    return;

  if (pWnd->IsKindOf(RUNTIME_CLASS(CWinGlkWndPair)))
  {
    ((CWinGlkWndPair*)pWnd)->SetArrangement(method,size,pKey);
    invalidate = true;
  }
}

extern "C" void glk_window_get_arrangement(winid_t win, glui32 *methodptr, glui32 *sizeptr, winid_t *keywinptr)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkWnd* pWnd = (CWinGlkWnd*)win;

  if (CWinGlkWnd::IsValidWindow(pWnd) == FALSE)
    return;

  if (pWnd->IsKindOf(RUNTIME_CLASS(CWinGlkWndPair)))
    ((CWinGlkWndPair*)pWnd)->GetArrangement(methodptr,sizeptr,(CWinGlkWnd**)keywinptr);
}

extern "C" winid_t glk_window_iterate(winid_t win, glui32 *rockptr)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  winid_t next_win = 0;

  if ((win == 0) || (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win)))
    next_win = (winid_t)CWinGlkWnd::IterateWindows((CWinGlkWnd*)win,rockptr);

  return next_win;
}

extern "C" glui32 glk_window_get_rock(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  glui32 rock = 0;

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
    rock = ((CWinGlkWnd*)win)->GetRock();
  
  return rock;
}

extern "C" glui32 glk_window_get_type(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  glui32 type = 0;
  CWinGlkWnd* pWnd = (CWinGlkWnd*)win;

  if (CWinGlkWnd::IsValidWindow(pWnd))
  {
    if (pWnd->IsKindOf(RUNTIME_CLASS(CWinGlkWndPair)))
      type = wintype_Pair;
    else if (pWnd->IsKindOf(RUNTIME_CLASS(CWinGlkWndTextBuffer)))
      type = wintype_TextBuffer;
    else if (pWnd->IsKindOf(RUNTIME_CLASS(CWinGlkWndTextGrid)))
      type = wintype_TextGrid;
    else if (pWnd->IsKindOf(RUNTIME_CLASS(CWinGlkWndGraphics)))
      type = wintype_Graphics;
    else
      type = wintype_Blank;
  }
  return type;
}

extern "C" winid_t glk_window_get_parent(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkWnd* pWnd = (CWinGlkWnd*)win;

  if (CWinGlkWnd::IsValidWindow(pWnd))
    return (winid_t)pWnd->GetParentWnd();
  return (winid_t)0;
}

extern "C" winid_t glk_window_get_sibling(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkWnd* pWnd = (CWinGlkWnd*)win;

  if (CWinGlkWnd::IsValidWindow(pWnd))
    return (winid_t)pWnd->GetSiblingWnd();
  return (winid_t)0;
}

extern "C" void glk_window_clear(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    ((CWinGlkWnd*)win)->ClearWindow();
    invalidate = true;
  }
}

extern "C" void glk_window_move_cursor(winid_t win, glui32 xpos, glui32 ypos)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    ((CWinGlkWnd*)win)->MoveCursor((int)xpos,(int)ypos);
    invalidate = true;
  }
}

extern "C" strid_t glk_window_get_stream(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  strid_t str = 0;

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
    str = (strid_t)CWinGlkStreamWnd::FindWindowStream((CWinGlkWnd*)win);
  return str;
}

extern "C" void glk_window_set_echo_stream(winid_t win, strid_t str)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
      ((CWinGlkWnd*)win)->SetEchoStream((CWinGlkStream*)str);
  }
}

extern "C" strid_t glk_window_get_echo_stream(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  strid_t str = 0;

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
    str = (strid_t)((CWinGlkWnd*)win)->GetEchoStream();
  return str;
}

extern "C" void glk_set_window(winid_t win)
{
  glk_stream_set_current(glk_window_get_stream(win));
}

extern "C" strid_t glk_stream_open_file(frefid_t fileref, glui32 fmode, glui32 rock)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkFileRef* pFileRef = (CWinGlkFileRef*)fileref;
  if (CWinGlkFileRef::IsValidFileRef(pFileRef) == false)
    return (strid_t)0;

  CWinGlkStreamFile* pStream = new CWinGlkStreamFile(rock);
  if (pStream->OpenFile(pFileRef,fmode) == false)
  {
    delete pStream;
    pStream = NULL;
  }
  return (strid_t)pStream;
}

extern "C" strid_t glk_stream_open_memory(char *buf, glui32 buflen, glui32 fmode, glui32 rock)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  switch (fmode)
  {
  case filemode_Read:
  case filemode_Write:
  case filemode_ReadWrite:
    break;
  default:
    return 0;
  }

  return (strid_t)new CWinGlkStreamMem(buf,buflen,rock);
}

extern "C" void glk_stream_close(strid_t str, stream_result_t *result)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkStream* pStream = (CWinGlkStream*)str;

  if (CWinGlkStream::IsValidStream(pStream))
  {
    // Don't allow the closing of window streams
    if (pStream->IsKindOf(RUNTIME_CLASS(CWinGlkStreamWnd)) == FALSE)
    {
      if (result)
      {
        result->readcount = (glui32)pStream->GetReadCount();
        result->writecount = (glui32)pStream->GetWriteCount();
      }
      delete pStream;
    }
  }
}

extern "C" strid_t glk_stream_iterate(strid_t str, glui32 *rockptr)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  strid_t next_str = 0;

  if ((str == 0) || (CWinGlkStream::IsValidStream((CWinGlkStream*)str)))
    next_str = (strid_t)CWinGlkStream::IterateStreams((CWinGlkStream*)str,rockptr);

  return next_str;
}

extern "C" glui32 glk_stream_get_rock(strid_t str)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  glui32 rock = 0;

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
    rock = ((CWinGlkStream*)str)->GetRock();
  
  return rock;
}

extern "C" void glk_stream_set_position(strid_t str, glsi32 pos, glui32 seekmode)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
    ((CWinGlkStream*)str)->SetPosition(pos,seekmode);
}

extern "C" glui32 glk_stream_get_position(strid_t str)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
    return ((CWinGlkStream*)str)->GetPosition();
  else
    return (glui32)0;
}

extern "C" void glk_stream_set_current(strid_t str)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (str == 0)
    CWinGlkStream::SetCurrentStream(NULL);
  else if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
    CWinGlkStream::SetCurrentStream((CWinGlkStream*)str);
}

extern "C" strid_t glk_stream_get_current(void)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  return (strid_t)CWinGlkStream::GetCurrentStream();
}

extern "C" void glk_put_char(unsigned char ch)
{
  glk_put_char_stream(glk_stream_get_current(),ch);
}

extern "C" void glk_put_char_stream(strid_t str, unsigned char ch)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
  {
    ((CWinGlkStream*)str)->PutCharacter(ch);
    invalidate = true;
  }
}

extern "C" void glk_put_string(char *s)
{
  glk_put_string_stream(glk_stream_get_current(),s);
}

extern "C" void glk_put_string_stream(strid_t str, char *s)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
  {
    for (; *s; s++)
      ((CWinGlkStream*)str)->PutCharacter((unsigned char)*s);
    invalidate = true;
  }
}

extern "C" void glk_put_buffer(char *buf, glui32 len)
{
  glk_put_buffer_stream(glk_stream_get_current(),buf,len);
}

extern "C" void glk_put_buffer_stream(strid_t str, char *buf, glui32 len)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
  {
    for (size_t i = 0; i < len; i++)
      ((CWinGlkStream*)str)->PutCharacter((unsigned char)buf[i]);
    invalidate = true;
  }
}

extern "C" void glk_set_style(glui32 styl)
{
  glk_set_style_stream(glk_stream_get_current(),styl);
}

extern "C" void glk_set_style_stream(strid_t str, glui32 styl)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
  {
    if ((styl >= 0) && (styl < style_NUMSTYLES))
      ((CWinGlkStream*)str)->SetStyle(styl);
  }
}

extern "C" glsi32 glk_get_char_stream(strid_t str)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
  {
    glsi32 c = ((CWinGlkStream*)str)->GetCharacter();
    if ((c == -1) || (c >= 0 && c <= 255))
      return c;
    return '?';
  }
  else
    return (glsi32)-1;
}

extern "C" glui32 glk_get_line_stream(strid_t str, char *buf, glui32 len)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
    return ((CWinGlkStream*)str)->GetLine(buf,len);
  else
    return (glui32)-1;
}

extern "C" glui32 glk_get_buffer_stream(strid_t str, char *buf, glui32 len)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
    return ((CWinGlkStream*)str)->GetBuffer(buf,len);
  else
    return (glui32)-1;
}

extern "C" void glk_stylehint_set(glui32 wintype, glui32 styl, glui32 hint, glsi32 val)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CGlkApp* pApp = (CGlkApp*)AfxGetApp();

  // Read user settings now, if this has not already been done
  pApp->ReadSettings();

  if (pApp->GetStyleHints())
  {
    switch (wintype)
    {
    case wintype_TextBuffer:
      CWinGlkWndTextBuffer::SetStyleHint(styl,hint,val);
      break;
    case wintype_TextGrid:
      CWinGlkWndTextGrid::SetStyleHint(styl,hint,val);
      break;
    case wintype_AllTypes:
      CWinGlkWndTextBuffer::SetStyleHint(styl,hint,val);
      CWinGlkWndTextGrid::SetStyleHint(styl,hint,val);
      break;
    }
  }
}

extern "C" void glk_stylehint_clear(glui32 wintype, glui32 styl, glui32 hint)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CGlkApp* pApp = (CGlkApp*)AfxGetApp();

  // Read user settings now, if this has not already been done
  pApp->ReadSettings();

  if (pApp->GetStyleHints())
  {
    switch (wintype)
    {
    case wintype_TextBuffer:
      CWinGlkWndTextBuffer::ClearStyleHint(styl,hint);
      break;
    case wintype_TextGrid:
      CWinGlkWndTextGrid::ClearStyleHint(styl,hint);
      break;
    case wintype_AllTypes:
      CWinGlkWndTextBuffer::ClearStyleHint(styl,hint);
      CWinGlkWndTextGrid::ClearStyleHint(styl,hint);
      break;
    }
  }
}

extern "C" glui32 glk_style_distinguish(winid_t win, glui32 styl1, glui32 styl2)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  // Read user settings now, if this has not already been done
  ((CGlkApp*)AfxGetApp())->ReadSettings();

  glui32 Distinguish = 0;

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    if (((CWinGlkWnd*)win)->DistinguishStyles(styl1,styl2))
      Distinguish = 1;
  }
  return Distinguish;
}

extern "C" glui32 glk_style_measure(winid_t win, glui32 styl, glui32 hint, glui32 *result)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  // Read user settings now, if this has not already been done
  ((CGlkApp*)AfxGetApp())->ReadSettings();

  glui32 Measure = 0;

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    if (((CWinGlkWnd*)win)->MeasureStyle(styl,hint,result))
      Measure = 1;
  }
  return Measure;
}

extern "C" frefid_t glk_fileref_create_temp(glui32 usage, glui32 rock)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkFileRef* pFileRef = new CWinGlkFileRef(usage,rock,true);
  return (frefid_t)pFileRef;
}

extern "C" frefid_t glk_fileref_create_by_name(glui32 usage, char *name, glui32 rock)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkFileRef* pFileRef = new CWinGlkFileRef(usage,rock);
  pFileRef->SetFileName(name,usage,false,true);

  return (frefid_t)pFileRef;
}

extern "C" frefid_t glk_fileref_create_by_prompt(glui32 usage, glui32 fmode, glui32 rock)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  return (frefid_t)CWinGlkFileRef::PromptForName(usage,fmode,rock);
}

extern "C" frefid_t glk_fileref_create_from_fileref(glui32 usage, frefid_t fref, glui32 rock)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkFileRef* pCopyRef = (CWinGlkFileRef*)fref;
  if (CWinGlkFileRef::IsValidFileRef(pCopyRef))
  {
    CWinGlkFileRef* pFileRef = new CWinGlkFileRef(pCopyRef,usage,rock);
    return (frefid_t)pFileRef;
  }
  return NULL;
}

extern "C" void glk_fileref_destroy(frefid_t fref)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkFileRef* pFileRef = (CWinGlkFileRef*)fref;
  if (CWinGlkFileRef::IsValidFileRef(pFileRef))
    delete pFileRef;
}

extern "C" frefid_t glk_fileref_iterate(frefid_t fref, glui32 *rockptr)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  frefid_t next_fref = 0;

  if ((fref == 0) || (CWinGlkFileRef::IsValidFileRef((CWinGlkFileRef*)fref)))
    next_fref = (frefid_t)CWinGlkFileRef::IterateFileRefs((CWinGlkFileRef*)fref,rockptr);

  return next_fref;
}

extern "C" glui32 glk_fileref_get_rock(frefid_t fref)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  glui32 rock = 0;

  if (CWinGlkFileRef::IsValidFileRef((CWinGlkFileRef*)fref))
    rock = ((CWinGlkFileRef*)fref)->GetRock();
  
  return rock;
}

extern "C" void glk_fileref_delete_file(frefid_t fref)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkFileRef::IsValidFileRef((CWinGlkFileRef*)fref))
    ((CWinGlkFileRef*)fref)->DeleteFile();
}

extern "C" glui32 glk_fileref_does_file_exist(frefid_t fref)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  glui32 exists = 0;

  if (CWinGlkFileRef::IsValidFileRef((CWinGlkFileRef*)fref))
    exists = ((CWinGlkFileRef*)fref)->FileExists() ? 1 : 0;

  return exists;
}

extern "C" void glk_select(event_t *event)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  gidebug_announce_cycle(gidebug_cycle_InputWait);

  if (invalidate)
  {
    // Force all windows to redraw
    ((CWinGlkMainWnd*)AfxGetMainWnd())->Invalidate();
    invalidate = false;
  }

  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  while (pApp->EventQueuesEmpty())
  {
    pApp->MessagePump(TRUE);

    if (gidebug_debugging_is_available() != 0)
    {
      while (char* line = pApp->DebugInput(false))
        gidebug_perform_command(line);
    }
  }
  pApp->GetNextEvent(event,false);

  gidebug_announce_cycle(gidebug_cycle_InputAccept);
}

extern "C" void glk_select_poll(event_t *event)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  // Force all windows to redraw
  ((CWinGlkMainWnd*)AfxGetMainWnd())->Invalidate();

  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  pApp->MessagePump(FALSE);

  memset(event,0,sizeof(event_t));
  event->type = evtype_None;
  pApp->GetNextEvent(event,true);
}

extern "C" void glk_request_timer_events(glui32 millisecs)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetMainWnd();
  pMainWnd->StartTimer(millisecs);
  invalidate = true;
}

extern "C" void glk_request_line_event(winid_t win, char *buf, glui32 maxlen, glui32 initlen)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    ((CWinGlkWnd*)win)->StartLineEvent(buf,false,maxlen,initlen);
    invalidate = true;

#ifdef WINGLK_SPEED_TEST
    static CStdioFile InputFile;
    static DWORD StartTime;
    if (InputFile.m_pStream == NULL)
    {
      InputFile.Open("TestInput.txt",CFile::modeRead|CFile::typeText);
      StartTime = ::GetTickCount();
    }
    if (InputFile.m_pStream != NULL)
    {
      CString InputLine;
      if (InputFile.ReadString(InputLine))
      {
        InputLine.Trim();
        int InputLen = min(InputLine.GetLength(),(int)maxlen);
        memcpy(buf,InputLine,InputLen);
        theApp.AddEvent(evtype_LineInput,win,InputLen,0);
        ((CWinGlkWnd*)win)->TestLineInput(InputLen);
        ((CWinGlkWnd*)win)->EndLineEvent(NULL);
        theApp.MessagePump(FALSE);
      }
      else
      {
        CString TestMsg;
        TestMsg.Format("Time since first input is %.1lfs",0.001*TickCountDiff(::GetTickCount(),StartTime));
        AfxMessageBox(TestMsg);
        glk_exit();
      }
    }
#endif
  }
}

extern "C" void glk_request_char_event(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    ((CWinGlkWnd*)win)->StartCharEvent(false);
    invalidate = true;

#ifdef WINGLK_SPEED_TEST
    theApp.AddEvent(evtype_CharInput,win,' ',0);
    ((CWinGlkWnd*)win)->EndCharEvent();
    theApp.MessagePump(FALSE);
#endif
  }
}

extern "C" void glk_request_mouse_event(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    ((CWinGlkWnd*)win)->StartMouseEvent();
    invalidate = true;
  }
}

extern "C" void glk_cancel_line_event(winid_t win, event_t *event)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    ((CWinGlkWnd*)win)->EndLineEvent(event);
    invalidate = true;
  }
}

extern "C" void glk_cancel_char_event(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    ((CWinGlkWnd*)win)->EndCharEvent();
    invalidate = true;
  }
}

extern "C" void glk_cancel_mouse_event(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    ((CWinGlkWnd*)win)->EndMouseEvent();
    invalidate = true;
  }
}

extern "C" glui32 glk_image_draw(winid_t win, glui32 image, glsi32 val1, glsi32 val2)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  int iDraw = 0;

  CWinGlkWnd* pWnd = (CWinGlkWnd*)win;
  if (CWinGlkWnd::IsValidWindow(pWnd))
  {
    CWinGlkGraphic* pGraphic = pApp->LoadGraphic(image,TRUE,TRUE);
    if (pGraphic)
    {
      bool bDelete = true;
      if (pWnd->DrawGraphic(pGraphic,val1,val2,-1,-1,bDelete))
      {
        iDraw = 1;
        invalidate = true;
      }
      if (bDelete)
        delete pGraphic;
    }
  }
  return iDraw;
}

extern "C" glui32 glk_image_draw_scaled(winid_t win, glui32 image, glsi32 val1, glsi32 val2, glui32 width, glui32 height)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  int iDraw = 0;

  CWinGlkWnd* pWnd = (CWinGlkWnd*)win;
  if (CWinGlkWnd::IsValidWindow(pWnd))
  {
    CWinGlkGraphic* pGraphic = pApp->LoadGraphic(image,TRUE,TRUE);
    if (pGraphic)
    {
      bool bDelete = true;
      if (pWnd->DrawGraphic(pGraphic,val1,val2,width,height,bDelete))
      {
        iDraw = 1;
        invalidate = true;
      }
      if (bDelete)
        delete pGraphic;
    }
  }
  return iDraw;
}

extern "C" glui32 glk_image_get_info(glui32 image, glui32 *width, glui32 *height)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  CWinGlkGraphic* pGraphic = pApp->LoadGraphic(image,FALSE,FALSE);
  if (pGraphic)
  {
    if (width)
      *width = pGraphic->m_dwWidth;
    if (height)
      *height = pGraphic->m_dwHeight;
    delete pGraphic;
    return 1;
  }
  return 0;
}

extern "C" void glk_window_flow_break(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkWnd* pWnd = (CWinGlkWnd*)win;

  if (CWinGlkWnd::IsValidWindow(pWnd))
  {
    if (pWnd->IsKindOf(RUNTIME_CLASS(CWinGlkWndTextBuffer)))
    {
      ((CWinGlkWndTextBuffer*)pWnd)->InsertFlowBreak();
      invalidate = true;
    }
  }
}

extern "C" void glk_window_erase_rect(winid_t win, glsi32 left, glsi32 top, glui32 width, glui32 height)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkWndGraphics* pWnd = (CWinGlkWndGraphics*)win;
  if (CWinGlkWnd::IsValidWindow(pWnd))
  {
    if (pWnd->IsKindOf(RUNTIME_CLASS(CWinGlkWndGraphics)))
    {
      CRect Rectange(left,top,left+width-1,top+height-1);
      pWnd->FillRectBack(Rectange);
      invalidate = true;
    }
  }
}

extern "C" void glk_window_fill_rect(winid_t win, glui32 color, glsi32 left, glsi32 top, glui32 width, glui32 height)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkWndGraphics* pWnd = (CWinGlkWndGraphics*)win;
  if (CWinGlkWnd::IsValidWindow(pWnd))
  {
    if (pWnd->IsKindOf(RUNTIME_CLASS(CWinGlkWndGraphics)))
    {
      CRect Rectange(left,top,left+width,top+height);
      pWnd->FillRect(Rectange,CWinGlkWnd::GetColour(color));
      invalidate = true;
    }
  }
}

extern "C" void glk_window_set_background_color(winid_t win, glui32 color)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkWndGraphics* pWnd = (CWinGlkWndGraphics*)win;
  if (CWinGlkWnd::IsValidWindow(pWnd))
  {
    if (pWnd->IsKindOf(RUNTIME_CLASS(CWinGlkWndGraphics)))
    {
      pWnd->SetBackColour(CWinGlkWnd::GetColour(color));
      invalidate = true;
    }
  }
}

extern "C" schanid_t glk_schannel_create(glui32 rock)
{
  return glk_schannel_create_ext(rock,0x10000);
}

extern "C" schanid_t glk_schannel_create_ext(glui32 rock, glui32 volume)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  // If the main window has not yet opened, run a message pump, otherwise DirectSound
  // will not believe that the application has the focus, and so will not set the
  // sound channel's buffer volume correctly.
  CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetMainWnd();
  if (pMainWnd)
  {
    if (pMainWnd->IsWindowVisible() == FALSE)
      ((CGlkApp*)AfxGetApp())->MessagePump(FALSE);
  }

  CWinGlkSndChannel* pChannel = new CWinGlkSndChannel(rock);
  pChannel->SetVolume(volume,0,0);
  return (schanid_t)pChannel;
}

extern "C" void glk_schannel_destroy(schanid_t chan)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkSndChannel* pChannel = (CWinGlkSndChannel*)chan;

  if (CWinGlkSndChannel::IsValidChannel(pChannel))
    delete pChannel;
}

extern "C" schanid_t glk_schannel_iterate(schanid_t chan, glui32 *rockptr)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  schanid_t next_chan = 0;

  if ((chan == 0) || (CWinGlkSndChannel::IsValidChannel((CWinGlkSndChannel*)chan)))
    next_chan = (schanid_t)CWinGlkSndChannel::IterateChannels((CWinGlkSndChannel*)chan,rockptr);

  return next_chan;
}

extern "C" glui32 glk_schannel_get_rock(schanid_t chan)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  glui32 rock = 0;

  if (CWinGlkSndChannel::IsValidChannel((CWinGlkSndChannel*)chan))
    rock = ((CWinGlkSndChannel*)chan)->GetRock();
  
  return rock;
}

extern "C" glui32 glk_schannel_play(schanid_t chan, glui32 snd)
{
  return glk_schannel_play_ext(chan,snd,1,0);
}

extern "C" glui32 glk_schannel_play_ext(schanid_t chan, glui32 snd, glui32 repeats, glui32 notify)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkSndChannel* pChannel = (CWinGlkSndChannel*)chan;
  if (CWinGlkSndChannel::IsValidChannel(pChannel))
  {
    if (repeats == 0)
    {
      pChannel->Stop();
      return 1;
    }
    else
    {
      CWinGlkSound* pSound = ((CGlkApp*)AfxGetApp())->LoadSound(snd);
      if (pSound)
      {
        pChannel->Prepare(pSound,snd,notify);
        if (pChannel->Play(repeats))
          return 1;
      }
      else
      {
        pChannel->Stop();
        return 0;
      }
    }
  }
  return 0;
}

extern "C" glui32 glk_schannel_play_multi(schanid_t *chanarray, glui32 chancount, glui32 *sndarray, glui32 soundcount, glui32 notify)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (chancount != soundcount)
    return 0;

  for (glui32 i = 0; i < chancount; i++)
  {
    CWinGlkSndChannel* pChannel = (CWinGlkSndChannel*)chanarray[i];
    if (CWinGlkSndChannel::IsValidChannel(pChannel))
    {
      CWinGlkSound* pSound = ((CGlkApp*)AfxGetApp())->LoadSound(sndarray[i]);
      if (pSound)
        pChannel->Prepare(pSound,sndarray[i],notify);
      else
        pChannel->Stop();
    }
  }

  glui32 playing = 0;
  for (glui32 i = 0; i < chancount; i++)
  {
    CWinGlkSndChannel* pChannel = (CWinGlkSndChannel*)chanarray[i];
    if (CWinGlkSndChannel::IsValidChannel(pChannel))
    {
      if (pChannel->Play(1))
        playing++;
    }
  }
  return playing;
}

extern "C" void glk_schannel_stop(schanid_t chan)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkSndChannel* pChannel = (CWinGlkSndChannel*)chan;
  if (CWinGlkSndChannel::IsValidChannel(pChannel))
    pChannel->Stop();
}

extern "C" void glk_schannel_pause(schanid_t chan)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkSndChannel* pChannel = (CWinGlkSndChannel*)chan;
  if (CWinGlkSndChannel::IsValidChannel(pChannel))
    pChannel->Pause(true);
}

extern "C" void glk_schannel_unpause(schanid_t chan)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkSndChannel* pChannel = (CWinGlkSndChannel*)chan;
  if (CWinGlkSndChannel::IsValidChannel(pChannel))
    pChannel->Pause(false);
}

extern "C" void glk_schannel_set_volume(schanid_t chan, glui32 vol)
{
  glk_schannel_set_volume_ext(chan,vol,0,0);
}

extern "C" void glk_schannel_set_volume_ext(schanid_t chan, glui32 vol, glui32 duration, glui32 notify)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkSndChannel* pChannel = (CWinGlkSndChannel*)chan;
  if (CWinGlkSndChannel::IsValidChannel(pChannel))
    pChannel->SetVolume(vol,duration,notify);
}

extern "C" void glk_sound_load_hint(glui32 snd, glui32 flag)
{
}

extern "C" void glk_set_hyperlink(glui32 linkval)
{
  glk_set_hyperlink_stream(glk_stream_get_current(),linkval);
}

extern "C" void glk_set_hyperlink_stream(strid_t str, glui32 linkval)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
  {
    ((CWinGlkStream*)str)->SetHyperlink(linkval);
    invalidate = true;
  }
}

extern "C" void glk_request_hyperlink_event(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    ((CWinGlkWnd*)win)->StartLinkEvent();
    invalidate = true;
  }
}

extern "C" void glk_cancel_hyperlink_event(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    ((CWinGlkWnd*)win)->EndLinkEvent();
    invalidate = true;
  }
}

extern "C" glui32 glk_buffer_to_lower_case_uni(glui32 *buf, glui32 len, glui32 numchars)
{
  return buffer_change_case(buf,len,numchars,CASE_LOWER,COND_ALL,TRUE);
}

extern "C" glui32 glk_buffer_to_upper_case_uni(glui32 *buf, glui32 len, glui32 numchars)
{
  return buffer_change_case(buf,len,numchars,CASE_UPPER,COND_ALL,TRUE);
}

extern "C" glui32 glk_buffer_to_title_case_uni(glui32 *buf, glui32 len, glui32 numchars, glui32 lowerrest)
{
  return buffer_change_case(buf,len,numchars,CASE_TITLE,COND_LINESTART,lowerrest);
}

extern "C" void glk_put_char_uni(glui32 ch)
{
  glk_put_char_stream_uni(glk_stream_get_current(),ch);
}

extern "C" void glk_put_string_uni(glui32 *s)
{
  glk_put_string_stream_uni(glk_stream_get_current(),s);
}

extern "C" void glk_put_buffer_uni(glui32 *buf, glui32 len)
{
  glk_put_buffer_stream_uni(glk_stream_get_current(),buf,len);
}

extern "C" void glk_put_char_stream_uni(strid_t str, glui32 ch)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
  {
    ((CWinGlkStream*)str)->PutCharacter(ch);
    invalidate = true;
  }
}

extern "C" void glk_put_string_stream_uni(strid_t str, glui32 *s)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
  {
    for (; *s; s++)
      ((CWinGlkStream*)str)->PutCharacter(*s);
    invalidate = true;
  }
}

extern "C" void glk_put_buffer_stream_uni(strid_t str, glui32 *buf, glui32 len)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
  {
    for (size_t i = 0; i < len; i++)
      ((CWinGlkStream*)str)->PutCharacter(buf[i]);
    invalidate = true;
  }
}

extern "C" glsi32 glk_get_char_stream_uni(strid_t str)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
    return ((CWinGlkStream*)str)->GetCharacter();
  else
    return (glsi32)-1;
}

extern "C" glui32 glk_get_buffer_stream_uni(strid_t str, glui32 *buf, glui32 len)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
    return ((CWinGlkStream*)str)->GetBuffer(buf,len);
  else
    return (glui32)-1;
}

extern "C" glui32 glk_get_line_stream_uni(strid_t str, glui32 *buf, glui32 len)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
    return ((CWinGlkStream*)str)->GetLine(buf,len);
  else
    return (glui32)-1;
}

extern "C" strid_t glk_stream_open_file_uni(frefid_t fileref, glui32 fmode, glui32 rock)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkFileRef* pFileRef = (CWinGlkFileRef*)fileref;
  if (CWinGlkFileRef::IsValidFileRef(pFileRef) == false)
    return (strid_t)0;

  CWinGlkStreamFile* pStream = new CWinGlkStreamFileUni(rock);
  if (pStream->OpenFile(pFileRef,fmode) == false)
  {
    delete pStream;
    pStream = NULL;
  }
  return (strid_t)pStream;
}

extern "C" strid_t glk_stream_open_memory_uni(glui32 *buf, glui32 buflen, glui32 fmode, glui32 rock)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  switch (fmode)
  {
  case filemode_Read:
  case filemode_Write:
  case filemode_ReadWrite:
    break;
  default:
    return 0;
  }

  return (strid_t)new CWinGlkStreamMemUni(buf,buflen,rock);
}

extern "C" void glk_request_char_event_uni(winid_t win)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    ((CWinGlkWnd*)win)->StartCharEvent(true);
    invalidate = true;
  }
}

extern "C" void glk_request_line_event_uni(winid_t win, glui32 *buf, glui32 maxlen, glui32 initlen)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)win))
  {
    ((CWinGlkWnd*)win)->StartLineEvent(buf,true,maxlen,initlen);
    invalidate = true;
  }
}

extern "C" void glk_set_echo_line_event(winid_t win, glui32 val)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkWnd* pWnd = (CWinGlkWnd*)win;

  if (CWinGlkWnd::IsValidWindow(pWnd))
  {
    if (pWnd->IsKindOf(RUNTIME_CLASS(CWinGlkWndTextBuffer)))
      ((CWinGlkWndTextBuffer*)pWnd)->SetNextEchoInput(val != 0);
  }
}

extern "C" void glk_set_terminators_line_event(winid_t win, glui32 *keycodes, glui32 count)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  std::set<unsigned long> keys;
  for (glui32 i = 0; i < count; i++)
  {
    switch (keycodes[i])
    {
    case keycode_Escape:
      keys.insert(0x10000+VK_ESCAPE);
      break;
    case keycode_PageUp:
      keys.insert(0x10000+VK_PRIOR);
      break;
    case keycode_PageDown:
      keys.insert(0x10000+VK_NEXT);
      break;
    case keycode_Func1:
      keys.insert(0x10000+VK_F1);
      break;
    case keycode_Func2:
      keys.insert(0x10000+VK_F2);
      break;
    case keycode_Func3:
      keys.insert(0x10000+VK_F3);
      break;
    case keycode_Func4:
      keys.insert(0x10000+VK_F4);
      break;
    case keycode_Func5:
      keys.insert(0x10000+VK_F5);
      break;
    case keycode_Func6:
      keys.insert(0x10000+VK_F6);
      break;
    case keycode_Func7:
      keys.insert(0x10000+VK_F7);
      break;
    case keycode_Func8:
      keys.insert(0x10000+VK_F8);
      break;
    case keycode_Func9:
      keys.insert(0x10000+VK_F9);
      break;
    case keycode_Func10:
      keys.insert(0x10000+VK_F10);
      break;
    case keycode_Func11:
      keys.insert(0x10000+VK_F11);
      break;
    case keycode_Func12:
      keys.insert(0x10000+VK_F12);
      break;
    }
  }
  theApp.SetInputTerminators(keys);
}

extern "C" glui32 glk_buffer_canon_decompose_uni(glui32 *buf, glui32 len, glui32 numchars)
{
  glui32 *dest = buffer_canon_decompose(buf,&numchars);
  if (dest == NULL)
    return 0;

  glui32 newlen = numchars;
  if (newlen > len)
    newlen = len;
  if (newlen > 0)
    memcpy(buf,dest,newlen * sizeof(glui32));
  free(dest);

  return numchars;
}

extern "C" glui32 glk_buffer_canon_normalize_uni(glui32 *buf, glui32 len, glui32 numchars)
{
  glui32 *dest = buffer_canon_decompose(buf,&numchars);
  if (dest == NULL)
    return 0;
  numchars = buffer_canon_compose(dest,numchars);

  glui32 newlen = numchars;
  if (newlen > len)
    newlen = len;
  if (newlen)
    memcpy(buf,dest,newlen * sizeof(glui32));
  free(dest);

  return numchars;
}

extern "C" void glk_current_time(glktimeval_t* time)
{
  ToGlkTime(GetNow(),time);
}

extern "C" glsi32 glk_current_simple_time(glui32 factor)
{
  if (factor == 0)
    return 0;
  return ToSimpleTime(GetNow(),factor);
}

extern "C" void glk_time_to_date_utc(glktimeval_t* time, glkdate_t* date)
{
  ToGlkDate(FromGlkTime(time),date);
}

extern "C" void glk_time_to_date_local(glktimeval_t* time, glkdate_t* date)
{
  ToGlkDate(ToLocal(FromGlkTime(time)),date);
}

extern "C" void glk_simple_time_to_date_utc(glsi32 time, glui32 factor, glkdate_t* date)
{
  ToGlkDate(FromSimpleTime(time,factor),date);
}

extern "C" void glk_simple_time_to_date_local(glsi32 time, glui32 factor, glkdate_t* date)
{
  ToGlkDate(ToLocal(FromSimpleTime(time,factor)),date);
}

extern "C" void glk_date_to_time_utc(glkdate_t* date, glktimeval_t* time)
{
  ToGlkTime(FromGlkDate(date),time);
}

extern "C" void glk_date_to_time_local(glkdate_t* date, glktimeval_t* time)
{
  ToGlkTime(ToLocal(FromGlkDate(date)),time);
}

extern "C" glsi32 glk_date_to_simple_time_utc(glkdate_t* date, glui32 factor)
{
  if (factor == 0)
    return 0;
  return ToSimpleTime(FromGlkDate(date),factor);
}

extern "C" glsi32 glk_date_to_simple_time_local(glkdate_t* date, glui32 factor)
{
  if (factor == 0)
    return 0;
  return ToSimpleTime(ToLocal(FromGlkDate(date)),factor);
}

extern "C" strid_t glk_stream_open_resource(glui32 filenum, glui32 rock)
{
  CWinGlkResource* pData = ((CGlkApp*)AfxGetApp())->LoadResource(filenum);
  if (pData == NULL)
    return 0;
  return (strid_t)new CWinGlkStreamResource(pData,rock);
}

extern "C" strid_t glk_stream_open_resource_uni(glui32 filenum, glui32 rock)
{
  CWinGlkResource* pData = ((CGlkApp*)AfxGetApp())->LoadResource(filenum);
  if (pData == NULL)
    return 0;
  return (strid_t)new CWinGlkStreamResourceUni(pData,rock);
}

/////////////////////////////////////////////////////////////////////////////
// Glk dispatch functions
/////////////////////////////////////////////////////////////////////////////

extern "C" void gidispatch_set_object_registry(
  gidispatch_rock_t (*reg)(void *obj, glui32 objclass),
  void (*unreg)(void *obj, glui32 objclass, gidispatch_rock_t objrock))
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkWnd* pWin;
  CWinGlkStream* pStr;
  CWinGlkFileRef* pRef;
  CWinGlkSndChannel* pSnd;

  RegisterObjFn = reg;
  UnregisterObjFn = unreg;

  if (RegisterObjFn)
  {
    for (pWin = CWinGlkWnd::IterateWindows(NULL,NULL); pWin; pWin = CWinGlkWnd::IterateWindows(pWin,NULL))
      pWin->SetDispRock((*RegisterObjFn)(pWin,gidisp_Class_Window));

    for (pStr = CWinGlkStream::IterateStreams(NULL,NULL); pStr; pStr = CWinGlkStream::IterateStreams(pStr,NULL))
      pStr->SetDispRock((*RegisterObjFn)(pStr,gidisp_Class_Stream));

    for (pRef = CWinGlkFileRef::IterateFileRefs(NULL,NULL); pRef; pRef = CWinGlkFileRef::IterateFileRefs(pRef,NULL))
      pRef->SetDispRock((*RegisterObjFn)(pRef,gidisp_Class_Fileref));

    for (pSnd = CWinGlkSndChannel::IterateChannels(NULL,NULL); pSnd; pSnd = CWinGlkSndChannel::IterateChannels(pSnd,NULL))
      pSnd->SetDispRock((*RegisterObjFn)(pSnd,gidisp_Class_Schannel));
  }
}

extern "C" gidispatch_rock_t gidispatch_get_objrock(void *obj, glui32 objclass)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  gidispatch_rock_t rock;
  rock.num = 0;

  switch (objclass)
  {
  case gidisp_Class_Window:
    if (CWinGlkWnd::IsValidWindow((CWinGlkWnd*)obj))
      rock = ((CWinGlkWnd*)obj)->GetDispRock();
    break;
  case gidisp_Class_Stream:
    if (CWinGlkStream::IsValidStream((CWinGlkStream*)obj))
      rock = ((CWinGlkStream*)obj)->GetDispRock();
    break;
  case gidisp_Class_Fileref:
    if (CWinGlkFileRef::IsValidFileRef((CWinGlkFileRef*)obj))
      rock = ((CWinGlkFileRef*)obj)->GetDispRock();
    break;
  case gidisp_Class_Schannel:
    if (CWinGlkSndChannel::IsValidChannel((CWinGlkSndChannel*)obj))
      rock = ((CWinGlkSndChannel*)obj)->GetDispRock();
    break;
  }

  return rock;
}

extern "C" void gidispatch_set_retained_registry(
  gidispatch_rock_t (*reg)(void *array, glui32 len, char *typecode), 
  void (*unreg)(void *array, glui32 len, char *typecode, gidispatch_rock_t objrock))
{
  RegisterArrFn = reg;
  UnregisterArrFn = unreg;
}

/////////////////////////////////////////////////////////////////////////////
// Glk Blorb functions
/////////////////////////////////////////////////////////////////////////////

static bool ShowGameDialog = false;

giblorb_err_t giblorb_set_resource_map(strid_t file)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  if (pApp->GetBlorbMap())
  {
    giblorb_destroy_map(pApp->GetBlorbMap());
    pApp->GetBlorbMap() = NULL;
  }

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)file) == false)
    return giblorb_err_NotFound;
  pApp->GetBlorbFile() = file;
  giblorb_err_t result = giblorb_create_map(file,&(pApp->GetBlorbMap()));

  if ((result == giblorb_err_None) && ShowGameDialog)
  {
    ShowGameDialog = false;
    pApp->LoadBabelMetadata();
    if (pApp->CheckGameId())
    {
      AboutGameDialog Dialog(AfxGetMainWnd());
      Dialog.DoModal();
    }
  }

  return result;
}

extern "C" giblorb_map_t* giblorb_get_resource_map(void)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  return ((CGlkApp*)AfxGetApp())->GetBlorbMap();
}

/////////////////////////////////////////////////////////////////////////////
// Glk Debug functions
/////////////////////////////////////////////////////////////////////////////

extern "C" void gidebug_output(char *text)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (gidebug_debugging_is_available() != 0)
    ((CGlkApp*)AfxGetApp())->DebugOutput(text);
}

extern "C" void gidebug_pause(void)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (gidebug_debugging_is_available() != 0)
  {
    gidebug_announce_cycle(gidebug_cycle_DebugPause);

    CGlkApp* pApp = (CGlkApp*)AfxGetApp();
    pApp->DebugToFront();

    while (true)
    {
      if (char* line = ((CGlkApp*)AfxGetApp())->DebugInput(true))
      {
        if (gidebug_perform_command(line) != 0)
          break;
      }
    }

    CWnd* main = AfxGetMainWnd();
    if (main)
      main->SetForegroundWindow();

    gidebug_announce_cycle(gidebug_cycle_DebugUnpause);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Windows specific Glk functions
/////////////////////////////////////////////////////////////////////////////

extern "C" strid_t winglk_stream_open_resource(const char* name, const char* type, glui32 rock)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if ((name != NULL) && (type != NULL))
    return (strid_t)new CWinGlkStreamWindowsResource(name,type,rock);
  return 0;
}

extern "C" void winglk_app_set_name(const char* name)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (name != NULL)
  {
    CGlkApp* pApp = (CGlkApp*)AfxGetApp();
    pApp->GetAppName() = name;
    pApp->GetAppTitle() = name;
    CWinGlkFileRef::SetDefaultNames(name);

    // Change the profile name in the CGlkApp instance
    free((void*)pApp->m_pszProfileName);
    pApp->m_pszProfileName = _strdup(name);

    // Now the application name is known, the settings can be read
    pApp->ReadSettings();
  }
}

extern "C" void winglk_window_set_title(const char* title)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (title != NULL)
  {
    ((CGlkApp*)AfxGetApp())->GetAppTitle() = title;

    CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetMainWnd();
    if (pMainWnd)
      pMainWnd->SetWindowText(title);
  }
}

extern "C" void winglk_set_about_text(const char* text)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (text != NULL)
    ((CGlkApp*)AfxGetApp())->GetAppAboutText() = text;
}

extern "C" void winglk_set_menu_name(const char* name)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (name != NULL)
    ((CGlkApp*)AfxGetApp())->GetMenuName() = name;
}

extern "C" void winglk_set_resource_directory(const char* dir)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (dir != NULL)
  {
    CString& strResDir = ((CGlkApp*)AfxGetApp())->GetResourceDir();
    strResDir = dir;
    if (strResDir.GetLength() > 0)
    {
      if (strResDir.GetAt(strResDir.GetLength()-1) != '\\')
        strResDir += '\\';
    }
  }
}

extern "C" const char* winglk_get_initial_filename(const char* cmdline, const char* title, const char* filter)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CString& strInitialDir = ((CGlkApp*)AfxGetApp())->GetInitialDir();

  // The file name is static so that the returned buffer remains valid
  static CString strFileName;

  if (cmdline != NULL)
  {
    if (cmdline[0] != '\0')
    {
      int i = 0;
      char c = '\0';

      if (cmdline[i] == '\"')
      {
        c = '\"';
        i++;
      }
      while ((cmdline[i] != '\0') && (cmdline[i] != c))
        strFileName += cmdline[i++];
    }
  }

  if (strFileName.GetLength() == 0)
  {
    SimpleFileDialog FileDlg(TRUE,NULL,NULL,OFN_HIDEREADONLY|OFN_ENABLESIZING,filter,NULL);
    FileDlg.m_ofn.lpstrInitialDir = strInitialDir;
    if (title != NULL)
      FileDlg.m_ofn.lpstrTitle = title;

    if (FileDlg.DoModal() == IDOK)
      strFileName = FileDlg.GetPathName();
  }

  if (strFileName.GetLength() > 0)
  {
    // Use the selected file to set default file names
    int iStart = strFileName.ReverseFind('\\');
    if (iStart <= 0)
      iStart = 0;
    else
      iStart++;
    int iStop = strFileName.ReverseFind('.');
    if ((iStop < 0) || (iStop <= iStart))
      iStop = strFileName.GetLength();
    CWinGlkFileRef::SetDefaultNames(strFileName.Mid(iStart,iStop-iStart));

    // Store the directory path
    strInitialDir = strFileName.Left(iStart);

    return strFileName;
  }
  return NULL;
}

extern "C" void winglk_set_gui(unsigned int id)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  ((CGlkApp*)AfxGetApp())->SetUserGuiID((UINT)id);
}

extern "C" void winglk_load_config_file(const char* gamename)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CString strGameName(gamename);
  int iStart = strGameName.ReverseFind('\\');
  int iStop = strGameName.ReverseFind('.');
  if ((iStop < 0) || (iStop <= iStart))
    iStop = strGameName.GetLength();

  CString strConfigName = strGameName.Left(iStop);
  strConfigName += ".cfg";
  ((CGlkApp*)AfxGetApp())->LoadConfigFile(strConfigName);
}

extern "C" void* winglk_get_resource_handle(void)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  return (void*)AfxGetResourceHandle();
}

extern "C" void winglk_set_help_file(const char* filename)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  ((CGlkApp*)AfxGetApp())->SetHelpFile(filename);
}

extern "C" frefid_t winglk_fileref_create_by_name(glui32 usage, char *name, glui32 rock, int validate)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkFileRef* pFileRef = new CWinGlkFileRef(usage,rock);
  pFileRef->SetFileName(name,usage,validate == 0,false);

  return (frefid_t)pFileRef;
}

extern "C" void winglk_show_game_dialog(void)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  theApp.GetGameInfo().showOptions = true;
  ShowGameDialog = true;
}

/////////////////////////////////////////////////////////////////////////////
// Unofficial Glk extensions
/////////////////////////////////////////////////////////////////////////////

extern "C" void sglk_set_basename(char *s)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CWinGlkFileRef::SetDefaultNames(s);
}

/////////////////////////////////////////////////////////////////////////////
// Gargoyle Glk extensions
/////////////////////////////////////////////////////////////////////////////

extern "C" void garglk_set_zcolors(glui32 fg, glui32 bg)
{
  garglk_set_zcolors_stream(glk_stream_get_current(),fg,bg);
}

extern "C" void garglk_set_zcolors_stream(strid_t str, glui32 fg, glui32 bg)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if ((fg == zcolor_Current) && (bg == zcolor_Current))
    return;

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
    ((CWinGlkStream*)str)->SetTextColours(fg,bg);
}

extern "C" void garglk_set_reversevideo(glui32 reverse)
{
  garglk_set_reversevideo_stream(glk_stream_get_current(),reverse);
}

extern "C" void garglk_set_reversevideo_stream(strid_t str, glui32 reverse)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if (CWinGlkStream::IsValidStream((CWinGlkStream*)str))
    ((CWinGlkStream*)str)->SetTextReverse(reverse != 0);
}
