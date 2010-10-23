/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC Glk Libraries
//
// GlkSound
// Glk sound objects
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkSoundAIFF.h"
#include "GlkSoundMOD.h"
#include "GlkSoundOGG.h"
#include "GlkSoundSONG.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Class for loaded sounds
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CWinGlkSound,CObject);

CWinGlkSound::CWinGlkSound(LPCTSTR pszFileName)
{
  m_bDelete = true;
  m_pData = NULL;
  m_iLength = 0;

  m_iRepeat = 0;

  CFile SoundFile;
  if (SoundFile.Open(pszFileName,CFile::modeRead|CFile::shareDenyWrite))
  {
    m_iLength = (int)SoundFile.GetLength();
    m_pData = new BYTE[m_iLength];
    SoundFile.Read(m_pData,m_iLength);
  }
}

CWinGlkSound::CWinGlkSound(BYTE* pData, int iLength)
{
  m_bDelete = false;
  m_pData = pData;
  m_iLength = iLength;

  m_iRepeat = 0;
}

CWinGlkSound::~CWinGlkSound()
{
  if (m_bDelete)
    delete[] m_pData;
}

double CWinGlkSound::DecibelVolume(int iVolume)
{
  // The volume argument is from 0 to 0x10000, with 0x8000
  // representing half volume. This is converted to a decibel
  // level, with -10dB representing half volume, -20dB one
  // quarter volume, and so on.
  // Zero volume is taken to be -100dB.

  if (iVolume < 0)
    iVolume = 0;
  if (iVolume > 0x10000)
    iVolume = 0x10000;

  double dDecibels;
  if (iVolume == 0)
    dDecibels = -100.0;
  else
    dDecibels = -10.0 * (log((double)0x10000 / (double)iVolume) / log(2.0));

  if (dDecibels > 0.0)
    dDecibels = 0.0;
  if (dDecibels < -100.0)
    dDecibels = -100.0;
  return dDecibels;
}

/////////////////////////////////////////////////////////////////////////////
// Base class for sound loaders
/////////////////////////////////////////////////////////////////////////////

// Set up the loaders
void CWinGlkSoundLoader::InitLoaders(void)
{
  m_Loaders.Add(new CWinGlkAIFFSoundLoader());
  m_Loaders.Add(new CWinGlkOGGSoundLoader());
  m_Loaders.Add(new CWinGlkMODSoundLoader());
#ifndef I7GLK
  m_Loaders.Add(new CWinGlkSONGSoundLoader());
#endif
}

// Notify loaders to clean up after sound playback has been stopped
void CWinGlkSoundLoader::AllSoundStopped(void)
{
  for (int i = 0; i < m_Loaders.GetSize(); i++)
    m_Loaders[i]->SoundStopped();
}

// Delete the loaders
void CWinGlkSoundLoader::RemoveLoaders(void)
{
  for (int i = 0; i < m_Loaders.GetSize(); i++)
    delete m_Loaders[i];
  m_Loaders.RemoveAll();
}

// Get a loader for a given sound identifier
CWinGlkSoundLoader* CWinGlkSoundLoader::GetLoaderForID(glui32 id)
{
  for (int i = 0; i < GetLoaderCount(); i++)
  {
    CWinGlkSoundLoader* pLoader = GetLoader(i);
    if (pLoader->GetIdentifier() == id)
      return pLoader;
  }
  return NULL;
}

CArray<CWinGlkSoundLoader*,CWinGlkSoundLoader*> CWinGlkSoundLoader::m_Loaders;
