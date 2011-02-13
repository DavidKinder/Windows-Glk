/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkWindow
// Base Glk windows
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_WINDOW_H_
#define WINGLK_WINDOW_H_

#include "GlkGraphic.h"
#include "GlkStyle.h"

extern "C"
{
#include "glk.h"
#include "gi_dispa.h"
}

#include <set>

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////

class CWinGlkWndPair;
class CWinGlkDC;

#ifndef WINGLK_STREAM_H_
class CWinGlkStream;
#endif

/////////////////////////////////////////////////////////////////////////////
// Base class for Glk windows
/////////////////////////////////////////////////////////////////////////////

class CWinGlkWnd : public CWnd
{
  DECLARE_DYNAMIC(CWinGlkWnd);

public:
  CWinGlkWnd(glui32 Rock);
  virtual ~CWinGlkWnd();
  
  glui32 GetRock(void);

  void SetDispRock(const gidispatch_rock_t& Rock) { m_DispRock = Rock; }
  gidispatch_rock_t& GetDispRock(void) { return m_DispRock; }

  void SetArrayRock(const gidispatch_rock_t& Rock) { m_ArrayRock = Rock; }
  gidispatch_rock_t& GetArrayRock(void) { return m_ArrayRock; }

  CWinGlkWndPair* GetParentWnd(void);
  void SetParentWnd(CWinGlkWndPair* pParentWnd);

  CWinGlkWnd* GetSiblingWnd(void);

  void RemoveFromParent(void);
  void ValidateKeyWindows(void);

  void CaretOn(void);
  void CaretOff(void);
  bool InputPending(void) { return m_bInputActive; }

  virtual void CallCreate(void);
  virtual void SetActiveWindow(void);
  virtual bool WillReleaseFocus(CWinGlkWnd* pToWnd) { return true; }
  virtual bool MouseMakesActive(void) { return m_bInputActive; }

  virtual void InitDC(CWinGlkDC& dc, CDC* pdcCompat = NULL) {}
  virtual bool CheckMorePending(void) { return false; }
  virtual int GetCaretHeight(void) { return 0; }

  virtual void CloseWindow(stream_result_t *pResult);
  virtual void SizeWindow(CRect* pSize);
  virtual void ClearWindow(void) {}
  virtual void PutCharacter(glui32 c);
  virtual void MoveCursor(int x, int y) {}
  virtual void DeleteWindow();

  virtual void GetSize(int& iWidth, int& iHeight);
  virtual void GetNeededSize(int iSize, int& iWidth, int& iHeight);

  virtual void StartLineEvent(void* pBuffer, bool bUnicode, int iMaxLength, int iStartLength);
  virtual void EndLineEvent(event_t* pEvent);
  virtual void StartCharEvent(bool bUnicode);
  virtual void EndCharEvent(void);
  virtual void StartLinkEvent(void) {}
  virtual void EndLinkEvent(void) {}
  virtual void InputChar(unsigned long InputChar);
  virtual bool AllowMoreLineInput(void) { return true; }
  virtual void TestLineInput(int iLineEnd) { m_iLineEnd = iLineEnd; }

  virtual void StartMouseEvent(void) {}
  virtual void EndMouseEvent(void) { m_bMouseActive = false; }
  virtual bool MouseClick(CPoint& Click) { return false; }
  virtual unsigned int GetLinkAtPoint(const CPoint& Point) { return 0; }

  CWinGlkStream* GetEchoStream(void);
  void SetEchoStream(CWinGlkStream* pStream);

  CStringW GetUniLineBuffer(void);
  int LineBufferFromUni(const CStringW& Line);

  virtual void SetStyle(int iStyle) {}
  virtual int GetStyle(void) { return style_Normal; }
  virtual CWinGlkStyle* GetStyle(int iStyle) { return NULL; }
  virtual void SetHyperlink(unsigned int iLink) {}
  virtual bool DistinguishStyles(int iStyle1, int iStyle2) { return false; }
  virtual bool MeasureStyle(int iStyle, int iHint, glui32* pResult) { return false; }

  virtual bool DrawGraphic(CWinGlkGraphic* pGraphic,
    int iValue1, int iValue2, int iWidth, int iHeight, bool& bDelete);

  virtual void Scrollback(void) {}

protected:
  glui32 m_Rock;
  gidispatch_rock_t m_DispRock;
  gidispatch_rock_t m_ArrayRock;
  CWinGlkWndPair* m_pParentWnd;
  bool m_bInputActive;
  bool m_bInputUnicode;
  bool m_bCaret;
  int m_iLineX;
  int m_iLineY;
  CWinGlkStream* m_pEcho;
  bool m_bMouseActive;
  bool m_bLinksActive;

