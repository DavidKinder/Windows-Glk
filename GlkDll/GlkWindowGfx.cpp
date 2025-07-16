/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkWindowGfx
// Graphics Glk windows
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkDll.h"
#include "GlkMainWnd.h"
#include "GlkStream.h"
#include "GlkWindowGfx.h"
#include "ScaleGfx.h"

#include <math.h>
#include <string.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Graphics windows
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkWndGraphics,CWinGlkWnd);

BEGIN_MESSAGE_MAP(CWinGlkWndGraphics, CWinGlkWnd)
  //{{AFX_MSG_MAP(CWinGlkWndGraphics)
  ON_WM_PAINT()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CWinGlkWndGraphics::CWinGlkWndGraphics(glui32 Rock) : CWinGlkWnd(Rock)
{
  m_BackColour = RGB(0xFF,0xFF,0xFF);
  m_pDibSection = NULL;
}

CWinGlkWndGraphics::~CWinGlkWndGraphics()
{
  if (m_pDibSection)
    delete m_pDibSection;
}

void CWinGlkWndGraphics::SizeWindow(CRect* pSize)
{
  CWinGlkWnd::SizeWindow(pSize);

  CDC* pWndDC = GetDC();
  CDC dcMem;
  dcMem.CreateCompatibleDC(pWndDC);

  CRect Client;
  GetClientRect(Client);

  if ((Client.Width() > 0) && (Client.Height() > 0))
  {
    CDibSection* pDibSection = new CDibSection;
    if (pDibSection->CreateBitmap(pWndDC->GetSafeHdc(),Client.Width(),Client.Height()))
    {
      CBitmap* pOldBitmap = CDibSection::SelectDibSection(dcMem,pDibSection);

      CBrush Brush;
      Brush.CreateSolidBrush(m_BackColour);
      dcMem.FillRect(Client,&Brush);

      if (m_pDibSection)
      {
        // Copy old image into new bitmap
        CDC dcMem2;
        dcMem2.CreateCompatibleDC(pWndDC);

        CSize Size = m_pDibSection->GetSize();
        CBitmap* pOldBitmap2 = CDibSection::SelectDibSection(dcMem2,m_pDibSection);
        dcMem.BitBlt(0,0,Size.cx,Size.cy,&dcMem2,0,0,SRCCOPY);
        dcMem2.SelectObject(pOldBitmap2);
      }

      dcMem.SelectObject(pOldBitmap);

      if (m_pDibSection)
        delete m_pDibSection;
      m_pDibSection = pDibSection;
    }
    else
      delete pDibSection;
  }
  ReleaseDC(pWndDC);
}

void CWinGlkWndGraphics::ClearWindow(void)
{
  CRect Client;
  GetClientRect(Client);
  FillRectBack(Client);
}

void CWinGlkWndGraphics::GetSize(int& iWidth, int& iHeight)
{
  CRect Client;
  GetClientRect(Client);

  iWidth = Client.Width();
  iHeight = Client.Height();
}

