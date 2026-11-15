/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkGraphicGIF
// Glk interface for GIF graphic loader
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_GRAPHIC_GIF_H_
#define WINGLK_GRAPHIC_GIF_H_

#include "GlkGraphic.h"

/////////////////////////////////////////////////////////////////////////////
// Class for GIF graphic loader
/////////////////////////////////////////////////////////////////////////////

class CWinGlkGIFGraphicLoader : public CWinGlkGraphicLoader
{
public:
  CWinGlkGIFGraphicLoader() {}
  virtual ~CWinGlkGIFGraphicLoader() {}

public:
  // Get the file extension for graphics supported for this loader
  virtual LPCTSTR GetFileExtension(void);

  // Get the identifier for graphics supported for this loader
  virtual glui32 GetIdentifier(void);
  
  // Load a graphic from the given data
  virtual CWinGlkGraphic* LoadGraphic(BYTE* pData, UINT iLength, BOOL bLoad, BOOL bApplyAlpha);
};

#endif // WINGLK_GRAPHIC_GIF_H_
