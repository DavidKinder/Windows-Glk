/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkWindowTextBuffer
// Text buffer Glk windows
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkDialogs.h"
#include "GlkDll.h"
#include "GlkMainWnd.h"
#include "GlkStream.h"
#include "GlkWindowTextBuffer.h"
#include "WinGlk.h"

#include <math.h>
#include <string.h>

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

  m_iCurrentStyle = style_Normal;
  m_iCurrentLink = 0;
  m_Styles = m_DefaultTextBufferStyles;
  m_bCheckDeleteText = false;
  m_bMorePending = false;
  m_bNextEchoInput = true;
  m_ScrollBuffer.SetSize(0,10000);
}

CWinGlkWndTextBuffer::~CWinGlkWndTextBuffer()
{
  for (int i = 0; i < m_TextBuffer.GetSize(); i++)
    delete m_TextBuffer[i];
}

void CWinGlkWndTextBuffer::SetActiveWindow(void)
{
  CWinGlkWnd::SetActiveWindow();

  CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetApp()->GetMainWnd();
  if (pMainWnd)
    pMainWnd->EnableScrollback(true);
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

  // Set colours
  dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
  dc.SetBkColor(GetColour(GetStyle(style_Normal)->m_BackColour));

  dc.SetStyle(style_Normal,0,NULL);
}

bool CWinGlkWndTextBuffer::CheckMorePending(bool update)
{
  if (update && m_bMorePending)
  {
    // Get the last paragraph and character that have been shown
    int iLastPara1, iLastChar1;
    GetLastShown(iLastPara1,iLastChar1);

    // This function is called as part of the messaging
    // handling for a key being input. If a [More] prompt
    // is pending, the marker for the last visible piece
    // of text is moved forward by calling Paint() with
    // the 'mark' flag set.
    Paint(true);

    // If the last paragraph and character shown has not advanced,
    // there must be something like a picture bigger than the display
    // preventing the advance, so just mark everything as shown.
    int iLastPara2, iLastChar2;
    GetLastShown(iLastPara2,iLastChar2);
    if ((iLastPara1 == iLastPara2) && (iLastChar1 == iLastChar2))
    {
      if (m_TextBuffer.GetSize() > 0)
        m_TextBuffer[m_TextBuffer.GetUpperBound()]->SetAsShown();
    }

    // Now just call Invalidate() to trigger another redraw,
    // this time on the main window, which will use the
    // updated marker.
    AfxGetApp()->GetMainWnd()->Invalidate();
  }
  return m_bMorePending;
}

int CWinGlkWndTextBuffer::GetCaretHeight(void)
{
  CWinGlkBufferDC dc(this);
  CDC* pWndDC = GetDC();
  dc.Attach(*pWndDC);
  InitDC(dc);
  dc.SetStyle(style_Input,0,NULL);

  int iHeight = dc.m_FontMetrics.tmHeight;
  ReleaseDC(pWndDC);
  return iHeight;
}

void CWinGlkWndTextBuffer::SizeWindow(CRect* pSize)
{
  CWinGlkWnd::SizeWindow(pSize);
  ClearFormatting(-1);
}

void CWinGlkWndTextBuffer::ClearWindow(void)
{
  for (int i = 0; i < m_TextBuffer.GetSize(); i++)
    delete m_TextBuffer[i];
  m_TextBuffer.RemoveAll();
}

