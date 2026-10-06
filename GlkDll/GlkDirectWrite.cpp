/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkDirectWrite
// Measuring and drawing text with DirectWrite
//
/////////////////////////////////////////////////////////////////////////////

// This file does not use the precompiled header, as the DirectWrite interfaces
// for font fallback need a later Windows version than the rest of the library.
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#define NTDDI_VERSION 0x0A000000
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <d2d1.h>
#include <dwrite_2.h>
#include <atlbase.h>

#include <math.h>
#include <stddef.h>
#include <map>
#include <string>

#include "GlkDirectWrite.h"

class CWinGlkFont
{
public:
  CWinGlkFont();

  bool CreateLayout(LPCWSTR pText, int iCount, IDWriteTextLayout** ppLayout);
  float GetWidth(LPCWSTR pText, int iCount);

  IDWriteTextFormat* m_pFormat;
  IDWriteFont* m_pFont;
  std::wstring m_Family;
  DWRITE_FONT_WEIGHT m_Weight;
  DWRITE_FONT_STYLE m_Style;
  DWRITE_FONT_STRETCH m_Stretch;
  bool m_bUnderline;
  int m_iPixels;
  int m_iAscent;
  int m_iDescent;
  int m_iLineGap;
  int m_iAveCharWidth;

  std::map<std::wstring,float> m_Widths;
};

namespace {

// DirectWrite and Direct2D objects, which live until the process exits
IDWriteFactory* Factory = NULL;
IDWriteFactory2* Factory2 = NULL;
IDWriteGdiInterop* GdiInterop = NULL;
ID2D1Factory* D2DFactory = NULL;
ID2D1DCRenderTarget* RenderTarget = NULL;
ID2D1SolidColorBrush* Brush = NULL;

// The device context and bound rectangle when drawing text in a batch
HDC BatchDC = NULL;
RECT BatchRect;

std::map<std::string,CWinGlkFont*> Fonts;

const WCHAR* Locale = L"en-us";

bool Init(void)
{
  static bool bInit = false;
  if (bInit)
    return (GdiInterop != NULL);
  bInit = true;

  if (FAILED(::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,__uuidof(IDWriteFactory),
    (IUnknown**)&Factory)))
  {
    return false;
  }
  if (FAILED(::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,&D2DFactory)))
    return false;

  // Only available from Windows 8.1
  Factory->QueryInterface(__uuidof(IDWriteFactory2),(void**)&Factory2);

  if (FAILED(Factory->GetGdiInterop(&GdiInterop)))
  {
    GdiInterop = NULL;
    return false;
  }
  return true;
}

// The render target is shared by all device contexts, each text output binding it
// to the device context that it draws into
bool GetRenderTarget(void)
{
  if (RenderTarget != NULL)
    return true;

  D2D1_RENDER_TARGET_PROPERTIES Props = D2D1::RenderTargetProperties(
    D2D1_RENDER_TARGET_TYPE_DEFAULT,
    D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_IGNORE),96.0f,96.0f);
  if (FAILED(D2DFactory->CreateDCRenderTarget(&Props,&RenderTarget)))
  {
    RenderTarget = NULL;
    return false;
  }
  if (FAILED(RenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f,0.0f,0.0f),&Brush)))
  {
    RenderTarget->Release();
    RenderTarget = NULL;
    Brush = NULL;
    return false;
  }
  RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
  return true;
}

void ReleaseRenderTarget(void)
{
  if (Brush != NULL)
    Brush->Release();
  Brush = NULL;
  if (RenderTarget != NULL)
    RenderTarget->Release();
  RenderTarget = NULL;
}

// Bind the render target to the rectangle, clipped to the device context
bool BindRenderTarget(HDC hdc, RECT& Bind)
{
  RECT Clip;
  if (::GetClipBox(hdc,&Clip) != ERROR)
  {
    if (!::IntersectRect(&Bind,&Bind,&Clip))
      return false;
  }
  if (!GetRenderTarget())
    return false;
  return SUCCEEDED(RenderTarget->BindDC(hdc,&Bind));
}

D2D1_COLOR_F ToColour(COLORREF Colour)
{
  return D2D1::ColorF(GetRValue(Colour)/255.0f,GetGValue(Colour)/255.0f,GetBValue(Colour)/255.0f);
}

