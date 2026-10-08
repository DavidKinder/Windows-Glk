/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkGraphicGIF
// Glk interface for GIF graphic loader
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkGraphicGIF.h"

#include <atlbase.h>
#include <wincodec.h>

extern "C"
{
#include "gi_blorb.h"
}

#pragma comment(lib,"windowscodecs.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Class for GIF graphic loader
/////////////////////////////////////////////////////////////////////////////

// Get the file extension for graphics supported for this loader
LPCTSTR CWinGlkGIFGraphicLoader::GetFileExtension(void)
{
  return "gif";
}

// Get the identifier for graphics supported for this loader. This is not one
// of the Blorb specification's picture formats, but ADRIFT 5 writes it.
glui32 CWinGlkGIFGraphicLoader::GetIdentifier(void)
{
  return giblorb_make_id('G','I','F',' ');
}

// Load a graphic from the given data. GIFs are decoded with the Windows
// Imaging Component; only the first frame of an animated GIF is shown.
CWinGlkGraphic* CWinGlkGIFGraphicLoader::LoadGraphic(BYTE* pData, UINT iLength, BOOL bLoad, BOOL bApplyAlpha)
{
  if ((iLength < 6) || (memcmp(pData,"GIF",3) != 0))
    return NULL;

  CComPtr<IWICImagingFactory> factory;
  if (FAILED(factory.CoCreateInstance(CLSID_WICImagingFactory)))
    return NULL;

  CComPtr<IWICStream> stream;
  if (FAILED(factory->CreateStream(&stream)))
    return NULL;
  if (FAILED(stream->InitializeFromMemory(pData,iLength)))
    return NULL;

  CComPtr<IWICBitmapDecoder> decoder;
  if (FAILED(factory->CreateDecoder(GUID_ContainerFormatGif,NULL,&decoder)))
    return NULL;
  if (FAILED(decoder->Initialize(stream,WICDecodeMetadataCacheOnDemand)))
    return NULL;

  CComPtr<IWICBitmapFrameDecode> frame;
  if (FAILED(decoder->GetFrame(0,&frame)))
    return NULL;

  UINT width = 0, height = 0;
  if (FAILED(frame->GetSize(&width,&height)) || (width == 0) || (height == 0))
    return NULL;

  CWinGlkGraphic* pGraphic = new CWinGlkGraphic;
  pGraphic->m_dwWidth = width;
  pGraphic->m_dwHeight = height;

  if (bLoad)
  {
    CComPtr<IWICFormatConverter> converter;
    if (FAILED(factory->CreateFormatConverter(&converter)) ||
      FAILED(converter->Initialize(frame,GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone,NULL,0.0,WICBitmapPaletteTypeCustom)))
    {
      delete pGraphic;
      return NULL;
    }

    pGraphic->m_pHeader = new BITMAPINFOHEADER;
    ZeroMemory(pGraphic->m_pHeader,sizeof(BITMAPINFOHEADER));
    pGraphic->m_pHeader->biSize = sizeof(BITMAPINFOHEADER);
    pGraphic->m_pHeader->biWidth = width;
    pGraphic->m_pHeader->biHeight = height*-1;
    pGraphic->m_pHeader->biPlanes = 1;
    pGraphic->m_pHeader->biBitCount = 32;
    pGraphic->m_pHeader->biCompression = BI_RGB;

    int size = width*height*4;
    pGraphic->m_pPixels = new BYTE[size];
    if (FAILED(converter->CopyPixels(NULL,width*4,size,pGraphic->m_pPixels)))
    {
      delete pGraphic;
      return NULL;
    }

    // A GIF has an alpha channel only if it has transparent pixels
    for (int i = 0; i < size; i += 4)
    {
      if (pGraphic->m_pPixels[i+3] != 0xFF)
        pGraphic->m_bAlpha = true;
    }
    if (bApplyAlpha && pGraphic->m_bAlpha)
    {
      for (int i = 0; i < size; i += 4)
      {
        int alpha = pGraphic->m_pPixels[i+3];

        // Rescale from 0..255 to 0..256
        alpha += alpha>>7;
        for (int j = 0; j < 3; j++)
          pGraphic->m_pPixels[i+j] = (BYTE)((alpha * pGraphic->m_pPixels[i+j]) >> 8);
      }
    }
  }
  return pGraphic;
}
