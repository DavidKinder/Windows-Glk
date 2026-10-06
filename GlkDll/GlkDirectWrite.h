/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkDirectWrite
// Measuring and drawing text with DirectWrite
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_DIRECTWRITE_H_
#define WINGLK_DIRECTWRITE_H_

// A DirectWrite font at a particular size
class CWinGlkFont;

namespace WinGlkDirectWrite
{
  // Get the font closest to the GDI font description, which gives the size in
  // pixels as the negative height
  CWinGlkFont* GetFont(const LOGFONT& Font);

  void GetMetrics(CWinGlkFont* pFont, TEXTMETRIC& Metrics);
  SIZE GetTextExtent(CWinGlkFont* pFont, LPCWSTR pText, int iCount);

  // Draw text in the device context's text colour, first filling in the background
  // if the background mode is opaque. If iFitWidth is not zero, the spacing of the
  // characters is adjusted to make the text that wide.
  void TextOut(CWinGlkFont* pFont, HDC hdc, int x, int y, LPCWSTR pText, int iCount,
    int iFitWidth);

  // Text output to the device context between these calls is drawn together, which
  // is much faster than drawing each piece of text separately. No other drawing into
  // the rectangle can be done until the end.
  void BeginDraw(HDC hdc, const RECT& Rect);
  void EndDraw(void);

  // Determine if the character can be shown, either in the font or a fallback font
  bool CanOutput(CWinGlkFont* pFont, UINT32 c);
}

#endif // WINGLK_DIRECTWRITE_H_
