/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkDialogs
// Dialog classes
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkDll.h"
#include "GlkDialogs.h"
#include "GlkTalk.h"
#include "GlkWindowTextBuffer.h"
#include "GlkWindowTextGrid.h"

#include <math.h>
#include <memory>

extern "C" __declspec(dllimport) void ScaleGfx(COLORREF*, UINT, UINT, COLORREF*, UINT, UINT);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Call-back function for streaming into rich edit controls
/////////////////////////////////////////////////////////////////////////////

static DWORD CALLBACK RichStreamCB(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
  CFile* pFile = (CFile*)dwCookie;
  *pcb = pFile->Read(pbBuff,cb);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Implementation of the CScrollBackDlg dialog
/////////////////////////////////////////////////////////////////////////////

CScrollBackDlg::CScrollBackDlg(CWnd* pParent) : BaseDialog(CScrollBackDlg::IDD, pParent)
{
  //{{AFX_DATA_INIT(CScrollBackDlg)
    // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
  m_Text = NULL;
  m_TextLen = 0;
}

void CScrollBackDlg::DoDataExchange(CDataExchange* pDX)
{
  BaseDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CScrollBackDlg)
    // NOTE: the ClassWizard will add DDX and DDV calls here
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CScrollBackDlg, BaseDialog)
  //{{AFX_MSG_MAP(CScrollBackDlg)
  ON_WM_SIZE()
  ON_BN_CLICKED(IDC_COPY, OnCopy)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScrollBackDlg message handlers
/////////////////////////////////////////////////////////////////////////////

BOOL CScrollBackDlg::OnInitDialog() 
{
  BaseDialog::OnInitDialog();
  
  // Subclass the text control
  if (m_RichEdit.SubclassDlgItem(IDC_TEXT,this) == FALSE)
    return FALSE;

  // Get the relative position of the top of the text control
  CRect size;
  m_RichEdit.GetWindowRect(size);
  ScreenToClient(size);
  m_iTextTop = size.top;

  // Change the window icon
  SetIcon(((CGlkApp*)AfxGetApp())->GetIcon(),TRUE);

  // Resize the dialog
  MoveWindow(m_DialogRect);

  // Set the control to only show plain text, but allow all code pages
  m_RichEdit.SetTextMode(TM_PLAINTEXT|TM_SINGLELEVELUNDO|TM_MULTICODEPAGE);

  // Set the control to format the text so that it fits into the window
  m_RichEdit.SetTargetDevice(NULL,0);
  
  // Set the background colour
  m_RichEdit.SetBackgroundColor(FALSE,GetSysColor(COLOR_3DFACE));

  // Put the text into the control
  if (m_Text != NULL)
  {
    CMemFile inFile((BYTE*)m_Text,m_TextLen*sizeof(wchar_t));
    EDITSTREAM stream;
    stream.dwCookie = (DWORD)&inFile;
    stream.pfnCallback = RichStreamCB;
    m_RichEdit.StreamIn(SF_TEXT|SF_UNICODE,stream);
  }

  // Put the cursor at the end of the buffer
  m_RichEdit.SetFocus();
  m_RichEdit.SetSel(-1,-1);
  m_RichEdit.SendMessage(EM_SCROLLCARET);

  return TRUE;
}

void CScrollBackDlg::OnSize(UINT nType, int cx, int cy) 
{
  BaseDialog::OnSize(nType, cx, cy);

  // Resize the text control
  if (m_RichEdit.GetSafeHwnd() != NULL)
    m_RichEdit.SetWindowPos(NULL,0,m_iTextTop,cx,cy-m_iTextTop,SWP_NOZORDER);
}

void CScrollBackDlg::OnCopy() 
{
  m_RichEdit.Copy();
}

/////////////////////////////////////////////////////////////////////////////
// Option property sheets
/////////////////////////////////////////////////////////////////////////////

static void DDX_TextNoError(CDataExchange* pDX, int nIDC, int& value)
{
  char txt[256];
  HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
  if (pDX->m_bSaveAndValidate)
  {
    ::GetWindowText(hWndCtrl,txt,256);
    if (sscanf(txt,"%d",&value) != 1)
      value = 0;
  }
  else
  {
    sprintf(txt,"%d",value);
    ::SetWindowText(hWndCtrl,txt);
  }
}

