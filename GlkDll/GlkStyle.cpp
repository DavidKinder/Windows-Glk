/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkStyle
// Glk styles
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkDll.h"
#include "GlkStyle.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Classes for Glk styles
/////////////////////////////////////////////////////////////////////////////

CWinGlkStyles::CWinGlkStyles()
{
  for (int i = 0; i < style_NUMSTYLES; i++)
  {
    m_Styles[i].SetStyle(i);
    m_Styles[i].m_bUserControl = true;
    m_NoHints[i] = m_Styles[i];
  }
}

CWinGlkStyles::CWinGlkStyles(const CWinGlkStyles& Copy)
{
  for (int i = 0; i < style_NUMSTYLES; i++)
  {
    m_Styles[i] = Copy.m_Styles[i];
    m_NoHints[i] = Copy.m_NoHints[i];
  }
}

CWinGlkStyles& CWinGlkStyles::operator=(const CWinGlkStyles& Copy)
{
  for (int i = 0; i < style_NUMSTYLES; i++)
  {
    m_Styles[i] = Copy.m_Styles[i];
    m_NoHints[i] = Copy.m_NoHints[i];
  }
  return *this;
}

CWinGlkStyle* CWinGlkStyles::GetStyle(int iStyle)
{
  if ((iStyle >= style_Normal) && (iStyle < style_NUMSTYLES))
    return &m_Styles[iStyle];
  return NULL;
}

CWinGlkStyle* CWinGlkStyles::GetNoHintStyle(int iStyle)
{
  if ((iStyle >= style_Normal) && (iStyle < style_NUMSTYLES))
    return &m_NoHints[iStyle];
  return NULL;
}

void CWinGlkStyles::ReadSettings(LPCTSTR pszFormat, int iVersion)
{
  CString strSection;
  CWinGlkStyle* pStyle;
  CWinApp* pApp = AfxGetApp();

  for (int i = 0; i < style_NUMSTYLES; i++)
  {
    strSection.Format(pszFormat,i);
    pStyle = GetStyle(i);

    glsi32 Indent = pApp->GetProfileInt(strSection,"Indentation",pStyle->m_Indent);
    if ((iVersion < 131) && (Indent < pStyle->m_Indent))
      Indent = pStyle->m_Indent;
    pStyle->m_Indent = Indent;

    pStyle->m_ParaIndent = pApp->GetProfileInt(strSection,"Paragraph indentation",pStyle->m_ParaIndent);
    pStyle->m_Justify = pApp->GetProfileInt(strSection,"Justification",pStyle->m_Justify);
    pStyle->m_Size = pApp->GetProfileInt(strSection,"Size",pStyle->m_Size);
    pStyle->m_Weight = pApp->GetProfileInt(strSection,"Weight",pStyle->m_Weight);
    pStyle->m_Oblique = pApp->GetProfileInt(strSection,"Oblique",pStyle->m_Oblique);
    pStyle->m_Proportional = pApp->GetProfileInt(strSection,"Proportional",pStyle->m_Proportional);
    pStyle->m_ReverseColour = pApp->GetProfileInt(strSection,"Reverse Colour",pStyle->m_ReverseColour);
  }

  for (int i = 0; i < style_NUMSTYLES; i++)
    m_NoHints[i] = m_Styles[i];
}

void CWinGlkStyles::WriteSettings(LPCTSTR pszFormat)
{
  CString strSection;
  CWinGlkStyle* pStyle;
  CWinApp* pApp = AfxGetApp();

  for (int i = 0; i < style_NUMSTYLES; i++)
  {
    strSection.Format(pszFormat,i);
    pStyle = GetStyle(i);
    if (pStyle->m_bUserControl)
    {
      pApp->WriteProfileInt(strSection,"Indentation",pStyle->m_Indent);
      pApp->WriteProfileInt(strSection,"Paragraph indentation",pStyle->m_ParaIndent);
      pApp->WriteProfileInt(strSection,"Justification",pStyle->m_Justify);
      pApp->WriteProfileInt(strSection,"Size",pStyle->m_Size);
      pApp->WriteProfileInt(strSection,"Weight",pStyle->m_Weight);
      pApp->WriteProfileInt(strSection,"Oblique",pStyle->m_Oblique);
      pApp->WriteProfileInt(strSection,"Proportional",pStyle->m_Proportional);
      pApp->WriteProfileInt(strSection,"Reverse Colour",pStyle->m_ReverseColour);
    }
  }
}