void DrawLayout(IDWriteTextLayout* pLayout, FLOAT x, FLOAT y, COLORREF Colour)
{
  // Colour fonts, such as for emoji, are supported from Windows 8.1
  D2D1_DRAW_TEXT_OPTIONS Options = (Factory2 != NULL) ?
    D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT : D2D1_DRAW_TEXT_OPTIONS_NONE;

  Brush->SetColor(ToColour(Colour));
  RenderTarget->DrawTextLayout(D2D1::Point2F(x,y),pLayout,Brush,Options);
}

std::wstring GetFamilyName(IDWriteFont* pFont)
{
  std::wstring Name;
  CComPtr<IDWriteFontFamily> Family;
  CComPtr<IDWriteLocalizedStrings> Names;
  if (FAILED(pFont->GetFontFamily(&Family)))
    return Name;
  if (FAILED(Family->GetFamilyNames(&Names)))
    return Name;

  UINT32 index = 0;
  BOOL exists = FALSE;
  if (FAILED(Names->FindLocaleName(Locale,&index,&exists)) || !exists)
    index = 0;
  UINT32 length = 0;
  if (FAILED(Names->GetStringLength(index,&length)))
    return Name;
  Name.resize(length+1);
  if (FAILED(Names->GetString(index,&Name[0],length+1)))
    return std::wstring();
  Name.resize(length);
  return Name;
}

// Get the average character width from the font's 'OS/2' table, as GDI does
bool GetAveCharWidth(IDWriteFont* pFont, double scale, int& iWidth)
{
  CComPtr<IDWriteFontFace> Face;
  if (FAILED(pFont->CreateFontFace(&Face)))
    return false;

  const BYTE* pTable = NULL;
  UINT32 iSize = 0;
  void* pContext = NULL;
  BOOL bExists = FALSE;
  if (FAILED(Face->TryGetFontTable(DWRITE_MAKE_OPENTYPE_TAG('O','S','/','2'),
    (const void**)&pTable,&iSize,&pContext,&bExists)))
  {
    return false;
  }
  bool bFound = false;
  if (bExists)
  {
    if (iSize >= 4)
    {
      short xAvgCharWidth = (short)((pTable[2]<<8)|pTable[3]);
      if (xAvgCharWidth > 0)
      {
        iWidth = (int)floor((xAvgCharWidth * scale) + 0.5);
        bFound = (iWidth > 0);
      }
    }
    Face->ReleaseFontTable(pContext);
  }
  return bFound;
}

// The source of the text for mapping characters to fallback fonts
class CTextSource : public IDWriteTextAnalysisSource
{
public:
  CTextSource(const WCHAR* pText, UINT32 iLength) : m_pText(pText), m_iLength(iLength)
  {
  }

  // The object is on the stack, so is not reference counted
  STDMETHOD(QueryInterface)(REFIID riid, void** ppv)
  {
    if ((riid == __uuidof(IUnknown)) || (riid == __uuidof(IDWriteTextAnalysisSource)))
    {
      *ppv = this;
      return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
  }
  STDMETHOD_(ULONG,AddRef)(void) { return 1; }
  STDMETHOD_(ULONG,Release)(void) { return 1; }

  STDMETHOD(GetTextAtPosition)(UINT32 iPos, const WCHAR** ppText, UINT32* pLength)
  {
    *ppText = (iPos < m_iLength) ? m_pText + iPos : NULL;
    *pLength = (iPos < m_iLength) ? m_iLength - iPos : 0;
    return S_OK;
  }
  STDMETHOD(GetTextBeforePosition)(UINT32 iPos, const WCHAR** ppText, UINT32* pLength)
  {
    *ppText = ((iPos > 0) && (iPos <= m_iLength)) ? m_pText : NULL;
    *pLength = ((iPos > 0) && (iPos <= m_iLength)) ? iPos : 0;
    return S_OK;
  }
  STDMETHOD_(DWRITE_READING_DIRECTION,GetParagraphReadingDirection)(void)
  {
    return DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
  }
  STDMETHOD(GetLocaleName)(UINT32 iPos, UINT32* pLength, const WCHAR** ppLocale)
  {
    *ppLocale = Locale;
    *pLength = (iPos < m_iLength) ? m_iLength - iPos : 0;
    return S_OK;
  }
  STDMETHOD(GetNumberSubstitution)(UINT32 iPos, UINT32* pLength, IDWriteNumberSubstitution** ppSub)
  {
    *ppSub = NULL;
    *pLength = (iPos < m_iLength) ? m_iLength - iPos : 0;
    return S_OK;
  }

private:
  const WCHAR* m_pText;
  UINT32 m_iLength;
};

} // unnamed namespace

