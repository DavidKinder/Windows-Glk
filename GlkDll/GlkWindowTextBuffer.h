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

#include <vector>

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
  virtual bool DistinguishStyles(int iStyle1, int iStyle2);
  virtual bool MeasureStyle(int iStyle, int iHint, glui32* pResult);

  virtual bool DrawGraphic(CWinGlkGraphic* pGraphic,
    int iValue1, int iValue2, int iWidth, int iHeight, bool& bDelete);

  virtual void Scrollback(void);

  void InsertFlowBreak(void);
  void SetNextEchoInput(bool bNext) { m_NextEchoInput = bNext; }

protected:
  //{{AFX_MSG(CWinGlkWndTextBuffer)
  afx_msg void OnPaint();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

private:
  // State used when measuring layout
  struct MeasureState;

  // Base interface class for a display element (e.g. a word)
  class ElementBase
  {
  public:
    virtual ~ElementBase() {}
    virtual void Measure(MeasureState& ms) = 0;
    virtual void PrePaint(CWinGlkDC& dc) const = 0;
    virtual void PaintFore(CWinGlkDC& dc, int x, int y) const = 0;
    virtual void PaintBack(CWinGlkDC& dc, int x, int y, COLORREF defaultBack) const = 0;
    virtual int Width() const = 0;
    virtual int HeightAboveBase() const = 0;
    virtual int HeightBelowBase() const = 0;
    virtual bool IsWordSeparator() const = 0;
  };

  // An element representing a space between words
  class ElementSpace : public ElementBase
  {
  public:
    ElementSpace();
    void SetJustifyWidth(int jw);
    virtual void Measure(MeasureState& ms);
    virtual void PrePaint(CWinGlkDC& dc) const;
    virtual void PaintFore(CWinGlkDC& dc, int x, int y) const;
    virtual void PaintBack(CWinGlkDC& dc, int x, int y, COLORREF defaultBack) const;
    virtual int Width() const;
    virtual int HeightAboveBase() const;
    virtual int HeightBelowBase() const;
    virtual bool IsWordSeparator() const;

  private:
    int m_Width, m_JustifyWidth;
    int m_HeightAboveBase, m_HeightBelowBase;
  };

  // An element that shows a word
  class ElementWord : public ElementBase
  {
  public:
    ElementWord();
    void PutCharacter(WCHAR c) { m_Text.AppendChar(c); }
    virtual void Measure(MeasureState& ms);
    virtual void PrePaint(CWinGlkDC& dc) const;
    virtual void PaintFore(CWinGlkDC& dc, int x, int y) const;
    virtual void PaintBack(CWinGlkDC& dc, int x, int y, COLORREF defaultBack) const;
    virtual int Width() const;
    virtual int HeightAboveBase() const;
    virtual int HeightBelowBase() const;
    virtual bool IsWordSeparator() const;

  private:
    CStringW m_Text;
    int m_Width, m_HeightAboveBase, m_HeightBelowBase;
  };

  // An element that changes the style
  class ElementStyle : public ElementBase
  {
  public:
    ElementStyle(int style);
    int GetStyle();
    virtual void Measure(MeasureState& ms);
    virtual void PrePaint(CWinGlkDC& dc) const;
    virtual void PaintFore(CWinGlkDC& dc, int x, int y) const;
    virtual void PaintBack(CWinGlkDC& dc, int x, int y, COLORREF defaultBack) const;
    virtual int Width() const;
    virtual int HeightAboveBase() const;
    virtual int HeightBelowBase() const;
    virtual bool IsWordSeparator() const;

  private:
    int m_Style, m_HeightAboveBase, m_HeightBelowBase;
  };

  // An element that shows a graphic
  class ElementGraphic : public ElementBase
  {
  public:
    ElementGraphic(CWinGlkGraphic* graphic);
    virtual ~ElementGraphic();
    virtual void Measure(MeasureState& ms);
    virtual void PrePaint(CWinGlkDC& dc) const;
    virtual void PaintFore(CWinGlkDC& dc, int x, int y) const;
    virtual void PaintBack(CWinGlkDC& dc, int x, int y, COLORREF defaultBack) const;
    virtual int Width() const;
    virtual int HeightAboveBase() const;
    virtual int HeightBelowBase() const;
    virtual bool IsWordSeparator() const;

  private:
    CWinGlkGraphic* m_Graphic;
    int m_HeightAboveBase;
  };

  // The layout of elements on a line
  struct LayoutLine
  {
    LayoutLine(int leftX, int maxX);
    LayoutLine TakeLastWord(int& x, int maxX);

    std::vector<ElementBase*> Elements;
    int LeftX, MaxX;
    int HeightAboveBase, HeightBelowBase;
  };

  // The layout of elements in lines
  struct LayoutResult
  {
    std::vector<LayoutLine> Lines;
  };

  // Representation of a paragraph as a collection of elements
  class Paragraph
  {
  public:
    Paragraph(int style);
    ~Paragraph();

    void PutCharacter(WCHAR c);
    void SetStyle(int style);
    bool DrawGraphic(CWinGlkGraphic* graphic);
    void Layout(CWinGlkDC& dc, LayoutResult& result, int width);
    void ClearMeasure();

  private:
    ElementBase* GetLastElement();

    std::vector<ElementBase*> m_Elements;
    bool m_NeedMeasure;
    static const int m_IndentStep;
  };

  std::vector<Paragraph*> m_TextBuffer;
  CWinGlkStyles m_Styles;
  int m_CurrentStyle;
  bool m_NextEchoInput;

public:
  static void SetStyleHint(int iStyle, int iHint, glsi32 Value);
  static void ClearStyleHint(int iStyle, int iHint);
  static CWinGlkStyles* GetDefaultStyles(void) { return &m_DefaultTextBufferStyles; }

private:
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

  virtual void GetFonts(LOGFONT*& pTextFont, LOGFONT*& pSizeFont);
  virtual void SetFontStyles(LOGFONT& Font);
  virtual int GetStyleFontSize(void);
};

#endif // WINGLK_WINDOW_TEXTBUFFER_H_
