/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkWindowTextBuffer
// Text buffer Glk windows
//
/////////////////////////////////////////////////////////////////////////////

// XXXXDK TO DO
// line input: scroll left/right, stop at right margin
// margin images, consider background colour
// [More]
// coalesce words and spaces for speed during layout?
// scroll bar
// remove scrollback
// selecting and copying

#include "StdAfx.h"
#include "GlkDll.h"
#include "GlkMainWnd.h"
#include "GlkStream.h"
#include "GlkWindowTextBuffer.h"
#include "Dib.h"

extern "C" __declspec(dllimport) void ScaleGfx(COLORREF*, UINT, UINT, COLORREF*, UINT, UINT);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Text buffer Glk windows
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkWndTextBuffer,CWinGlkWnd);

BEGIN_MESSAGE_MAP(CWinGlkWndTextBuffer, CWinGlkWnd)
  //{{AFX_MSG_MAP(CWinGlkWndTextBuffer)
  ON_WM_PAINT()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CWinGlkWndTextBuffer::CWinGlkWndTextBuffer(glui32 Rock) : CWinGlkWnd(Rock)
{
  if (m_strMore.GetLength() == 0)
    m_strMore.LoadString(IDS_MORE);

  m_Styles = m_DefaultTextBufferStyles;
  m_CurrentStyle = style_Normal;
  m_NextEchoInput = true;
}

CWinGlkWndTextBuffer::~CWinGlkWndTextBuffer()
{
  for (size_t i = 0; i < m_TextBuffer.size(); i++)
    delete m_TextBuffer[i];
}

void CWinGlkWndTextBuffer::SetActiveWindow(void)
{
  CWinGlkWnd::SetActiveWindow();
}

bool CWinGlkWndTextBuffer::WillReleaseFocus(CWinGlkWnd* pToWnd)
{
  if (pToWnd == this)
    return true;
  return !m_bInputActive;
}

void CWinGlkWndTextBuffer::InitDC(CWinGlkDC& dc, CDC* pdcCompat)
{
  if (pdcCompat)
    dc.CreateCompatibleDC(pdcCompat);

  dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
  dc.SetBkColor(GetColour(GetStyle(style_Normal)->m_BackColour));
  dc.SetStyle(style_Normal,0);
}

bool CWinGlkWndTextBuffer::CheckMorePending(bool update)
{
  return false; //XXXXDK more
}

int CWinGlkWndTextBuffer::GetCaretHeight(void)
{
  CWinGlkBufferDC dc(this);
  CDC* wdc = GetDC();
  dc.Attach(*wdc);
  InitDC(dc);
  dc.SetStyle(style_Input,0);
  int height = dc.m_FontMetrics.tmHeight;
  ReleaseDC(wdc);
  return height;
}

void CWinGlkWndTextBuffer::SizeWindow(CRect* pSize)
{
  CWinGlkWnd::SizeWindow(pSize);
  for (size_t i = 0; i < m_TextBuffer.size(); i++)
    m_TextBuffer[i]->ClearMeasure();
}

void CWinGlkWndTextBuffer::ClearWindow(void)
{
  for (size_t i = 0; i < m_TextBuffer.size(); i++)
    delete m_TextBuffer[i];
  m_TextBuffer.resize(0);
}

void CWinGlkWndTextBuffer::PutCharacter(glui32 c)
{
  CWinGlkWnd::PutCharacter(c);

  if (c == L'\n')
  {
    //XXXXDK speak
    if (m_TextBuffer.empty())
      m_TextBuffer.push_back(new Paragraph(m_CurrentStyle));
    m_TextBuffer.push_back(new Paragraph(m_CurrentStyle));
  }
  else
  {
    if ((c >= 32 && c <= 126) || (c >= 160 && c <= 0xFFFF))
    {
      // Valid character
      if (m_TextBuffer.empty())
        m_TextBuffer.push_back(new Paragraph(m_CurrentStyle));
      m_TextBuffer.back()->PutCharacter((WCHAR)c);
    }
    else
    {
      // Invalid character
      char pszError[16];
      if (c <= 0xFF)
        sprintf(pszError,"[0x%02X]",(int)c);
      else
        sprintf(pszError,"[0x%08X]",(int)c);
      for (int i = 0; i < (int)strlen(pszError); i++)
        PutCharacter(pszError[i]);
    }
  }
}

void CWinGlkWndTextBuffer::GetSize(int& iWidth, int& iHeight)
{
  // Measure the size of the new window
  CRect ClientArea;
  GetClientRect(ClientArea);

  CWinGlkBufferDC dc(this);
  CDC* wdc = GetDC();
  dc.Attach(*wdc);
  InitDC(dc);

  iWidth = ClientArea.Width() / dc.m_FontMetrics.tmAveCharWidth;
  iHeight = ClientArea.Height() / dc.m_FontMetrics.tmHeight;

  ReleaseDC(wdc);
}

