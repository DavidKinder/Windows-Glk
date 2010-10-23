/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkStyle
// Glk style classes
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_STYLE_H_
#define WINGLK_STYLE_H_

extern "C"
{
#include "glk.h"
}

/////////////////////////////////////////////////////////////////////////////
// Classes for Glk styles
/////////////////////////////////////////////////////////////////////////////

struct CWinGlkStyle
{
  void SetStyle(int iStyle);

  glsi32 m_Indent;
  glsi32 m_ParaIndent;
  glsi32 m_Justify;
  glsi32 m_Size;
  glsi32 m_Weight;
  glsi32 m_Oblique;
  glsi32 m_Proportional;
  glsi32 m_TextColour;
  glsi32 m_BackColour;
  glsi32 m_ReverseColour;

  bool m_bUserControl;
};

class CWinGlkStyles
{
public:
  CWinGlkStyles();
  CWinGlkStyles(const CWinGlkStyles& Copy);
  CWinGlkStyles& operator=(const CWinGlkStyles& Copy);
  CWinGlkStyle* GetStyle(int iStyle);
  CWinGlkStyle* GetNoHintStyle(int iStyle);

  void ReadSettings(LPCTSTR pszFormat, int iVersion);
  void WriteSettings(LPCTSTR pszFormat);

protected:
  CWinGlkStyle m_Styles[style_NUMSTYLES];
  CWinGlkStyle m_NoHints[style_NUMSTYLES];
};

#endif // WINGLK_STYLE_H_