void CWinGlkWndGraphics::GetNeededSize(int iSize, int& iWidth, int& iHeight)
{
  iWidth = iSize;
  iHeight = iSize;

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

void CWinGlkWndGraphics::StartMouseEvent(void)
{
  m_bMouseActive = true;
}

bool CWinGlkWndGraphics::MouseClick(CPoint& Click)
{
  if (m_bMouseActive)
  {
    ((CGlkApp*)AfxGetApp())->AddEvent(evtype_MouseInput,(winid_t)this,Click.x,Click.y);
    m_bMouseActive = false;
    return true;
  }
  return false;
}

bool CWinGlkWndGraphics::DrawGraphic(CWinGlkGraphic* pGraphic, int iValue1, int iValue2,
  int iWidth, int iHeight, unsigned int iImageRule, unsigned int iMaxWidth, bool& bDelete)
{
  bool bDraw = false;
  bool bScale = false;
  bDelete = true;

  if (pGraphic)
  {
    // Is the graphic set up correctly?
    if (pGraphic->m_pPixels && pGraphic->m_pHeader)
    {
      // Does this window have a DIBSection to draw into?
      if (m_pDibSection)
      {
        CSize Size = m_pDibSection->GetSize();

        // Get the width and height and whether or not the graphic
        // is to be scaled.
        switch (iImageRule & imagerule_WidthMask)
        {
        case imagerule_WidthOrig:
          iWidth = pGraphic->m_pHeader->biWidth;
          break;
        case imagerule_WidthFixed:
          bScale = true;
          break;
        case imagerule_WidthRatio:
          {
            CRect Client;
            GetClientRect(Client);

            double scale = ((double)iWidth) / 0x10000;
            iWidth = (int)(scale * Client.Width());
            bScale = true;
          }
          break;
        default:
          return false;
        }
        switch (iImageRule & imagerule_HeightMask)
        {
        case imagerule_HeightOrig:
          iHeight = abs(pGraphic->m_pHeader->biHeight);
          break;
        case imagerule_HeightFixed:
          bScale = true;
          break;
        case imagerule_AspectRatio:
          {
            double aspect = ((double)abs(pGraphic->m_pHeader->biHeight)) / pGraphic->m_pHeader->biWidth;
            double scale = ((double)iHeight) / 0x10000;
            iHeight = (int)(aspect * scale * iWidth);
            bScale = true;
        }
          break;
        default:
          return false;
        }
        if ((iWidth <= 0) || (iHeight <= 0))
          return false;

        // Work out clipping values for graphics which are partially
        // or completely out of the graphics window.
        int x1 = 0, x2 = iWidth, y1 = 0, y2 = iHeight;
        if (iValue1 < 0)
          x1 = iValue1 * -1;
        if (iValue2 < 0)
          y1 = iValue2 * -1;
        if (iValue1 + x2 > Size.cx)
          x2 = Size.cx - iValue1;
        if (iValue2 + y2 > Size.cy)
          y2 = Size.cy - iValue2;

        CDibSection DibSection;
        DWORD* ppvBits = NULL;

        // If the graphic is being scaled, copy and stretch the graphic
        // into a temporary DIBSection, otherwise just use the bits from
        // the graphic.
        if (bScale)
        {
          // Create a temporary DIBSection
          CDC* pWndDC = GetDC();
          BOOL created = DibSection.CreateBitmap(pWndDC->GetSafeHdc(),iWidth,iHeight);
          ReleaseDC(pWndDC);
          if (!created)
            return false;

          // Draw the graphic into the temporary DIBSection
          ppvBits = DibSection.GetBits();
          ScaleGfx((COLORREF*)pGraphic->m_pPixels,
            pGraphic->m_pHeader->biWidth,
            abs(pGraphic->m_pHeader->biHeight),
            ppvBits,iWidth,iHeight);
        }
        else
          ppvBits = (DWORD*)pGraphic->m_pPixels;

        DWORD SrcColour, DestColour;
        int sr, sg, sb, dr, dg, db, a;

        // Alpha blend each pixel of the graphic into the
        // graphics window.
        ::GdiFlush();
        for (int y = y1; y < y2; y++)
        {
          for (int x = x1; x < x2; x++)
          {
            // Get the colour of the pixel.
            SrcColour = CDibSection::GetPixel(ppvBits,iWidth,x,y);

            if (pGraphic->m_bAlpha)
            {
              // Split it into red, green, blue and alpha.
              sb = SrcColour & 0xFF;
              SrcColour >>= 8;
              sg = SrcColour & 0xFF;
              SrcColour >>= 8;
              sr = SrcColour & 0xFF;
              SrcColour >>= 8;
              a = SrcColour & 0xFF;

              // Get the colour of the destination pixel.
              DestColour = m_pDibSection->GetPixel(x+iValue1,y+iValue2);

              // Split it into red, green and blue.
              db = DestColour & 0xFF;
              DestColour >>= 8;
              dg = DestColour & 0xFF;
              DestColour >>= 8;
              dr = DestColour & 0xFF;

              // Perform alpha blending
              if (a == 0)
              {
              }
              else if (a == 0xFF)
              {
                dr = sr;
                dg = sg;
                db = sb;
              }
              else
              {
                // Rescale from 0..255 to 0..256
                a += a>>7;

                dr += sr - ((a * dr) >> 8);
                dg += sg - ((a * dg) >> 8);
                db += sb - ((a * db) >> 8);
                if (dr > 0xFF)
                  dr = 0xFF;
                if (dg > 0xFF)
                  dg = 0xFF;
                if (db > 0xFF)
                  db = 0xFF;
              }

              m_pDibSection->SetPixel(x+iValue1,y+iValue2,(dr<<16)|(dg<<8)|db);
            }
            else
              m_pDibSection->SetPixel(x+iValue1,y+iValue2,SrcColour);
          }
        }
        bDraw = true;
      }
    }
  }
  return bDraw;
}

void CWinGlkWndGraphics::FillRect(CRect& Rect,COLORREF Colour)
{
  if (m_pDibSection)
  {
    CDC* pWndDC = GetDC();
    CDC dcMem;
    dcMem.CreateCompatibleDC(pWndDC);

    CBitmap* pOldBitmap = CDibSection::SelectDibSection(dcMem,m_pDibSection);

    CBrush Brush;
    Brush.CreateSolidBrush(Colour);
    dcMem.FillRect(Rect,&Brush);

    dcMem.SelectObject(pOldBitmap);
    ReleaseDC(pWndDC);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Message handlers

void CWinGlkWndGraphics::OnPaint(void)
{
  CPaintDC dc(this);

  CRect Client;
  GetClientRect(Client);

  if (m_pDibSection)
  {
    CDC* pWndDC = GetDC();
    CDC dcMem;
    dcMem.CreateCompatibleDC(pWndDC);

    CSize Size = m_pDibSection->GetSize();
    CBitmap* pOldBitmap = CDibSection::SelectDibSection(dcMem,m_pDibSection);
    dc.BitBlt(0,0,Size.cx,Size.cy,&dcMem,0,0,SRCCOPY);
    dcMem.SelectObject(pOldBitmap);

    ReleaseDC(pWndDC);
  }
  else
  {
    CBrush BackBrush;
    BackBrush.CreateSolidBrush(m_BackColour);
    dc.FillRect(Client,&BackBrush);
  }
}
