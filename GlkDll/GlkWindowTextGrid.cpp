/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkWindowTextGrid
// Text grid Glk windows
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkDialogs.h"
#include "GlkDll.h"
#include "GlkMainWnd.h"
#include "GlkStream.h"
#include "GlkWindowTextGrid.h"
#include "WinGlk.h"

#include <math.h>
#include <string.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Text grid Glk windows
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkWndTextGrid,CWinGlkWnd);

BEGIN_MESSAGE_MAP(CWinGlkWndTextGrid, CWinGlkWnd)
  //{{AFX_MSG_MAP(CWinGlkWndTextGrid)
  ON_WM_PAINT()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CWinGlkWndTextGrid::CWinGlkWndTextGrid(glui32 Rock) : CWinGlkWnd(Rock)
{
  m_iCursorX = 0;
  m_iCursorY = 0;
  m_iCurrentStyle = style_Normal;
  m_iCurrentLink = 0;
  m_Styles = m_DefaultTextGridStyles;
}

CWinGlkWndTextGrid::~CWinGlkWndTextGrid()
{
}

void CWinGlkWndTextGrid::InitDC(CWinGlkDC& dc, CDC* pdcCompat)
{
  if (pdcCompat)
    dc.CreateCompatibleDC(pdcCompat);

  // Set colours
  dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
  dc.SetBkColor(GetColour(GetStyle(style_Normal)->m_BackColour));

  dc.SetStyle(style_Normal,false,NULL);
}

int CWinGlkWndTextGrid::GetCaretHeight(void)
{
  CWinGlkGridDC dc(this,GetStyle(style_Normal)->m_Size);
  CDC* pWndDC = GetDC();
  dc.Attach(*pWndDC);
  InitDC(dc);

  int iHeight = dc.m_FontMetrics.tmHeight;
  ReleaseDC(pWndDC);
  return iHeight;
}

void CWinGlkWndTextGrid::SizeWindow(CRect* pSize)
{
  CWinGlkWnd::SizeWindow(pSize);

  int iNewWidth = 0;
  int iNewHeight = 0;
  GetSize(iNewWidth,iNewHeight);

  // Resize the text grid data
  m_TextGrid.SetSize(iNewHeight);
  for (int i = 0; i < iNewHeight; i++)
  {
    LPWSTR pszRow = m_TextGrid[i].GetBuffer(iNewWidth);
    BOOL bText = TRUE;
    for (int j = 0; j < iNewWidth; j++)
    {
      if (bText)
      {
        if (pszRow[j] == L'\0')
        {
          pszRow[j] = L' ';
          m_TextGrid[i].SetStyle(j,style_Normal);
          bText = FALSE;
        }
      }
      else
      {
        pszRow[j] = L' ';
        m_TextGrid[i].SetStyle(j,style_Normal);
      }
    }
    m_TextGrid[i].ReleaseBuffer(iNewWidth);
  }
}

void CWinGlkWndTextGrid::ClearWindow(void)
{
  m_iCursorX = 0;
  m_iCursorY = 0;

  CTextColours defaultColours;
  for (int i = 0; i < m_TextGrid.GetSize(); i++)
  {
    CGridRow& Row = m_TextGrid[i];
    for (int j = 0; j < Row.GetLength(); j++)
    {
      Row.SetChar(j,L' ');
      Row.SetStyle(j,style_Normal);
      Row.SetLink(j,0);
      Row.SetColours(j,defaultColours);
    }
  }
}