void CWinGlkWndTextBuffer::GetNeededSize(int iSize, int& iWidth, int& iHeight)
{
  CWinGlkBufferDC dc(this);
  CDC* wdc = GetDC();
  dc.Attach(*wdc);
  InitDC(dc);

  iWidth = dc.m_FontMetrics.tmAveCharWidth*iSize;
  iHeight = dc.m_FontMetrics.tmHeight*iSize;
  ReleaseDC(wdc);

  static int iBorderWidth = 0;
  static int iBorderHeight = 0;

  CRect WindowArea, ClientArea;
  GetWindowRect(WindowArea);
  GetClientRect(ClientArea);

  // Add enough space for the window borders. If the window is
  // being restored from the minimized state, use the previous
  // border sizes.
  if (WindowArea.Width() > 0)
  {
    iBorderWidth = WindowArea.Width() - ClientArea.Width();
    iBorderHeight = WindowArea.Height() - ClientArea.Height();
  }
  
  iWidth += iBorderWidth;
  iHeight += iBorderHeight;
}

void CWinGlkWndTextBuffer::StartLineEvent(void* pBuffer, bool bUnicode, int iMaxLength, int iStartLength)
{
  if (!m_bInputActive)
  {
    //XXXXDK speak
    m_bEchoInput = m_NextEchoInput;
  }
  CWinGlkWnd::StartLineEvent(pBuffer,bUnicode,iMaxLength,iStartLength);
}

void CWinGlkWndTextBuffer::StartCharEvent(bool unicode)
{
  //XXXXDK more
  CWinGlkWnd::StartCharEvent(unicode);
}

void CWinGlkWndTextBuffer::StartLinkEvent(void)
{
  //XXXXDK
}

void CWinGlkWndTextBuffer::EndLinkEvent(void)
{
  //XXXXDK
}

void CWinGlkWndTextBuffer::InputChar(unsigned long InputChar)
{
  //XXXXDK speak
  CWinGlkWnd::InputChar(InputChar);
}

void CWinGlkWndTextBuffer::TestLineInput(int iLineEnd)
{
  CWinGlkWnd::TestLineInput(iLineEnd);
  //XXXXDK more
}

bool CWinGlkWndTextBuffer::MouseClick(CPoint& Click)
{
  //XXXXDK
  return false;
}

unsigned int CWinGlkWndTextBuffer::GetLinkAtPoint(const CPoint& Point)
{
  //XXXXDK
  return 0;
}

void CWinGlkWndTextBuffer::SetStyle(int iStyle)
{
  m_CurrentStyle = iStyle;
  if (!m_TextBuffer.empty())
    m_TextBuffer.back()->SetStyle(iStyle);
}

int CWinGlkWndTextBuffer::GetStyle(void)
{
  return m_CurrentStyle;
}

CWinGlkStyle* CWinGlkWndTextBuffer::GetStyle(int iStyle)
{
  return m_Styles.GetStyle(iStyle);
}

void CWinGlkWndTextBuffer::SetHyperlink(unsigned int iLink)
{
  //XXXXDK
}

bool CWinGlkWndTextBuffer::DistinguishStyles(int iStyle1, int iStyle2)
{
  CWinGlkStyle* pStyle1 = GetStyle(iStyle1);
  CWinGlkStyle* pStyle2 = GetStyle(iStyle2);

  if (pStyle1 && pStyle2)
  {
    if (pStyle1->m_Indent != pStyle2->m_Indent)
      return true;
    if (pStyle1->m_ParaIndent != pStyle2->m_ParaIndent)
      return true;
    if (pStyle1->m_Justify != pStyle2->m_Justify)
      return true;
    if (pStyle1->m_Size != pStyle2->m_Size)
      return true;
    if (pStyle1->m_Weight != pStyle2->m_Weight)
      return true;
    if (pStyle1->m_Oblique != pStyle2->m_Oblique)
      return true;
    if (pStyle1->m_Proportional != pStyle2->m_Proportional)
      return true;
    if (GetColour(pStyle1->m_TextColour) != GetColour(pStyle2->m_TextColour))
      return true;
    if (GetColour(pStyle1->m_BackColour) != GetColour(pStyle2->m_BackColour))
      return true;
    if (GetColour(pStyle1->m_ReverseColour) != GetColour(pStyle2->m_ReverseColour))
      return true;
  }
  return false;
}

