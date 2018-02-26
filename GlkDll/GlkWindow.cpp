/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkWindow
// Base Glk windows
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkDll.h"
#include "GlkMainWnd.h"
#include "GlkStream.h"
#include "GlkWindow.h"
#include "WinGlk.h"

#include <math.h>
#include <string.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Base class for Glk windows
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkWnd,CWnd);

BEGIN_MESSAGE_MAP(CWinGlkWnd, CWnd)
  //{{AFX_MSG_MAP(CWinGlkWnd)
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_SETCURSOR()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CWinGlkWnd::CWinGlkWnd(glui32 Rock) : CWnd()
{
  m_Rock = Rock;
  m_pParentWnd = NULL;
  m_pEcho = NULL;
  m_bMouseActive = false;
  m_bInputActive = false;
  m_bInputUnicode = false;
  m_bLinksActive = false;
  m_iLineX = 0;
  m_iLineY = 0;
  m_bCaret = false;
  m_pLineBuffer = NULL;
  m_iLineLength = 0;
  m_iLineEnd = 0;
  m_iLinePos = 0;
  m_iHistory = -1;
  m_iPrevStyle = style_Normal;
  m_bEchoInput = true;

  // Add to the global map of all windows
  WindowMap[this] = 0;

  // Add a new stream for this window
  new CWinGlkStreamWnd(this,0);

  if (RegisterObjFn)
    SetDispRock((*RegisterObjFn)(this,gidisp_Class_Window));
  else
    m_DispRock.num = 0;
}

CWinGlkWnd::~CWinGlkWnd()
{
  // Remove the associated stream
  CWinGlkStreamWnd* pStream = CWinGlkStreamWnd::FindWindowStream(this);
  if (pStream)
    delete pStream;

  if (UnregisterObjFn)
    (*UnregisterObjFn)(this,gidisp_Class_Window,GetDispRock());

  if (m_bInputActive && m_pLineBuffer)
  {
    if (UnregisterArrFn)
    {
      (*UnregisterArrFn)(m_pLineBuffer,m_iLineLength,
        m_bInputUnicode ? "&+#!Iu" : "&+#!Cn",GetArrayRock());
    }
  }

  // Remove from the global map of all windows
  WindowMap.RemoveKey(this);

  if (m_pMainWnd == this)
    m_pMainWnd = NULL;
  if (m_pActiveWnd == this)
    m_pActiveWnd = NULL;
}

glui32 CWinGlkWnd::GetRock(void)
{
  return m_Rock;
}

CWinGlkWndPair* CWinGlkWnd::GetParentWnd(void)
{
  return m_pParentWnd;
}

void CWinGlkWnd::SetParentWnd(CWinGlkWndPair* pParentWnd)
{
  m_pParentWnd = pParentWnd;
}

CWinGlkWnd* CWinGlkWnd::GetSiblingWnd(void)
{
  if (m_pParentWnd)
  {
    if (m_pParentWnd->GetChild1Window() == this)
      return m_pParentWnd->GetChild2Window();
    if (m_pParentWnd->GetChild2Window() == this)
      return m_pParentWnd->GetChild1Window();
  }
  return NULL;
}

void CWinGlkWnd::RemoveFromParent(void)
{
  CWinGlkWndPair* pParent1 = NULL;
  CWinGlkWndPair* pParent2 = NULL;

  pParent1 = GetParentWnd();
  if (pParent1)
  {
    CWinGlkWnd* pOtherChild = NULL;
    if (pParent1->GetChild1Window() == this)
      pOtherChild = pParent1->GetChild2Window();
    else
      pOtherChild = pParent1->GetChild1Window();

    pParent2 = pParent1->GetParentWnd();
    if (pParent2)
    {
      pOtherChild->m_pParentWnd = pParent2;
      pParent2->ReplaceChildWnd(pParent1,pOtherChild);
    }
    else
    {
      pOtherChild->m_pParentWnd = NULL;
      m_pMainWnd = pOtherChild;
    }
    delete pParent1;
  }
}

void CWinGlkWnd::ValidateKeyWindows(void)
{
  CWinGlkWnd* pWnd;
  int i;

  POSITION MapPos = WindowMap.GetStartPosition();
  while (MapPos)
  {
    WindowMap.GetNextAssoc(MapPos,pWnd,i);
    if (pWnd->IsKindOf(RUNTIME_CLASS(CWinGlkWndPair)))
    {
      CWinGlkWndPair* pPairWnd = (CWinGlkWndPair*)pWnd;
      if (IsValidWindow(pPairWnd->GetKeyWindow()) == FALSE)
        pPairWnd->SetKeyWindow(NULL);
    }
  }
}

void CWinGlkWnd::CaretOn(void)
{
  if (m_bInputActive)
  {
    if (m_bCaret == false)
    {
      ShowCaret();
      m_bCaret = true;
    }
  }
}

void CWinGlkWnd::CaretOff(void)
{
  if (m_bCaret == true)
  {
    HideCaret();
    m_bCaret = false;
  }
}

