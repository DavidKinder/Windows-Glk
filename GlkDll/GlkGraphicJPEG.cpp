/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkGraphicJPEG
// Glk interface for JPEG graphic loader
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkGraphicJPEG.h"

#include <setjmp.h>

#include "jpeglib.h"

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
// Error Handling
/////////////////////////////////////////////////////////////////////////////

struct ErrorInfo
{
  struct jpeg_error_mgr Base;
  jmp_buf ErrorJump;
};

void ErrorExit(j_common_ptr cinfo)
{
  (*cinfo->err->output_message)(cinfo);
  struct ErrorInfo* error = (struct ErrorInfo*)cinfo->err;
  longjmp(error->ErrorJump,1);
}

void OutputMessage(j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];
  (*cinfo->err->format_message)(cinfo,buffer);
  TRACE("Glk JPEG: %s\n",buffer);
}

/////////////////////////////////////////////////////////////////////////////
// Memory Data Source
/////////////////////////////////////////////////////////////////////////////

void MemInit(j_decompress_ptr cinfo)
{
}

boolean MemFillInput(j_decompress_ptr cinfo)
{
  return FALSE;
}

void MemSkipInput(j_decompress_ptr cinfo, long num_bytes)
{
  if (num_bytes > 0)
  {
    if (num_bytes > (long)cinfo->src->bytes_in_buffer)
      num_bytes = (long)cinfo->src->bytes_in_buffer;

    cinfo->src->next_input_byte += num_bytes;
    cinfo->src->bytes_in_buffer -= num_bytes;
  }
}

void MemTerm(j_decompress_ptr cinfo)
{
}

} // unnamed namespace

/////////////////////////////////////////////////////////////////////////////
// Class for JPEG graphic loader
/////////////////////////////////////////////////////////////////////////////

// Get the file extension for graphics supported for this loader
LPCTSTR CWinGlkJPEGGraphicLoader::GetFileExtension(void)
{
  return "jpg";
}

// Get the identifier for graphics supported for this loader
glui32 CWinGlkJPEGGraphicLoader::GetIdentifier(void)
{
  return giblorb_make_id('J','P','E','G');
}

// Load a graphic from the given data
CWinGlkGraphic* CWinGlkJPEGGraphicLoader::LoadGraphic(BYTE* pData, UINT iLength, BOOL bLoad, BOOL bApplyAlpha)
{
  CWinGlkGraphic* pGraphic = NULL;

  struct jpeg_decompress_struct JpegInfo;
  struct ErrorInfo JpegError;

  JpegInfo.err = jpeg_std_error(&(JpegError.Base));
  JpegError.Base.error_exit = ErrorExit;
  JpegError.Base.output_message = OutputMessage;
  if (setjmp(JpegError.ErrorJump))
  {
    jpeg_destroy_decompress(&JpegInfo);
    if (pGraphic)
      delete pGraphic;
    return NULL;
  }

  jpeg_create_decompress(&JpegInfo);

  JpegInfo.src = (struct jpeg_source_mgr*)(JpegInfo.mem->alloc_small)
    ((j_common_ptr)(&JpegInfo),JPOOL_PERMANENT,sizeof(jpeg_source_mgr));
  JpegInfo.src->init_source = MemInit;
  JpegInfo.src->fill_input_buffer = MemFillInput;
  JpegInfo.src->skip_input_data = MemSkipInput;
  JpegInfo.src->resync_to_restart = jpeg_resync_to_restart;
  JpegInfo.src->term_source = MemTerm;
  JpegInfo.src->bytes_in_buffer = iLength;
  JpegInfo.src->next_input_byte = pData;

  jpeg_read_header(&JpegInfo,TRUE);
  jpeg_calc_output_dimensions(&JpegInfo);
  int width = JpegInfo.output_width;
  int height = JpegInfo.output_height;

  pGraphic = new CWinGlkGraphic();
  pGraphic->m_dwWidth = width;
  pGraphic->m_dwHeight = height;
    
  if (bLoad)
  {
    pGraphic->m_pHeader = new BITMAPINFOHEADER;
    ZeroMemory(pGraphic->m_pHeader,sizeof(BITMAPINFOHEADER));
    pGraphic->m_pHeader->biSize = sizeof(BITMAPINFOHEADER);
    pGraphic->m_pHeader->biWidth = width;
    pGraphic->m_pHeader->biHeight = height*-1;
    pGraphic->m_pHeader->biPlanes = 1;
    pGraphic->m_pHeader->biBitCount = 32;
    pGraphic->m_pHeader->biCompression = BI_RGB;
    pGraphic->m_pPixels = new BYTE[width*height*4];

    // Force RGB output
    JpegInfo.out_color_space = JCS_RGB;

    // Get an output buffer
    JSAMPARRAY buffer = (*JpegInfo.mem->alloc_sarray)
      ((j_common_ptr)&JpegInfo,JPOOL_IMAGE,width*3,1);

    jpeg_start_decompress(&JpegInfo);
    while ((int)JpegInfo.output_scanline < height)
    {
      jpeg_read_scanlines(&JpegInfo,buffer,1);

      BYTE* pPixelRow = pGraphic->m_pPixels+
        (width*(JpegInfo.output_scanline-1)*4);
      for (int i = 0; i < width; i++)
      {
        pPixelRow[(i*4)+0] = (*buffer)[(i*3)+2];
        pPixelRow[(i*4)+1] = (*buffer)[(i*3)+1];
        pPixelRow[(i*4)+2] = (*buffer)[(i*3)+0];
        pPixelRow[(i*4)+3] = 0x00;
      }
    }
    jpeg_finish_decompress(&JpegInfo);
  }
  jpeg_destroy_decompress(&JpegInfo);
  return pGraphic;
}