bool CWinGlkWndTextBuffer::MeasureStyle(int iStyle, int iHint, glui32* pResult)
{
  bool bMeasured = true;

  CWinGlkStyle* pStyle = GetStyle(iStyle);
  if (pStyle)
  {
    switch (iHint)
    {
    case stylehint_Indentation:
      if (pResult)
        *pResult = pStyle->m_Indent;
      break;
    case stylehint_ParaIndentation:
      if (pResult)
        *pResult = pStyle->m_ParaIndent;
      break;
    case stylehint_Justification:
      if (pResult)
        *pResult = pStyle->m_Justify;
      break;
    case stylehint_Size:
      if (pResult)
        *pResult = pStyle->m_Size;
      break;
    case stylehint_Weight:
      if (pResult)
        *pResult = pStyle->m_Weight;
      break;
    case stylehint_Oblique:
      if (pResult)
        *pResult = pStyle->m_Oblique;
      break;
    case stylehint_Proportional:
      if (pResult)
        *pResult = pStyle->m_Proportional;
      break;
    case stylehint_TextColor:
      if (pResult)
      {
        int iColour = GetColour(pStyle->m_TextColour);
        BYTE r = (BYTE)((iColour & 0x00FF0000) >> 16);
        BYTE g = (BYTE)((iColour & 0x0000FF00) >> 8);
        BYTE b = (BYTE)((iColour & 0x000000FF));
        (*pResult) = RGB(r,g,b);
      }
      break;
    case stylehint_BackColor:
      if (pResult)
      {
        int iColour = GetColour(pStyle->m_BackColour);
        BYTE r = (BYTE)((iColour & 0x00FF0000) >> 16);
        BYTE g = (BYTE)((iColour & 0x0000FF00) >> 8);
        BYTE b = (BYTE)((iColour & 0x000000FF));
        (*pResult) = RGB(r,g,b);
      }
      break;
    case stylehint_ReverseColor:
      if (pResult)
        *pResult = pStyle->m_ReverseColour;
      break;
    default:
      bMeasured = false;
      break;
    }
  }
  else
    bMeasured = false;

  return bMeasured;
}

bool CWinGlkWndTextBuffer::DrawGraphic(CWinGlkGraphic* pGraphic, int iValue1, int iValue2, int iWidth, int iHeight, bool& bDelete)
{
  if (pGraphic && pGraphic->m_pPixels && pGraphic->m_pHeader)
  {
    if (iWidth < 0)
      iWidth = pGraphic->m_pHeader->biWidth;
    if (iHeight < 0)
      iHeight = abs(pGraphic->m_pHeader->biHeight);

    pGraphic->m_iWidth = iWidth;
    pGraphic->m_iHeight = iHeight;
    pGraphic->m_iDisplay = iValue1;

    if (m_TextBuffer.empty())
      m_TextBuffer.push_back(new Paragraph(m_CurrentStyle));
    if (m_TextBuffer.back()->DrawGraphic(pGraphic))
    {
      bDelete = false;
      return true;
    }
  }

  bDelete = true;
  return false;
}

void CWinGlkWndTextBuffer::InsertFlowBreak(void)
{
  //XXXXDK
}

void CWinGlkWndTextBuffer::Scrollback(void)
{
  //XXXXDK
}

/////////////////////////////////////////////////////////////////////////////
// Helper classes

struct CWinGlkWndTextBuffer::MeasureState
{
  CWinGlkDC& dc;
  int spaceWidth;

  MeasureState(CWinGlkDC& dc_) : dc(dc_), spaceWidth(-1)
  {
  }
};

CWinGlkWndTextBuffer::ElementSpace::ElementSpace()
{
  m_Width = 0;
  m_JustifyWidth = 0;
  m_HeightAboveBase = 0;
  m_HeightBelowBase = 0;
}

void CWinGlkWndTextBuffer::ElementSpace::SetJustifyWidth(int jw)
{
  m_JustifyWidth = jw;
}

void CWinGlkWndTextBuffer::ElementSpace::Measure(MeasureState& ms)
{
  if (ms.spaceWidth < 0)
    ms.spaceWidth = ms.dc.GetTextExtent(L" ").cx;
  m_Width = ms.spaceWidth;
  m_JustifyWidth = 0;
  m_HeightAboveBase = ms.dc.m_FontMetrics.tmAscent;
  m_HeightBelowBase = ms.dc.m_FontMetrics.tmDescent;
}

void CWinGlkWndTextBuffer::ElementSpace::PrePaint(CWinGlkDC&) const
{
}

void CWinGlkWndTextBuffer::ElementSpace::PaintFore(CWinGlkDC&, int, int) const
{
}

void CWinGlkWndTextBuffer::ElementSpace::PaintBack(CWinGlkDC& dc, int x, int y, COLORREF defaultBack) const
{
  COLORREF styleBack = dc.GetBkColor();
  if (styleBack != defaultBack)
  {
	  CRect r(x,y - m_HeightAboveBase,x + m_Width + m_JustifyWidth,y + m_HeightBelowBase);
	  dc.ExtTextOut(0,0,ETO_OPAQUE,r,NULL,0,NULL);
  }
}