void CWinGlkWnd::CallCreate(void)
{
  CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetApp()->GetMainWnd();

  CRect InitRect;
  pMainWnd->GetView()->GetClientRect(InitRect);

  DWORD dwExStyle = 0;
  if (((CGlkApp*)AfxGetApp())->GetWindowBorders())
    dwExStyle = WS_EX_CLIENTEDGE;
  
  CreateEx(dwExStyle,
    AfxRegisterWndClass(0,AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
    NULL,WS_CHILD|WS_VISIBLE,InitRect,pMainWnd->GetView(),0,NULL);
}

void CWinGlkWnd::SetActiveWindow(void)
{
  CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetApp()->GetMainWnd();
  if (pMainWnd)
  {
    if (m_pActiveWnd)
      m_pActiveWnd->CaretOff();
    m_pActiveWnd = this;

    if (GetSafeHwnd())
    {
      int iCaretWidth = ::GetSystemMetrics(SM_CXBORDER);
      if (iCaretWidth < 2)
        iCaretWidth = 2;
      CreateSolidCaret(iCaretWidth,GetCaretHeight());
    }

    if (m_bInputActive)
    {
      CaretOn();
      SetCaretPos(CPoint(m_iLineX,m_iLineY));
    }
    else
      CaretOff();

    pMainWnd->EnableScrollback(false);
  }
}

void CWinGlkWnd::CloseWindow(stream_result_t *pResult)
{
  if (pResult)
  {
    // Find the stream associated with this window
    CWinGlkStream* pStream = CWinGlkStreamWnd::FindWindowStream(this);
    if (pStream)
    {
      pResult->readcount = (glui32)pStream->GetReadCount();
      pResult->writecount = (glui32)pStream->GetWriteCount();
    }
  }

  RemoveFromParent();
  DeleteWindow();
  ValidateKeyWindows();
  SizeAllWindows();
}

void CWinGlkWnd::SizeWindow(CRect* pSize)
{
  CRect NewClient;
  if (pSize)
    NewClient = *pSize;
  else
  {
    // This is going to be the main window, so size to
    // the client area of the main application window
    CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetApp()->GetMainWnd();
    pMainWnd->GetView()->GetClientRect(NewClient);
  }

  SetWindowPos(NULL,NewClient.left,NewClient.top,
               NewClient.Width(),NewClient.Height(),
               SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOOWNERZORDER|SWP_NOREDRAW|SWP_NOZORDER);
}

void CWinGlkWnd::PutCharacter(glui32 c)
{
  if (m_pEcho)
    m_pEcho->PutCharacter(c);
  if (m_bInputActive == false)
    m_bFinalOutput = true;
}

void CWinGlkWnd::DeleteWindow()
{
  DestroyWindow();
  delete this;
};

void CWinGlkWnd::GetSize(int& iWidth, int& iHeight)
{
  iWidth = 0;
  iHeight = 0;
}

void CWinGlkWnd::GetNeededSize(int iSize, int& iWidth, int& iHeight)
{
  iWidth = 0;
  iHeight = 0;
}

void CWinGlkWnd::StartLineEvent(void* pBuffer, bool bUnicode, int iMaxLength, int iStartLength)
{
  if (m_bInputActive == false)
  {
    m_bInputActive = true;
    m_bInputUnicode = bUnicode;
    m_bFinalOutput = false;
    m_pLineBuffer = pBuffer;
    m_iLineLength = iMaxLength;
    m_iLineEnd = iStartLength;
    m_iLinePos = iStartLength;
    m_iHistory = -1;
    m_iPrevStyle = GetStyle();
    SetStyle(style_Input);
    m_InputTerminators = ((CGlkApp*)AfxGetApp())->GetInputTerminators();

    if (RegisterArrFn)
    {
      SetArrayRock((*RegisterArrFn)(m_pLineBuffer,m_iLineLength,
        m_bInputUnicode ? "&+#!Iu" : "&+#!Cn"));
    }

    CWinGlkWnd* pActiveWnd = GetActiveWindow();
    if ((pActiveWnd == NULL) || pActiveWnd->WillReleaseFocus(this))
      SetActiveWindow();

    fflush(NULL);
  }
}

void CWinGlkWnd::EndLineEvent(event_t* pEvent)
{
  if (m_bInputActive && m_pLineBuffer)
  {
    if (pEvent)
    {
      pEvent->type = evtype_LineInput;
      pEvent->win = (winid_t)this;
      pEvent->val1 = m_iLineEnd;
      pEvent->val2 = 0;
    }

    if (m_bEchoInput)
    {
      for (int i = 0; i < m_iLineEnd; i++)
      {
        PutCharacter(m_bInputUnicode ?
          ((glui32*)m_pLineBuffer)[i] : ((unsigned char*)m_pLineBuffer)[i]);
      }
      PutCharacter('\n');
    }

    if (UnregisterArrFn)
    {
      (*UnregisterArrFn)(m_pLineBuffer,m_iLineLength,
        m_bInputUnicode ? "&+#!Iu" : "&+#!Cn",GetArrayRock());
    }

    m_bInputActive = false;
    m_pLineBuffer = NULL;
    m_iLineLength = 0;
    m_iLineEnd = 0;
    m_iLinePos = 0;
    SetStyle(m_iPrevStyle);
  }
  else
  {
    if (pEvent)
    {
      memset(pEvent,0,sizeof(event_t));
      pEvent->type = evtype_None;
    }
  }
}

CStringW CWinGlkWnd::GetUniLineBuffer(void)
{
  CStringW str;
  str.Preallocate(m_iLineEnd);

  for (int i = 0; i < m_iLineEnd; i++)
  {
    if (m_bInputUnicode)
      str.AppendChar((wchar_t)((glui32*)m_pLineBuffer)[i]);
    else
      str.AppendChar((wchar_t)((unsigned char*)m_pLineBuffer)[i]);
  }
  return str;
}

int CWinGlkWnd::LineBufferFromUni(const CStringW& Line)
{
  int len = Line.GetLength();
  if (len > m_iLineLength)
    len = m_iLineLength;

  for (int i = 0; i < len; i++)
  {
    if (m_bInputUnicode)
      ((glui32*)m_pLineBuffer)[i] = Line[i];
    else
    {
      if (Line[i] <= 255)
        ((char*)m_pLineBuffer)[i] = (unsigned char)Line[i];
      else
        ((char*)m_pLineBuffer)[i] = '?';
    }
  }

  return len;
}

void CWinGlkWnd::StartCharEvent(bool bUnicode)
{
  if (m_bInputActive == false)
  {
    m_bInputActive = true;
    m_bInputUnicode = bUnicode;
    m_bFinalOutput = false;
    m_pLineBuffer = NULL;
    m_iLineLength = 0;
    m_iLineEnd = 0;
    m_iLinePos = 0;

    CWinGlkWnd* pActiveWnd = GetActiveWindow();
    if ((pActiveWnd == NULL) || pActiveWnd->WillReleaseFocus(this))
      SetActiveWindow();

    fflush(NULL);
  }
}

void CWinGlkWnd::EndCharEvent(void)
{
  if (m_bInputActive && (m_pLineBuffer == NULL))
    m_bInputActive = false;
}

void CWinGlkWnd::InputChar(unsigned long InputChar)
{
  if (CheckMorePending(true) == false)
  {
    CGlkApp* pApp = (CGlkApp*)AfxGetApp();
    if (m_bInputActive)
    {
      // Character or line input?
      if (m_pLineBuffer)
      {
        switch (InputChar)
        {
        case 0x10000+VK_LEFT:
          if (m_iLinePos > 0)
          {
            m_iLinePos--;
            if (::GetKeyState(VK_CONTROL) & 0x8000)
            {
              // Find the previous start of a word
              while (m_iLinePos > 0)
              {
                glui32 c0 = m_bInputUnicode ?
                    ((glui32*)m_pLineBuffer)[m_iLinePos] : ((char*)m_pLineBuffer)[m_iLinePos];
                glui32 c1 = m_bInputUnicode ?
                    ((glui32*)m_pLineBuffer)[m_iLinePos-1] : ((char*)m_pLineBuffer)[m_iLinePos-1];
                if ((c1 == 32) && (c0 != 32))
                  break;
                m_iLinePos--;
              }
            }
          }
          break;
        case 0x10000+VK_RIGHT:
          if (m_iLinePos < m_iLineEnd)
          {
            m_iLinePos++;
            if (::GetKeyState(VK_CONTROL) & 0x8000)
            {
              // Find the next end of a word
              while (m_iLinePos < m_iLineEnd)
              {
                glui32 c0 = m_bInputUnicode ?
                    ((glui32*)m_pLineBuffer)[m_iLinePos] : ((char*)m_pLineBuffer)[m_iLinePos];
                glui32 c1 = m_bInputUnicode ?
                    ((glui32*)m_pLineBuffer)[m_iLinePos-1] : ((char*)m_pLineBuffer)[m_iLinePos-1];
                if ((c1 != 32) && (c0 == 32))
                  break;
                m_iLinePos++;
              }
            }
          }
          break;
        case 0x10000+VK_HOME:
          m_iLinePos = 0;
          break;
        case 0x10000+VK_END:
          m_iLinePos = m_iLineEnd;
          break;
        case 0x10000+VK_DELETE:
          if (m_iLinePos < m_iLineEnd)
          {
            for (int i = m_iLinePos; i < m_iLineEnd; i++)
            {
              if (m_bInputUnicode)
                ((glui32*)m_pLineBuffer)[i] = ((glui32*)m_pLineBuffer)[i+1];
              else
                ((char*)m_pLineBuffer)[i] = ((char*)m_pLineBuffer)[i+1];
            }
            m_iLineEnd--;
          }
          break;
        case L'\b':
          if (m_iLinePos > 0)
          {
            for (int i = m_iLinePos-1; i < m_iLineEnd; i++)
            {
              if (m_bInputUnicode)
                ((glui32*)m_pLineBuffer)[i] = ((glui32*)m_pLineBuffer)[i+1];
              else
                ((char*)m_pLineBuffer)[i] = ((char*)m_pLineBuffer)[i+1];
            }
            m_iLinePos--;
            m_iLineEnd--;
          }
          break;
        case 0x10000+VK_UP:
          if (m_iHistory < m_History.GetSize()-1)
            m_iHistory++;
          if ((m_iHistory >= 0) && (m_History.GetSize() > 0))
          {
            m_iLineEnd = LineBufferFromUni(m_History[m_iHistory]);
            m_iLinePos = m_iLineEnd;
          }
          break;
        case 0x10000+VK_DOWN:
          if (m_iHistory > 0)
            m_iHistory--;
          if ((m_iHistory >= 0) && (m_History.GetSize() > 0))
          {
            m_iLineEnd = LineBufferFromUni(m_History[m_iHistory]);
            m_iLinePos = m_iLineEnd;
          }
          break;
        default:
          if ((InputChar == L'\r') || (m_InputTerminators.count(InputChar) > 0))
          {
            glui32 Value2 = 0;
            switch (InputChar)
            {
            case 0x10000+VK_ESCAPE:
              Value2 = keycode_Escape;
              break;
            case 0x10000+VK_PRIOR:
              Value2 = keycode_PageUp;
              break;
            case 0x10000+VK_NEXT:
              Value2 = keycode_PageDown;
              break;
            case 0x10000+VK_F1:
              Value2 = keycode_Func1;
              break;
            case 0x10000+VK_F2:
              Value2 = keycode_Func2;
              break;
            case 0x10000+VK_F3:
              Value2 = keycode_Func3;
              break;
            case 0x10000+VK_F4:
              Value2 = keycode_Func4;
              break;
            case 0x10000+VK_F5:
              Value2 = keycode_Func5;
              break;
            case 0x10000+VK_F6:
              Value2 = keycode_Func6;
              break;
            case 0x10000+VK_F7:
              Value2 = keycode_Func7;
              break;
            case 0x10000+VK_F8:
              Value2 = keycode_Func8;
              break;
            case 0x10000+VK_F9:
              Value2 = keycode_Func9;
              break;
            case 0x10000+VK_F10:
              Value2 = keycode_Func10;
              break;
            case 0x10000+VK_F11:
              Value2 = keycode_Func11;
              break;
            case 0x10000+VK_F12:
              Value2 = keycode_Func12;
              break;
            }
            pApp->AddEvent(evtype_LineInput,(winid_t)this,m_iLineEnd,Value2);

            if (m_iLineEnd > 0)
            {
              CStringW line = GetUniLineBuffer();
              m_History.InsertAt(0,line);
              if (m_History.GetSize() > 20)
                m_History.RemoveAt(m_History.GetSize()-1);
            }
            EndLineEvent(NULL);
          }
          else if (m_bInputUnicode)
          {
            if ((InputChar >= 32 && InputChar <= 126) || (InputChar >= 160 && InputChar <= 0xFFFF))
            {
              if (m_iLineEnd < m_iLineLength)
              {
                if (AllowMoreLineInput())
                {
                  for (int i = m_iLineEnd-1; i >= m_iLinePos; i--)
                    ((glui32*)m_pLineBuffer)[i+1] = ((glui32*)m_pLineBuffer)[i];
                  ((glui32*)m_pLineBuffer)[m_iLinePos++] = InputChar;
                  m_iLineEnd++;
                }
              }
            }
          }
          else
          {
            if ((InputChar >= 32 && InputChar <= 126) || (InputChar >= 160 && InputChar <= 255))
            {
              if (m_iLineEnd < m_iLineLength)
              {
                if (AllowMoreLineInput())
                {
                  for (int i = m_iLineEnd-1; i >= m_iLinePos; i--)
                    ((char*)m_pLineBuffer)[i+1] = ((char*)m_pLineBuffer)[i];
                  ((char*)m_pLineBuffer)[m_iLinePos++] = (unsigned char)InputChar;
                  m_iLineEnd++;
                }
              }
            }
          }
          break;
        }
      }
      else
      {
        glui32 Key = keycode_Unknown;

        switch (InputChar)
        {
        case L'\r':
          Key = keycode_Return;
          break;
        case 0x10000+VK_LEFT:
          Key = keycode_Left;
          break;
        case 0x10000+VK_RIGHT:
          Key = keycode_Right;
          break;
        case 0x10000+VK_UP:
          Key = keycode_Up;
          break;
        case 0x10000+VK_DOWN:
          Key = keycode_Down;
          break;
        case 0x10000+VK_DELETE:
        case L'\b':
          Key = keycode_Delete;
          break;
        case 0x10000+VK_ESCAPE:
          Key = keycode_Escape;
          break;
        case 0x10000+VK_PRIOR:
          Key = keycode_PageUp;
          break;
        case 0x10000+VK_NEXT:
          Key = keycode_PageDown;
          break;
        case 0x10000+VK_HOME:
          Key = keycode_Home;
          break;
        case 0x10000+VK_END:
          Key = keycode_End;
          break;
        case 0x10000+VK_F1:
          Key = keycode_Func1;
          break;
        case 0x10000+VK_F2:
          Key = keycode_Func2;
          break;
        case 0x10000+VK_F3:
          Key = keycode_Func3;
          break;
        case 0x10000+VK_F4:
          Key = keycode_Func4;
          break;
        case 0x10000+VK_F5:
          Key = keycode_Func5;
          break;
        case 0x10000+VK_F6:
          Key = keycode_Func6;
          break;
        case 0x10000+VK_F7:
          Key = keycode_Func7;
          break;
        case 0x10000+VK_F8:
          Key = keycode_Func8;
          break;
        case 0x10000+VK_F9:
          Key = keycode_Func9;
          break;
        case 0x10000+VK_F10:
          Key = keycode_Func10;
          break;
        case 0x10000+VK_F11:
          Key = keycode_Func11;
          break;
        case 0x10000+VK_F12:
          Key = keycode_Func12;
          break;
        default:
          {
            unsigned long max = m_bInputUnicode ? 0xFFFF : 0xFF;
            if ((InputChar >= 32 && InputChar <= 126) || (InputChar >= 160 && InputChar <= max))
              Key = InputChar;
          }
          break;
        }
        
        pApp->AddEvent(evtype_CharInput,(winid_t)this,Key,0);
        m_bInputActive = false;
      }
    }

    // If a pending [MORE] prompt isn't involved,
    // always force a redraw.
    pApp->GetMainWnd()->Invalidate();
  }
}

CWinGlkStream* CWinGlkWnd::GetEchoStream(void)
{
  if (CWinGlkStream::IsValidStream(m_pEcho) == FALSE)
    m_pEcho = NULL;
  return m_pEcho;
}

void CWinGlkWnd::SetEchoStream(CWinGlkStream* pStream)
{
  m_pEcho = pStream;
}

bool CWinGlkWnd::DrawGraphic(CWinGlkGraphic* pGraphic, int iValue1, int iValue2, int iWidth, int iHeight, bool& bDelete)
{
  bDelete = true;
  return false;
}

/////////////////////////////////////////////////////////////////////////////
// Message handlers

void CWinGlkWnd::OnPaint(void)
{
  CPaintDC dc(this);

  CRect Client;
  GetClientRect(Client);
  dc.FillSolidRect(Client,::GetSysColor(COLOR_WINDOW));
}

void CWinGlkWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
  if ((GetActiveWindow() == NULL) || MouseMakesActive())
    SetActiveWindow();
  if (MouseClick(point) == false)
  {
    CWnd* pMainWnd = AfxGetMainWnd();
    if (::GetWindowLong(pMainWnd->GetSafeHwnd(),GWL_STYLE) & WS_POPUP)
      pMainWnd->SendMessage(WM_NCLBUTTONDOWN,HTCAPTION,POINTTOPOINTS(point));
  }
  CWnd::OnLButtonDown(nFlags,point);
}