/////////////////////////////////////////////////////////////////////////////
// CWinGlkGeneralPage property sheet
/////////////////////////////////////////////////////////////////////////////

CWinGlkGeneralPage::CWinGlkGeneralPage()
  : CPropertyPage(CWinGlkGeneralPage::IDD)
{
  //{{AFX_DATA_INIT(CWinGlkGeneralPage)
  m_bBorders = FALSE;
  m_bLinkColour = FALSE;
  m_bUnderlineColour = FALSE;
  m_bGUI = FALSE;
  m_bStyleHints = FALSE;
  m_iFiction = 0;
  //}}AFX_DATA_INIT
}

void CWinGlkGeneralPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CWinGlkGeneralPage)
  DDX_Control(pDX, IDC_USE_LINK_COLOUR, m_UseLinkColour);
  DDX_Check(pDX, IDC_BORDERS, m_bBorders);
  DDX_Check(pDX, IDC_USE_LINK_COLOUR, m_bLinkColour);
  DDX_Check(pDX, IDC_USE_LINK_UNDER, m_bUnderlineColour);
  DDX_Check(pDX, IDC_GUI, m_bGUI);
  DDX_Check(pDX, IDC_STYLEHINT, m_bStyleHints);
  DDX_CBIndex(pDX, IDC_SHOW_IFICTION, m_iFiction);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWinGlkGeneralPage, CPropertyPage)
  //{{AFX_MSG_MAP(CWinGlkGeneralPage)
  ON_BN_CLICKED(IDC_USE_LINK_COLOUR, OnUseLinkColour)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CWinGlkGeneralPage::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();

  m_Link.SubclassDlgItem(IDC_LINK_COLOUR,this);
  SetControlState();

  return TRUE;
}

void CWinGlkGeneralPage::OnUseLinkColour() 
{
  SetControlState();
}

COLORREF CWinGlkGeneralPage::GetCustomLinkColour(void)
{
  return m_Link.GetCurrentColour();
}

void CWinGlkGeneralPage::SetCustomLinkColour(COLORREF Colour)
{
  m_Link.SetCurrentColour(Colour);
}

void CWinGlkGeneralPage::SetControlState(void)
{
  m_Link.EnableWindow(m_UseLinkColour.GetCheck() != 0);
  int ShowStartup = ((CGlkApp*)AfxGetApp())->GetGameInfo().showOptions ? SW_SHOW : SW_HIDE;
  GetDlgItem(IDC_STARTUP)->ShowWindow(ShowStartup);
  GetDlgItem(IDC_LABEL_IFICTION)->ShowWindow(ShowStartup);
  GetDlgItem(IDC_SHOW_IFICTION)->ShowWindow(ShowStartup);
}

/////////////////////////////////////////////////////////////////////////////
// CWinGlkSpeechPage property sheet
/////////////////////////////////////////////////////////////////////////////

CWinGlkSpeechPage::CWinGlkSpeechPage()
  : CPropertyPage(CWinGlkSpeechPage::IDD)
{
  //{{AFX_DATA_INIT(CWinGlkSpeechPage)
  m_bSpeak = FALSE;
  m_iRate = 0;
  //}}AFX_DATA_INIT
}

void CWinGlkSpeechPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CWinGlkSpeechPage)
  DDX_Check(pDX, IDC_SPEAK, m_bSpeak);
  DDX_Control(pDX, IDC_VOICE, m_VoiceCtrl);
  DDX_Control(pDX, IDC_SPEECH_RATE, m_RateCtrl);
  DDX_Slider(pDX, IDC_SPEECH_RATE, m_iRate);
  //}}AFX_DATA_MAP

  if (pDX->m_bSaveAndValidate)
  {
    int sel = m_VoiceCtrl.GetCurSel();
    if (sel != CB_ERR)
      m_VoiceCtrl.GetLBText(sel,m_strVoice);
  }
  else
  {
    if (m_VoiceCtrl.SelectString(-1,m_strVoice) == CB_ERR)
    {
      if (m_VoiceCtrl.SelectString(-1,m_strDefaultVoice) == CB_ERR)
        m_VoiceCtrl.SetCurSel(0);
    }
  }
}