int CWinGlkWndTextBuffer::ElementSpace::Width() const
{
  return m_Width + m_JustifyWidth;
}

int CWinGlkWndTextBuffer::ElementSpace::HeightAboveBase() const
{
  return m_HeightAboveBase;
}

int CWinGlkWndTextBuffer::ElementSpace::HeightBelowBase() const
{
  return m_HeightBelowBase;
}

bool CWinGlkWndTextBuffer::ElementSpace::IsWordSeparator() const
{
  return true;
}

CWinGlkWndTextBuffer::ElementWord::ElementWord()
{
  m_Width = 0;
  m_HeightAboveBase = 0;
  m_HeightBelowBase = 0;
}

void CWinGlkWndTextBuffer::ElementWord::Measure(MeasureState& ms)
{
  m_Width = ms.dc.GetTextExtent(m_Text).cx;
  m_HeightAboveBase = ms.dc.m_FontMetrics.tmAscent;
  m_HeightBelowBase = ms.dc.m_FontMetrics.tmDescent;
}

void CWinGlkWndTextBuffer::ElementWord::PrePaint(CWinGlkDC&) const
{
}

void CWinGlkWndTextBuffer::ElementWord::PaintFore(CWinGlkDC& dc, int x, int y) const
{
  dc.SetBkMode(TRANSPARENT);
  dc.TextOut(x,y,m_Text);
}

void CWinGlkWndTextBuffer::ElementWord::PaintBack(CWinGlkDC& dc, int x, int y, COLORREF defaultBack) const
{
  COLORREF styleBack = dc.GetBkColor();
  if (styleBack != defaultBack)
  {
	  CRect r(x,y - m_HeightAboveBase,x + m_Width,y + m_HeightBelowBase);
	  dc.ExtTextOut(0,0,ETO_OPAQUE,r,NULL,0,NULL);
  }
}

int CWinGlkWndTextBuffer::ElementWord::Width() const
{
  return m_Width;
}

int CWinGlkWndTextBuffer::ElementWord::HeightAboveBase() const
{
  return m_HeightAboveBase;
}

int CWinGlkWndTextBuffer::ElementWord::HeightBelowBase() const
{
  return m_HeightBelowBase;
}

bool CWinGlkWndTextBuffer::ElementWord::IsWordSeparator() const
{
  return false;
}

CWinGlkWndTextBuffer::ElementStyle::ElementStyle(int style)
{
  m_Style = style;
  m_HeightAboveBase = 0;
  m_HeightBelowBase = 0;
}

int CWinGlkWndTextBuffer::ElementStyle::GetStyle()
{
  return m_Style;
}

void CWinGlkWndTextBuffer::ElementStyle::Measure(MeasureState& ms)
{
  ms.dc.SetStyle(m_Style,0);
  m_HeightAboveBase = ms.dc.m_FontMetrics.tmAscent;
  m_HeightBelowBase = ms.dc.m_FontMetrics.tmDescent;
}

void CWinGlkWndTextBuffer::ElementStyle::PrePaint(CWinGlkDC& dc) const
{
  dc.SetStyle(m_Style,0);
}

void CWinGlkWndTextBuffer::ElementStyle::PaintFore(CWinGlkDC&, int, int) const
{
}

void CWinGlkWndTextBuffer::ElementStyle::PaintBack(CWinGlkDC&, int, int, COLORREF) const
{
}

int CWinGlkWndTextBuffer::ElementStyle::Width() const
{
  return 0;
}

int CWinGlkWndTextBuffer::ElementStyle::HeightAboveBase() const
{
  return m_HeightAboveBase;
}

int CWinGlkWndTextBuffer::ElementStyle::HeightBelowBase() const
{
  return m_HeightBelowBase;
}

bool CWinGlkWndTextBuffer::ElementStyle::IsWordSeparator() const
{
  return false;
}

CWinGlkWndTextBuffer::ElementGraphic::ElementGraphic(CWinGlkGraphic* graphic)
{
  m_Graphic = graphic;
  m_HeightAboveBase = 0;
}

CWinGlkWndTextBuffer::ElementGraphic::~ElementGraphic()
{
  delete m_Graphic;
}

void CWinGlkWndTextBuffer::ElementGraphic::Measure(MeasureState& ms)
{
  switch (m_Graphic->m_iDisplay)
  {
  case imagealign_InlineUp:
    m_HeightAboveBase = m_Graphic->m_iHeight;
    break;
  case imagealign_InlineDown:
    m_HeightAboveBase = ms.dc.m_FontMetrics.tmAscent;
    break;
  case imagealign_InlineCenter:
    m_HeightAboveBase = (ms.dc.m_FontMetrics.tmAscent + m_Graphic->m_iHeight) / 2;
    break;
  }
}