void CWinGlkWndTextBuffer::PutCharacter(glui32 c)
{
  CWinGlkWnd::PutCharacter(c);

  if (c == L'\n')
  {
    // Speak the current paragraph
    if (m_TextBuffer.GetSize() > 0)
      m_TextBuffer[m_TextBuffer.GetUpperBound()]->Speak();

    // Add a new paragraph
    if (m_TextBuffer.GetSize() == 0)
      AddNewParagraph();
    AddNewParagraph();

    m_ScrollBuffer.Add(L'\r');
    m_ScrollBuffer.Add(L'\n');
  }
  else
  {
    if ((c >= 32 && c <= 126) || (c >= 160 && c <= 0xFFFF))
    {
      // Valid character
      if (m_TextBuffer.GetSize() == 0)
        AddNewParagraph();
      int last = m_TextBuffer.GetUpperBound();
      m_TextBuffer[last]->AddCharacter((wchar_t)c);
      m_ScrollBuffer.Add((wchar_t)c);

      // Invalidate the formatting for this paragraph
      ClearFormatting(last);
    }
    else if (c >= 0x10000 && c <= 0x10FFFF)
    {
      // Unicode high-plane character
      const UINT32 LEAD_OFFSET = 0xD800 - (0x10000 >> 10);
      UINT16 hi = (UINT16)(LEAD_OFFSET + (c >> 10));
      UINT16 lo = (UINT16)(0xDC00 + (c & 0x3FF));
      if (m_TextBuffer.GetSize() == 0)
        AddNewParagraph();
      int last = m_TextBuffer.GetUpperBound();
      m_TextBuffer[last]->AddCharacter((wchar_t)hi);
      m_TextBuffer[last]->AddCharacter((wchar_t)lo);
      m_ScrollBuffer.Add((wchar_t)hi);
      m_ScrollBuffer.Add((wchar_t)lo);
      ClearFormatting(last);
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
  CDC* pWndDC = GetDC();
  dc.Attach(*pWndDC);
  InitDC(dc);

  iWidth = ClientArea.Width() / dc.m_FontMetrics.tmAveCharWidth;
  iHeight = ClientArea.Height() / dc.m_FontMetrics.tmHeight;

  ReleaseDC(pWndDC);
}

void CWinGlkWndTextBuffer::GetNeededSize(int iSize, int& iWidth, int& iHeight)
{
  CWinGlkBufferDC dc(this);
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

void CWinGlkWndTextBuffer::StartLineEvent(void* pBuffer, bool bUnicode, int iMaxLength, int iStartLength)
{
  if (m_bInputActive == false)
  {
    if (m_TextBuffer.GetSize() > 0)
    {
      // Speak the current paragraph
      m_TextBuffer[m_TextBuffer.GetUpperBound()]->Speak();

      // Mark the current paragraph as having been associated with input
      m_TextBuffer[m_TextBuffer.GetUpperBound()]->HasHadInput();
    }
    m_bEchoInput = m_bNextEchoInput;
  }
  CWinGlkWnd::StartLineEvent(pBuffer,bUnicode,iMaxLength,iStartLength);
}

void CWinGlkWndTextBuffer::StartCharEvent(bool unicode)
{
  if (m_bInputActive == false)
  {
    if (m_TextBuffer.GetSize() > 0)
      m_TextBuffer[m_TextBuffer.GetUpperBound()]->HasHadInput();
  }
  CWinGlkWnd::StartCharEvent(unicode);
}

void CWinGlkWndTextBuffer::StartLinkEvent(void)
{
  m_bLinksActive = true;
}

void CWinGlkWndTextBuffer::EndLinkEvent(void)
{
  m_bLinksActive = false;
}

void CWinGlkWndTextBuffer::InputChar(unsigned long InputChar)
{
  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  if (pApp->GetCanSpeak())
  {
    if ((m_bMorePending == false) && m_bInputActive)
    {
      if (m_pLineBuffer && (InputChar == L'\r') && (m_iLineEnd > 0))
      {
        // Speak the input line
        if (m_bInputUnicode)
        {
          CStringW strInput = GetUniLineBuffer();
          pApp->Speak(strInput);
        }
        else
        {
          CString strInput((LPCTSTR)m_pLineBuffer,m_iLineEnd);
          pApp->Speak(strInput);
        }
      }
    }
  }
  CWinGlkWnd::InputChar(InputChar);
}

void CWinGlkWndTextBuffer::TestLineInput(int iLineEnd)
{
  CWinGlkWnd::TestLineInput(iLineEnd);
  m_bMorePending = false;
  for (int i = 0; i < m_TextBuffer.GetSize(); i++)
    m_TextBuffer[i]->SetLastShown(1);
}

bool CWinGlkWndTextBuffer::MouseClick(CPoint& Click)
{
  if (m_bLinksActive)
  {
    unsigned int iLink = GetLinkAtPoint(Click);
    if (iLink != 0)
    {
      ((CGlkApp*)AfxGetApp())->AddEvent(evtype_Hyperlink,(winid_t)this,iLink,0);
      m_bLinksActive = false;
    }
  }
  return true;
}

unsigned int CWinGlkWndTextBuffer::GetLinkAtPoint(const CPoint& Point)
{
  for (int i = 0; i < m_Hyperlinks.GetSize(); i++)
  {
    if (m_Hyperlinks[i].PtInRect(Point))
      return m_Hyperlinks[i].m_iLink;
  }
  return 0;
}

void CWinGlkWndTextBuffer::SetStyle(int iStyle)
{
  m_iCurrentStyle = iStyle;

  // If the last paragraph is empty, add as its initial style
  if (m_TextBuffer.GetSize() > 0)
  {
    CParagraph* pLast = m_TextBuffer[m_TextBuffer.GetUpperBound()];
    if (pLast->GetLength() == 0)
      pLast->SetInitialStyle(iStyle);
    else
      pLast->AddStyleChange(iStyle);
  }
}

int CWinGlkWndTextBuffer::GetStyle(void)
{
  return m_iCurrentStyle;
}

CWinGlkStyle* CWinGlkWndTextBuffer::GetStyle(int iStyle)
{
  return m_Styles.GetStyle(iStyle);
}

void CWinGlkWndTextBuffer::SetHyperlink(unsigned int iLink)
{
  m_iCurrentLink = iLink;

  if (m_TextBuffer.GetSize() > 0)
  {
    CParagraph* pLast = m_TextBuffer[m_TextBuffer.GetUpperBound()];
    if (pLast->GetLength() == 0)
      pLast->SetInitialLink(iLink);
    else
      pLast->AddLinkChange(iLink);
  }
}

void CWinGlkWndTextBuffer::SetTextColours(glui32 fg, glui32 bg)
{
  if (fg != zcolor_Current)
    m_CurrentColours.fore = fg;
  if (bg != zcolor_Current)
    m_CurrentColours.back = bg;

  if (m_TextBuffer.GetSize() > 0)
  {
    CParagraph* pLast = m_TextBuffer[m_TextBuffer.GetUpperBound()];
    if (pLast->GetLength() == 0)
      pLast->SetInitialColours(m_CurrentColours);
    else
      pLast->AddColourChange(m_CurrentColours);
  }
}

void CWinGlkWndTextBuffer::SetTextReverse(bool reverse)
{
  m_CurrentColours.reverse = reverse;

  if (m_TextBuffer.GetSize() > 0)
  {
    CParagraph* pLast = m_TextBuffer[m_TextBuffer.GetUpperBound()];
    if (pLast->GetLength() == 0)
      pLast->SetInitialColours(m_CurrentColours);
    else
      pLast->AddColourChange(m_CurrentColours);
  }
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
  if (pGraphic)
  {
    if (pGraphic->m_pPixels && pGraphic->m_pHeader)
    {
      if (iWidth < 0)
        iWidth = pGraphic->m_pHeader->biWidth;
      if (iHeight < 0)
        iHeight = abs(pGraphic->m_pHeader->biHeight);

      if (m_TextBuffer.GetSize() == 0)
        AddNewParagraph();
      CParagraph* last = m_TextBuffer[m_TextBuffer.GetUpperBound()];

      pGraphic->m_iWidth = iWidth;
      pGraphic->m_iHeight = iHeight;
      pGraphic->m_iDisplay = iValue1;

      // Store the graphic
      if (last->AddGraphic(pGraphic))
      {
        ClearFormatting(m_TextBuffer.GetUpperBound());
        bDelete = false;
        return true;
      }
    }
  }

  bDelete = true;
  return false;
}

void CWinGlkWndTextBuffer::InsertFlowBreak(void)
{
  if (m_TextBuffer.GetSize() == 0)
    AddNewParagraph();
  int last = m_TextBuffer.GetUpperBound();
  m_TextBuffer[last]->AddCharacter(CParagraph::FlowBreak);

  // Invalidate the formatting for this paragraph
  ClearFormatting(last);
}

void CWinGlkWndTextBuffer::Scrollback(void)
{
  CScrollBackDlg ScrollDlg;

  CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetMainWnd();
  pMainWnd->GetWindowRect(ScrollDlg.m_DialogRect);

  ScrollDlg.m_Text = m_ScrollBuffer.GetData();
  ScrollDlg.m_TextLen = m_ScrollBuffer.GetSize();
  ScrollDlg.DoModal();

  if (m_pActiveWnd)
    m_pActiveWnd->SetActiveWindow();
}

/////////////////////////////////////////////////////////////////////////////
// Helper functions

void CWinGlkWndTextBuffer::Paint(bool bMark)
{
  CPaintDC dcPaint(this);
  CWinGlkBufferDC dcMem(this);
  InitDC(dcMem,&dcPaint);

  // Set up a bitmap in memory to draw into
  CBitmap bmp;
  CRect ClientArea;
  GetClientRect(ClientArea);
  CSize size(ClientArea.Width(),ClientArea.Height());
  if (bmp.CreateCompatibleBitmap(&dcPaint,size.cx,size.cy) == FALSE)
    return;
  CBitmap* pbmpOld = dcMem.SelectObject(&bmp);

  // Clear the window
  dcMem.FillSolidRect(ClientArea,
    GetColour(GetStyle(style_Normal)->m_BackColour));

  // Format each paragraph
  CPaintInfo FormatInfo(0,0,ClientArea.Width(),ClientArea.Height(),dcMem,m_Hyperlinks);
  int i, numParas = m_TextBuffer.GetSize();
  for (i = 0; i < numParas; i++)
    m_TextBuffer[i]->Format(FormatInfo);

  // Determine where to start displaying text
  int offset = 0;
  CPaintInfo PaintInfo(0,0,ClientArea.Width(),ClientArea.Height(),dcMem,m_Hyperlinks);
  PreparePaintInfo(PaintInfo,ClientArea,i,offset,bMark);

  // Draw each paragraph
  int left = 0;
  int top = 0;
  for (int j = 0; j < numParas; j++)
  {
    if (j < i)
      m_TextBuffer[j]->Update(PaintInfo,offset,this);
    else
    {
      // If there were margin images present for the last paragraph, the stored
      // line formatting must be reset when more text is added
      if ((j == numParas-1) && (PaintInfo.m_iWidth < size.cx))
        m_TextBuffer[j]->SetClearAll();

      // Print paragraphs until the displayable area is full
      if (m_TextBuffer[j]->Paint(PaintInfo,left,top,bMark) == false)
        break;
    }
  }
  if (PaintInfo.m_iWidth < size.cx)
    m_TextBuffer[numParas-1]->SetClearAll();

  // Is there a [More] prompt now pending?
  if (m_bMorePending)
  {
    m_iLineX = left;
    m_iLineY = top;

    CWinGlkDC::CDisplay Display = dcMem.GetDisplay();
    dcMem.SetStyle(style_Input,0,NULL);
    dcMem.TextOut(m_iLineX,m_iLineY,m_strMore);
    CSize MoreSize = dcMem.GetTextExtent(m_strMore);
    dcMem.SetDisplay(Display);
    m_iLineX += MoreSize.cx;

    if (GetActiveWindow() == this)
      SetCaretPos(CPoint(m_iLineX,m_iLineY));
  }
  else
  {
    // If input is active, show the current line and set the caret position if active
    if (m_bInputActive)
    {
      m_iLineX = left;
      m_iLineY = top;

      if (m_pLineBuffer)
      {
        if (m_bInputUnicode)
        {
          CStringW str = GetUniLineBuffer();
          PaintInputBuffer(dcMem,(LPCWSTR)str,str.GetLength(),PaintInfo);
        }
        else
          PaintInputBuffer(dcMem,(LPCSTR)m_pLineBuffer,m_iLineEnd,PaintInfo);
      }
      if (GetActiveWindow() == this)
        SetCaretPos(CPoint(m_iLineX,m_iLineY));
    }
  }

  // Copy the bitmap into the window
  dcPaint.BitBlt(0,0,size.cx,size.cy,&dcMem,0,0,SRCCOPY);
  dcMem.SelectObject(pbmpOld);

  CheckDeleteOldText(i,ClientArea.Height());
}

void CWinGlkWndTextBuffer::CheckDeleteOldText(int iTop, int iClientHeight)
{
  // Only run this check after a new paragraph has been added
  if (m_bCheckDeleteText)
  {
    int height = 0;

    // Work out how much vertical space is being used by
    // paragraphs off the top of the display
    int i;
    for (i = 0; i < iTop; i++)
      height += m_TextBuffer[i]->GetHeight();

    // If over 10 times the height of the display,
    // delete half of it.
    int min = iClientHeight * 5;
    if (height > 2 * min)
    {
      bool del = true;
      i = 0;
      while (del && (i < iTop))
      {
        int pheight = m_TextBuffer[0]->GetHeight();
        if (height - pheight > min)
        {
          delete m_TextBuffer[0];
          m_TextBuffer.RemoveAt(0);
          height -= pheight;
          i++;
        }
        else
          del = false;
      }

      // If any text has been deleted, make sure text is reformatted
      if (i > 0)
        ClearFormatting(-1);
    }

    m_bCheckDeleteText = false;
  }
}

void CWinGlkWndTextBuffer::PreparePaintInfo(
  CPaintInfo& Info, CRect& ClientArea, int& iParagraph, int& iOffset, bool bMark)
{
  iParagraph = m_TextBuffer.GetUpperBound();
  iOffset = 0;

  // Step back through the paragraphs until there is more text
  // to display than there is space to display it in.
  int height = ClientArea.Height();
  bool more = true;
  while ((iParagraph >= 0) && (height > 0))
  {
    if (m_TextBuffer[iParagraph]->GetLastShown() >= m_TextBuffer[iParagraph]->GetFinalLastShown())
      more = false;

    if (ShowParagraph(iParagraph))
      height -= m_TextBuffer[iParagraph]->GetHeight();
    iParagraph--;
  }
  iParagraph++;

  // Find the last shown paragraph to see if a [More] prompt is needed.
  m_bMorePending = false;
  if (more)
  {
    bool found = false;
    int para = iParagraph;
    while ((para >= 0) && (para < m_TextBuffer.GetSize()) && (found == false))
    {
      if (m_TextBuffer[para]->GetLastShown() >= 0)
        found = true;
      if (para == 0)
        found = true;
      para--;
    }
    para++;

    if (found)
    {
      if (para < iParagraph)
      {
        // A [More] prompt is definitely required.
        m_bMorePending = true;
        iParagraph = para;
      }
      else if (para == iParagraph)
      {
        // A [More] prompt is required if there is more to show than can fit in the window.
        CRect r;
        GetClientRect(r);
        int iHeightLeft = m_TextBuffer[para]->GetHeight()-m_TextBuffer[para]->LastShownHeight();
        for (int i = para + 1; i < m_TextBuffer.GetSize(); i++)
        {
          if (ShowParagraph(i))
            iHeightLeft += m_TextBuffer[i]->GetHeight();
        }
        if (iHeightLeft > r.Height())
        {
          m_bMorePending = true;
          iParagraph = para;
        }
      }
    }
  }

  if (m_bMorePending)
  {
    Info.m_iTop = 0;

    // Make enough space for the [More] prompt at the bottom of the display
    CWinGlkDC& dc = Info.m_DeviceContext;
    CWinGlkDC::CDisplay Display = dc.GetDisplay();
    dc.SetStyle(style_Input,0,NULL);
    CSize MoreSize = dc.GetTextExtent(m_strMore);
    dc.SetDisplay(Display);
    Info.m_iHeight -= MoreSize.cy;

    if (iParagraph < m_TextBuffer.GetSize())
    {
      // Work out where in the paragraph the last displayed line is
      Info.m_iTop -= m_TextBuffer[iParagraph]->LastShownHeight();

      // Step back to find the offset to the start of the text
      for (int i = iParagraph - 1; i >= 0; i--)
        iOffset += m_TextBuffer[i]->GetHeight();
    }
  }
  else
  {
    if (iParagraph < m_TextBuffer.GetSize())
    {
      Info.m_iTop = (height < 0) ? height : 0;

      // Step back to find the offset to the start of the text
      for (int i = iParagraph - 1; i >= 0; i--)
        iOffset += m_TextBuffer[i]->GetHeight();
    }
    else
      Info.m_iTop = 0;

    // If waiting for input, mark all paragraphs as being displayed
    if (m_bInputActive)
    {
      for (int i = 0; i < m_TextBuffer.GetSize(); i++)
        m_TextBuffer[i]->SetAsShown();
    }
  }
}

// Don't show the last paragraph if it doesn't contain any text and no input is pending
bool CWinGlkWndTextBuffer::ShowParagraph(int iPara)
{
  if (m_bInputActive)
    return true;
  if (iPara < m_TextBuffer.GetUpperBound())
    return true;
  return m_TextBuffer[iPara]->ShowIfLast();
}

void CWinGlkWndTextBuffer::AddNewParagraph(void)
{
  // Clear formatting, if necessary
  int last = m_TextBuffer.GetUpperBound();
  if (last >= 0)
  {
    if (m_TextBuffer[last]->GetClearAll())
      ClearFormatting(-1);
  }

  // Add the new paragraph
  m_TextBuffer.Add(new CParagraph(
    m_iCurrentStyle,m_iCurrentLink,m_CurrentColours.CopyOrNull()));

  // Make sure that after a paragraph has been added, Paint()
  // will check if old text can be deleted
  m_bCheckDeleteText = true;
}

void CWinGlkWndTextBuffer::GetLastShown(int &iLastPara, int& iLastChar)
{
  for (int i = m_TextBuffer.GetSize()-1; i >= 0; i--)
  {
    int iLastShown = m_TextBuffer[i]->GetLastShown();
    if (iLastShown != -1)
    {
      iLastPara = i;
      iLastChar = iLastShown;
      return;
    }
  }

  iLastPara = -1;
  iLastChar = -1;
}

void CWinGlkWndTextBuffer::ClearFormatting(int iPara)
{
  if ((iPara == -1) || m_TextBuffer[iPara]->ClearFormatting())
  {
    // Clear from every paragraph
    for (int i = 0; i < m_TextBuffer.GetSize(); i++)
      m_TextBuffer[i]->ClearFormatting();
  }
}

template<class XCHAR> void CWinGlkWndTextBuffer::PaintInputBuffer(
  CWinGlkDC& dc, const XCHAR* input, int inputLen, CPaintInfo& Info)
{
  // Get the left and right margins for the input lines
  int margin = 0;
  if (m_TextBuffer.GetUpperBound() > 0)
    margin = m_TextBuffer[m_TextBuffer.GetUpperBound()]->GetMargin(this);

  // Fonts occasionally occupy a pixel or so outside of the expected area, so
  // make the margin larger to be safe
  margin++;

  // Work out how many lines the input will use
  int w = Info.m_iWidth-(margin*2);
  int h = dc.m_FontMetrics.tmHeight;
  CArray<int,int> lineIdx;
  GetTextFitIndexes(lineIdx,dc,w,m_iLineX-Info.m_iLeft-margin,input,inputLen);

  // If the input lines take up more space than is left, scroll the display
  CRect ClientArea;
  GetClientRect(ClientArea);
  int extraH = ClientArea.Height() - (m_iLineY+(lineIdx.GetSize()*h));
  if (extraH < 0)
  {
    dc.ScrollDC(0,extraH,NULL,NULL,NULL,NULL);
    m_iLineY += extraH;

    COLORREF oldBack = dc.GetBkColor();
    COLORREF newBack = GetColour(GetStyle(style_Normal)->m_BackColour);
    if (Info.m_iLeft+Info.m_iWidth < ClientArea.Width())
      dc.FillSolidRect(m_iLineX,m_iLineY,Info.m_iLeft+Info.m_iWidth-margin+1-m_iLineX,h,newBack);
    else
      dc.FillSolidRect(m_iLineX,m_iLineY,ClientArea.Width()-m_iLineX,h,newBack);
    dc.FillSolidRect(0,m_iLineY+h,ClientArea.Width(),ClientArea.Height()-m_iLineY-h,newBack);
    dc.SetBkColor(oldBack);

    for (int i = 0; i < m_Hyperlinks.GetSize(); i++)
      m_Hyperlinks[i].OffsetRect(0,extraH);
  }

  // Draw the input lines
  for (int i = 0; i < lineIdx.GetSize(); i++)
  {
    int idx1 = lineIdx.GetAt(i);
    int idx2 = (i == lineIdx.GetUpperBound()) ? inputLen : lineIdx.GetAt(i+1);
    dc.TextOut(
      (i == 0) ? m_iLineX : Info.m_iLeft+margin,m_iLineY+(i*h),input+idx1,idx2-idx1);
  }

  // Work out where the input caret is
  CArray<int,int> caretIdx;
  GetTextFitIndexes(caretIdx,dc,w,m_iLineX-Info.m_iLeft-margin,input,m_iLinePos);
  m_iLineX = (caretIdx.GetSize() == 1) ? m_iLineX : Info.m_iLeft+margin;
  int idx1 = caretIdx.GetAt(caretIdx.GetUpperBound());
  m_iLineX += dc.GetTextExtent(input+idx1,m_iLinePos-idx1).cx;
  m_iLineY += caretIdx.GetUpperBound() * h;
}

template<class XCHAR> void CWinGlkWndTextBuffer::GetTextFitIndexes(
  CArray<int,int>& indexes, CWinGlkDC& dc, int width, int x1, const XCHAR* str, int strLen)
{
  indexes.Add(0);

  int i = 0;
  while (true)
  {
    int j = MeasureTextFit(dc,indexes.GetSize() == 1 ? width-x1 : width,str+i,strLen-i);
    i += j;
    if (i >= strLen)
      return;
    indexes.Add(i);
  }
}

template<class XCHAR> int CWinGlkWndTextBuffer::MeasureTextFit(
  CWinGlkDC& dc, int width, const XCHAR* str, int strLen)
{
  int loLen = 0;
  int loWidth = 0;

  // Check if the text fits completely - if so, just return the whole length of the string
  int hiLen = strLen;
  int hiWidth = dc.GetTextExtent(str,hiLen).cx;
  if (width >= hiWidth)
    return hiLen;
  if (width <= loWidth)
    return hiLen;

  // Iterate until at a point of knowing the largest string that will fit, and the smallest
  // that will not
  while (hiLen > loLen+1)
  {
    // Guess a new length to measure
    int newLen = loLen+(((hiLen-loLen)*(width-loWidth))/(hiWidth-loWidth));
    if (newLen <= loLen)
      newLen = loLen+1;
    if (newLen >= hiLen)
      newLen = hiLen-1;
    int newWidth = dc.GetTextExtent(str,newLen).cx;

    // Set the new length as either the largest that fits, or the smallest that does not
    if (newWidth <= width)
    {
      loLen = newLen;
      loWidth = newWidth;
    }
    else
    {
      hiLen = newLen;
      hiWidth = newWidth;
    }
  }
  return loLen;
}

/////////////////////////////////////////////////////////////////////////////
// Message handlers

void CWinGlkWndTextBuffer::OnPaint(void)
{
  Paint(false);
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
const int CWinGlkWndTextBuffer::CParagraph::m_iIndentStep = 4;

/////////////////////////////////////////////////////////////////////////////
// Painting and formatting information
/////////////////////////////////////////////////////////////////////////////

CWinGlkWndTextBuffer::CPaintInfo::CPaintInfo(int iLeft, int iTop,
  int iWidth, int iHeight, CWinGlkDC& DeviceContext,
  CArray<CHyperlink,CHyperlink&>& Hyperlinks)
  : m_DeviceContext(DeviceContext), m_Hyperlinks(Hyperlinks)
{
  m_iLeft = iLeft;
  m_iTop = iTop;
  m_iWidth = iWidth;
  m_iHeight = iHeight;

  m_Hyperlinks.SetSize(0,32);
}

CWinGlkWndTextBuffer::CPaintInfo::~CPaintInfo()
{
}

void CWinGlkWndTextBuffer::CPaintInfo::MarginAdd(CWinGlkGraphic* pGraphic)
{
  CMarginInsert mi;
  mi.m_iHeightLeft = pGraphic->m_iHeight;
  mi.m_iGraphicWidth = pGraphic->m_iWidth;
  mi.m_bOnLeft = (pGraphic->m_iDisplay == imagealign_MarginLeft);
  m_Margins.Add(mi);

  m_iWidth -= pGraphic->m_iWidth;
  if (pGraphic->m_iDisplay == imagealign_MarginLeft)
    m_iLeft += pGraphic->m_iWidth;
}

void CWinGlkWndTextBuffer::CPaintInfo::MarginPaint(CWinGlkGraphic* pGraphic, int iIndent)
{
  CMarginInsert mi;
  mi.m_iHeightLeft = pGraphic->m_iHeight;
  mi.m_iGraphicWidth = pGraphic->m_iWidth;
  mi.m_bOnLeft = (pGraphic->m_iDisplay == imagealign_MarginLeft);
  m_Margins.Add(mi);

  // Draw the graphic
  if (pGraphic->m_iDisplay == imagealign_MarginLeft)
  {
    DrawGraphic(pGraphic,m_iLeft+iIndent,m_iTop);

    CRect Rect(CPoint(m_iLeft+iIndent,m_iTop),
      CSize(pGraphic->m_iWidth,pGraphic->m_iHeight));
    CheckHyperlink(Rect);
  }
  else
  {
    DrawGraphic(pGraphic,m_iLeft-iIndent+m_iWidth-pGraphic->m_iWidth,m_iTop);

    CRect Rect(CPoint(m_iLeft-iIndent+m_iWidth-pGraphic->m_iWidth,m_iTop),
      CSize(pGraphic->m_iWidth,pGraphic->m_iHeight));
    CheckHyperlink(Rect);
  }

  m_iWidth -= pGraphic->m_iWidth;
  if (pGraphic->m_iDisplay == imagealign_MarginLeft)
    m_iLeft += pGraphic->m_iWidth;
}

void CWinGlkWndTextBuffer::CPaintInfo::MarginPaintPrev(CWinGlkGraphic* pGraphic, int iOffset, int iIndent)
{
  CMarginInsert mi;
  mi.m_iHeightLeft = pGraphic->m_iHeight;
  mi.m_iGraphicWidth = pGraphic->m_iWidth;
  mi.m_bOnLeft = (pGraphic->m_iDisplay == imagealign_MarginLeft);
  m_Margins.Add(mi);

  if (m_iTop - iOffset + pGraphic->m_iHeight > 0)
  {
    // Draw the graphic
    if (pGraphic->m_iDisplay == imagealign_MarginLeft)
    {
      DrawGraphic(pGraphic,m_iLeft+iIndent,m_iTop-iOffset);

      CRect Rect(CPoint(m_iLeft+iIndent,m_iTop-iOffset),
        CSize(pGraphic->m_iWidth,pGraphic->m_iHeight));
      CheckHyperlink(Rect);
    }
    else
    {
      DrawGraphic(pGraphic,m_iLeft-iIndent+m_iWidth-pGraphic->m_iWidth,
        m_iTop-iOffset);

      CRect Rect(CPoint(m_iLeft-iIndent+m_iWidth-pGraphic->m_iWidth,m_iTop-iOffset),
        CSize(pGraphic->m_iWidth,pGraphic->m_iHeight));
      CheckHyperlink(Rect);
    }
  }

  m_iWidth -= pGraphic->m_iWidth;
  if (pGraphic->m_iDisplay == imagealign_MarginLeft)
    m_iLeft += pGraphic->m_iWidth;
}

void CWinGlkWndTextBuffer::CPaintInfo::NextLine(int iStep, bool bDown)
{
  if (bDown)
    m_iTop += iStep;

  bool removeLeft = true;
  bool removeRight = true;
  for (int i = m_Margins.GetUpperBound(); i >= 0; i--)
  {
    CMarginInsert& mi = m_Margins[i];
    mi.m_iHeightLeft -= iStep;

    if (removeLeft && (mi.m_bOnLeft == true))
    {
      // Has the text gone further down than this margin graphic?
      if (mi.m_iHeightLeft <= 0)
      {
        m_iLeft -= mi.m_iGraphicWidth;
        m_iWidth += mi.m_iGraphicWidth;
        m_Margins.RemoveAt(i);
      }
      else
        removeLeft = false;
    }
    if (removeRight && (mi.m_bOnLeft == false))
    {
      if (mi.m_iHeightLeft <= 0)
      {
        m_iWidth += mi.m_iGraphicWidth;
        m_Margins.RemoveAt(i);
      }
      else
        removeRight = false;
    }
  }
}

bool CWinGlkWndTextBuffer::CPaintInfo::HasMarginGraphic(void)
{
  return m_Margins.GetSize() > 0;
}

int CWinGlkWndTextBuffer::CPaintInfo::GetMaxMarginHeight(void)
{
  int max = 0;
  for (int i = 0; i < m_Margins.GetSize(); i++)
  {
    CMarginInsert& mi = m_Margins[i];
    if (mi.m_iHeightLeft > max)
      max = mi.m_iHeightLeft;
  }
  return max;
}

void CWinGlkWndTextBuffer::CPaintInfo::CheckHyperlink(const CRect& Rect)
{
  unsigned int iLink = m_DeviceContext.GetLink();
  if (iLink != 0)
  {
    CHyperlink Link(Rect,iLink);
    m_Hyperlinks.Add(Link);
  }
}

CWinGlkWndTextBuffer::CPaintInfo::CMarginInsert::CMarginInsert()
{
  m_iHeightLeft = 0;
  m_iGraphicWidth = 0;
  m_bOnLeft = true;
}

void CWinGlkWndTextBuffer::CPaintInfo::DrawGraphic(CWinGlkGraphic* pGraphic, int iLeft, int iTop)
{
  CDC dcMem;
  dcMem.CreateCompatibleDC(&m_DeviceContext);

  // Create a temporary DIBSection
  CDibSection DibSection;
  DibSection.CreateBitmap(m_DeviceContext.GetSafeHdc(),
    pGraphic->m_iWidth,pGraphic->m_iHeight);
  CBitmap* pOldBitmap = CDibSection::SelectDibSection(dcMem,&DibSection);

  // Is this graphic being scaled?
  bool bScale = true;
  if (pGraphic->m_iWidth == pGraphic->m_pHeader->biWidth)
  {
    if (pGraphic->m_iHeight == abs(pGraphic->m_pHeader->biHeight))
      bScale = false;
  }

  if (bScale)
  {
    // Copy and stretch the graphic into the temporary DIBSection
    ScaleGfx((COLORREF*)pGraphic->m_pPixels,
      pGraphic->m_pHeader->biWidth,
      abs(pGraphic->m_pHeader->biHeight),
      DibSection.GetBits(),
      pGraphic->m_iWidth,pGraphic->m_iHeight);
  }
  else
  {
    // Copy the bitmap directly
    ::GdiFlush();
    memcpy(DibSection.GetBits(),pGraphic->m_pPixels,
      pGraphic->m_iWidth*pGraphic->m_iHeight*4);
  }

  COLORREF BackColour = GetColour(
    m_DeviceContext.GetStyleFromWindow(style_Normal)->m_BackColour);

  // Split the background colour into red, green and blue.
  int br = GetRValue(BackColour);
  int bg = GetGValue(BackColour);
  int bb = GetBValue(BackColour);

  DWORD PicColour;
  int pr, pg, pb, a;

  // Alpha blend each pixel of the graphic with the background colour.
  ::GdiFlush();
  for (int y = 0; y < pGraphic->m_iHeight; y++)
  {
    for (int x = 0; x < pGraphic->m_iWidth; x++)
    {
      // Get the colour of the pixel.
      PicColour = DibSection.GetPixel(x,y);

      if (pGraphic->m_bAlpha)
      {
        // Split it into red, green, blue and alpha.
        pb = PicColour & 0xFF;
        PicColour >>= 8;
        pg = PicColour & 0xFF;
        PicColour >>= 8;
        pr = PicColour & 0xFF;
        PicColour >>= 8;
        a = PicColour & 0xFF;

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

        DibSection.SetPixel(x,y,(pr<<16)|(pg<<8)|pb);
      }
      else
        DibSection.SetPixel(x,y,PicColour);
    }
  }

  // Blit the alpha blended graphic onto the display
  m_DeviceContext.BitBlt(iLeft,iTop,
    pGraphic->m_iWidth,pGraphic->m_iHeight,&dcMem,0,0,SRCCOPY);

  dcMem.SelectObject(pOldBitmap);
}

/////////////////////////////////////////////////////////////////////////////
// Formatting for a line
/////////////////////////////////////////////////////////////////////////////

CWinGlkWndTextBuffer::CLineFormat::CLineFormat()
{
  m_iFirstCharacter = 0;
  m_iLastCharacter = 0;
  m_iMaxAboveBaseline = 0;
  m_iMaxBelowBaseline = 0;
  m_iLineLength = 0;
}

CWinGlkWndTextBuffer::CLineFormat::~CLineFormat()
{
}

/////////////////////////////////////////////////////////////////////////////
// Paragraphs for text buffer windows
/////////////////////////////////////////////////////////////////////////////

CWinGlkWndTextBuffer::CParagraph::
  CParagraph(int iStyle, unsigned int iLink, CTextColours* pColours)
{
  m_iInitialStyle = iStyle;
  m_iInitialLink = iLink;
  m_pInitialColours = pColours;
  m_iLastShown = -1;
  m_bSpoken = false;
  m_bHadInput = false;
  m_bClearAll = false;
  m_Text.SetSize(0,32);
}

CWinGlkWndTextBuffer::CParagraph::~CParagraph()
{
  ClearFormatting();
  delete m_pInitialColours;

  for (int i = 0; i < m_InlineGraphics.GetSize(); i++)
    delete m_InlineGraphics[i];
  m_InlineGraphics.RemoveAll();
  for (int i = 0; i < m_MarginGraphics.GetSize(); i++)
    delete m_MarginGraphics[i];
  m_MarginGraphics.RemoveAll();
  for (int i = 0; i < m_TextColours.GetSize(); i++)
    delete m_TextColours[i];
  m_TextColours.RemoveAll();
}

void CWinGlkWndTextBuffer::CParagraph::AddCharacter(wchar_t c)
{
  m_Text.Add(c);
}

bool CWinGlkWndTextBuffer::CParagraph::AddGraphic(CWinGlkGraphic* pGraphic)
{
  bool added = false;
  switch (pGraphic->m_iDisplay)
  {
  case imagealign_MarginLeft:
  case imagealign_MarginRight:
    // Add to the array of margin graphics
    m_MarginGraphics.Add(pGraphic);

    // Add a marker into the text stream
    m_Text.Add(MarginGraphic);
    AddInteger(m_MarginGraphics.GetUpperBound());
    added = true;
    break;
  case imagealign_InlineUp:
  case imagealign_InlineDown:
  case imagealign_InlineCenter:
  default:
    // Add to the array of inline graphics
    m_InlineGraphics.Add(pGraphic);

    // Add a marker into the text stream
    m_Text.Add(InlineGraphic);
    AddInteger(m_InlineGraphics.GetUpperBound());
    added = true;
    break;
  }
  return added;
}

int CWinGlkWndTextBuffer::CParagraph::GetLength()
{
  return m_Text.GetSize();
}

void CWinGlkWndTextBuffer::CParagraph::SetInitialStyle(int iStyle)
{
  m_iInitialStyle = iStyle;
}

void CWinGlkWndTextBuffer::CParagraph::AddStyleChange(int iStyle)
{
  m_Text.Add(StyleChange);
  m_Text.Add((wchar_t)iStyle);
}

void CWinGlkWndTextBuffer::CParagraph::SetInitialLink(unsigned int iLink)
{
  m_iInitialLink = iLink;
}

void CWinGlkWndTextBuffer::CParagraph::SetInitialColours(const CTextColours& colours)
{
  delete m_pInitialColours;
  m_pInitialColours = colours.CopyOrNull();
}

void CWinGlkWndTextBuffer::CParagraph::AddLinkChange(unsigned int iLink)
{
  m_Text.Add(LinkChange);
  AddInteger(iLink);
}

void CWinGlkWndTextBuffer::CParagraph::AddColourChange(const CTextColours& colours)
{
  m_TextColours.Add(colours.CopyOrNull());
  m_Text.Add(ColourChange);
  AddInteger(m_TextColours.GetUpperBound());
}

bool CWinGlkWndTextBuffer::CParagraph::ClearFormatting(void)
{
  for (int i = 0; i < m_Formatting.GetSize(); i++)
    delete m_Formatting[i];
  m_Formatting.RemoveAll();
  return m_bClearAll;
}

void CWinGlkWndTextBuffer::CParagraph::Format(CPaintInfo& Info)
{
  // Only format if there is no formatting already set up
  if (m_Formatting.GetSize() > 0)
  {
    for (int i = 0; i < m_Formatting.GetSize(); i++)
    {
      CLineFormat* lf = m_Formatting[i];

      // Move down to the next line  
      Info.NextLine(lf->m_iMaxAboveBaseline + lf->m_iMaxBelowBaseline,true);
    }
    return;
  }

  // Initialize the device context
  Info.m_DeviceContext.SetStyle(m_iInitialStyle,m_iInitialLink,m_pInitialColours);

  // Set up indentation
  int in1 = m_iIndentStep * Info.m_DeviceContext.m_Style.m_Indent;
  int in2 = in1 + (m_iIndentStep * Info.m_DeviceContext.m_Style.m_ParaIndent);
  if (in1 < 0)
    in1 = 0;
  if (in2 < 0)
    in2 = 0;

  // Total space occupied by text in different styles and by graphics
  int left = 0;
  switch (Info.m_DeviceContext.m_Style.m_Justify)
  {
  case stylehint_just_Centered:
  case stylehint_just_RightFlush:
    left = in1;
    break;
  case stylehint_just_LeftFlush:
  case stylehint_just_LeftRight:
  default:
    left = in2;
    break;
  }
  int leftStart = left;

  // String in the current style
  CStringW str;

  // Previous and previous possible break characters
  int lastBreak = 0;
  int lastPossible = 0;

  // Length (in pixels) of the last displayable line
  int lastLength = 0;

  // Current maximum from the baseline for this line
  int maxUp = Info.m_DeviceContext.m_FontMetrics.tmAscent;
  int maxDown = Info.m_DeviceContext.m_FontMetrics.tmDescent;

  // If true, stop formatting
  bool exit = false;

  // If true, current position is the start of a new line
  bool newLine = true;

  // Array of margin images for the next line of formatting
  CArray<int,int> margins;

  int i = 0;
  while (i <= m_Text.GetSize())
  {
    wchar_t c = L'\0';
    if (i < m_Text.GetSize())
      c = m_Text[i];

    switch (c)
    {
    case StyleChange:
    case LinkChange:
    case ColourChange:
      {
        CSize sz;
        bool bTest = TestLineLength(Info,str,sz,left,in1,false,i,
          lastBreak,lastPossible,lastLength,maxUp,maxDown,exit,margins);
        if (exit)
          return;
        if (bTest == false)
        {
          // Move formatting to start after the style change
          sz = Info.m_DeviceContext.GetTextExtent(str);
          left += sz.cx;
          str.Empty();

          i++;
          if (c == StyleChange)
          {
            // Switch to the new style
            int iNewStyle = style_Normal;
            if (i < m_Text.GetSize())
              iNewStyle = m_Text[i];
            Info.m_DeviceContext.SetStyle(iNewStyle,
              Info.m_DeviceContext.GetLink(),Info.m_DeviceContext.GetColours());
          }
          else if (c == LinkChange)
          {
            // Switch to the new link
            unsigned int iNewLink = GetInteger(i);
            i++;
            Info.m_DeviceContext.SetStyle(Info.m_DeviceContext.GetStyle(),
              iNewLink,Info.m_DeviceContext.GetColours());
          }
          else if (c == ColourChange)
          {
            // Switch to the new colours
            unsigned int iNewColours = GetInteger(i);
            i++;
            Info.m_DeviceContext.SetStyle(Info.m_DeviceContext.GetStyle(),
              Info.m_DeviceContext.GetLink(),m_TextColours[iNewColours]);
          }

          // Check that the next entry is not another style change
          bool bCheckHeight = true;
          if (i+1 < m_Text.GetSize())
          {
            if (m_Text[i+1] == c)
              bCheckHeight = false;
          }

          // Check if the line height needs to be increased
          if (bCheckHeight)
          {
            if (maxUp < Info.m_DeviceContext.m_FontMetrics.tmAscent)
              maxUp = Info.m_DeviceContext.m_FontMetrics.tmAscent;
            if (maxDown < Info.m_DeviceContext.m_FontMetrics.tmDescent)
              maxDown = Info.m_DeviceContext.m_FontMetrics.tmDescent;
          }
        }
      }
      newLine = false;
      break;
    case InlineGraphic:
      {
        CSize sz;
        bool bTest = TestLineLength(Info,str,sz,left,in1,false,i,
          lastBreak,lastPossible,lastLength,maxUp,maxDown,exit,margins);
        if (exit)
          return;
        if (bTest == false)
        {
          // Move formatting to start after the graphic
          sz = Info.m_DeviceContext.GetTextExtent(str);
          left += sz.cx;
          str.Empty();

          // Get the graphic
          CWinGlkGraphic* gfx = NULL;
          i++;
          int pic_num = GetInteger(i);
          i++;
          if (pic_num < m_InlineGraphics.GetSize())
            gfx = m_InlineGraphics[pic_num];

          if (gfx)
          {
            left += gfx->m_iWidth;

            int ascent = 0;
            int descent = 0;
            switch (gfx->m_iDisplay)
            {
            case imagealign_InlineUp:
              ascent = gfx->m_iHeight;
              break;
            case imagealign_InlineDown:
              ascent = Info.m_DeviceContext.m_FontMetrics.tmAscent;
              if (gfx->m_iHeight > ascent)
                descent = gfx->m_iHeight - ascent;
              break;
            case imagealign_InlineCenter:
            default:
              {
                int over = gfx->m_iHeight -
                  Info.m_DeviceContext.m_FontMetrics.tmAscent;
                if (over > 0)
                {
                  ascent = Info.m_DeviceContext.m_FontMetrics.tmAscent +
                    (over / 2);
                  descent = over - (over / 2);
                }
              }
              break;
            }

            // Check if the line height needs to be increased
            if (maxUp < ascent)
              maxUp = ascent;
            if (maxDown < descent)
              maxDown = descent;
          }
        }
      }
      newLine = false;
      break;
    case FlowBreak:
      {
        CSize sz;
        bool bTest = TestLineLength(Info,str,sz,left,in1,false,i,
          lastBreak,lastPossible,lastLength,maxUp,maxDown,exit,margins);
        if (exit)
          return;
        if (bTest == false)
        {
          // Move formatting to start after the flow break
          sz = Info.m_DeviceContext.GetTextExtent(str);
          left += sz.cx;
          str.Empty();

          // Is there a margin graphic here?
          if (Info.HasMarginGraphic())
          {
            if (str.IsEmpty() && (left == leftStart))
            {
              maxUp = Info.GetMaxMarginHeight();
              maxDown = 0;
            }

            TestLineLength(Info,str,sz,left,in1,true,i,lastBreak,
              lastPossible,lastLength,maxUp,maxDown,exit,margins);
            if (exit)
              return;
            newLine = true;
          }
        }
      }
      break;
    case MarginGraphic:
      {
        i++;
        int pic_num = GetInteger(i);
        i++;

        // Only add the picture if at the start of a new line
        if (newLine)
        {
          margins.Add(pic_num);

          // Adjust formatting for the margin picture
          Info.MarginAdd(m_MarginGraphics[pic_num]);
        }
      }
      break;
    case L' ':   // Space
    case L'\0':  // End of paragraph
      {
        CSize sz;
        bool bTest = TestLineLength(Info,str,sz,left,in1,c == '\0',i,
          lastBreak,lastPossible,lastLength,maxUp,maxDown,exit,margins);
        if (exit)
          return;
        if (bTest == false)
        {
          str += c;
          lastPossible = i;
          lastLength = sz.cx;
        }
      }
      newLine = false;
      break;
    default:    // Normal character
      str += c;
      newLine = false;
      break;
    }
    i++;
  }
}

bool CWinGlkWndTextBuffer::CParagraph::TestLineLength(CPaintInfo& Info,
  CStringW& strLine, CSize& Size, int& iLeftEdge, int iIndent,
  bool bFinal, int& iIndex, int& iLastBreak, int& iLastPossible,
  int& iLastLength, int& iMaxUp, int& iMaxDown, bool& bExit,
  CArray<int,int>& MarginIndexes)
{
  // Is this line now too long?
  Size = Info.m_DeviceContext.GetTextExtent(strLine);
  Size.cx += iLeftEdge;
  if ((Size.cx > Info.m_iWidth - iIndent) || bFinal)
  {
    // If this is a break and not the end of the line, step back
    if (Size.cx > Info.m_iWidth - iIndent)
    {
      if (iLastPossible > iLastBreak)
        iIndex = iLastPossible;
      else
      {
        if (iIndex < m_Text.GetSize())
          return false;
      }
    }
    else
    {
      // Otherwise, update the line length
      iLastLength = Size.cx;
    }

    CLineFormat* lf = new CLineFormat;
    lf->m_iFirstCharacter = iLastBreak;
    if (iIndex == 0)
      lf->m_iLastCharacter = 0;
    else
      lf->m_iLastCharacter = iIndex - 1;
    lf->m_iMaxAboveBaseline = iMaxUp;
    lf->m_iMaxBelowBaseline = iMaxDown;
    lf->m_iLineLength = iLastLength;

    // Add any margin images
    lf->m_MarginIndexes.Copy(MarginIndexes);
    MarginIndexes.RemoveAll();

    // Add to the list of formatting info
    m_Formatting.Add(lf);

    // Find the next character to start the next line with
    while (iIndex < m_Text.GetSize())
    {
      if (m_Text[iIndex] != ' ')
      {
        iIndex--;
        break;
      }
      iIndex++;
    }

    // Move down to the next line  
    Info.NextLine(lf->m_iMaxAboveBaseline + lf->m_iMaxBelowBaseline,true);

    // Check if formatting should stop now
    if (iIndex == m_Text.GetSize())
      bExit = true;
    else
    {
      // Clear information on this line
      iLeftEdge = iIndent;
      strLine.Empty();
      iLastBreak = iIndex + 1;
      iLastPossible = iIndex + 1;
      iLastLength = 0;
      iMaxUp = Info.m_DeviceContext.m_FontMetrics.tmAscent;
      iMaxDown = Info.m_DeviceContext.m_FontMetrics.tmDescent;
    }
    return true;
  }
  return false;
}

bool CWinGlkWndTextBuffer::CParagraph::Paint(CPaintInfo& Info, int& iFinalLeft, int& iFinalTop, bool bMark)
{
  // If by the end of this routine result is false,
  // then the text has run off the bottom of the
  // displayable area.
  bool result = true;

  // Initialize the device context and text output array
  Info.m_DeviceContext.SetStyle(m_iInitialStyle,m_iInitialLink,m_pInitialColours);
  m_TextOut.RemoveAll();

  // Set up indentation and justification
  int in1 = m_iIndentStep * Info.m_DeviceContext.m_Style.m_Indent;
  int in2 = in1 + (m_iIndentStep * Info.m_DeviceContext.m_Style.m_ParaIndent);
  if (in1 < 0)
    in1 = 0;
  if (in2 < 0)
    in2 = 0;
  int just = Info.m_DeviceContext.m_Style.m_Justify;

  // Grab a buffer large enough for all the text
  wchar_t* buffer = new wchar_t[m_Text.GetSize()];
  int pos = 0;
  int left = 0;

  // Justification values
  int justLength = 0;
  int justSpace = 0;
  int justSpaces = 0;

  // Output each line using the formatting information
  for (int i = 0; i < m_Formatting.GetSize(); i++)
  {
    CLineFormat* lf = m_Formatting[i];

    // Check for justification
    justLength = 0;
    switch (just)
    {
    case stylehint_just_LeftRight:
      left = (i == 0) ? in2 : in1;
      if (i <  m_Formatting.GetSize()-1)
      {
        justLength = JustifyLength(lf,Info,buffer,justSpaces);
        justSpace = Info.m_iWidth - justLength - left - in1;
      }
      break;
    case stylehint_just_Centered:
      left = (Info.m_iWidth - lf->m_iLineLength + in1) / 2;
      break;
    case stylehint_just_RightFlush:
      left = Info.m_iWidth - lf->m_iLineLength;
      break;
    case stylehint_just_LeftFlush:
    default:
      left = (i == 0) ? in2 : in1;
      break;
    }

    // Stop if this is off the bottom of the displayable area
    if (Info.m_iTop + lf->m_iMaxAboveBaseline + lf->m_iMaxBelowBaseline > Info.m_iHeight)
    {
      iFinalLeft = Info.m_iLeft+left;
      iFinalTop = Info.m_iTop;
      result = false;
      break;
    }

    // Copy text into the output buffer
    pos = 0;
    int j = lf->m_iFirstCharacter;
    while (j <= lf->m_iLastCharacter)
    {
      if (j < m_Text.GetSize())
      {
        switch (m_Text[j])
        {
        case StyleChange:
          {
            TextOut(lf,Info,left,buffer,pos);
            if (bMark)
              SetLastShown(Info,j);
            j++;

            int iNewStyle = style_Normal;
            if (j < m_Text.GetSize())
              iNewStyle = m_Text[j];
            Info.m_DeviceContext.SetStyle(iNewStyle,
              Info.m_DeviceContext.GetLink(),Info.m_DeviceContext.GetColours());
          }
          break;
        case LinkChange:
          {
            TextOut(lf,Info,left,buffer,pos);
            if (bMark)
              SetLastShown(Info,j);
            j++;

            unsigned int iNewLink = GetInteger(j);
            j++;
            Info.m_DeviceContext.SetStyle(Info.m_DeviceContext.GetStyle(),
              iNewLink,Info.m_DeviceContext.GetColours());
          }
          break;
        case ColourChange:
          {
            TextOut(lf,Info,left,buffer,pos);
            if (bMark)
              SetLastShown(Info,j);
            j++;

            unsigned int iNewColours = GetInteger(j);
            j++;
            Info.m_DeviceContext.SetStyle(Info.m_DeviceContext.GetStyle(),
              Info.m_DeviceContext.GetLink(),m_TextColours[iNewColours]);
          }
          break;
        case InlineGraphic:
          {
            TextOut(lf,Info,left,buffer,pos);
            if (bMark)
              SetLastShown(Info,j);
            j++;

            // Get the graphic
            CWinGlkGraphic* gfx = NULL;
            int pic_num = GetInteger(j);
            j++;
            if (pic_num < m_InlineGraphics.GetSize())
              gfx = m_InlineGraphics[pic_num];

            if (gfx)
            {
              int top = Info.m_iTop;

              // Find where to draw the graphic
              switch (gfx->m_iDisplay)
              {
              case imagealign_InlineUp:
                top += lf->m_iMaxAboveBaseline;
                top -= gfx->m_iHeight;
                break;
              case imagealign_InlineDown:
                top += lf->m_iMaxAboveBaseline;
                top -= Info.m_DeviceContext.m_FontMetrics.tmAscent;
                break;
              case imagealign_InlineCenter:
              default:
                {
                  top += lf->m_iMaxAboveBaseline;
                  top -= Info.m_DeviceContext.m_FontMetrics.tmAscent / 2;
                  top -= gfx->m_iHeight / 2;
                }
                break;
              }

              // Draw the graphic
              Info.DrawGraphic(gfx,Info.m_iLeft+left,top);

              CRect Rect(CPoint(Info.m_iLeft+left,top),
                CSize(gfx->m_iWidth,gfx->m_iHeight));
              Info.CheckHyperlink(Rect);

              left += gfx->m_iWidth;
            }
          }
          break;
        case FlowBreak:
          break;
        case MarginGraphic:
          {
            j++;
            int pic_num = GetInteger(j);
            j++;

            // Should this margin picture be shown?
            for (int mgi = 0; mgi < lf->m_MarginIndexes.GetSize(); mgi++)
            {
              if (lf->m_MarginIndexes[mgi] == pic_num)
              {
                Info.MarginPaint(m_MarginGraphics[pic_num],
                  m_iIndentStep * Info.m_DeviceContext.m_Style.m_Indent);
              }
            }
          }
          break;
        case L' ':  // Space
          if (justSpaces > 0)
          {
            TextOut(lf,Info,left,buffer,pos);
            if (bMark)
              SetLastShown(Info,j);

            // Work out the justification gap
            int gap = justSpace / justSpaces;
            justSpace -= gap;
            justSpaces--;

            // Fill the justification gap with the
            // current style background colour
            int gapTop = Info.m_iTop + lf->m_iMaxAboveBaseline -
              Info.m_DeviceContext.m_FontMetrics.tmAscent;
            int gapHeight = Info.m_DeviceContext.m_FontMetrics.tmAscent +
              Info.m_DeviceContext.m_FontMetrics.tmDescent;
            Info.m_DeviceContext.FillSolidRect(Info.m_iLeft+left,gapTop,
              gap,gapHeight,Info.m_DeviceContext.GetBkColor());

            left += gap;
          }
          else
          {
            buffer[pos] = m_Text[j];
            pos++;
          }
          break;
        default:    // Normal character
          buffer[pos] = m_Text[j];
          pos++;
          break;
        }
      }
      j++;
    }

    // Flush out any remaining text
    TextOut(lf,Info,left,buffer,pos);
    if (bMark)
      SetLastShown(Info,j);

    // If this is the last line, store the position
    if (i == m_Formatting.GetUpperBound())
    {
      iFinalLeft = Info.m_iLeft+left;
      iFinalTop = Info.m_iTop;
    }

    // Move down to the next line  
    Info.NextLine(lf->m_iMaxAboveBaseline + lf->m_iMaxBelowBaseline,true);
  }

  // Draw the text after everything else to prevent clipping of overhanging text
  for (int i = 0; i < m_TextOut.GetSize(); i++)
  {
    CTextOut& Out = m_TextOut[i];
    Info.m_DeviceContext.SetDisplay(Out.m_Display);
    Info.m_DeviceContext.SetBkMode(TRANSPARENT);
    Info.m_DeviceContext.TextOut(Out.m_Position.x,Out.m_Position.y,Out.m_Text);
  }
  m_TextOut.RemoveAll();
  Info.m_DeviceContext.SetBkMode(OPAQUE);

  delete buffer;
  return result;
}

void CWinGlkWndTextBuffer::CParagraph::Update(CPaintInfo& Info, int& iOffset, CWinGlkWnd* pWnd)
{
  int indent = 0;
  CWinGlkStyle* pStyle = pWnd->GetStyle(m_iInitialStyle);
  if (pStyle)
    indent = m_iIndentStep * pStyle->m_Indent;

  for (int i = 0; i < m_Formatting.GetSize(); i++)
  {
    CLineFormat* lf = m_Formatting[i];

    // Check for margin pictures on this line
    for (int mgi = 0; mgi < lf->m_MarginIndexes.GetSize(); mgi++)
    {
        Info.MarginPaintPrev(m_MarginGraphics[lf->m_MarginIndexes[mgi]],
          iOffset,indent);
    }

    // Update margins without moving down
    int height = lf->m_iMaxAboveBaseline + lf->m_iMaxBelowBaseline;
    Info.NextLine(height,false);
    iOffset -= height;
  }
}

void CWinGlkWndTextBuffer::CParagraph::TextOut(CLineFormat* pFormat,
  CPaintInfo& Info, int& iLeft, wchar_t* pBuffer, int& iBufferPos)
{
  // Work out how far below the baseline this text should be
  int offset = pFormat->m_iMaxAboveBaseline - Info.m_DeviceContext.m_FontMetrics.tmAscent;

  // Store where to output the text
  CTextOut Out;
  Out.m_Display = Info.m_DeviceContext.m_Display;
  Out.m_Position = CPoint(Info.m_iLeft + iLeft,Info.m_iTop + offset);
  Out.m_Text = CStringW(pBuffer,iBufferPos);
  m_TextOut.Add(Out);

  // Fill the text background
  CSize sz = Info.m_DeviceContext.GetTextExtent(pBuffer,iBufferPos);
  CRect Rect(CPoint(Info.m_iLeft + iLeft,Info.m_iTop + offset),sz);
  Info.m_DeviceContext.FillSolidRect(Rect,Info.m_DeviceContext.GetBkColor());

  // If this is a hyperlink, store in the hyperlink array
  Info.CheckHyperlink(Rect);

  // Update the current position
  iLeft += sz.cx;
  iBufferPos = 0;
}

void CWinGlkWndTextBuffer::CParagraph::SetLastShown(CPaintInfo& Info, int iIndex)
{
  if (Info.m_iTop <= Info.m_iHeight)
    m_iLastShown = iIndex;
}

void CWinGlkWndTextBuffer::CParagraph::SetLastShown(int iIndex)
{
  m_iLastShown = iIndex;
}

void CWinGlkWndTextBuffer::CParagraph::SetAsShown(void)
{
  if (m_Formatting.GetSize() > 0)
  {
    CLineFormat* lf = m_Formatting[m_Formatting.GetUpperBound()];
    if (lf->m_iFirstCharacter > m_iLastShown)
      m_iLastShown = lf->m_iFirstCharacter;
  }
}

void CWinGlkWndTextBuffer::CParagraph::HasHadInput(void)
{
  m_bHadInput = true;
}

void CWinGlkWndTextBuffer::CParagraph::SetClearAll(void)
{
  m_bClearAll = true;
}

bool CWinGlkWndTextBuffer::CParagraph::GetClearAll(void) const
{
  return m_bClearAll;
}

int CWinGlkWndTextBuffer::CParagraph::JustifyLength(CLineFormat* pFormat,
  CPaintInfo& Info, wchar_t* pBuffer, int& iSpaces)
{
  int length = 0;
  CWinGlkDC::CDisplay Display = Info.m_DeviceContext.GetDisplay();

  int pos = 0;
  int i = pFormat->m_iFirstCharacter;
  while (i <= pFormat->m_iLastCharacter)
  {
    if (i < m_Text.GetSize())
    {
      switch (m_Text[i])
      {
      case StyleChange:
        {
          length += Info.m_DeviceContext.GetTextExtent(pBuffer,pos).cx;
          pos = 0;
          i++;

          int iNewStyle = style_Normal;
          if (i < m_Text.GetSize())
            iNewStyle = m_Text[i];
          Info.m_DeviceContext.SetStyle(iNewStyle,
            Info.m_DeviceContext.GetLink(),Info.m_DeviceContext.GetColours());
        }
        break;
      case LinkChange:
        {
          length += Info.m_DeviceContext.GetTextExtent(pBuffer,pos).cx;
          pos = 0;
          i++;

          unsigned int iNewLink = GetInteger(i);
          Info.m_DeviceContext.SetStyle(Info.m_DeviceContext.GetStyle(),
            iNewLink,Info.m_DeviceContext.GetColours());
        }
        break;
      case ColourChange:
        {
          length += Info.m_DeviceContext.GetTextExtent(pBuffer,pos).cx;
          pos = 0;
          i++;

          unsigned int iNewColours = GetInteger(i);
          Info.m_DeviceContext.SetStyle(Info.m_DeviceContext.GetStyle(),
            Info.m_DeviceContext.GetLink(),m_TextColours[iNewColours]);
        }
        break;
      case InlineGraphic:
        {
          length += Info.m_DeviceContext.GetTextExtent(pBuffer,pos).cx;
          pos = 0;
          i++;

          // Get the graphic
          CWinGlkGraphic* gfx = NULL;
          int pic_num = GetInteger(i);
          i++;
          if (pic_num < m_InlineGraphics.GetSize())
            gfx = m_InlineGraphics[pic_num];

          if (gfx)
            length += gfx->m_iWidth;
        }
        break;
      case FlowBreak:
        break;
      case MarginGraphic:
        i += 2;
        break;
      case L' ':    // Space
        length += Info.m_DeviceContext.GetTextExtent(pBuffer,pos).cx;
        pos = 0;
        iSpaces++;
        break;
      default:    // Normal character
        pBuffer[pos] = m_Text[i];
        pos++;
        break;
      }
    }
    i++;
  }

  // Measure any remaining text
  length += Info.m_DeviceContext.GetTextExtent(pBuffer,pos).cx;

  if (Info.m_DeviceContext.GetDisplay() != Display)
    Info.m_DeviceContext.SetDisplay(Display);
  return length;
}

int CWinGlkWndTextBuffer::CParagraph::GetCharCount(void) const
{
  int count = 0;
  int i = 0;

  while (i < m_Text.GetSize())
  {
    switch (m_Text[i])
    {
    case StyleChange:
      i++;
      break;
    case LinkChange:
      i += 2;
      break;
    case ColourChange:
      i += 2;
      break;
    case InlineGraphic:
      i += 2;
      break;
    case MarginGraphic:
      i += 2;
      break;
    case FlowBreak:
      break;
    default:    // Normal character
      count++;
      break;
    }
    i++;
  }
  return count;
}

int CWinGlkWndTextBuffer::CParagraph::GetHeight(void) const
{
  int height = 0;
  for (int i = 0; i < m_Formatting.GetSize(); i++)
  {
    CLineFormat* lf = m_Formatting[i];
    height += lf->m_iMaxAboveBaseline;
    height += lf->m_iMaxBelowBaseline;
  }
  return height;
}

int CWinGlkWndTextBuffer::CParagraph::GetLastShown(void) const
{
  return m_iLastShown;
}

int CWinGlkWndTextBuffer::CParagraph::GetFinalLastShown(void) const
{
  if (m_Formatting.GetSize() > 0)
  {
    CLineFormat* lf = m_Formatting[m_Formatting.GetUpperBound()];
    return lf->m_iFirstCharacter;
  }
  return 0;
}

int CWinGlkWndTextBuffer::CParagraph::LastShownHeight(void) const
{
  int height = 0;
  int line = 0;
  for (int i = 0; i < m_Formatting.GetSize(); i++)
  {
    CLineFormat* lf = m_Formatting[i];
    if (lf->m_iFirstCharacter > m_iLastShown)
      break;
    height += line;
    line = lf->m_iMaxAboveBaseline;
    line += lf->m_iMaxBelowBaseline;
  }
  return height;
}

bool CWinGlkWndTextBuffer::CParagraph::ShowIfLast(void) const
{
  if (m_Text.GetSize() > 0)
    return true;
  if (m_bHadInput)
    return true;
  return false;
}

int CWinGlkWndTextBuffer::CParagraph::GetMargin(CWinGlkWnd* pWnd) const
{
  CWinGlkStyle* pStyle = pWnd->GetStyle(m_iInitialStyle);
  if (pStyle)
    return m_iIndentStep * pStyle->m_Indent;
  return 0;
}

void CWinGlkWndTextBuffer::CParagraph::AddInteger(unsigned int iValue)
{
  m_Text.Add((wchar_t)(iValue & 0xFFFF));
  iValue >>= 16;
  m_Text.Add((wchar_t)(iValue & 0xFFFF));
}

unsigned int CWinGlkWndTextBuffer::CParagraph::GetInteger(int iPos) const
{
  unsigned int iValue = 0;

  if (iPos + 1 < m_Text.GetSize())
  {
    iValue |= m_Text[iPos+1];
    iValue <<= 16;
    iValue |= m_Text[iPos];
  }
  return iValue;
}

void CWinGlkWndTextBuffer::CParagraph::Speak(void)
{
  // Has this paragraph already been spoken?
  if (m_bSpoken)
    return;
  m_bSpoken = true;

  // Check if speech is enabled
  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  if (pApp->GetCanSpeak() == false)
    return;

  wchar_t* text = new wchar_t[GetCharCount()+1];
  wchar_t* textptr = text;

  // Get the text of the paragraph as a string
  int i = 0;
  while (i < m_Text.GetSize())
  {
    switch (m_Text[i])
    {
    case StyleChange:
      i++;
      break;
    case LinkChange:
      i += 2;
      break;
    case ColourChange:
      i += 2;
      break;
    case InlineGraphic:
      i += 2;
      break;
    case MarginGraphic:
      i += 2;
      break;
    case FlowBreak:
      break;
    default:
      if (pApp->CanSpeakChar(m_Text[i]))
        *textptr = m_Text[i];
      else
        *textptr = L' ';
      textptr++;
      break;
    }
    i++;
  }

  *textptr = L'\0';

  // Send the text to the speech engine
  pApp->Speak(text);
  delete[] text;
}

/////////////////////////////////////////////////////////////////////////////
// Hyperlinks for text buffer windows
/////////////////////////////////////////////////////////////////////////////

CWinGlkWndTextBuffer::CHyperlink::CHyperlink(const CRect& Rect, unsigned int iLink) : CRect(Rect)
{
  m_iLink = iLink;
}

CWinGlkWndTextBuffer::CHyperlink::CHyperlink() : CRect(0,0,0,0)
{
  m_iLink = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Device context class
/////////////////////////////////////////////////////////////////////////////

CString CWinGlkBufferDC::GetFontName(void)
{
  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  return m_Style.m_Proportional ?
    pApp->GetPropFontName() : pApp->GetFixedFontName();
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