BEGIN_MESSAGE_MAP(CWinGlkSpeechPage, CPropertyPage)
  //{{AFX_MSG_MAP(CWinGlkSpeechPage)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CWinGlkSpeechPage::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();

  CStringArray Voices;
  TextToSpeech::GetSpeechEngine().GetVoices(Voices,m_strDefaultVoice);
  for (int i = 0; i < Voices.GetSize(); i++)
    m_VoiceCtrl.AddString(Voices.GetAt(i));

  m_RateCtrl.SetRange(-10,10,TRUE);
  m_RateCtrl.SetPos(m_iRate);
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CWinGlkStylePage property sheet
/////////////////////////////////////////////////////////////////////////////

CWinGlkStylePage::CWinGlkStylePage() : CPropertyPage(CWinGlkStylePage::IDD),
  m_TextBufferStyles(*(CWinGlkWndTextBuffer::GetDefaultStyles())),
  m_TextGridStyles(*(CWinGlkWndTextGrid::GetDefaultStyles()))
{
  //{{AFX_DATA_INIT(CWinGlkStylePage)
  m_bOblique = FALSE;
  m_iJustification = -1;
  m_bProportional = FALSE;
  m_iWeight = -1;
  m_iWindowStyle = -1;
  m_iWindowType = -1;
  m_iIndentation = 0;
  m_iParaIndentation = 0;
  m_iSize = 0;
  m_bReverse = FALSE;
  //}}AFX_DATA_INIT

  m_pStyles = NULL;
  m_iStyle = 0;
}

void CWinGlkStylePage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CWinGlkStylePage)
  DDX_Control(pDX, IDC_MESSAGE, m_MessageCtrl);
  DDX_Control(pDX, IDC_WARNICON, m_WarnCtrl);
  DDX_Control(pDX, IDC_TEXT_SIZE, m_Size);
  DDX_Control(pDX, IDC_PARAGRAPH, m_ParaIndent);
  DDX_Control(pDX, IDC_INDENTATION, m_Indent);
  DDX_Check(pDX, IDC_OBLIQUE, m_bOblique);
  DDX_CBIndex(pDX, IDC_JUSTIFICATION, m_iJustification);
  DDX_Check(pDX, IDC_PROPORTIONAL, m_bProportional);
  DDX_CBIndex(pDX, IDC_WEIGHT, m_iWeight);
  DDX_CBIndex(pDX, IDC_WINDOW_STYLE, m_iWindowStyle);
  DDX_CBIndex(pDX, IDC_WINDOW_TYPE, m_iWindowType);
  DDX_Check(pDX, IDC_REVERSE, m_bReverse);
  //}}AFX_DATA_MAP
  DDX_TextNoError(pDX, IDC_INDENTATION, m_iIndentation);
  DDX_TextNoError(pDX, IDC_PARAGRAPH, m_iParaIndentation);
  DDX_TextNoError(pDX, IDC_TEXT_SIZE, m_iSize);
}

BEGIN_MESSAGE_MAP(CWinGlkStylePage, CPropertyPage)
  //{{AFX_MSG_MAP(CWinGlkStylePage)
  ON_CBN_SELCHANGE(IDC_WINDOW_TYPE, OnChangeWindowType)
  ON_CBN_SELCHANGE(IDC_WINDOW_STYLE, OnChangeStyle)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CWinGlkStylePage::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();

  m_Text.SubclassDlgItem(IDC_TEXT_COLOUR,this);
  m_Back.SubclassDlgItem(IDC_BACK_COLOUR,this);

  m_Indent.SetLimitText(32);
  m_ParaIndent.SetLimitText(32);
  m_Size.SetLimitText(32);

  SetControlAndMsgState();

  return TRUE;
}

void CWinGlkStylePage::OnChangeWindowType() 
{
  OnChangeStyle();
}

void CWinGlkStylePage::OnChangeStyle() 
{
  UpdateData(TRUE);
  StoreStyleSettings();

  switch (m_iWindowType)
  {
  case 0:
  default:
    SetCurrentStyle(wintype_TextBuffer,m_iWindowStyle);
    break;
  case 1:
    SetCurrentStyle(wintype_TextGrid,m_iWindowStyle);
    break;
  }
  SetControlAndMsgState();
  UpdateData(FALSE);
}

void CWinGlkStylePage::OnOK() 
{
  UpdateData(TRUE);
  StoreStyleSettings();

  *(CWinGlkWndTextBuffer::GetDefaultStyles()) = m_TextBufferStyles;
  *(CWinGlkWndTextGrid::GetDefaultStyles()) = m_TextGridStyles;

  EndDialog(IDOK);
}

