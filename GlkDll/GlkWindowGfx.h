/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkWindowGfx
// Graphics Glk windows
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_WINDOW_GFX_H_
#define WINGLK_WINDOW_GFX_H_

#include "GlkGraphic.h"
#include "GlkWindow.h"
#include "Dib.h"

extern "C"
{
#include "glk.h"
}

/////////////////////////////////////////////////////////////////////////////
// Graphics windows
/////////////////////////////////////////////////////////////////////////////

class CWinGlkWndGraphics : public CWinGlkWnd
{
  DECLARE_DYNAMIC(CWinGlkWndGraphics);

public:
  CWinGlkWndGraphics(glui32 Rock);
  virtual ~CWinGlkWndGraphics();

  virtual void SizeWindow(CRect* pSize);
  virtual void ClearWindow(void);

  virtual void GetSize(int& iWidth, int& iHeight);
  virtual void GetNeededSize(int iSize, int& iWidth, int& iHeight);

  virtual void StartMouseEvent(void);
  virtual bool MouseClick(CPoint& Click);

  virtual bool DrawGraphic(CWinGlkGraphic* pGraphic,
    int iValue1, int iValue2, int iWidth, int iHeight, bool& bDelete);

  void SetBackColour(COLORREF Colour) { m_BackColour = Colour; }
  void FillRect(CRect& Rect,COLORREF Colour);
  void FillRectBack(CRect& Rect) { FillRect(Rect,m_BackColour); }

protected:
  //{{AFX_MSG(CWinGlkWndGraphics)
  afx_msg void OnPaint();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

protected:
  COLORREF m_BackColour;
  CDibSection* m_pDibSection;
};

#endif // WINGLK_WINDOW_GFX_H_