void CWinGlkWndTextBuffer::ElementGraphic::PrePaint(CWinGlkDC& dc) const
{
}

void CWinGlkWndTextBuffer::ElementGraphic::PaintFore(CWinGlkDC& dc, int x, int y) const
{
  y -= HeightAboveBase();

  CDC dcTemp;
  dcTemp.CreateCompatibleDC(&dc);

  // Create a temporary DIBSection bitmap
  CDibSection bitmapTemp;
  bitmapTemp.CreateBitmap(dc.GetSafeHdc(),m_Graphic->m_iWidth,m_Graphic->m_iHeight);
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dcTemp,&bitmapTemp);

  // Is this graphic being scaled?
  bool scale = true;
  if (m_Graphic->m_iWidth == m_Graphic->m_pHeader->biWidth)
  {
    if (m_Graphic->m_iHeight == abs(m_Graphic->m_pHeader->biHeight))
      scale = false;
  }

  if (scale)
  {
    // Copy and stretch the graphic into the temporary bitmap
    ScaleGfx((COLORREF*)m_Graphic->m_pPixels,
      m_Graphic->m_pHeader->biWidth,
      abs(m_Graphic->m_pHeader->biHeight),
      bitmapTemp.GetBits(),
      m_Graphic->m_iWidth,m_Graphic->m_iHeight);
  }
  else
  {
    // Copy the bitmap directly
    ::GdiFlush();
    memcpy(bitmapTemp.GetBits(),m_Graphic->m_pPixels,
      m_Graphic->m_iWidth*m_Graphic->m_iHeight*4);
  }

  // Split the background colour into red, green and blue
  COLORREF back = dc.GetBkColor();
  int br = GetRValue(back);
  int bg = GetGValue(back);
  int bb = GetBValue(back);

  // Alpha blend each pixel of the graphic with the background colour
  DWORD colour;
  int pr, pg, pb, a;
  ::GdiFlush();
  for (int iy = 0; iy < m_Graphic->m_iHeight; iy++)
  {
    for (int ix = 0; ix < m_Graphic->m_iWidth; ix++)
    {
      // Get the colour of the pixel
      colour = bitmapTemp.GetPixel(ix,iy);

      if (m_Graphic->m_bAlpha)
      {
        // Split it into red, green, blue and alpha
        pb = colour & 0xFF;
        colour >>= 8;
        pg = colour & 0xFF;
        colour >>= 8;
        pr = colour & 0xFF;
        colour >>= 8;
        a = colour & 0xFF;

        // Perform alpha blending
        if (a == 0)
        {
          pr = br;
          pg = bg;
          pb = bb;
        }
        else if (a == 255)
        {
        }
        else
        {
          // Rescale from 0..255 to 0..256
          a += a>>7;

          pr += br - ((a * br) >> 8);
          pg += bg - ((a * bg) >> 8);
          pb += bb - ((a * bb) >> 8);
          if (pr > 0xFF)
            pr = 0xFF;
          if (pg > 0xFF)
            pg = 0xFF;
          if (pb > 0xFF)
            pb = 0xFF;
       }

        bitmapTemp.SetPixel(ix,iy,(pr<<16)|(pg<<8)|pb);
      }
      else
        bitmapTemp.SetPixel(ix,iy,colour);
    }
  }

  // Blit the alpha blended graphic onto the display
  dc.BitBlt(x,y,m_Graphic->m_iWidth,m_Graphic->m_iHeight,&dcTemp,0,0,SRCCOPY);
  dcTemp.SelectObject(oldBitmap);
}

void CWinGlkWndTextBuffer::ElementGraphic::PaintBack(CWinGlkDC&, int, int, COLORREF) const
{
}

int CWinGlkWndTextBuffer::ElementGraphic::Width() const
{
  return m_Graphic->m_iWidth;
}

int CWinGlkWndTextBuffer::ElementGraphic::HeightAboveBase() const
{
  return m_HeightAboveBase;
}

int CWinGlkWndTextBuffer::ElementGraphic::HeightBelowBase() const
{
  if (m_Graphic->m_iHeight > m_HeightAboveBase)
    return m_Graphic->m_iHeight - m_HeightAboveBase;
  return 0;
}

bool CWinGlkWndTextBuffer::ElementGraphic::IsWordSeparator() const
{
  return false;
}

CWinGlkWndTextBuffer::LayoutLine::LayoutLine(int leftX, int maxX)
{
  LeftX = leftX;
  MaxX = maxX;
  HeightAboveBase = 0;
  HeightBelowBase = 0;
}