void CWinGlkStylePage::NewStyleValuesToDialog(const CWinGlkStyle& Style)
{
  m_iIndentation = Style.m_Indent;
  m_iParaIndentation = Style.m_ParaIndent;
  m_iSize = Style.m_Size;

  m_bOblique = (Style.m_Oblique) == 1 ? TRUE : FALSE;
  m_bProportional = (Style.m_Proportional) == 1 ? TRUE : FALSE;
  m_bReverse = (Style.m_ReverseColour) == 1 ? TRUE : FALSE;

  switch (Style.m_Justify)
  {
  case stylehint_just_LeftFlush:
  default:
    m_iJustification = 0;
    break;
  case stylehint_just_LeftRight:
    m_iJustification = 1;
    break;
  case stylehint_just_RightFlush:
    m_iJustification = 2;
    break;
  case stylehint_just_Centered:
    m_iJustification = 3;
    break;
  }
  switch (Style.m_Weight)
  {
  case 0:
  default:
    m_iWeight = 0;
    break;
  case 1:
    m_iWeight = 1;
    break;
  case -1:
    m_iWeight = 2;
    break;
  }

  m_Text.SetCurrentColour(CWinGlkWnd::GetColour(Style.m_TextColour));
  if (m_Text.GetSafeHwnd())
    m_Text.Invalidate();
  m_Back.SetCurrentColour(CWinGlkWnd::GetColour(Style.m_BackColour));
  if (m_Back.GetSafeHwnd())
    m_Back.Invalidate();
}

void CWinGlkStylePage::SetCurrentStyle(int iWinType, int iStyle)
{
  switch (iWinType)
  {
  case wintype_TextBuffer:
  default:
    m_iWindowType = 0;
    m_pStyles = &m_TextBufferStyles;
    break;
  case wintype_TextGrid:
    m_iWindowType = 1;
    m_pStyles = &m_TextGridStyles;
    break;
  }
  m_iWindowStyle = iStyle;
  m_iStyle = iStyle;

  if (m_pStyles)
  {
    CWinGlkStyle* pStyle = m_pStyles->GetStyle(iStyle);
    NewStyleValuesToDialog(*pStyle);
  }
}

void CWinGlkStylePage::SetControlAndMsgState(void)
{
  bool bUserEdit = false;
  if (m_pStyles)
  {
    CWinGlkStyle* pStyle = m_pStyles->GetStyle(m_iStyle);
    bUserEdit = pStyle->m_bUserControl;
  }
  bool bAllStyles = (m_iWindowType == 0) && bUserEdit;

  m_Indent.EnableWindow(bAllStyles);
  m_ParaIndent.EnableWindow(bAllStyles);
  m_Size.EnableWindow(bUserEdit && ((m_iWindowType == 0) || (m_iStyle == 0)));
  GetDlgItem(IDC_OBLIQUE)->EnableWindow(bAllStyles);
  GetDlgItem(IDC_JUSTIFICATION)->EnableWindow(bAllStyles);
  GetDlgItem(IDC_PROPORTIONAL)->EnableWindow(bAllStyles);
  GetDlgItem(IDC_WEIGHT)->EnableWindow(bAllStyles);

  GetDlgItem(IDC_REVERSE)->EnableWindow(bUserEdit);
  m_Text.EnableWindow(bUserEdit);
  m_Back.EnableWindow(bUserEdit);

  CString strMessage;
  LPCTSTR Icon;
  if (bUserEdit)
  {
    Icon = IDI_ASTERISK;
    strMessage.LoadString(IDS_STYLE_NOTE);
  }
  else
  {
    Icon = IDI_EXCLAMATION;
    strMessage.LoadString(IDS_STYLE_NOEDIT);
  }
  m_WarnCtrl.SetIcon(AfxGetApp()->LoadStandardIcon(Icon));
  m_MessageCtrl.SetWindowText(strMessage);
}

