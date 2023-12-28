/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkDialogs
// Dialog classes
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_DIALOGS_H_
#define WINGLK_DIALOGS_H_

#include "GlkStyle.h"
#include "Resource.h"

#include "ColourButton.h"
#include "DarkMode.h"
#include "Dialogs.h"
#include "Dib.h"

#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// Base class for Glk dialogs
/////////////////////////////////////////////////////////////////////////////

class GlkDialog : public BaseDialog
{
  DECLARE_DYNAMIC(GlkDialog)

public:
  GlkDialog(UINT templateId, CWnd* parent = NULL);

  virtual INT_PTR DoModal();

  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CScrollBackDlg dialog
/////////////////////////////////////////////////////////////////////////////

class CScrollBackDlg : public GlkDialog
{
// Construction
public:
  CScrollBackDlg(CWnd* pParent = NULL);   // standard constructor

  void SetDarkMode(DarkMode* dark);

// Dialog Data
  //{{AFX_DATA(CScrollBackDlg)
  enum { IDD = IDD_SCROLLBACK };
    // NOTE: the ClassWizard will add data members here
  //}}AFX_DATA

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CScrollBackDlg)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CScrollBackDlg)
  virtual BOOL OnInitDialog();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnCopy();
  //}}AFX_MSG
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  afx_msg LRESULT OnSameSizeAsMain(WPARAM, LPARAM);
  DECLARE_MESSAGE_MAP()

public:
  CRect m_DialogRect;
  wchar_t* m_Text;
  INT_PTR m_TextLen;

protected:
  DarkModeRichEditCtrl m_RichEdit;
  DarkModeButton m_CopyButton;
  DarkModeButton m_CloseButton;
  int m_iTextTop;
  int m_dpi;
};

/////////////////////////////////////////////////////////////////////////////
// CWinGlkGeneralPage property page
/////////////////////////////////////////////////////////////////////////////

class CWinGlkGeneralPage : public DarkModePropertyPage
{
// Construction
public:
  CWinGlkGeneralPage();   // standard constructor

// Dialog Data
  //{{AFX_DATA(CWinGlkGeneralPage)
  enum { IDD = IDD_OPTIONS_GENERAL };
  BOOL  m_bBorders;
  BOOL  m_bGUI;
  BOOL  m_bStyleHints;
  CString m_PropFontName;
  CString m_FixedFontName;
  CString m_FontSize;
  int   m_iFiction;
  //}}AFX_DATA

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CWinGlkGeneralPage)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CWinGlkGeneralPage)
  virtual BOOL OnInitDialog();
  virtual void OnOK();
  //}}AFX_MSG

public:
  COLORREF GetTextColour(void);
  void SetTextColour(COLORREF Colour);
  COLORREF GetBackColour(void);
  void SetBackColour(COLORREF Colour);
  COLORREF GetLinkColour(void);
  void SetLinkColour(COLORREF Colour);

protected:
  void SetControlState(void);

  // Called when enumerating fonts, and populates the font drop down lists in the dialog
  static int CALLBACK ListFonts(ENUMLOGFONTEX *font, NEWTEXTMETRICEX *metric, DWORD fontType, LPARAM param);

  DarkModeGroupBox m_FontGroup;
  DarkModeComboBox m_FontSizeCombo;
  DarkModeComboBox m_PropFont;
  DarkModeComboBox m_FixedFont;
  ColourButton m_Text;
  ColourButton m_Back;
  ColourButton m_Link;
  DarkModeGroupBox m_OptionsGroup;
  DarkModeCheckButton m_BordersCheck;
  DarkModeCheckButton m_GUICheck;
  DarkModeCheckButton m_StyleHintsCheck;
  DarkModeGroupBox m_StartGroup;
  DarkModeComboBox m_iFictionCombo;
};

/////////////////////////////////////////////////////////////////////////////
// CWinGlkStylePage property page
/////////////////////////////////////////////////////////////////////////////

class CWinGlkStylePage : public DarkModePropertyPage
{
// Construction
public:
  CWinGlkStylePage();   // standard constructor

// Dialog Data
  //{{AFX_DATA(CWinGlkStylePage)
  enum { IDD = IDD_OPTIONS_STYLE };
  CStatic  m_MessageCtrl;
  CStatic  m_WarnCtrl;
  DarkModeEdit  m_Size;
  DarkModeEdit  m_ParaIndent;
  DarkModeEdit  m_Indent;
  BOOL  m_bOblique;
  int  m_iJustification;
  BOOL  m_bProportional;
  int  m_iWeight;
  int  m_iWindowStyle;
  int  m_iWindowType;
  BOOL  m_bReverse;
  //}}AFX_DATA
  int  m_iIndentation;
  int  m_iParaIndentation;
  int  m_iSize;

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CWinGlkStylePage)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CWinGlkStylePage)
  virtual BOOL OnInitDialog();
  afx_msg void OnChangeWindowType();
  afx_msg void OnChangeStyle();
  virtual void OnOK();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