CWinGlkFont::CWinGlkFont()
{
  m_pFormat = NULL;
  m_pFont = NULL;
  m_Weight = DWRITE_FONT_WEIGHT_NORMAL;
  m_Style = DWRITE_FONT_STYLE_NORMAL;
  m_Stretch = DWRITE_FONT_STRETCH_NORMAL;
  m_bUnderline = false;
  m_iPixels = 0;
  m_iAscent = 0;
  m_iDescent = 0;
  m_iLineGap = 0;
  m_iAveCharWidth = 0;
}

bool CWinGlkFont::CreateLayout(LPCWSTR pText, int iCount, IDWriteTextLayout** ppLayout)
{
  if (FAILED(Factory->CreateTextLayout(pText,iCount,m_pFormat,100000.0f,100000.0f,ppLayout)))
    return false;
  if (m_bUnderline)
  {
    DWRITE_TEXT_RANGE Range = { 0, (UINT32)iCount };
    (*ppLayout)->SetUnderline(TRUE,Range);
  }
  return true;
}

float CWinGlkFont::GetWidth(LPCWSTR pText, int iCount)
{
  std::wstring Text(pText,iCount);
  std::map<std::wstring,float>::const_iterator it = m_Widths.find(Text);
  if (it != m_Widths.end())
    return it->second;

  float width = 0.0f;
  CComPtr<IDWriteTextLayout> Layout;
  if (CreateLayout(pText,iCount,&Layout))
  {
    DWRITE_TEXT_METRICS Metrics;
    if (SUCCEEDED(Layout->GetMetrics(&Metrics)))
      width = Metrics.widthIncludingTrailingWhitespace;
  }

  if (m_Widths.size() > 4096)
    m_Widths.clear();
  m_Widths[Text] = width;
  return width;
}

CWinGlkFont* WinGlkDirectWrite::GetFont(const LOGFONT& Font)
{
  if (!Init())
    return NULL;

  int iPixels = abs(Font.lfHeight);
  if (iPixels < 1)
    iPixels = 1;

  char key[LF_FACESIZE+64];
  sprintf(key,"%.*s|%d|%d|%d|%d",LF_FACESIZE,Font.lfFaceName,iPixels,(int)Font.lfWeight,
    (int)Font.lfItalic,(int)Font.lfUnderline);
  std::map<std::string,CWinGlkFont*>::const_iterator it = Fonts.find(key);
  if (it != Fonts.end())
    return it->second;

  // Find the font from its GDI description, so that GDI names such as "Arial Narrow"
  // are mapped to the right DirectWrite family, stretch and weight
  LOGFONTW WideFont;
  memcpy(&WideFont,&Font,offsetof(LOGFONTA,lfFaceName));
  ::MultiByteToWideChar(CP_ACP,0,Font.lfFaceName,-1,WideFont.lfFaceName,LF_FACESIZE);
  WideFont.lfFaceName[LF_FACESIZE-1] = 0;

  CComPtr<IDWriteFont> FoundFont;
  if (FAILED(GdiInterop->CreateFontFromLOGFONT(&WideFont,&FoundFont)))
  {
    wcscpy(WideFont.lfFaceName,L"Arial");
    if (FAILED(GdiInterop->CreateFontFromLOGFONT(&WideFont,&FoundFont)))
    {
      Fonts[key] = NULL;
      return NULL;
    }
  }

  // DirectWrite synthesizes bold or oblique if the family lacks them
  DWRITE_FONT_WEIGHT Weight = (Font.lfWeight != FW_DONTCARE) ?
    (DWRITE_FONT_WEIGHT)Font.lfWeight : FoundFont->GetWeight();
  DWRITE_FONT_STYLE Style = Font.lfItalic ?
    DWRITE_FONT_STYLE_ITALIC : FoundFont->GetStyle();
  DWRITE_FONT_STRETCH Stretch = FoundFont->GetStretch();
  std::wstring Family = GetFamilyName(FoundFont);

  CComPtr<IDWriteTextFormat> Format;
  if (FAILED(Factory->CreateTextFormat(Family.c_str(),NULL,Weight,Style,Stretch,
    (FLOAT)iPixels,Locale,&Format)))
  {
    Fonts[key] = NULL;
    return NULL;
  }
  Format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

  // Get the font that the text format will use for the family
  CComPtr<IDWriteFont> MatchFont;
  CComPtr<IDWriteFontFamily> FontFamily;
  if (SUCCEEDED(FoundFont->GetFontFamily(&FontFamily)))
    FontFamily->GetFirstMatchingFont(Weight,Stretch,Style,&MatchFont);
  if (MatchFont == NULL)
    MatchFont = FoundFont;

  CWinGlkFont* pFont = new CWinGlkFont;
  pFont->m_pFormat = Format.Detach();
  pFont->m_pFont = MatchFont.Detach();
  pFont->m_Family = Family;
  pFont->m_Weight = Weight;
  pFont->m_Style = Style;
  pFont->m_Stretch = Stretch;
  pFont->m_bUnderline = (Font.lfUnderline != 0);
  pFont->m_iPixels = iPixels;

  DWRITE_FONT_METRICS Metrics;
  pFont->m_pFont->GetMetrics(&Metrics);
  double scale = (double)iPixels / Metrics.designUnitsPerEm;
  pFont->m_iAscent = (int)floor((Metrics.ascent * scale) + 0.5);
  pFont->m_iDescent = (int)floor((Metrics.descent * scale) + 0.5);
  pFont->m_iLineGap = (int)floor((Metrics.lineGap * scale) + 0.5);
  if (!GetAveCharWidth(pFont->m_pFont,scale,pFont->m_iAveCharWidth))
    pFont->m_iAveCharWidth = (int)floor(pFont->GetWidth(L"x",1) + 0.5f);
  if (pFont->m_iAveCharWidth < 1)
    pFont->m_iAveCharWidth = 1;

  Fonts[key] = pFont;
  return pFont;
}