BOOL CWinGlkWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
  if (m_bLinksActive)
  {
    if (nHitTest == HTCLIENT)
    {
      const MSG* msg = GetCurrentMessage();
      if (msg != NULL)
      {
        // Get the mouse position in client coordinates
        CPoint MousePos(msg->pt);
        ScreenToClient(&MousePos);

        // Is there a hyperlink under the point?
        if (GetLinkAtPoint(MousePos) != 0)
        {
          HCURSOR hand = ::LoadCursor(0,IDC_HAND);
          if (hand == 0)
            hand = ::LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_LINK_HAND));
          ::SetCursor(hand);
          return 1;
        }
      }
    }
  }
  return CWnd::OnSetCursor(pWnd,nHitTest,message);
}

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

CWinGlkWnd* CWinGlkWnd::m_pMainWnd = NULL;
CWinGlkWnd* CWinGlkWnd::m_pActiveWnd = NULL;
CMap<CWinGlkWnd*,CWinGlkWnd*,int,int> CWinGlkWnd::WindowMap;

bool CWinGlkWnd::m_bFinalOutput = false;
bool CWinGlkWnd::m_bExiting = false;

CWinGlkWnd* CWinGlkWnd::GetMainWindow(void)
{
  return m_pMainWnd;
}

CWinGlkWnd* CWinGlkWnd::GetActiveWindow(void)
{
  if (IsValidWindow(m_pActiveWnd) == false)
    m_pActiveWnd = NULL;
  return m_pActiveWnd;
}

