/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkGraphicCustom
// Glk interface for custom graphic loader
//
/////////////////////////////////////////////////////////////////////////////

#ifndef WINGLK_GRAPHIC_CUSTOM_H_
#define WINGLK_GRAPHIC_CUSTOM_H_

#include "GlkGraphic.h"
#include "WinGlk.h"

/////////////////////////////////////////////////////////////////////////////
// Class for custom graphic loader
/////////////////////////////////////////////////////////////////////////////

class CWinGlkCustomGraphicLoader : public CWinGlkGraphicLoader
{
public:
  CWinGlkCustomGraphicLoader(winglk_graphic_format* pGraphicFormat);
  virtual ~CWinGlkCustomGraphicLoader() {}

public:
  // Get the file extension for graphics supported for this loader
  virtual LPCTSTR GetFileExtension(void);

  // Get the identifier for graphics supported for this loader
  virtual glui32 GetIdentifier(void);
  
  // Load a graphic from the given data
  virtual CWinGlkGraphic* LoadGraphic(BYTE* pData, UINT iLength, BOOL bLoad, BOOL bApplyAlpha);

protected:
  // Details of the custom format
  winglk_graphic_format m_GraphicFormat;
};

#endif // WINGLK_GRAPHIC_CUSTOM_H_