void WinGlkDirectWrite::GetMetrics(CWinGlkFont* pFont, TEXTMETRIC& Metrics)
{
  memset(&Metrics,0,sizeof Metrics);
  Metrics.tmAscent = pFont->m_iAscent;
  Metrics.tmDescent = pFont->m_iDescent;
  Metrics.tmHeight = pFont->m_iAscent + pFont->m_iDescent;
  Metrics.tmInternalLeading = max(Metrics.tmHeight - pFont->m_iPixels,0);
  Metrics.tmExternalLeading = pFont->m_iLineGap;
  Metrics.tmAveCharWidth = pFont->m_iAveCharWidth;
  Metrics.tmMaxCharWidth = pFont->m_iAveCharWidth;
  Metrics.tmWeight = pFont->m_Weight;
  Metrics.tmItalic = (pFont->m_Style != DWRITE_FONT_STYLE_NORMAL);
  Metrics.tmUnderlined = pFont->m_bUnderline;
}

SIZE WinGlkDirectWrite::GetTextExtent(CWinGlkFont* pFont, LPCWSTR pText, int iCount)
{
  SIZE Size = { 0, 0 };
  if (pFont != NULL)
  {
    Size.cy = pFont->m_iAscent + pFont->m_iDescent;
    if (iCount > 0)
      Size.cx = (LONG)floor(pFont->GetWidth(pText,iCount) + 0.5f);
  }
  return Size;
}

void WinGlkDirectWrite::TextOut(CWinGlkFont* pFont, HDC hdc, int x, int y,
  LPCWSTR pText, int iCount, int iFitWidth)
{
  if ((pFont == NULL) || (iCount <= 0))
    return;
  CComPtr<IDWriteTextLayout> Layout;
  if (!pFont->CreateLayout(pText,iCount,&Layout))
    return;

  int iWidth = iFitWidth;
  if (iFitWidth > 0)
  {
    // Spread the difference from the natural width over each character
    CComQIPtr<IDWriteTextLayout1> Layout1(Layout);
    UINT32 iClusters = 0;
    Layout->GetClusterMetrics(NULL,0,&iClusters);
    if ((Layout1 != NULL) && (iClusters > 0))
    {
      float spacing = (iFitWidth - pFont->GetWidth(pText,iCount)) / iClusters;
      DWRITE_TEXT_RANGE Range = { 0, (UINT32)iCount };
      Layout1->SetCharacterSpacing(0.0f,spacing,0.0f,Range);
    }
  }
  else
    iWidth = GetTextExtent(pFont,pText,iCount).cx;

  int iHeight = pFont->m_iAscent + pFont->m_iDescent;
  bool bOpaque = (::GetBkMode(hdc) == OPAQUE);
  RECT Back = { x, y, x + iWidth, y + iHeight };

  // Keep the baseline where the font's metrics put it, even if a fallback font
  // has made the line taller
  float baseline = (float)pFont->m_iAscent;
  DWRITE_LINE_METRICS Line;
  UINT32 iLines = 0;
  if (SUCCEEDED(Layout->GetLineMetrics(&Line,1,&iLines)) && (iLines > 0))
    baseline = Line.baseline;

  if (hdc == BatchDC)
  {
    if (bOpaque)
    {
      Brush->SetColor(ToColour(::GetBkColor(hdc)));
      RenderTarget->FillRectangle(D2D1::RectF((FLOAT)(Back.left - BatchRect.left),
        (FLOAT)(Back.top - BatchRect.top),(FLOAT)(Back.right - BatchRect.left),
        (FLOAT)(Back.bottom - BatchRect.top)),Brush);
    }
    DrawLayout(Layout,(FLOAT)(x - BatchRect.left),
      (FLOAT)(y + pFont->m_iAscent - BatchRect.top) - baseline,::GetTextColor(hdc));
    return;
  }

  if (bOpaque)
    ::ExtTextOutW(hdc,0,0,ETO_OPAQUE,&Back,NULL,0,NULL);

  // Leave room for glyphs that overhang the text box, such as italics
  int iPad = iHeight;
  RECT Bind = { x - iPad, y - iPad, x + iWidth + iPad, y + iHeight + iPad };
  if (!BindRenderTarget(hdc,Bind))
    return;

  RenderTarget->BeginDraw();
  DrawLayout(Layout,(FLOAT)(x - Bind.left),
    (FLOAT)(y + pFont->m_iAscent - Bind.top) - baseline,::GetTextColor(hdc));
  if (RenderTarget->EndDraw() == D2DERR_RECREATE_TARGET)
    ReleaseRenderTarget();
}

