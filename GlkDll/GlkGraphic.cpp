/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkGraphic
// Glk interface for graphic loaders
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkGraphicJPEG.h"
#include "GlkGraphicPNG.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Class for returned graphic
/////////////////////////////////////////////////////////////////////////////

CWinGlkGraphic::CWinGlkGraphic()
{
  m_pPixels = NULL;
  m_pHeader = NULL;
  m_dwWidth = 0;
  m_dwHeight = 0;

  m_iWidth = 0;
  m_iHeight = 0;
  m_bAlpha = false;

  m_iDisplay = 0;
  m_ImageRule = 0;
  m_iFixedWidth = 0;
  m_iFixedHeight = 0;
  m_ScaleWidth = 1.0;
  m_ScaleHeight = 1.0;
  m_MaxWidth = 1.0;
};

CWinGlkGraphic::~CWinGlkGraphic()
{
  if (m_pHeader)
    delete m_pHeader;
  if (m_pPixels)
    delete[] m_pPixels;
}

/////////////////////////////////////////////////////////////////////////////
// Base class for graphic loaders
/////////////////////////////////////////////////////////////////////////////

// Set up the loaders
void CWinGlkGraphicLoader::InitLoaders(void)
{
  m_Loaders.Add(new CWinGlkJPEGGraphicLoader());
  m_Loaders.Add(new CWinGlkPNGGraphicLoader());
}

// Delete the loaders
void CWinGlkGraphicLoader::RemoveLoaders(void)
{
  for (int i = 0; i < m_Loaders.GetSize(); i++)
    delete m_Loaders[i];
  m_Loaders.RemoveAll();
}

// Get a loader for a given graphic identifier
CWinGlkGraphicLoader* CWinGlkGraphicLoader::GetLoaderForID(glui32 id)
{
  for (int i = 0; i < GetLoaderCount(); i++)
  {
    CWinGlkGraphicLoader* pLoader = GetLoader(i);
    if (pLoader->GetIdentifier() == id)
      return pLoader;
  }
  return NULL;
}

CArray<CWinGlkGraphicLoader*,CWinGlkGraphicLoader*> CWinGlkGraphicLoader::m_Loaders;
