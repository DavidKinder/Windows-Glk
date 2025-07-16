/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkGraphic
// Glk interface for graphic loaders
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_GRAPHIC_H_
#define WINGLK_GRAPHIC_H_

extern "C"
{
#include "glk.h"
}

/////////////////////////////////////////////////////////////////////////////
// Class for returned graphic
/////////////////////////////////////////////////////////////////////////////

class CWinGlkGraphic
{
public:
  CWinGlkGraphic();
  ~CWinGlkGraphic();

  BYTE* m_pPixels;
  BITMAPINFOHEADER *m_pHeader;
  DWORD m_dwWidth;
  DWORD m_dwHeight;

  int m_iWidth;
  int m_iHeight;
  bool m_bAlpha;

  int m_iDisplay;
  unsigned int m_ImageRule;
  int m_iFixedWidth;
  int m_iFixedHeight;
  double m_ScaleWidth;
  double m_ScaleHeight;
  double m_MaxWidth;
};

/////////////////////////////////////////////////////////////////////////////
// Base class for graphic loaders
/////////////////////////////////////////////////////////////////////////////

class CWinGlkGraphicLoader
{
public:
  CWinGlkGraphicLoader() {}
  virtual ~CWinGlkGraphicLoader() {}

public:
  // Get the file extension for graphics supported for this loader
  virtual LPCTSTR GetFileExtension(void) = 0;

  // Get the identifier for graphics supported for this loader
  virtual glui32 GetIdentifier(void) = 0;
  
  // Load a graphic from the given data
  virtual CWinGlkGraphic* LoadGraphic(BYTE* pData, UINT iLength, BOOL bLoad, BOOL bApplyAlpha) = 0;

/////////////////////////////////////////////////////////////////////////////
// Static data and member functions

public:
  // Set up the loaders
  static void InitLoaders(void);

  // Delete the loaders
  static void RemoveLoaders(void);

  // Get the number of loaders
  static int GetLoaderCount(void) { return m_Loaders.GetSize(); }

  // Get a loader by index
  static CWinGlkGraphicLoader* GetLoader(int iIndex) { return m_Loaders.GetAt(iIndex); }

  // Get a loader for a given graphic identifier
  static CWinGlkGraphicLoader* GetLoaderForID(glui32 id);

protected:
  static CArray<CWinGlkGraphicLoader*,CWinGlkGraphicLoader*> m_Loaders;
};

#endif // WINGLK_GRAPHIC_H_