public:
  void SetCurrentStyle(int iWinType, int iStyle);

protected:
  void NewStyleValuesToDialog(const CWinGlkStyle& Style);
  void SetControlAndMsgState(void);
  void StoreStyleSettings(void);

protected:
  CWinGlkStyles m_TextBufferStyles;
  CWinGlkStyles m_TextGridStyles;
  CWinGlkStyles* m_pStyles;
  int m_iStyle;

  DarkModeGroupBox m_StyleGroup;
  DarkModeComboBox m_WindowTypeCombo;
  DarkModeComboBox m_WindowStyleCombo;
  DarkModeComboBox m_JustificationCombo;
  DarkModeComboBox m_WeightCombo;
  DarkModeCheckButton m_ObliqueCheck;
  DarkModeCheckButton m_ProportionalCheck;
  DarkModeCheckButton m_ReverseCheck;
};

/////////////////////////////////////////////////////////////////////////////
// CWinGlkSpeechPage property page
/////////////////////////////////////////////////////////////////////////////

class CWinGlkSpeechPage : public DarkModePropertyPage
{
// Construction
public:
  CWinGlkSpeechPage();   // standard constructor

// Dialog Data
  //{{AFX_DATA(CWinGlkSpeechPage)
  enum { IDD = IDD_OPTIONS_SPEECH };
  BOOL m_bSpeak;
  DarkModeComboBox m_VoiceCtrl;
  DarkModeSliderCtrl m_RateCtrl;
  int m_iRate;
  //}}AFX_DATA

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CWinGlkSpeechPage)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CWinGlkSpeechPage)
  virtual BOOL OnInitDialog();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

public:
  CString m_strVoice, m_strDefaultVoice;

  DarkModeGroupBox m_SpeechGroup;
  DarkModeCheckButton m_SpeakCheck;
};

/////////////////////////////////////////////////////////////////////////////
// CWinGlkPropertySheet property sheet
/////////////////////////////////////////////////////////////////////////////

class CWinGlkPropertySheet : public DarkModePropertySheet
{
protected:
  RECT m_page;
  LOGFONT m_logFont;
  CFont m_font;

  int m_dpi;
  double m_fontHeightPerDpi;

  DECLARE_DYNAMIC(CWinGlkPropertySheet)

public:
  CWinGlkPropertySheet(UINT caption, CWnd* parentWnd);
  virtual INT_PTR DoModal();

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CMySheet)
  virtual BOOL OnInitDialog();
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
  //}}AFX_VIRTUAL

  // Generated message map functions
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  afx_msg LONG OnResizePage(UINT, LONG);

  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CAboutDialog dialog
/////////////////////////////////////////////////////////////////////////////

class CLogoStatic : public CStatic
{
protected:
  afx_msg void OnNcPaint();
  DECLARE_MESSAGE_MAP()
};

class CAboutDialog : public GlkDialog
{
// Construction
public:
  CAboutDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
  //{{AFX_DATA(CAboutDialog)
  enum { IDD = IDD_ABOUT };
  CLogoStatic m_LogoCtrl;
  //}}AFX_DATA
  CSize m_LogoSize;

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAboutDialog)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CAboutDialog)
  //}}AFX_MSG
  virtual BOOL OnInitDialog();
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  DECLARE_MESSAGE_MAP()

  DarkModeButton m_Ok;
  DarkModeGroupBox m_AboutGroup;
  DarkModeGroupBox m_AdditionalGroup;
};

/////////////////////////////////////////////////////////////////////////////
// AboutGameDialog dialog
/////////////////////////////////////////////////////////////////////////////

class CRichInfo : public DarkModeRichEditCtrl
{
  DECLARE_DYNAMIC(CRichInfo)

protected:
  DECLARE_MESSAGE_MAP()

  virtual void PreSubclassWindow();

public:
  afx_msg void OnSetFocus(CWnd* pOldWnd);
  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

  void SetText(int format, UINT id);
  void SetText(int format, const CString& text);
};

class AboutGameDialog : public GlkDialog
{
  DECLARE_DYNAMIC(AboutGameDialog)

public:
  AboutGameDialog(CWnd* pParent = NULL);   // standard constructor

  virtual void SetDarkMode(DarkMode* dark);

// Dialog Data
  enum { IDD = IDD_ABOUTGAME };

protected:
  //{{AFX_MSG(FrotzWnd)
  afx_msg void OnPaint();
  //}}AFX_MSG
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  DECLARE_MESSAGE_MAP()

protected:
  CRichInfo m_Info;
  DarkModeButton m_Ok;

  CRect m_CoverRect;
  CDibSection m_CoverBitmap;
  int m_dpi;
  int m_headingEnd;
};

#endif // WINGLK_DIALOGS_H_
