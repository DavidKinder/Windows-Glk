/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkGraphicCustom
// Glk interface for custom graphic loader
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkGraphicCustom.h"

extern "C"
{
#include "gi_blorb.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Class for custom graphic loader
/////////////////////////////////////////////////////////////////////////////

CWinGlkCustomGraphicLoader::CWinGlkCustomGraphicLoader(winglk_graphic_format* pGraphicFormat)
{
  m_GraphicFormat = *pGraphicFormat;
}

// Get the file extension for graphics supported for this loader
LPCTSTR CWinGlkCustomGraphicLoader::GetFileExtension(void)
{
  return m_GraphicFormat.ext;
}

// Get the identifier for graphics supported for this loader
glui32 CWinGlkCustomGraphicLoader::GetIdentifier(void)
{
  return m_GraphicFormat.id;
}

// Load a graphic from the given data
CWinGlkGraphic* CWinGlkCustomGraphicLoader::LoadGraphic(BYTE* pData, UINT iLength, BOOL bLoad, BOOL bApplyAlpha)
{
  // Get the dimensions
  int iWidth, iHeight;
  DWORD_PTR id = m_GraphicFormat.create(pData,(int)iLength,&iWidth,&iHeight);
  if (id == 0)
    return false;

  // Create the graphic object
  CWinGlkGraphic* pGraphic = new CWinGlkGraphic;
  pGraphic->m_dwWidth = (DWORD)iWidth;
  pGraphic->m_dwHeight = (DWORD)iHeight;

  if (bLoad)
  {
    pGraphic->m_pHeader = new BITMAPINFOHEADER;
    ZeroMemory(pGraphic->m_pHeader,sizeof(BITMAPINFOHEADER));
    pGraphic->m_pHeader->biSize = sizeof(BITMAPINFOHEADER);
    pGraphic->m_pHeader->biWidth = iWidth;
    pGraphic->m_pHeader->biHeight = iHeight*-1;
    pGraphic->m_pHeader->biPlanes = 1;
    pGraphic->m_pHeader->biBitCount = 32;
    pGraphic->m_pHeader->biCompression = BI_RGB;

    int iSize = iWidth*iHeight*4;
    pGraphic->m_pPixels = new BYTE[iSize];

    if (m_GraphicFormat.draw(id,pGraphic->m_pPixels))
    {
      for (int i = 0; i < iSize; i += 4)
      {
        if (pGraphic->m_pPixels[i+3] != 0)
          pGraphic->m_bAlpha = true;
      }
      if (bApplyAlpha && pGraphic->m_bAlpha)
      {
        for (int i = 0; i < iSize; i += 4)
        {
          int alpha = pGraphic->m_pPixels[i+3];

          // Rescale from 0..255 to 0..256
          alpha += alpha>>7;
          for (int j = 0; j < 3; j++)
            pGraphic->m_pPixels[i+j] = (BYTE)((alpha * pGraphic->m_pPixels[i+j]) >> 8);
        }
      }
    }
    else
    {
      delete pGraphic;
      pGraphic = NULL;
    }
  }

  m_GraphicFormat.destroy(id);
  return pGraphic;
}