CWinGlkWndTextBuffer::LayoutLine CWinGlkWndTextBuffer::LayoutLine::TakeLastWord(int& x, int maxX)
{
  LayoutLine newLine(x,maxX);

  // Find the last word separator
  int last = -1;
  for (int i = Elements.size()-1; i >= 0; i--)
  {
    if (Elements[i]->IsWordSeparator())
    {
      last = i;
      break;
    }
  }

  // Move the last word to the new line
  if (last >= 0)
  {
    for (size_t i = last+1; i < Elements.size(); i++)
    {
      ElementBase* element = Elements[i];
      newLine.Elements.push_back(element);
      x += element->Width();
    }
    Elements.resize(last);
  }
  return newLine;
}

CWinGlkWndTextBuffer::Paragraph::Paragraph(int style)
{
  m_NeedMeasure = true;
  SetStyle(style);
}

CWinGlkWndTextBuffer::Paragraph::~Paragraph()
{
  for (size_t i = 0; i < m_Elements.size(); i++)
    delete m_Elements[i];
}

void CWinGlkWndTextBuffer::Paragraph::PutCharacter(WCHAR c)
{
  if (c == ' ')
    m_Elements.push_back(new ElementSpace());
  else
  {
    ElementWord* last = dynamic_cast<ElementWord*>(GetLastElement());
    if (last == NULL)
    {
      last = new ElementWord();
      m_Elements.push_back(last);
    }
    last->PutCharacter(c);
  }
  m_NeedMeasure = true;
}

void CWinGlkWndTextBuffer::Paragraph::SetStyle(int style)
{
  m_Elements.push_back(new ElementStyle(style));
}

bool CWinGlkWndTextBuffer::Paragraph::DrawGraphic(CWinGlkGraphic* graphic)
{
  switch (graphic->m_iDisplay)
  {
  case imagealign_InlineUp:
  case imagealign_InlineDown:
  case imagealign_InlineCenter:
    m_Elements.push_back(new ElementGraphic(graphic));
    return true;

  case imagealign_MarginLeft:
  case imagealign_MarginRight:
    // Only allowed if the paragraph contains nothing but style changes
    for (size_t i = 0; i < m_Elements.size(); i++)
    {
      ElementStyle* element = dynamic_cast<ElementStyle*>(m_Elements[i]);
      if (element == NULL)
        return false;
    }
    m_Elements.push_back(new ElementGraphic(graphic));
    return true;

  default:
    return false;
  }
}

void CWinGlkWndTextBuffer::Paragraph::Layout(CWinGlkDC& dc, LayoutResult& result, int width)
{
  if (m_NeedMeasure)
  {
    // Measure the dimensions of all the elements in the paragraph
    MeasureState ms(dc);
    for (size_t i = 0; i < m_Elements.size(); i++)
      m_Elements[i]->Measure(ms);
    m_NeedMeasure = false;
  }

  // Find the style for paragraph properties
  CWinGlkStyle* paraStyle = NULL;
  for (size_t i = 0; i < m_Elements.size(); i++)
  {
    ElementStyle* element = dynamic_cast<ElementStyle*>(m_Elements[i]);
    if (element != NULL)
      paraStyle = dc.GetStyleFromWindow(element->GetStyle());
    else
      break;
  }
  int in = 0, in1 = 0, just = stylehint_just_LeftFlush;
  if (paraStyle)
  {
    in = paraStyle->m_Indent * m_IndentStep;
    in1 = paraStyle->m_ParaIndent * m_IndentStep;
    just = paraStyle->m_Justify;
  }

  // Lay out the elements of the paragraph in lines
  int x = in + in1;
  int maxX = width - in;
  LayoutLine line(x,maxX);
  size_t firstNewLine = result.Lines.size();
  for (size_t i = 0; i < m_Elements.size(); i++)
  {
    ElementBase* element = m_Elements[i];

    // Split the line if it will be too long
    if ((x > maxX) && element->IsWordSeparator())
    {
      result.Lines.push_back(line);
      x = in;
      line = result.Lines.back().TakeLastWord(x,maxX);
    }

    line.Elements.push_back(element);
    x += element->Width();
  }

  // Add the last line, splitting it if needed
  if (!line.Elements.empty())
  {
    result.Lines.push_back(line);
    if (x > maxX)
    {
      x = in;
      line = result.Lines.back().TakeLastWord(x,maxX);
      if (!line.Elements.empty())
        result.Lines.push_back(line);
    }
  }

  // Run over the newly added lines
  for (size_t i = firstNewLine; i < result.Lines.size(); i++)
  {
    LayoutLine& line = result.Lines[i];

    // Justify the line
    int w = 0, sc = 0;
    for (size_t j = 0; j < line.Elements.size(); j++)
    {
      ElementSpace* es = dynamic_cast<ElementSpace*>(line.Elements[j]);
      if (es != NULL)
      {
        sc++;
        es->SetJustifyWidth(0);
      }
      w += line.Elements[j]->Width();
    }
    switch (just)
    {
    case stylehint_just_LeftFlush:
      break;
    case stylehint_just_LeftRight:
      {
        int spaceLeft = line.MaxX - line.LeftX - w;
        for (size_t j = 0; j < line.Elements.size(); j++)
        {
          ElementSpace* es = dynamic_cast<ElementSpace*>(line.Elements[j]);
          if ((es != NULL) && (sc > 0))
          {
            int jw = spaceLeft / sc;
            es->SetJustifyWidth(jw);
            spaceLeft -= jw;
            sc--;
          }
        }
      }
      break;
    case stylehint_just_Centered:
      line.LeftX += (line.MaxX - line.LeftX - w) / 2;
      break;
    case stylehint_just_RightFlush:
      line.LeftX += line.MaxX - line.LeftX - w;
      break;
    }

    // Set the height of the line
    for (size_t j = 0; j < line.Elements.size(); j++)
    {
      ElementBase* element = line.Elements[j];
      if (line.HeightAboveBase < element->HeightAboveBase())
        line.HeightAboveBase = element->HeightAboveBase();
      if (line.HeightBelowBase < element->HeightBelowBase())
        line.HeightBelowBase = element->HeightBelowBase();
    }
  }
}