CMap<CWinGlkWnd*,CWinGlkWnd*,int,int>& CWinGlkWnd::GetWindowMap(void)
{
  return WindowMap;
}

CWinGlkWnd* CWinGlkWnd::OpenWindow(CWinGlkWnd* pNewWnd, CWinGlkWnd* pSplitWnd, glui32 Method, glui32 Size)
{
  pNewWnd->CallCreate();

  if (pNewWnd)
  {
    if (pSplitWnd == NULL)
    {
      m_pMainWnd = pNewWnd;
      pNewWnd->SetParentWnd(NULL);
    }
    else
    {
      CWinGlkWndPair* pParentWnd = pSplitWnd->GetParentWnd();

      // Replace the current parent window with a new pair window
      CWinGlkWndPair* pPairWnd = new CWinGlkWndPair(pSplitWnd,pNewWnd,Method,Size);

      if (pParentWnd)
      {
        pParentWnd->ReplaceChildWnd(pSplitWnd,pPairWnd);
        pPairWnd->SetParentWnd(pParentWnd);
      }
      else
      {
        m_pMainWnd = pPairWnd;
        pPairWnd->SetParentWnd(NULL);
      }
    }

    // Resize all windows
    m_pMainWnd->SizeWindow(NULL);
  }

  return pNewWnd;
}