void CWinGlkStylePage::StoreStyleSettings(void)
{
  if (m_pStyles)
  {
    CWinGlkStyle* pStyle = m_pStyles->GetStyle(m_iStyle);
    if (pStyle->m_bUserControl)
    {
      pStyle->m_Indent = m_iIndentation;
      pStyle->m_ParaIndent = m_iParaIndentation;
      pStyle->m_Size = m_iSize;

      pStyle->m_Oblique = m_bOblique ? 1 : 0;
      pStyle->m_Proportional = m_bProportional ? 1 : 0;
      pStyle->m_ReverseColour = m_bReverse ? 1 : 0;

      switch (m_iJustification)
      {
      case 0:
      default:
        pStyle->m_Justify = stylehint_just_LeftFlush;
        break;
      case 1:
        pStyle->m_Justify = stylehint_just_LeftRight;
        break;
      case 2:
        pStyle->m_Justify = stylehint_just_RightFlush;
        break;
      case 3:
        pStyle->m_Justify = stylehint_just_Centered;
        break;
      }
      switch (m_iWeight)
      {
      case 0:
      default:
        pStyle->m_Weight = 0;
        break;
      case 1:
        pStyle->m_Weight = 1;
        break;
      case 2:
        pStyle->m_Weight = -1;
        break;
      }

      pStyle->m_TextColour = ColourToGlk(m_Text.GetCurrentColour());
      pStyle->m_BackColour = ColourToGlk(m_Back.GetCurrentColour());

      CWinGlkStyle* pNoHintStyle = m_pStyles->GetNoHintStyle(m_iStyle);
      *pNoHintStyle = *pStyle;
    }
  }
}

int CWinGlkStylePage::ColourToGlk(COLORREF Colour)
{
  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  int iColour = 0;

  if (Colour == ::GetSysColor(COLOR_WINDOWTEXT))
    iColour = 0xFFFFFFFF;
  else if (Colour == ::GetSysColor(COLOR_WINDOW))
    iColour = 0xFFFFFFFE;
  else
  {
    int r = GetRValue(Colour);
    int g = GetGValue(Colour);
    int b = GetBValue(Colour);

    iColour = (r<<16) | (g<<8) | b;
  }
  return iColour;
}

/////////////////////////////////////////////////////////////////////////////
// CWinGlkPropertySheet property sheet
/////////////////////////////////////////////////////////////////////////////

#define WM_RESIZEPAGE WM_APP+1

static void ChangeDialogFont(CWnd* wnd, CFont* font)
{
  CRect windowRect;

  TEXTMETRIC tmOld, tmNew;
  CDC* dc = wnd->GetDC();
  CFont* oldFont = dc->SelectObject(wnd->GetFont());
  dc->GetTextMetrics(&tmOld);
  dc->SelectObject(font);
  dc->GetTextMetrics(&tmNew);
  dc->SelectObject(oldFont);
  wnd->ReleaseDC(dc);

  long oldHeight = tmOld.tmHeight+tmOld.tmExternalLeading;
  long newHeight = tmNew.tmHeight+tmNew.tmExternalLeading;

  // calculate new dialog window rectangle
  CRect clientRect, newClientRect, newWindowRect;

  wnd->GetWindowRect(windowRect);
  wnd->GetClientRect(clientRect);
  long xDiff = windowRect.Width() - clientRect.Width();
  long yDiff = windowRect.Height() - clientRect.Height();

  newClientRect.left = newClientRect.top = 0;
  newClientRect.right = clientRect.right * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth;
  newClientRect.bottom = clientRect.bottom * newHeight / oldHeight;

  newWindowRect.left = windowRect.left - (newClientRect.right - clientRect.right)/2;
  newWindowRect.top = windowRect.top - (newClientRect.bottom - clientRect.bottom)/2;
  newWindowRect.right = newWindowRect.left + newClientRect.right + xDiff;
  newWindowRect.bottom = newWindowRect.top + newClientRect.bottom + yDiff;

  wnd->MoveWindow(newWindowRect);
  wnd->SetFont(font);

  CWnd* childWnd = wnd->GetWindow(GW_CHILD);
  while (childWnd)
  {
    childWnd->SetFont(font);
    childWnd->GetWindowRect(windowRect);

    CString strClass;
    ::GetClassName(childWnd->GetSafeHwnd(),strClass.GetBufferSetLength(32),31);
    strClass.MakeUpper();
    if (strClass == "COMBOBOX")
    {
      CRect rect;
      childWnd->SendMessage(CB_GETDROPPEDCONTROLRECT,0,(LPARAM)&rect);
      windowRect.right = rect.right;
      windowRect.bottom = rect.bottom;
    }

    wnd->ScreenToClient(windowRect);

    if (childWnd->GetDlgCtrlID() == IDC_WARNICON)
    {
      CRect initialRect(windowRect);

      windowRect.left = windowRect.left * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth;
      windowRect.right = windowRect.right * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth;
      windowRect.top = windowRect.top * newHeight / oldHeight;
      windowRect.bottom = windowRect.bottom * newHeight / oldHeight;

      windowRect.InflateRect((initialRect.Width()-windowRect.Width())/2,
        (initialRect.Height()-windowRect.Height())/2);
    }
    else
    {
      windowRect.left = windowRect.left * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth;
      windowRect.right = windowRect.right * tmNew.tmAveCharWidth / tmOld.tmAveCharWidth;
      windowRect.top = windowRect.top * newHeight / oldHeight;
      windowRect.bottom = windowRect.bottom * newHeight / oldHeight;
    }
    childWnd->MoveWindow(windowRect);

    childWnd = childWnd->GetWindow(GW_HWNDNEXT);
  }
}

