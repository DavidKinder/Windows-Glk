/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkWindowTextGrid
// Text grid Glk windows
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_WINDOW_TEXTGRID_H_
#define WINGLK_WINDOW_TEXTGRID_H_

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
// Text grid Glk windows
/////////////////////////////////////////////////////////////////////////////

class CWinGlkWndTextGrid : public CWinGlkWnd
{
  DECLARE_DYNAMIC(CWinGlkWndTextGrid);

public:
  CWinGlkWndTextGrid(glui32 Rock);
  virtual ~CWinGlkWndTextGrid();

  virtual void InitDC(CWinGlkDC& dc, CDC* pdcCompat = NULL);
  virtual int GetCaretHeight(void);

  virtual void SizeWindow(CRect* pSize);
  virtual void ClearWindow(void);
  virtual void PutCharacter(glui32 c);
  virtual void MoveCursor(int x, int y);

  virtual void GetSize(int& iWidth, int& iHeight);
  virtual void GetNeededSize(int iSize, int& iWidth, int& iHeight);

  virtual void StartLinkEvent(void);
  virtual void EndLinkEvent(void);
  virtual bool AllowMoreLineInput(void);

  virtual void StartMouseEvent(void);
  virtual bool MouseClick(CPoint& Click);
  virtual unsigned int GetLinkAtPoint(const CPoint& Point);

  virtual void SetStyle(int iStyle);
  virtual int GetStyle(void);
  virtual CWinGlkStyle* GetStyle(int iStyle);
  virtual void SetHyperlink(unsigned int iLink);
  virtual bool DistinguishStyles(int iStyle1, int iStyle2);
  virtual bool MeasureStyle(int iStyle, int iHint, glui32* pResult);

protected:
  //{{AFX_MSG(CWinGlkWndTextGrid)
  afx_msg void OnPaint();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

protected:
  class CGridCellInfo
  {
  public:
    CGridCellInfo();
    ~CGridCellInfo() {}

    bool operator==(const CGridCellInfo& RowInfo);
    bool operator!=(const CGridCellInfo& RowInfo);

    int GetStyle(void) const { return m_iStyle; }
    void SetStyle(int iStyle) { m_iStyle = iStyle; }

    unsigned int GetLink(void) const { return m_iLink; }
    void SetLink(unsigned int iLink) { m_iLink = iLink; }

  protected:
    int m_iStyle;
    unsigned int m_iLink;
  };

  class CGridRow
  {
  public:
    CGridRow() {}
    ~CGridRow() {}

    void DrawRow(CWinGlkDC& dc, int x, int y);

    LPWSTR GetBuffer(int iNewLength);
    void ReleaseBuffer(int iNewLength);
    int GetLength(void) const;
    void SetChar(int i, wchar_t c);
    LPCWSTR GetString(void) const;

    void SetStyle(int i, int iStyle);
    void SetLink(int i, unsigned int iLink);
    unsigned int GetLink(int i) const;

  protected:
    CArray<CGridCellInfo,CGridCellInfo&> m_RowInfo;
    CStringW m_strRow;
  };

  CArray<CGridRow,CGridRow&> m_TextGrid;
  int m_iCursorX;
  int m_iCursorY;
  CWinGlkStyles m_Styles;
  int m_iCurrentStyle;
  unsigned int m_iCurrentLink;

public:
  static void SetStyleHint(int iStyle, int iHint, glsi32 Value);
  static void ClearStyleHint(int iStyle, int iHint);
  static CWinGlkStyles* GetDefaultStyles(void) { return &m_DefaultTextGridStyles; }

protected:
  static CWinGlkStyles m_DefaultTextGridStyles;
};

/////////////////////////////////////////////////////////////////////////////
// Device context class
/////////////////////////////////////////////////////////////////////////////

class CWinGlkGridDC : public CWinGlkDC
{
public:
  CWinGlkGridDC(CWinGlkWnd* pWnd, int iSize);
  virtual ~CWinGlkGridDC() {}

  virtual void GetFonts(LOGFONT*& pTextFont, LOGFONT*& pSizeFont);
  virtual void SetFontStyles(LOGFONT& Font);
  virtual int GetStyleFontSize(void);

protected:
  int m_iSize;
};

#endif // WINGLK_WINDOW_TEXTGRID_H_
