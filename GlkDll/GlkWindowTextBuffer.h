/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkWindowTextBuffer
// Text buffer Glk windows
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_WINDOW_TEXTBUFFER_H_
#define WINGLK_WINDOW_TEXTBUFFER_H_

#include "GlkGraphic.h"
#include "GlkStyle.h"
#include "GlkWindow.h"

extern "C"
{
#include "glk.h"
}

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////

class CWinGlkDC;

/////////////////////////////////////////////////////////////////////////////
// Text buffer Glk windows
/////////////////////////////////////////////////////////////////////////////

class CWinGlkWndTextBuffer : public CWinGlkWnd
{
  DECLARE_DYNAMIC(CWinGlkWndTextBuffer);

public:
  CWinGlkWndTextBuffer(glui32 Rock);
  virtual ~CWinGlkWndTextBuffer();

  virtual void SetActiveWindow(void);
  virtual bool WillReleaseFocus(CWinGlkWnd* pToWnd);
  virtual bool MouseMakesActive(void) { return true; }

  virtual void InitDC(CWinGlkDC& dc, CDC* pdcCompat = NULL);
  virtual bool CheckMorePending(bool update);
  virtual int GetCaretHeight(void);

  virtual void SizeWindow(CRect* pSize);
  virtual void ClearWindow(void);
  virtual void PutCharacter(glui32 c);

  virtual void GetSize(int& iWidth, int& iHeight);
  virtual void GetNeededSize(int iSize, int& iWidth, int& iHeight);

  virtual void StartLineEvent(
    void* pBuffer, bool bUnicode, int iMaxLength, int iStartLength);
  virtual void StartCharEvent(bool unicode);
  virtual void StartLinkEvent(void);
  virtual void EndLinkEvent(void);
  virtual void InputChar(unsigned long InputChar);
  virtual void TestLineInput(int iLineEnd);

  virtual bool MouseClick(CPoint& Click);
  virtual unsigned int GetLinkAtPoint(const CPoint& Point);

  virtual void SetStyle(int iStyle);
  virtual int GetStyle(void);
  virtual CWinGlkStyle* GetStyle(int iStyle);
  virtual void SetHyperlink(unsigned int iLink);
  virtual void SetTextColours(glui32 fg, glui32 bg);
  virtual void SetTextReverse(bool reverse);
  virtual bool DistinguishStyles(int iStyle1, int iStyle2);
  virtual bool MeasureStyle(int iStyle, int iHint, glui32* pResult);

  virtual bool DrawGraphic(CWinGlkGraphic* pGraphic,
    int iValue1, int iValue2, int iWidth, int iHeight, bool& bDelete);

  virtual void Scrollback(void);

  void InsertFlowBreak(void);
  void SetNextEchoInput(bool bNext) { m_bNextEchoInput = bNext; }

protected:
  //{{AFX_MSG(CWinGlkWndTextBuffer)
  afx_msg void OnPaint();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

protected:

  class CHyperlink : public CRect
  {
  public:
    CHyperlink(const CRect& Rect, unsigned int iLink);
    CHyperlink();

    unsigned int m_iLink;
  };

  class CPaintInfo
  {
  public:
    CPaintInfo(int iLeft, int iTop, int iWidth, int iHeight,
      CWinGlkDC& DeviceContext, CArray<CHyperlink,CHyperlink&>& Hyperlinks);
    ~CPaintInfo();

    void MarginAdd(CWinGlkGraphic* pGraphic);
    void MarginPaint(CWinGlkGraphic* pGraphic, int iIndent);
    void MarginPaintPrev(CWinGlkGraphic* pGraphic, int iOffset, int iIndent);
    void NextLine(int iStep, bool bDown);
    bool HasMarginGraphic(void);
    int GetMaxMarginHeight(void);
    void CheckHyperlink(const CRect& Rect);
    void DrawGraphic(CWinGlkGraphic* pGraphic, int iLeft, int iTop);

  public:
    int m_iLeft;
    int m_iTop;
    int m_iWidth;
    int m_iHeight;
    CWinGlkDC& m_DeviceContext;

  protected:
    class CMarginInsert
    {
    public:
      CMarginInsert();

    public:
      int m_iHeightLeft;
      int m_iGraphicWidth;
      bool m_bOnLeft;
    };

    CArray<CMarginInsert,CMarginInsert&> m_Margins;

  protected:
    CArray<CHyperlink,CHyperlink&>& m_Hyperlinks;
  };

  class CLineFormat
  {
  public:
    CLineFormat();
    ~CLineFormat();

  public:
    int m_iFirstCharacter;
    int m_iLastCharacter;
    int m_iMaxAboveBaseline;
    int m_iMaxBelowBaseline;
    int m_iLineLength;
    CArray<int,int> m_MarginIndexes;
  };

  class CParagraph
  {
  public:
    CParagraph(int iStyle, unsigned int iLink, CTextColours* pColours);
    ~CParagraph();

    void AddCharacter(wchar_t c);
    bool AddGraphic(CWinGlkGraphic* pGraphic);
    int GetLength();
    void SetInitialStyle(int iStyle);
    void SetInitialLink(unsigned int iLink);
    void SetInitialColours(const CTextColours& colours);
    void AddStyleChange(int iStyle);
    void AddLinkChange(unsigned int iLink);
    void AddColourChange(const CTextColours& colours);

