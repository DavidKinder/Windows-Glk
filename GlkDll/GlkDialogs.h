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

#include "ColourButton.h"
#include "GlkStyle.h"
#include "Dialogs.h"
#include "Dib.h"
#include "Resource.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// CScrollBackDlg dialog
/////////////////////////////////////////////////////////////////////////////

class CScrollBackDlg : public BaseDialog
{
// Construction
public:
  CScrollBackDlg(CWnd* pParent = NULL);   // standard constructor

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
  DECLARE_MESSAGE_MAP()

public:
  CRect m_DialogRect;
  wchar_t* m_Text;
  INT_PTR m_TextLen;

protected:
  CRichEditCtrl m_RichEdit;
  int m_iTextTop;
};

/////////////////////////////////////////////////////////////////////////////
// CWinGlkGeneralPage property page
/////////////////////////////////////////////////////////////////////////////

class CWinGlkGeneralPage : public CPropertyPage
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

  ColourButton m_Text;
  ColourButton m_Back;
  ColourButton m_Link;
};

/////////////////////////////////////////////////////////////////////////////
// CWinGlkStylePage property page
/////////////////////////////////////////////////////////////////////////////

class CWinGlkStylePage : public CPropertyPage
{
// Construction
public:
  CWinGlkStylePage();   // standard constructor

// Dialog Data
  //{{AFX_DATA(CWinGlkStylePage)
  enum { IDD = IDD_OPTIONS_STYLE };
  CStatic  m_MessageCtrl;
  CStatic  m_WarnCtrl;
  CEdit  m_Size;
  CEdit  m_ParaIndent;
  CEdit  m_Indent;
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
};

/////////////////////////////////////////////////////////////////////////////
// CWinGlkSpeechPage property page
/////////////////////////////////////////////////////////////////////////////

class CWinGlkSpeechPage : public CPropertyPage
{
// Construction
public:
  CWinGlkSpeechPage();   // standard constructor

// Dialog Data
  //{{AFX_DATA(CWinGlkSpeechPage)
  enum { IDD = IDD_OPTIONS_SPEECH };
  BOOL m_bSpeak;
  CComboBox m_VoiceCtrl;
  CSliderCtrl m_RateCtrl;
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
};

/////////////////////////////////////////////////////////////////////////////
// CWinGlkPropertySheet property sheet
/////////////////////////////////////////////////////////////////////////////

class CWinGlkPropertySheet : public CPropertySheet
{
protected:
  RECT m_page;
  LOGFONT m_logFont;
  CFont m_font;

  DECLARE_DYNAMIC(CWinGlkPropertySheet)

public:
  CWinGlkPropertySheet(UINT caption, CWnd* parentWnd);

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CMySheet)
  virtual BOOL OnInitDialog();
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
  //}}AFX_VIRTUAL

  // Generated message map functions
  afx_msg LONG OnResizePage(UINT, LONG);

  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CAboutDialog dialog
/////////////////////////////////////////////////////////////////////////////

class CAboutDialog : public BaseDialog
{
// Construction
public:
  CAboutDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
  //{{AFX_DATA(CAboutDialog)
  enum { IDD = IDD_ABOUT };
    // NOTE: the ClassWizard will add data members here
  //}}AFX_DATA

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
  DECLARE_MESSAGE_MAP()

public:
  virtual BOOL OnInitDialog();
};

/////////////////////////////////////////////////////////////////////////////
// AboutGameDialog dialog
/////////////////////////////////////////////////////////////////////////////

class CRichInfo : public CRichEditCtrl
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

class AboutGameDialog : public BaseDialog
{
  DECLARE_DYNAMIC(AboutGameDialog)

public:
  AboutGameDialog(CWnd* pParent = NULL);   // standard constructor
  virtual ~AboutGameDialog();

// Dialog Data
  enum { IDD = IDD_ABOUTGAME };

protected:
  //{{AFX_MSG(FrotzWnd)
  afx_msg void OnPaint();
  //}}AFX_MSG
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  DECLARE_MESSAGE_MAP()

protected:
  CRichInfo m_Info;
  CButton m_Ok;

  CRect m_CoverRect;
  CDibSection m_CoverBitmap;
};

#endif // WINGLK_DIALOGS_H_
