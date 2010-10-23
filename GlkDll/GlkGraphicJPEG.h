/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkGraphicJPEG
// Glk interface for JPEG graphic loader
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_GRAPHIC_JPEG_H_
#define WINGLK_GRAPHIC_JPEG_H_

#include "GlkGraphic.h"

/////////////////////////////////////////////////////////////////////////////
// Class for JPEG graphic loader
/////////////////////////////////////////////////////////////////////////////

class CWinGlkJPEGGraphicLoader : public CWinGlkGraphicLoader
{
public:
  CWinGlkJPEGGraphicLoader() {}
  virtual ~CWinGlkJPEGGraphicLoader() {}

public:
  // Get the file extension for graphics supported for this loader
  virtual LPCTSTR GetFileExtension(void);

  // Get the identifier for graphics supported for this loader
  virtual glui32 GetIdentifier(void);
  
  // Load a graphic from the given data
  virtual CWinGlkGraphic* LoadGraphic(BYTE* pData, UINT iLength, BOOL bLoad, BOOL bApplyAlpha);
};

#endif // WINGLK_GRAPHIC_JPEG_H_