void CWinGlkWndTextBuffer::Paragraph::ClearMeasure()
{
  m_NeedMeasure = true;
}

CWinGlkWndTextBuffer::ElementBase* CWinGlkWndTextBuffer::Paragraph::GetLastElement()
{
  return m_Elements.empty() ? NULL : m_Elements.back();
}

/////////////////////////////////////////////////////////////////////////////
// Message handlers

void CWinGlkWndTextBuffer::OnPaint(void)
{
  CPaintDC dcPaint(this);
  CWinGlkBufferDC dcMem(this);
  InitDC(dcMem,&dcPaint);
  dcMem.SetTextAlign(TA_BASELINE);

  // Set up a bitmap in memory to draw into
  CBitmap bmp;
  CRect ClientArea;
  GetClientRect(ClientArea);
  CSize size(ClientArea.Width(),ClientArea.Height());
  if (bmp.CreateCompatibleBitmap(&dcPaint,size.cx,size.cy) == FALSE)
    return;
  CBitmap* pbmpOld = dcMem.SelectObject(&bmp);

  // Measure and layout the contents of the window
  LayoutResult layout;
  for (size_t i = 0; i < m_TextBuffer.size(); i++)
    m_TextBuffer[i]->Layout(dcMem,layout,ClientArea.Width());

  // Clear the window
  COLORREF backColour = GetColour(GetStyle(style_Normal)->m_BackColour);
  dcMem.FillSolidRect(ClientArea,backColour);

  // Find the vertical origin
  int height = 0;
  for (size_t i = 0; i < layout.Lines.size(); i++)
  {
    const LayoutLine& line = layout.Lines[i];
    height += (line.HeightAboveBase + line.HeightBelowBase);
  }
  int originY = 0;
  if (height > ClientArea.Height())
    originY = ClientArea.Height() - height;

  // Draw the background contents of the window
  int x = 0, y = originY;
  for (size_t i = 0; i < layout.Lines.size(); i++)
  {
    const LayoutLine& line = layout.Lines[i];
    x = line.LeftX;
    int lineHeight = line.HeightAboveBase + line.HeightBelowBase;

    for (size_t j = 0; j < line.Elements.size(); j++)
    {
      const ElementBase* element = line.Elements[j];
      element->PrePaint(dcMem);
      if ((y + lineHeight > 0) && (y < ClientArea.Height()))
        element->PaintBack(dcMem,x,y + line.HeightAboveBase,backColour);
      x += element->Width();
    }
    if (i < layout.Lines.size()-1)
      y += lineHeight;
  }

  // Draw the foreground contents of the window
  y = originY;
  int aboveBase = 0;
  for (size_t i = 0; i < layout.Lines.size(); i++)
  {
    const LayoutLine& line = layout.Lines[i];
    x = line.LeftX;
    int lineHeight = line.HeightAboveBase + line.HeightBelowBase;
    aboveBase = line.HeightAboveBase;

    for (size_t j = 0; j < line.Elements.size(); j++)
    {
      const ElementBase* element = line.Elements[j];
      element->PrePaint(dcMem);
      if ((y + lineHeight > 0) && (y < ClientArea.Height()))
        element->PaintFore(dcMem,x,y + line.HeightAboveBase);
      x += element->Width();
    }
    if (i < layout.Lines.size()-1)
      y += lineHeight;
  }

  // Draw the input line
  if (m_bInputActive && m_pLineBuffer)
  {
    x++;
    dcMem.SetStyle(style_Input,0);

    CStringW inputLineU;
    if (m_bInputUnicode)
    {
      inputLineU = GetUniLineBuffer();
      dcMem.TextOut(x,y+aboveBase,(LPCWSTR)inputLineU,inputLineU.GetLength());
      x += dcMem.GetTextExtent((LPCWSTR)inputLineU,m_iLinePos).cx;
    }
    else
    {
      dcMem.TextOut(x,y+aboveBase,(LPCSTR)m_pLineBuffer,m_iLineEnd);
      x += dcMem.GetTextExtent((LPCSTR)m_pLineBuffer,m_iLinePos).cx;
    }
  }
  if (GetActiveWindow() == this)
    SetCaretPos(CPoint(x,y+aboveBase-dcMem.m_FontMetrics.tmAscent));

  // Copy the bitmap into the window
  dcPaint.BitBlt(0,0,size.cx,size.cy,&dcMem,0,0,SRCCOPY);
  dcMem.SelectObject(pbmpOld);
}

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