void CWinGlkWnd::CloseAllWindows(void)
{
  CWinGlkWnd* pWnd;
  CArray<CWinGlkWnd*,CWinGlkWnd*> WndArray;
  int i;

  POSITION MapPos = WindowMap.GetStartPosition();
  while (MapPos)
  {
    WindowMap.GetNextAssoc(MapPos,pWnd,i);
    WndArray.Add(pWnd);
  }

  for (i = 0; i < WndArray.GetSize(); i++)
    delete WndArray[i];
}

bool CWinGlkWnd::IsValidWindow(CWinGlkWnd* pWnd)
{
  int iDummy;
  if (WindowMap.Lookup(pWnd,iDummy))
    return true;
  return false;
}

CWinGlkWnd* CWinGlkWnd::IterateWindows(CWinGlkWnd* pWnd, glui32* pRockPtr)
{
  CWinGlkWnd* pMapWnd = NULL;
  CWinGlkWnd* pPrevWnd = NULL;
  int iDummy;

  POSITION MapPos = WindowMap.GetStartPosition();
  while (MapPos)
  {
    WindowMap.GetNextAssoc(MapPos,pMapWnd,iDummy);
    if (pWnd == pPrevWnd)
    {
      if (pRockPtr)
        *pRockPtr = pMapWnd->GetRock();
      return pMapWnd;
    }
    pPrevWnd = pMapWnd;
  }
  return NULL;
}

