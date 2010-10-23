/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkGraphicPNG
// Glk interface for PNG graphic loader
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_GRAPHIC_PNG_H_
#define WINGLK_GRAPHIC_PNG_H_

#include "GlkGraphic.h"

/////////////////////////////////////////////////////////////////////////////
// Class for PNG graphic loader
/////////////////////////////////////////////////////////////////////////////

class CWinGlkPNGGraphicLoader : public CWinGlkGraphicLoader
{
public:
  CWinGlkPNGGraphicLoader() {}
  virtual ~CWinGlkPNGGraphicLoader() {}

public:
  // Get the file extension for graphics supported for this loader
  virtual LPCTSTR GetFileExtension(void);

  // Get the identifier for graphics supported for this loader
  virtual glui32 GetIdentifier(void);
  
  // Load a graphic from the given data
  virtual CWinGlkGraphic* LoadGraphic(BYTE* pData, UINT iLength, BOOL bLoad, BOOL bApplyAlpha);
};

#endif // WINGLK_GRAPHIC_PNG_H_