void CWinGlkWndTextGrid::PutCharacter(glui32 c)
{
  CWinGlkWnd::PutCharacter(c);

  if ((m_iCursorY >= 0) && (m_iCursorY < m_TextGrid.GetSize()))
  {
    CGridRow& Row = m_TextGrid[m_iCursorY];
    if ((m_iCursorX >= 0) && (m_iCursorX < Row.GetLength()))
    {
      if (c == L'\n')
      {
        m_iCursorX = 0;
        m_iCursorY++;
      }
      else
      {
        if ((c >= 32 && c <= 126) || (c >= 160 && c <= 0xFFFF))
        {
          Row.SetChar(m_iCursorX,(wchar_t)c);
          Row.SetStyle(m_iCursorX,m_iCurrentStyle);
          Row.SetLink(m_iCursorX,m_iCurrentLink);
          Row.SetColours(m_iCursorX,m_CurrentColours);
          m_iCursorX++;
          if (m_iCursorX >= Row.GetLength())
          {
            m_iCursorX = 0;
            m_iCursorY++;
          }
        }
        else
        {
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
  }
}

void CWinGlkWndTextGrid::MoveCursor(int x, int y)
{
  m_iCursorX = x;
  m_iCursorY = y;
}

void CWinGlkWndTextGrid::GetSize(int& iWidth, int& iHeight)
{
  // Measure the size of the new window
  CRect ClientArea;
  GetClientRect(ClientArea);

  CWinGlkGridDC dc(this,GetStyle(style_Normal)->m_Size);
  CDC* pWndDC = GetDC();
  dc.Attach(*pWndDC);
  InitDC(dc);

  iWidth = ClientArea.Width() / dc.m_FontMetrics.tmAveCharWidth;
  iHeight = ClientArea.Height() / dc.m_FontMetrics.tmHeight;

  ReleaseDC(pWndDC);
}

void CWinGlkWndTextGrid::GetNeededSize(int iSize, int& iWidth, int& iHeight)
{
  CWinGlkGridDC dc(this,GetStyle(style_Normal)->m_Size);
  CDC* pWndDC = GetDC();
  dc.Attach(*pWndDC);
  InitDC(dc);

  iWidth = dc.m_FontMetrics.tmAveCharWidth*iSize;
  iHeight = dc.m_FontMetrics.tmHeight*iSize;

  ReleaseDC(pWndDC);

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

  if (iSize > 0)
  {
    iWidth += iBorderWidth;
    iHeight += iBorderHeight;
  }
}

void CWinGlkWndTextGrid::StartLinkEvent(void)
{
  m_bLinksActive = true;
}

void CWinGlkWndTextGrid::EndLinkEvent(void)
{
  m_bLinksActive = false;
}

bool CWinGlkWndTextGrid::AllowMoreLineInput(void)
{
  int iWidth, iHeight;
  GetSize(iWidth,iHeight);

  if (m_iCursorX + m_iLineEnd < iWidth - 1)
    return true;
  return false;
}

void CWinGlkWndTextGrid::StartMouseEvent(void)
{
  m_bMouseActive = true;
}

bool CWinGlkWndTextGrid::MouseClick(CPoint& Click)
{
  // First check if the mouse is over a hyperlink
  if (m_bLinksActive)
  {
    unsigned int iLink = GetLinkAtPoint(Click);
    if (iLink != 0)
    {
      ((CGlkApp*)AfxGetApp())->AddEvent(evtype_Hyperlink,(winid_t)this,iLink,0);
      m_bLinksActive = false;
      return true;
    }
  }

  // Now check if a mouse event is requested
  if (m_bMouseActive)
  {
    CWinGlkGridDC dc(this,GetStyle(style_Normal)->m_Size);
    CDC* pWndDC = GetDC();
    dc.Attach(*pWndDC);
    InitDC(dc);

    int iWidth = dc.m_FontMetrics.tmAveCharWidth;
    int iHeight = dc.m_FontMetrics.tmHeight;

    ReleaseDC(pWndDC);

    ((CGlkApp*)AfxGetApp())->AddEvent(evtype_MouseInput,(winid_t)this,
      Click.x/iWidth,Click.y/iHeight);
    m_bMouseActive = false;
    return true;
  }
  return false;
}

unsigned int CWinGlkWndTextGrid::GetLinkAtPoint(const CPoint& Point)
{
  CWinGlkGridDC dc(this,GetStyle(style_Normal)->m_Size);
  CDC* pWndDC = GetDC();
  dc.Attach(*pWndDC);
  InitDC(dc);

  int x = Point.x / dc.m_FontMetrics.tmAveCharWidth;
  int y = Point.y / dc.m_FontMetrics.tmHeight;

  if ((y >= 0) && (y < m_TextGrid.GetSize()))
  {
    const CGridRow& Row = m_TextGrid[y];
    if ((x >= 0) && (x < Row.GetLength()))
      return Row.GetLink(x);
  }
  return 0;
}

void CWinGlkWndTextGrid::SetStyle(int iStyle)
{
  m_iCurrentStyle = iStyle;
}

int CWinGlkWndTextGrid::GetStyle(void)
{
  return m_iCurrentStyle;
}

CWinGlkStyle* CWinGlkWndTextGrid::GetStyle(int iStyle)
{
  return m_Styles.GetStyle(iStyle);
}

void CWinGlkWndTextGrid::SetHyperlink(unsigned int iLink)
{
  m_iCurrentLink = iLink;
}

void CWinGlkWndTextGrid::SetTextColours(glui32 fg, glui32 bg)
{
  if (fg != zcolor_Current)
    m_CurrentColours.fore = fg;
  if (bg != zcolor_Current)
    m_CurrentColours.back = bg;
}

void CWinGlkWndTextGrid::SetTextReverse(bool reverse)
{
  m_CurrentColours.reverse = reverse;
}

bool CWinGlkWndTextGrid::DistinguishStyles(int iStyle1, int iStyle2)
{
  CWinGlkStyle* pStyle1 = GetStyle(iStyle1);
  CWinGlkStyle* pStyle2 = GetStyle(iStyle2);

  if (pStyle1 && pStyle2)
  {
    if (GetColour(pStyle1->m_TextColour) != GetColour(pStyle2->m_TextColour))
      return true;
    if (GetColour(pStyle1->m_BackColour) != GetColour(pStyle2->m_BackColour))
      return true;
    if (GetColour(pStyle1->m_ReverseColour) != GetColour(pStyle2->m_ReverseColour))
      return true;
  }
  return false;
}

bool CWinGlkWndTextGrid::MeasureStyle(int iStyle, int iHint, glui32* pResult)
{
  bool bMeasured = true;

  CWinGlkStyle* pStyle = GetStyle(iStyle);
  if (pStyle)
  {
    switch (iHint)
    {
    case stylehint_Indentation:
    case stylehint_ParaIndentation:
    case stylehint_Weight:
    case stylehint_Oblique:
    case stylehint_Proportional:
      if (pResult)
        *pResult = 0;
      break;
    case stylehint_Size:
      if (pResult)
        *pResult = pStyle->m_Size;
      break;
    case stylehint_Justification:
      if (pResult)
        *pResult = stylehint_just_LeftFlush;
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

/////////////////////////////////////////////////////////////////////////////
// Message handlers

void CWinGlkWndTextGrid::OnPaint(void)
{
  CPaintDC dcPaint(this);
  CWinGlkGridDC dcMem(this,GetStyle(style_Normal)->m_Size);
  InitDC(dcMem,&dcPaint);

  CBitmap bmp;
  CRect ClientArea;
  GetClientRect(ClientArea);
  CSize size(ClientArea.Width(),ClientArea.Height());
  if (bmp.CreateCompatibleBitmap(&dcPaint,size.cx,size.cy) == FALSE)
    return;
  CBitmap* pbmpOld = dcMem.SelectObject(&bmp);

  // Clear the window
  CWinGlkStyle* pNormal = GetStyle(style_Normal);
  dcMem.FillSolidRect(ClientArea,GetColour(
    pNormal->m_ReverseColour ? pNormal->m_TextColour : pNormal->m_BackColour));

  // Draw the text
  int y = 0;
  for (int i = 0; i < m_TextGrid.GetSize(); i++)
  {
    m_TextGrid[i].DrawRow(dcMem,0,y);
    y += dcMem.m_FontMetrics.tmHeight;
  }

  m_iLineX = m_iCursorX * dcMem.m_FontMetrics.tmAveCharWidth;
  m_iLineY = m_iCursorY * dcMem.m_FontMetrics.tmHeight;

  // Add any line input
  if (m_bInputActive)
  {
    if (m_pLineBuffer)
    {
      if (m_bInputUnicode)
        dcMem.TextOut(m_iLineX,m_iLineY,GetUniLineBuffer());
      else
        dcMem.TextOut(m_iLineX,m_iLineY,(char*)m_pLineBuffer,m_iLineEnd);
      m_iLineX += m_iLinePos * dcMem.m_FontMetrics.tmAveCharWidth;
    }
    if (GetActiveWindow() == this)
      SetCaretPos(CPoint(m_iLineX,m_iLineY));
  }

  dcPaint.BitBlt(0,0,size.cx,size.cy,&dcMem,0,0,SRCCOPY);
  dcMem.SelectObject(pbmpOld);
}

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

void CWinGlkWndTextGrid::SetStyleHint(int iStyle, int iHint, glsi32 Value)
{
  CWinGlkStyle* pStyle = m_DefaultTextGridStyles.GetStyle(iStyle);
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

void CWinGlkWndTextGrid::ClearStyleHint(int iStyle, int iHint)
{
  CWinGlkStyle* pStyle = m_DefaultTextGridStyles.GetStyle(iStyle);
  CWinGlkStyle* pNoHintStyle = m_DefaultTextGridStyles.GetNoHintStyle(iStyle);

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

CWinGlkStyles CWinGlkWndTextGrid::m_DefaultTextGridStyles;

/////////////////////////////////////////////////////////////////////////////
// Helper classes

CWinGlkWndTextGrid::CGridCellInfo::CGridCellInfo()
{
  m_iStyle = style_Normal;
  m_iLink = 0;
}

bool CWinGlkWndTextGrid::CGridCellInfo::operator==(const CGridCellInfo& Info)
{
  if (m_iStyle != Info.m_iStyle)
    return false;
  if (m_iLink != Info.m_iLink)
    return false;
  if (m_Colours != Info.m_Colours)
    return false;
  return true;
}

bool CWinGlkWndTextGrid::CGridCellInfo::operator!=(const CGridCellInfo& Info)
{
  return (operator==)(Info) ? false : true;
}

void CWinGlkWndTextGrid::CGridRow::DrawRow(CWinGlkDC& dc, int x, int y)
{
  CGridCellInfo Info;
  if (m_RowInfo.GetSize() > 0)
    Info = m_RowInfo[0];
  dc.SetStyle(Info.GetStyle(),Info.GetLink(),Info.GetColours());

  int i1 = 0, i2 = 1;
  while (i1+i2 < GetLength())
  {
    if (m_RowInfo[i1+i2] != Info)
    {
      dc.TextOut(x,y,GetString()+i1,i2);
      x += dc.m_FontMetrics.tmAveCharWidth * i2;

      Info = m_RowInfo[i1+i2];
      dc.SetStyle(Info.GetStyle(),Info.GetLink(),Info.GetColours());

      i1 += i2;
      i2 = 0;
    }
    i2++;
  }

  // Add last section of the row
  dc.TextOut(x,y,GetString()+i1,i2);
}

LPWSTR CWinGlkWndTextGrid::CGridRow::GetBuffer(int iNewLength)
{
  m_RowInfo.SetSize(iNewLength);
  return m_strRow.GetBufferSetLength(iNewLength);
}

void CWinGlkWndTextGrid::CGridRow::ReleaseBuffer(int iNewLength)
{
  m_strRow.ReleaseBuffer(iNewLength);
}

int CWinGlkWndTextGrid::CGridRow::GetLength(void) const
{
  return m_strRow.GetLength();
}

void CWinGlkWndTextGrid::CGridRow::SetChar(int i, wchar_t c)
{
  m_strRow.SetAt(i,c);
}

LPCWSTR CWinGlkWndTextGrid::CGridRow::GetString(void) const
{
  return m_strRow;
}

void CWinGlkWndTextGrid::CGridRow::SetStyle(int i, int iStyle)
{
  if (i < m_RowInfo.GetSize())
    m_RowInfo[i].SetStyle(iStyle);
}

void CWinGlkWndTextGrid::CGridRow::SetLink(int i, unsigned int iLink)
{
  if (i < m_RowInfo.GetSize())
    m_RowInfo[i].SetLink(iLink);
}

void CWinGlkWndTextGrid::CGridRow::SetColours(int i, const CTextColours& colours)
{
  if (i < m_RowInfo.GetSize())
    m_RowInfo[i].SetColours(colours);
}

unsigned int CWinGlkWndTextGrid::CGridRow::GetLink(int i) const
{
  if (i < m_RowInfo.GetSize())
    return m_RowInfo[i].GetLink();
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Device context class
/////////////////////////////////////////////////////////////////////////////

CWinGlkGridDC::CWinGlkGridDC(CWinGlkWnd* pWnd, int iSize) : CWinGlkDC(pWnd)
{
  m_iSize = iSize;
}

void CWinGlkGridDC::GetFonts(LOGFONT*& pTextFont, LOGFONT*& pSizeFont)
{
  CGlkApp* pApp = (CGlkApp*)AfxGetApp();

  pTextFont = pApp->GetFixedFont();
  pSizeFont = pApp->GetFixedFont();
}

void CWinGlkGridDC::SetFontStyles(LOGFONT& Font)
{
}

int CWinGlkGridDC::GetStyleFontSize(void)
{
  return m_iSize;
}