IMPLEMENT_DYNAMIC(CWinGlkPropertySheet, CPropertySheet)

CWinGlkPropertySheet::CWinGlkPropertySheet(UINT caption, CWnd* parentWnd)
  : CPropertySheet(caption,parentWnd,0)
{
  GetFontDialog getFont(m_logFont,IDD_ABOUTGAME,parentWnd);
  getFont.DoModal();
}

BEGIN_MESSAGE_MAP(CWinGlkPropertySheet, CPropertySheet)
//{{AFX_MSG_MAP(CWinGlkPropertySheet)
//}}AFX_MSG_MAP
ON_MESSAGE (WM_RESIZEPAGE, OnResizePage)  
END_MESSAGE_MAP()

BOOL CWinGlkPropertySheet::OnInitDialog() 
{
  CPropertySheet::OnInitDialog();

  m_font.CreateFontIndirect(&m_logFont);
  ChangeDialogFont(this,&m_font);
  CPropertyPage* page = GetActivePage();
  for (int i = 0; i < GetPageCount(); i++)
  {
    SetActivePage(i);
    CPropertyPage* page = GetActivePage();
    ChangeDialogFont(page,&m_font);
  }
  SetActivePage(page);

  CTabCtrl* tab = GetTabControl();
  tab->GetWindowRect(&m_page);
  ScreenToClient(&m_page);
  tab->AdjustRect(FALSE,&m_page);

  page->MoveWindow(&m_page);
  return TRUE;
}

BOOL CWinGlkPropertySheet::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
  NMHDR* pnmh = (LPNMHDR) lParam;
  if (TCN_SELCHANGE == pnmh->code)
    PostMessage(WM_RESIZEPAGE);
  return CPropertySheet::OnNotify(wParam, lParam, pResult);
}

