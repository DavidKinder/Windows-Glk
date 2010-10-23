/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkGraphicPNG
// Glk interface for PNG graphic loader
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkGraphicPNG.h"

#include "png.h"

extern "C"
{
#include "gi_blorb.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Ignore setjmp() warning
#pragma warning(disable : 4611)

namespace {

/////////////////////////////////////////////////////////////////////////////
// Memory Data Source
/////////////////////////////////////////////////////////////////////////////

struct DataIO
{
  BYTE* pGraphicData;
  ULONG ulOffset;
};

void ReadData(png_structp png_ptr, png_bytep data, png_size_t length)
{
  DataIO* pData = (DataIO*)png_get_io_ptr(png_ptr);
  memcpy(data,pData->pGraphicData+pData->ulOffset,length);
  pData->ulOffset += length;
}

} // unnamed namespace

/////////////////////////////////////////////////////////////////////////////
// Class for PNG graphic loader
/////////////////////////////////////////////////////////////////////////////

// Get the file extension for graphics supported for this loader
LPCTSTR CWinGlkPNGGraphicLoader::GetFileExtension(void)
{
  return "png";
}

// Get the identifier for graphics supported for this loader
glui32 CWinGlkPNGGraphicLoader::GetIdentifier(void)
{
  return giblorb_make_id('P','N','G',' ');
}

// Load a graphic from the given data
CWinGlkGraphic* CWinGlkPNGGraphicLoader::LoadGraphic(BYTE* pData, UINT iLength, BOOL bLoad, BOOL bApplyAlpha)
{
  CWinGlkGraphic* pGraphic = NULL;
  png_bytep* pRowPointers = NULL;

  if (!png_check_sig(pData,8))
    return NULL;

  png_structp png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING,(png_voidp)NULL,NULL,NULL);
  if (!png_ptr)
    return NULL;

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct(&png_ptr,
      (png_infopp)NULL,(png_infopp)NULL);
    return NULL;
  }

  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info)
  {
    png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
    return NULL;
  }

  if (setjmp(png_ptr->jmpbuf))
  {
    png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
    if (pRowPointers)
      delete[] pRowPointers;
    if (pGraphic)
      delete pGraphic;
    return NULL;
  }

  DataIO data;
  data.pGraphicData = pData;
  data.ulOffset = 8;
  png_set_read_fn(png_ptr,&data,ReadData);
  
  png_set_sig_bytes(png_ptr,8);
  png_read_info(png_ptr,info_ptr);

  png_uint_32 width = png_get_image_width(png_ptr,info_ptr);
  png_uint_32 height = png_get_image_height(png_ptr,info_ptr);
  int bit_depth = png_get_bit_depth(png_ptr,info_ptr);
  int color_type = png_get_color_type(png_ptr,info_ptr);

  pGraphic = new CWinGlkGraphic;
  pGraphic->m_dwWidth = width;
  pGraphic->m_dwHeight = height;

  if (bLoad)
  {
    if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8)
      png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
      png_set_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
      png_set_tRNS_to_alpha(png_ptr);

    if (bit_depth == 16)
      png_set_strip_16(png_ptr);
    if (bit_depth < 8)
      png_set_packing(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      png_set_gray_to_rgb(png_ptr);

    png_set_bgr(png_ptr);
    png_set_filler(png_ptr,0,PNG_FILLER_AFTER);

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

    pRowPointers = new png_bytep[height];
    for (int i = 0; i < (int)height; i++)
      pRowPointers[i] = pGraphic->m_pPixels+(width*i*4);
    png_read_image(png_ptr,pRowPointers);
    png_read_end(png_ptr,end_info);

    for (i = 0; i < size; i += 4)
    {
      if (pGraphic->m_pPixels[i+3] != 0)
        pGraphic->m_bAlpha = true;
    }
    if (bApplyAlpha && pGraphic->m_bAlpha)
    {
      for (i = 0; i < size; i += 4)
      {
        int alpha = pGraphic->m_pPixels[i+3];

        // Rescale from 0..255 to 0..256
        alpha += alpha>>7;
        for (int j = 0; j < 3; j++)
          pGraphic->m_pPixels[i+j] = (BYTE)((alpha * pGraphic->m_pPixels[i+j]) >> 8);
      }
    }
  }

  png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
  if (pRowPointers)
    delete[] pRowPointers;

  return pGraphic;
}