void CWinGlkWnd::SetExiting(void)
{
  m_bExiting = true;

  if (m_pMainWnd)
    m_pMainWnd->SetActiveWindow();
}

void CWinGlkWnd::SizeAllWindows(void)
{
  CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetMainWnd();
  if (pMainWnd)
  {
    CRect ClientRect;
    pMainWnd->GetView()->GetClientRect(ClientRect);

    if (m_pMainWnd)
      m_pMainWnd->SizeWindow(&ClientRect);

    pMainWnd->Invalidate();
  }
}

COLORREF CWinGlkWnd::GetColour(glsi32 iColour)
{
  COLORREF Colour;

  switch (iColour)
  {
  case 0xFFFFFFFF:
    iColour = ((CGlkApp*)AfxGetApp())->GetTextColour();
    break;
  case 0xFFFFFFFE:
    iColour = ((CGlkApp*)AfxGetApp())->GetBackColour();
    break;
  }
  switch (iColour)
  {
  case 0xFFFFFFFF:
    Colour = ::GetSysColor(COLOR_WINDOWTEXT);
    break;
  case 0xFFFFFFFE:
    Colour = ::GetSysColor(COLOR_WINDOW);
    break;
  default:
    {
      BYTE r = (BYTE)((iColour & 0x00FF0000) >> 16);
      BYTE g = (BYTE)((iColour & 0x0000FF00) >> 8);
      BYTE b = (BYTE)((iColour & 0x000000FF));
      Colour = RGB(r,g,b);
    }
    break;
  }
  return Colour;
}

/////////////////////////////////////////////////////////////////////////////
// Pair windows
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkWndPair,CWinGlkWnd);

CWinGlkWndPair::CWinGlkWndPair(CWinGlkWnd* pChild1, CWinGlkWnd* pChild2, glui32 Method, glui32 Size) : CWinGlkWnd(0)
{
  m_pChild1 = pChild1;
  m_pChild2 = pChild2;
  m_pKey = m_pChild2;

  m_pChild1->SetParentWnd(this);
  m_pChild2->SetParentWnd(this);

  m_Method = Method;
  m_Size = Size;
}

void CWinGlkWndPair::SizeWindow(CRect* pSize)
{
  CRect PairSize, Child1Size, Child2Size;

  if (pSize == NULL)
  {
    // This is going to be the main window, so size to
    // the client area of the main application window
    CRect AppClientRect;
    CWinGlkMainWnd* pMainWnd = (CWinGlkMainWnd*)AfxGetApp()->GetMainWnd();
    pMainWnd->GetView()->GetClientRect(PairSize);
  }
  else
    PairSize = *pSize;

  Child1Size = PairSize;
  Child2Size = PairSize;

  // Now split the PairSize rectangle between the two child windows
  switch (m_Method & winmethod_DivisionMask)
  {
  case winmethod_Proportional:
    switch (m_Method & winmethod_DirMask)
    {
    case winmethod_Above:
      Child2Size.bottom = Child2Size.top + (int)(PairSize.Height() * m_Size * 0.01);
      Child1Size.top = Child2Size.bottom;
      break;
    case winmethod_Below:
      Child2Size.top = Child2Size.bottom - (int)(PairSize.Height() * m_Size * 0.01);
      Child1Size.bottom = Child2Size.top;
      break;
    case winmethod_Left:
      Child2Size.right = Child2Size.left + (int)(PairSize.Width() * m_Size * 0.01);
      Child1Size.left = Child2Size.right;
      break;
    case winmethod_Right:
      Child2Size.left = Child2Size.right - (int)(PairSize.Width() * m_Size * 0.01);
      Child1Size.right = Child2Size.left;
      break;
    }
    break;
  case winmethod_Fixed:
    {
      int iWidth = 0;
      int iHeight = 0;
      if (m_pKey)
        m_pKey->GetNeededSize(m_Size,iWidth,iHeight);

      switch (m_Method & winmethod_DirMask)
      {
      case winmethod_Above:
        Child2Size.bottom = Child2Size.top + iHeight;
        Child1Size.top = Child2Size.bottom;
        break;
      case winmethod_Below:
        Child2Size.top = Child2Size.bottom - iHeight;
        Child1Size.bottom = Child2Size.top;
        break;
      case winmethod_Left:
        Child2Size.right = Child2Size.left + iWidth;
        Child1Size.left = Child2Size.right;
        break;
      case winmethod_Right:
        Child2Size.left = Child2Size.right - iWidth;
        Child1Size.right = Child2Size.left;
        break;
      }
    }
    break;
  }

  m_pChild1->SizeWindow(&Child1Size);
  m_pChild2->SizeWindow(&Child2Size);
}