void CWinGlkStyle::SetStyle(int iStyle)
{
  switch (iStyle)
  {
  case style_Normal:
  case style_User1:
  case style_User2:
  default:
    m_Indent = 1;
    m_ParaIndent = 0;
    m_Justify = stylehint_just_LeftFlush;
    m_Size = 0;
    m_Weight = 0;
    m_Oblique = 0;
    m_Proportional = 1;
    m_TextColour = WINGLK_COLOUR_TEXT;
    m_BackColour = WINGLK_COLOUR_BACK;
    m_ReverseColour = 0;
    break;
  case style_Emphasized:
    m_Indent = 1;
    m_ParaIndent = 0;
    m_Justify = stylehint_just_LeftFlush;
    m_Size = 0;
    m_Weight = 1;
    m_Oblique = 0;
    m_Proportional = 1;
    m_TextColour = WINGLK_COLOUR_TEXT;
    m_BackColour = WINGLK_COLOUR_BACK;
    m_ReverseColour = 0;
    break;
  case style_Preformatted:
    m_Indent = 1;
    m_ParaIndent = 0;
    m_Justify = stylehint_just_LeftFlush;
    m_Size = 0;
    m_Weight = 0;
    m_Oblique = 0;
    m_Proportional = 0;
    m_TextColour = WINGLK_COLOUR_TEXT;
    m_BackColour = WINGLK_COLOUR_BACK;
    m_ReverseColour = 0;
    break;
  case style_Header:
    m_Indent = 1;
    m_ParaIndent = 0;
    m_Justify = stylehint_just_LeftFlush;
    m_Size = 2;
    m_Weight = 1;
    m_Oblique = 0;
    m_Proportional = 1;
    m_TextColour = WINGLK_COLOUR_TEXT;
    m_BackColour = WINGLK_COLOUR_BACK;
    m_ReverseColour = 0;
    break;
  case style_Subheader:
    m_Indent = 1;
    m_ParaIndent = 0;
    m_Justify = stylehint_just_LeftFlush;
    m_Size = 1;
    m_Weight = 1;
    m_Oblique = 0;
    m_Proportional = 1;
    m_TextColour = WINGLK_COLOUR_TEXT;
    m_BackColour = WINGLK_COLOUR_BACK;
    m_ReverseColour = 0;
    break;
  case style_Alert:
    m_Indent = 1;
    m_ParaIndent = 0;
    m_Justify = stylehint_just_LeftFlush;
    m_Size = 1;
    m_Weight = 1;
    m_Oblique = 0;
    m_Proportional = 1;
    m_TextColour = WINGLK_COLOUR_TEXT;
    m_BackColour = WINGLK_COLOUR_BACK;
    m_ReverseColour = 0;
    break;
  case style_Note:
    m_Indent = 1;
    m_ParaIndent = 0;
    m_Justify = stylehint_just_LeftFlush;
    m_Size = 0;
    m_Weight = 0;
    m_Oblique = 1;
    m_Proportional = 1;
    m_TextColour = WINGLK_COLOUR_TEXT;
    m_BackColour = WINGLK_COLOUR_BACK;
    m_ReverseColour = 0;
    break;
  case style_BlockQuote:
    m_Indent = 3;
    m_ParaIndent = 0;
    m_Justify = stylehint_just_LeftFlush;
    m_Size = 0;
    m_Weight = 0;
    m_Oblique = 0;
    m_Proportional = 1;
    m_TextColour = WINGLK_COLOUR_TEXT;
    m_BackColour = WINGLK_COLOUR_BACK;
    m_ReverseColour = 0;
    break;
  case style_Input:
    m_Indent = 1;
    m_ParaIndent = 0;
    m_Justify = stylehint_just_LeftFlush;
    m_Size = 0;
    m_Weight = 1;
    m_Oblique = 0;
    m_Proportional = 1;
    m_TextColour = WINGLK_COLOUR_TEXT;
    m_BackColour = WINGLK_COLOUR_BACK;
    m_ReverseColour = 0;
    break;
  }
}