LONG CWinGlkPropertySheet::OnResizePage(UINT, LONG)
{
  CPropertyPage* page = GetActivePage();
  page->MoveWindow(&m_page);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDialog dialog
/////////////////////////////////////////////////////////////////////////////

CAboutDialog::CAboutDialog(CWnd* pParent) : BaseDialog(CAboutDialog::IDD, pParent)
{
  //{{AFX_DATA_INIT(CAboutDialog)
    // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}

void CAboutDialog::DoDataExchange(CDataExchange* pDX)
{
  BaseDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAboutDialog)
  // NOTE: the ClassWizard will add DDX and DDV calls here
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDialog, BaseDialog)
  //{{AFX_MSG_MAP(CAboutDialog)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAboutDialog::OnInitDialog()
{
  BaseDialog::OnInitDialog();
  CGlkApp* pApp = (CGlkApp*)AfxGetApp();

  CWnd* group = GetDlgItem(IDC_ABOUT_GROUP);
  CString groupName;
  group->GetWindowText(groupName);
  pApp->AddMenuName(groupName);
  group->SetWindowText(groupName);

  CWnd* ctrl = GetDlgItem(IDC_ABOUT_TEXT);
  CString about;
  ctrl->GetWindowText(about);
  about.Replace("%glk%","0.7.4");
  about.Replace("%winglk%","1.44");
  ctrl->SetWindowText(about);

  const CString& appAbout = pApp->GetAppAboutText();
  if (appAbout.IsEmpty() == FALSE)
  {
    ctrl = GetDlgItem(IDC_ABOUT_TEXT);
    CString about;
    ctrl->GetWindowText(about);
    about = appAbout + CString("\n") + about;
    ctrl->SetWindowText(about);

    // Work out the height of the extra text
    CRect size;
    ctrl->GetWindowRect(size);
    int extra = (size.Height()/3)+1;

    // Resize the controls
    ScreenToClient(size);
    ctrl->MoveWindow(size.left,size.top,size.Width(),size.Height()+extra);

    ctrl = GetDlgItem(IDC_ABOUT_GROUP);
    ctrl->GetWindowRect(size);
    ScreenToClient(size);
    ctrl->MoveWindow(size.left,size.top,size.Width(),size.Height()+extra);

    ctrl = GetDlgItem(IDC_ADDITION_TEXT);
    ctrl->GetWindowRect(size);
    ScreenToClient(size);
    ctrl->MoveWindow(size.left,size.top+extra,size.Width(),size.Height());

    ctrl = GetDlgItem(IDC_ADDITION_GROUP);
    ctrl->GetWindowRect(size);
    ScreenToClient(size);
    ctrl->MoveWindow(size.left,size.top+extra,size.Width(),size.Height());

    ctrl = GetDlgItem(IDOK);
    ctrl->GetWindowRect(size);
    ScreenToClient(size);
    ctrl->MoveWindow(size.left,size.top+extra,size.Width(),size.Height());

    GetWindowRect(size);
    MoveWindow(size.left,size.top,size.Width(),size.Height()+extra);
  }
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// About This Game dialog
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CRichInfo, CRichEditCtrl)

BEGIN_MESSAGE_MAP(CRichInfo, CRichEditCtrl)
  ON_WM_SETFOCUS()
  ON_WM_SETCURSOR()
END_MESSAGE_MAP()

void CRichInfo::OnSetFocus(CWnd*)
{
  // Don't accept the focus ...
}

BOOL CRichInfo::OnSetCursor(CWnd*, UINT, UINT)
{
  // Don't let the cursor change ...
  return TRUE;
}

void CRichInfo::PreSubclassWindow()
{
  SetBackgroundColor(FALSE,GetSysColor(COLOR_3DFACE));

  // Set the control to word wrap the text
  SetTargetDevice(NULL,0);

  // Notify the parent window of the control's required size
  SetEventMask(ENM_REQUESTRESIZE);

  CRichEditCtrl::PreSubclassWindow();
}

void CRichInfo::SetText(int format, UINT id)
{
  CString text;
  if (text.LoadString(id))
    SetText(format,text);
}

void CRichInfo::SetText(int format, const CString& text)
{
  CMemFile inFile((BYTE*)((LPCTSTR)text),text.GetLength());

  EDITSTREAM stream;
  stream.dwCookie = (DWORD)&inFile;
  stream.pfnCallback = RichStreamCB;
  StreamIn(format,stream);
}

IMPLEMENT_DYNAMIC(AboutGameDialog, BaseDialog)

AboutGameDialog::AboutGameDialog(CWnd* pParent) : BaseDialog(AboutGameDialog::IDD, pParent)
{
}

AboutGameDialog::~AboutGameDialog()
{
}

void AboutGameDialog::DoDataExchange(CDataExchange* pDX)
{
  BaseDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDOK, m_Ok);
}