    bool ClearFormatting(void);
    void Format(CPaintInfo& Info);
    void Update(CPaintInfo& Info, int& iOffset, CWinGlkWnd* pWnd);
    bool Paint(CPaintInfo& Info, int& iFinalLeft, int& iFinalTop, bool bMark);
    void SetAsShown(void);
    void SetLastShown(int iIndex);
    void HasHadInput(void);
    void SetClearAll(void);
    bool GetClearAll(void) const;

    int GetCharCount(void) const;
    int GetHeight(void) const;
    int GetLastShown(void) const;
    int GetFinalLastShown(void) const;
    int LastShownHeight(void) const;
    bool ShowIfLast(void) const;
    int GetMargin(CWinGlkWnd* pWnd) const;

    void AddInteger(unsigned int iValue);
    unsigned int GetInteger(int iPos) const;

    void Speak(void);

    enum TextCodes
    {
      StyleChange = 1,
      LinkChange,
      ColourChange,
      InlineGraphic,
      MarginGraphic,
      FlowBreak,
    };

  protected:
    struct CTextOut
    {
      CWinGlkDC::CDisplay m_Display;
      CPoint m_Position;
      CStringW m_Text;
    };

  protected:
    bool TestLineLength(CPaintInfo& Info, CStringW& strLine,
      CSize& Size, int& iLeftEdge, int iIndent, bool bFinal,
      int& iIndex, int& iLastBreak, int& iLastPossible,
      int& iLastLength, int& iMaxUp, int& iMaxDown, bool& bExit,
      CArray<int,int>& MarginIndexes);
    void TextOut(CLineFormat* pFormat, CPaintInfo& Info,
      int& iLeft, wchar_t* pBuffer, int& iBufferPos);
    void SetLastShown(CPaintInfo& Info, int iIndex);
    int JustifyLength(CLineFormat* pFormat, CPaintInfo& Info,
      wchar_t* pBuffer, int& iSpaces);

  protected:
    CArray<wchar_t,wchar_t> m_Text;
    CArray<CLineFormat*,CLineFormat*> m_Formatting;
    CArray<CWinGlkGraphic*,CWinGlkGraphic*> m_InlineGraphics;
    CArray<CWinGlkGraphic*,CWinGlkGraphic*> m_MarginGraphics;
    CArray<CTextColours*,CTextColours*> m_TextColours;
    CArray<CTextOut,CTextOut&> m_TextOut;
    int m_iInitialStyle;
    unsigned int m_iInitialLink;
    CTextColours* m_pInitialColours;
    int m_iLastShown;
    bool m_bSpoken;
    bool m_bHadInput;
    bool m_bClearAll;
    static const int m_iIndentStep;
  };

protected:
  void Paint(bool bMark);
  void CheckDeleteOldText(int iTop, int iClientHeight);
  void PreparePaintInfo(CPaintInfo& Info, CRect& ClientArea,
    int& iParagraph, int& iOffset, bool bMark);
  bool ShowParagraph(int iPara);
  void AddNewParagraph(void);
  void GetLastShown(int &iLastPara, int& iLastChar);
  void ClearFormatting(int iPara);

  template<class XCHAR> void PaintInputBuffer(
    CWinGlkDC& dc, const XCHAR* input, int inputLen, CPaintInfo& Info);
  template<class XCHAR> void GetTextFitIndexes(
    CArray<int,int>& indexes, CWinGlkDC& dc, int width, int x1, const XCHAR* str, int strLen);
  template<class XCHAR> int MeasureTextFit(
    CWinGlkDC& dc, int width, const XCHAR* str, int strLen);

  CArray<CParagraph*,CParagraph*> m_TextBuffer;
  CArray<CHyperlink,CHyperlink&> m_Hyperlinks;
  CArray<wchar_t,wchar_t> m_ScrollBuffer;
  CWinGlkStyles m_Styles;
  int m_iCurrentStyle;
  unsigned int m_iCurrentLink;
  CTextColours m_CurrentColours;
  bool m_bCheckDeleteText;
  bool m_bMorePending;
  bool m_bNextEchoInput;

public:
  static void SetStyleHint(int iStyle, int iHint, glsi32 Value);
  static void ClearStyleHint(int iStyle, int iHint);
  static CWinGlkStyles* GetDefaultStyles(void) { return &m_DefaultTextBufferStyles; }

protected:
  static CWinGlkStyles m_DefaultTextBufferStyles;
  static CStringW m_strMore;
};

/////////////////////////////////////////////////////////////////////////////
// Device context class
/////////////////////////////////////////////////////////////////////////////

class CWinGlkBufferDC : public CWinGlkDC
{
public:
  CWinGlkBufferDC(CWinGlkWnd* pWnd) : CWinGlkDC(pWnd) {}
  virtual ~CWinGlkBufferDC() {}

  virtual CString GetFontName(void);
  virtual void SetFontStyles(LOGFONT& Font);
  virtual int GetStyleFontSize(void);
};

#endif // WINGLK_WINDOW_TEXTBUFFER_H_