void CWinGlkWndPair::DeleteWindow()
{
  m_pChild1->DeleteWindow();
  m_pChild2->DeleteWindow();
  CWinGlkWnd::DeleteWindow();
}

void CWinGlkWndPair::ReplaceChildWnd(CWinGlkWnd* pOldChild, CWinGlkWnd* pNewChild)
{
  if (pOldChild == m_pChild1)
    m_pChild1 = pNewChild;
  if (pOldChild == m_pChild2)
    m_pChild2 = pNewChild;
}

void CWinGlkWndPair::SetArrangement(glui32 Method, glui32 Size, CWinGlkWnd* pKey)
{
  if (pKey)
  {
    BOOL bKey = FALSE;

    // Check that the new key window is a descendant of this one
    CWinGlkWnd* pWnd = pKey;
    while (pWnd && (bKey == FALSE))
    {
      if (pWnd == m_pChild2)
        bKey = TRUE;
      else if (pWnd == m_pChild1)
        bKey = TRUE;
      pWnd = pWnd->GetParentWnd();
    }
    if (bKey == FALSE)
      return;
  }

  // Check that the change in constraint is valid
  glui32 m1 = m_Method & winmethod_DirMask;
  glui32 m2 = Method & winmethod_DirMask;
  if (m1 != m2)
  {
    // If the direction constraint has changed, validate the
    // constraint and then swap the windows if valid
    if ((m1 == winmethod_Above) && (m2 != winmethod_Below))
      return;
    if ((m1 == winmethod_Below) && (m2 != winmethod_Above))
      return;
    if ((m1 == winmethod_Left) && (m2 != winmethod_Right))
      return;
    if ((m1 == winmethod_Right) && (m2 != winmethod_Left))
      return;
    CWinGlkWnd* pChild = m_pChild1;
    m_pChild1 = m_pChild2;
    m_pChild2 = pChild;
  }

  m_Method = Method;
  m_Size = Size;
  if (pKey)
    m_pKey = pKey;
  SizeAllWindows();
}

void CWinGlkWndPair::GetArrangement(glui32* MethodPtr, glui32* SizePtr, CWinGlkWnd** pKeyPtr)
{
  if (MethodPtr)
    *MethodPtr = m_Method;
  if (SizePtr)
    *SizePtr = m_Size;
  if (pKeyPtr)
    *pKeyPtr = m_pKey;
}


/////////////////////////////////////////////////////////////////////////////
// Base device context class
/////////////////////////////////////////////////////////////////////////////

CTextColours::CTextColours()
{
  fore = zcolor_Default;
  back = zcolor_Default;
  reverse = false;
}

bool CTextColours::operator!=(const CTextColours& Compare)
{
  if (fore != Compare.fore)
    return true;
  if (back != Compare.back)
    return true;
  if (reverse != Compare.reverse)
    return true;
  return false;
}

CTextColours* CTextColours::CopyOrNull(void) const
{
  if ((fore == zcolor_Default) && (back == zcolor_Default) && !reverse)
    return NULL;
  return new CTextColours(*this);
}

CWinGlkDC::CWinGlkDC(CWinGlkWnd* pWnd) : m_Display(style_Normal,0,NULL)
{
  m_pWnd = pWnd;
  m_pFont = NULL;
  m_pOldFont = NULL;
  memset(&m_FontMetrics,0,sizeof(TEXTMETRIC));

  for (int i = 0; i < style_NUMSTYLES * 2; i++)
    m_Fonts[i] = NULL;
}

CWinGlkDC::~CWinGlkDC()
{
  if (m_pOldFont)
    SelectObject(m_pOldFont);

  for (int i = 0; i < style_NUMSTYLES * 2; i++)
  {
    if (m_Fonts[i])
      delete m_Fonts[i];
  }
}

CWinGlkDC::CDisplay::CDisplay()
{
  m_iStyle = 0;
  m_iLink = 0;
  m_iIndex = 0;
}

CWinGlkDC::CDisplay::CDisplay(int iStyle, unsigned int iLink, const CTextColours* pColours)
{
  m_iStyle = iStyle;
  m_iLink = iLink;
  m_pColours = pColours;
  m_iIndex = (iStyle * 2) + ((iLink != 0) ? 1 : 0);
}

bool CWinGlkDC::CDisplay::operator==(const CDisplay& Compare)
{
  if (m_iStyle != Compare.m_iStyle)
    return false;
  if (m_iLink != Compare.m_iLink)
    return false;
  if (m_pColours != Compare.m_pColours)
    return false;
  return true;
}

bool CWinGlkDC::CDisplay::operator!=(const CDisplay& Compare)
{
  return (operator==)(Compare) ? false : true;
}