BEGIN_MESSAGE_MAP(AboutGameDialog, BaseDialog)
  ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL AboutGameDialog::OnInitDialog()
{
  BaseDialog::OnInitDialog();

  // Change the window icon
  if (GetParent() == NULL)
    SetIcon(((CGlkApp*)AfxGetApp())->GetIcon(),TRUE);

  CWaitCursor wc;
  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  const CGlkApp::GameInfo& GameInfo = pApp->GetGameInfo();

  // Initialize the rich edit text control
  if (m_Info.SubclassDlgItem(IDC_INFO,this) == FALSE)
    return FALSE;
  m_Info.SetText(SF_TEXT,GameInfo.description);

  // Add the title, author, etc.
  CString heading;
  heading.Append(GameInfo.title);
  heading.AppendChar('\r');
  if (GameInfo.seriesNumber.IsEmpty() == FALSE)
  {
    heading.Append(GameInfo.series);
    heading.Append(" #");
    heading.Append(GameInfo.seriesNumber);
    heading.AppendChar('\r');
  }
  if (GameInfo.headline.IsEmpty() == FALSE)
  {
    heading.Append(GameInfo.headline);
    heading.AppendChar('\r');
  }
  heading.Append(GameInfo.author);
  if (GameInfo.year.IsEmpty() == FALSE)
  {
    heading.Append(" (");
    heading.Append(GameInfo.year);
    heading.AppendChar(')');
  }
  heading.Append("\r\r");
  
  m_Info.SetSel(0,0);
  CHARFORMAT format;
  format.cbSize = sizeof format;
  format.dwMask = CFM_BOLD;
  format.dwEffects = CFE_BOLD;
  m_Info.SetSelectionCharFormat(format);
  m_Info.ReplaceSel(heading);

  // Set the dialog title
  SetWindowText(GameInfo.title);

  // Get the initial position of the rich edit, used for spacing
  CRect initRect;
  m_Info.GetWindowRect(initRect);
  ScreenToClient(initRect);

  // Get the cover art
  std::auto_ptr<CWinGlkGraphic> coverGfx;
  if (GameInfo.cover != -1)
    coverGfx.reset(pApp->LoadGraphic(GameInfo.cover,TRUE,FALSE));

  // Choose a size for the cover art
  CRect screen = pApp->GetScreenSize(true);
  if (coverGfx.get() != NULL)
  {
    double aspect = (double)coverGfx->m_pHeader->biWidth / fabs((double)coverGfx->m_pHeader->biHeight);
    int coverX = screen.Width()/3;
    int coverY = (int)(coverX / aspect);
    m_CoverRect = CRect(initRect.TopLeft(),CSize(coverX,coverY));
  }
  else
    m_CoverRect = CRect(initRect.TopLeft(),CSize(screen.Width()/3,screen.Width()/3));
  m_CoverRect.top = (int)(initRect.top*0.6);

  // Resize the cover art
  if (coverGfx.get() != NULL)
  {
    CDC* dc = GetDC();
    BOOL create = m_CoverBitmap.CreateBitmap(dc->GetSafeHdc(),
      m_CoverRect.Width(),m_CoverRect.Height());
    ReleaseDC(dc);
    if (!create)
      return FALSE;

    DWORD Tick1 = ::GetTickCount();

    ScaleGfx((COLORREF*)coverGfx->m_pPixels,
      coverGfx->m_pHeader->biWidth,abs(coverGfx->m_pHeader->biHeight),
      m_CoverBitmap.GetBits(),m_CoverRect.Width(),m_CoverRect.Height());

    DWORD Tick2 = ::GetTickCount();
    CString TimeMsg;
    TimeMsg.Format("Scaling the dialog picture took %dms\n",(Tick2-Tick1));
    ::OutputDebugString(TimeMsg);
  }

  // Resize the rich edit control
  CRect infoRect = m_CoverRect;
  if (coverGfx.get() != NULL)
    infoRect.OffsetRect(m_CoverRect.right,0);
  m_Info.MoveWindow(infoRect);

  // Get the size of the OK button
  CRect okRect;
  m_Ok.GetClientRect(okRect);

  // Resize the OK button
  okRect.MoveToXY(
    initRect.left+((infoRect.right+initRect.left)/2)-(okRect.Width()/2),
    m_CoverRect.bottom+infoRect.top);
  m_Ok.MoveWindow(okRect,FALSE);

  // Get the initial dialog size
  CRect dlgCRect, dlgWRect;
  GetClientRect(dlgCRect);
  GetWindowRect(dlgWRect);
  int dlgX = dlgWRect.Width()-dlgCRect.Width();
  int dlgY = dlgWRect.Height()-dlgCRect.Height();

  // Resize the dialog
  MoveWindow(0,0,
    infoRect.right+initRect.left+dlgX,
    okRect.bottom+initRect.top+dlgY,FALSE);
  CenterWindow();
  return TRUE;
}

void AboutGameDialog::OnPaint()
{
  if (m_CoverBitmap.GetBits() != NULL)
  {
    CPaintDC paintDC(this);

    CDC memDC;
    memDC.CreateCompatibleDC(&paintDC);
    CDibSection::SelectDibSection(memDC,&m_CoverBitmap);

    paintDC.BitBlt(
      m_CoverRect.left,m_CoverRect.top,m_CoverRect.Width(),m_CoverRect.Height(),
      &memDC,0,0,SRCCOPY);
  }
  else
    Default();
}