  // Line input
  void* m_pLineBuffer;
  int m_iLineLength;
  int m_iLineEnd;
  int m_iLinePos;
  CArray<CStringW,CStringW&> m_History;
  int m_iHistory;
  int m_iPrevStyle;
  bool m_bEchoLineInput;
  std::set<unsigned long> m_InputTerminators;

/////////////////////////////////////////////////////////////////////////////
// Message handlers

public:
// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CWinGlkWnd)
  //}}AFX_VIRTUAL
  // Generated message map functions

protected:
  //{{AFX_MSG(CWinGlkWnd)
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

public:
  static CWinGlkWnd* GetMainWindow(void);
  static CWinGlkWnd* GetActiveWindow(void);
  static CMap<CWinGlkWnd*,CWinGlkWnd*,int,int>& GetWindowMap(void);

  static CWinGlkWnd* OpenWindow(CWinGlkWnd* pNewWnd, CWinGlkWnd* pSplitWnd,
    glui32 Method, glui32 Size);
  static void CloseAllWindows(void);

  static bool IsValidWindow(CWinGlkWnd* pWnd);
  static CWinGlkWnd* IterateWindows(CWinGlkWnd* pWnd, glui32* pRockPtr);

  static bool GetFinalOutput(void) { return m_bFinalOutput; }
  static bool GetExiting(void) { return m_bExiting; }
  static void SetExiting(void);
  static void SizeAllWindows(void);

  static COLORREF GetColour(glsi32 iColour);

protected:
  static CWinGlkWnd* m_pMainWnd;
  static CWinGlkWnd* m_pActiveWnd;
  static CMap<CWinGlkWnd*,CWinGlkWnd*,int,int> WindowMap;

  static bool m_bFinalOutput;
  static bool m_bExiting;
};

/////////////////////////////////////////////////////////////////////////////
// Pair windows
/////////////////////////////////////////////////////////////////////////////

class CWinGlkWndPair : public CWinGlkWnd
{
  DECLARE_DYNAMIC(CWinGlkWndPair);

public:
  CWinGlkWndPair(CWinGlkWnd* pChild1, CWinGlkWnd* pChild2, glui32 Method,
    glui32 Size);
  virtual ~CWinGlkWndPair() {}
  
  virtual void CallCreate(void) {}

  virtual void SizeWindow(CRect* pSize);
  virtual void DeleteWindow();

  virtual void StartLineEvent(unsigned char* pBuffer, int iMaxLength,
    int iStartLength) {}
  virtual void StartCharEvent(bool unicode) {}
  virtual void InputChar(unsigned long InputChar) {}

  void ReplaceChildWnd(CWinGlkWnd* pOldChild, CWinGlkWnd* pNewChild);
  void SetArrangement(glui32 Method, glui32 Size, CWinGlkWnd* pKey);
  void GetArrangement(glui32* MethodPtr, glui32* SizePtr, CWinGlkWnd** pKeyPtr);

  CWinGlkWnd* GetKeyWindow(void) { return m_pKey; }
  CWinGlkWnd* GetChild1Window(void) { return m_pChild1; }
  CWinGlkWnd* GetChild2Window(void) { return m_pChild2; }
  void SetKeyWindow(CWinGlkWnd* pKey) { m_pKey = pKey; }

protected:
  CWinGlkWnd* m_pChild1;
  CWinGlkWnd* m_pChild2;
  CWinGlkWnd* m_pKey;

  glui32 m_Method;
  glui32 m_Size;
};

/////////////////////////////////////////////////////////////////////////////
// Base device context class
/////////////////////////////////////////////////////////////////////////////

class CWinGlkDC : public CDC
{
public:
  CWinGlkDC(CWinGlkWnd* pWnd);
  virtual ~CWinGlkDC();

  class CDisplay
  {
  public:
    CDisplay();
    CDisplay(int iStyle, unsigned int iLink);
    bool operator==(const CDisplay& Compare);
    bool operator!=(const CDisplay& Compare);

    int m_iStyle;
    unsigned int m_iLink;
    int m_iIndex;
  };

  void SetStyle(int iStyle, unsigned int iLink);
  void SetDisplay(const CDisplay& Display);

  CDisplay GetDisplay(void) { return m_Display; }
  int GetStyle(void) { return m_Display.m_iStyle; }
  unsigned int GetLink(void) { return m_Display.m_iLink; }

  CWinGlkStyle* GetStyleFromWindow(int iStyle);

  virtual void GetFonts(LOGFONT*& pTextFont, LOGFONT*& pSizeFont) = 0;
  virtual void SetFontStyles(LOGFONT& Font) = 0;
  virtual int GetStyleFontSize(void) = 0;

  BOOL TextOut(int x, int y, LPCSTR lpszString, int nCount);
  CSize GetTextExtent(LPCSTR lpszString, int nCount) const;

  BOOL TextOut(int x, int y, LPCWSTR lpszString, int nCount);
  BOOL TextOut(int x, int y, const CStringW& str);
  CSize GetTextExtent(LPCWSTR lpszString, int nCount) const;
  CSize GetTextExtent(const CStringW& str) const;

public:
  TEXTMETRIC m_FontMetrics;
  CWinGlkWnd* m_pWnd;
  CFont* m_pFont;
  CFont* m_pOldFont;
  CWinGlkStyle m_Style;
  CDisplay m_Display;

protected:
  CFont* m_Fonts[style_NUMSTYLES * 2];
};

#endif // WINGLK_WINDOW_H_