void CWinGlkDC::SetStyle(int iStyle, unsigned int iLink, const CTextColours* pColours)
{
  CDisplay Display(iStyle,iLink,pColours);
  SetDisplay(Display);
}

void CWinGlkDC::SetDisplay(const CDisplay& Display)
{
  CGlkApp* pApp = (CGlkApp*)AfxGetApp();
  m_Display = Display;

  CWinGlkStyle* pNewStyle = NULL;
  if (m_pWnd)
    pNewStyle = m_pWnd->GetStyle(m_Display.m_iStyle);
  if (pNewStyle)
    m_Style = *pNewStyle;
  else
    m_Style.SetStyle(style_Normal);

  // Is there already a font previously allocated for this style?
  if (m_Fonts[m_Display.m_iIndex])
    m_pFont = m_Fonts[m_Display.m_iIndex];
  else
  {
    LOGFONT* pTextFont = NULL;
    LOGFONT* pSizeFont = NULL;
    GetFonts(pTextFont,pSizeFont);
    if (pTextFont)
    {
      LOGFONT TextLogFont;
      CopyMemory(&TextLogFont,pTextFont,sizeof(LOGFONT));
      SetFontStyles(TextLogFont);

      if (m_Display.m_iLink != 0)
        TextLogFont.lfUnderline = TRUE;

      TextLogFont.lfCharSet = ANSI_CHARSET;
      TextLogFont.lfOutPrecision = OUT_TT_PRECIS;
      TextLogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
      TextLogFont.lfQuality = PROOF_QUALITY;
      TextLogFont.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
      if (pSizeFont)
        TextLogFont.lfHeight = pSizeFont->lfHeight;

      int iPointSize = -MulDiv(TextLogFont.lfHeight,72,GetDeviceCaps(LOGPIXELSY));
      int iStyleSize = GetStyleFontSize();
      double dPointInc = iPointSize*fabs((double)iStyleSize)*0.1;
      if (dPointInc < 1.0)
        dPointInc = 1.0;
      if (iStyleSize > 0)
        iPointSize += (int)ceil(dPointInc);
      else if (iStyleSize < 0)
        iPointSize -= (int)ceil(dPointInc);
      if (iPointSize < 4)
        iPointSize = 4;
      TextLogFont.lfHeight = -MulDiv(iPointSize,GetDeviceCaps(LOGPIXELSY),72);

      m_pFont = new CFont;
      m_pFont->CreateFontIndirect(&TextLogFont);
      m_Fonts[m_Display.m_iIndex] = m_pFont;
    }
  }

  if (m_pOldFont)
    SelectObject(m_pOldFont);
  if (m_pFont)
    m_pOldFont = SelectObject(m_pFont);

  // Get the metrics of the currently selected font
  GetTextMetrics(&m_FontMetrics);

  // Set the text and background colours
  CTextColours overColours;
  if (m_Display.m_pColours != NULL)
    overColours = *(m_Display.m_pColours);
  COLORREF BackColour = CWinGlkWnd::GetColour(
    overColours.back == zcolor_Default ? m_Style.m_BackColour : overColours.back);
  COLORREF TextColour = CWinGlkWnd::GetColour(
    overColours.fore == zcolor_Default ? m_Style.m_TextColour : overColours.fore);
  if (m_Style.m_ReverseColour || overColours.reverse)
  {
    SetTextColor(BackColour);
    SetBkColor(TextColour);
  }
  else
  {
    SetTextColor(TextColour);
    SetBkColor(BackColour);
  }
  if (m_Display.m_iLink != 0)
    SetTextColor(pApp->GetLinkColour());
  SetBkMode(OPAQUE);
}

CWinGlkStyle* CWinGlkDC::GetStyleFromWindow(int iStyle)
{
  CWinGlkStyle* pStyle = NULL;
  if (m_pWnd)
    pStyle = m_pWnd->GetStyle(iStyle);
  return pStyle;
}

BOOL CWinGlkDC::TextOut(int x, int y, LPCSTR lpszString, int nCount)
{
  return ::TextOut(m_hDC,x,y,lpszString,nCount);
}

CSize CWinGlkDC::GetTextExtent(LPCSTR lpszString, int nCount) const
{
  SIZE size;
  ::GetTextExtentPoint32(m_hAttribDC,lpszString,nCount,&size);
  return size;
}

BOOL CWinGlkDC::TextOut(int x, int y, LPCWSTR lpszString, int nCount)
{
  ((CWinGlkMainWnd*)AfxGetApp()->GetMainWnd())->GetTextOut().
    TextOut(m_hDC,x,y,lpszString,nCount);
  return TRUE;
}

BOOL CWinGlkDC::TextOut(int x, int y, const CStringW& str)
{
  return TextOut(x,y,str,(int)str.GetLength());
}

CSize CWinGlkDC::GetTextExtent(LPCWSTR lpszString, int nCount) const
{
  return ((CWinGlkMainWnd*)AfxGetApp()->GetMainWnd())->GetTextOut().
    GetTextExtent(m_hDC,lpszString,nCount);
}

CSize CWinGlkDC::GetTextExtent(const CStringW& str) const
{
  return GetTextExtent(str,(int)str.GetLength());
}