void CWinGlkWndTextBuffer::SetStyleHint(int iStyle, int iHint, glsi32 Value)
{
  CWinGlkStyle* pStyle = m_DefaultTextBufferStyles.GetStyle(iStyle);
  if (pStyle)
  {
    switch (iHint)
    {
    case stylehint_Indentation:
      pStyle->m_Indent = Value;
      break;
    case stylehint_ParaIndentation:
      pStyle->m_ParaIndent = Value;
      break;
    case stylehint_Justification:
      pStyle->m_Justify = Value;
      break;
    case stylehint_Size:
      pStyle->m_Size = Value;
      break;
    case stylehint_Weight:
      pStyle->m_Weight = Value;
      break;
    case stylehint_Oblique:
      pStyle->m_Oblique = Value;
      break;
    case stylehint_Proportional:
      pStyle->m_Proportional = Value;
      break;
    case stylehint_TextColor:
      pStyle->m_TextColour = Value;
      break;
    case stylehint_BackColor:
      pStyle->m_BackColour = Value;
      break;
    case stylehint_ReverseColor:
      pStyle->m_ReverseColour = Value;
      break;
    }
    pStyle->m_bUserControl = false;
  }
}

void CWinGlkWndTextBuffer::ClearStyleHint(int iStyle, int iHint)
{
  CWinGlkStyle* pStyle = m_DefaultTextBufferStyles.GetStyle(iStyle);
  CWinGlkStyle* pNoHintStyle = m_DefaultTextBufferStyles.GetNoHintStyle(iStyle);

  if (pStyle && pNoHintStyle)
  {
    switch (iHint)
    {
    case stylehint_Indentation:
      pStyle->m_Indent = pNoHintStyle->m_Indent;
      break;
    case stylehint_ParaIndentation:
      pStyle->m_ParaIndent = pNoHintStyle->m_ParaIndent;
      break;
    case stylehint_Justification:
      pStyle->m_Justify = pNoHintStyle->m_Justify;
      break;
    case stylehint_Size:
      pStyle->m_Size = pNoHintStyle->m_Size;
      break;
    case stylehint_Weight:
      pStyle->m_Weight = pNoHintStyle->m_Weight;
      break;
    case stylehint_Oblique:
      pStyle->m_Oblique = pNoHintStyle->m_Oblique;
      break;
    case stylehint_Proportional:
      pStyle->m_Proportional = pNoHintStyle->m_Proportional;
      break;
    case stylehint_TextColor:
      pStyle->m_TextColour = pNoHintStyle->m_TextColour;
      break;
    case stylehint_BackColor:
      pStyle->m_BackColour = pNoHintStyle->m_BackColour;
      break;
    case stylehint_ReverseColor:
      pStyle->m_ReverseColour = pNoHintStyle->m_ReverseColour;
      break;
    }
  }
}

CWinGlkStyles CWinGlkWndTextBuffer::m_DefaultTextBufferStyles;
CStringW CWinGlkWndTextBuffer::m_strMore;
const int CWinGlkWndTextBuffer::Paragraph::m_IndentStep = 4;

/////////////////////////////////////////////////////////////////////////////
// Device context class
/////////////////////////////////////////////////////////////////////////////

void CWinGlkBufferDC::GetFonts(LOGFONT*& pTextFont, LOGFONT*& pSizeFont)
{
  CGlkApp* pApp = (CGlkApp*)AfxGetApp();

  if (m_Style.m_Proportional)
    pTextFont = pApp->GetPropFont();
  else
    pTextFont = pApp->GetFixedFont();
  pSizeFont = pApp->GetPropFont();
}

void CWinGlkBufferDC::SetFontStyles(LOGFONT& Font)
{
  if (m_Style.m_Weight < 0)
    Font.lfWeight = FW_THIN;
  else if (m_Style.m_Weight > 0)
    Font.lfWeight = FW_BOLD;
  if (m_Style.m_Oblique > 0)
    Font.lfItalic = TRUE;
}

int CWinGlkBufferDC::GetStyleFontSize(void)
{
  return m_Style.m_Size;
}