void WinGlkDirectWrite::BeginDraw(HDC hdc, const RECT& Rect)
{
  RECT Bind = Rect;
  if (!BindRenderTarget(hdc,Bind))
    return;

  RenderTarget->BeginDraw();
  BatchDC = hdc;
  BatchRect = Bind;
}

void WinGlkDirectWrite::EndDraw(void)
{
  if (BatchDC == NULL)
    return;

  BatchDC = NULL;
  if (RenderTarget->EndDraw() == D2DERR_RECREATE_TARGET)
    ReleaseRenderTarget();
}

bool WinGlkDirectWrite::CanOutput(CWinGlkFont* pFont, UINT32 c)
{
  if (pFont == NULL)
    return false;

  BOOL bHas = FALSE;
  if (SUCCEEDED(pFont->m_pFont->HasCharacter(c,&bHas)) && bHas)
    return true;

  if (Factory2 != NULL)
  {
    // Find the font that DirectWrite would use for the character
    WCHAR Text[2];
    UINT32 iLength = 1;
    if (c >= 0x10000)
    {
      Text[0] = (WCHAR)(0xD800 + ((c - 0x10000) >> 10));
      Text[1] = (WCHAR)(0xDC00 + (c & 0x3FF));
      iLength = 2;
    }
    else
      Text[0] = (WCHAR)c;

    CComPtr<IDWriteFontFallback> Fallback;
    if (FAILED(Factory2->GetSystemFontFallback(&Fallback)))
      return false;
    CTextSource Source(Text,iLength);
    UINT32 iMapped = 0;
    CComPtr<IDWriteFont> MappedFont;
    FLOAT scale = 1.0f;
    if (FAILED(Fallback->MapCharacters(&Source,0,iLength,NULL,pFont->m_Family.c_str(),
      pFont->m_Weight,pFont->m_Style,pFont->m_Stretch,&iMapped,&MappedFont,&scale)))
    {
      return false;
    }
    if (MappedFont == NULL)
      return false;
    return (SUCCEEDED(MappedFont->HasCharacter(c,&bHas)) && bHas);
  }

  // Without the system font fallback, look for any installed font with the character
  CComPtr<IDWriteFontCollection> Collection;
  if (FAILED(Factory->GetSystemFontCollection(&Collection)))
    return false;
  UINT32 iFamilies = Collection->GetFontFamilyCount();
  for (UINT32 i = 0; i < iFamilies; i++)
  {
    CComPtr<IDWriteFontFamily> Family;
    CComPtr<IDWriteFont> Font;
    if (FAILED(Collection->GetFontFamily(i,&Family)))
      continue;
    if (FAILED(Family->GetFirstMatchingFont(DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL,DWRITE_FONT_STYLE_NORMAL,&Font)))
    {
      continue;
    }
    if (SUCCEEDED(Font->HasCharacter(c,&bHas)) && bHas)
      return true;
  }
  return false;
}
