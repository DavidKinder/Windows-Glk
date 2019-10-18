/////////////////////////////////////////////////////////////////////
// Simple speech interface for Glk
/////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GlkTalk.h"

#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////
// Interface to SAPI 5
/////////////////////////////////////////////////////////////////////

#ifndef __ISpVoice_INTERFACE_DEFINED__

typedef enum SPVPRIORITY {} SPVPRIORITY;
typedef enum SPEVENTENUM {} SPEVENTENUM;
typedef enum SPDATAKEYLOCATION {} SPDATAKEYLOCATION;

typedef enum SPEAKFLAGS {
  SPF_ASYNC = (1L<<0)
} SPEAKFLAGS;

MIDL_INTERFACE("5EFF4AEF-8487-11D2-961C-00C04F8EE628")
ISpNotifySource : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetNotifySink(IUnknown*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetNotifyWindowMessage(HWND, UINT, WPARAM, LPARAM) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetNotifyCallbackFunction(VOID*, WPARAM, LPARAM) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetNotifyCallbackInterface(IUnknown*, WPARAM, LPARAM) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetNotifyWin32Event(VOID) = 0;
  virtual HRESULT STDMETHODCALLTYPE WaitForNotifyEvent(DWORD) = 0;
  virtual HANDLE STDMETHODCALLTYPE GetNotifyEventHandle(VOID) = 0;
};

MIDL_INTERFACE("BE7A9CCE-5F9E-11D2-960F-00C04F8EE628")
ISpEventSource : public ISpNotifySource
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetInterest(ULONGLONG, ULONGLONG) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetEvents(ULONG, VOID*, ULONG*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetInfo(VOID*) = 0;
};

MIDL_INTERFACE("6C44DF74-72B9-4992-A1EC-EF996E0422D4")
ISpVoice : public ISpEventSource
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetOutput(IUnknown*, BOOL) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetOutputObjectToken(IUnknown**) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetOutputStream(IUnknown**) = 0;
  virtual HRESULT STDMETHODCALLTYPE Pause(VOID) = 0;
  virtual HRESULT STDMETHODCALLTYPE Resume(VOID) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetVoice(IUnknown*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetVoice(IUnknown**) = 0;
  virtual HRESULT STDMETHODCALLTYPE Speak(LPCWSTR, DWORD, ULONG*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SpeakStream(IStream*, DWORD, ULONG*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetStatus(VOID*, LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE Skip(LPCWSTR, long, ULONG*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetPriority(SPVPRIORITY) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetPriority(SPVPRIORITY*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetAlertBoundary(SPEVENTENUM) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetAlertBoundary(SPEVENTENUM*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetRate(long) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetRate(long*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetVolume(USHORT) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetVolume(USHORT*) = 0;
  virtual HRESULT STDMETHODCALLTYPE WaitUntilDone(ULONG) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetSyncSpeakTimeout(ULONG) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetSyncSpeakTimeout(ULONG*) = 0;
  virtual HANDLE STDMETHODCALLTYPE SpeakCompleteEvent(VOID) = 0;
  virtual HRESULT STDMETHODCALLTYPE IsUISupported(LPCWSTR, VOID*, ULONG, BOOL*) = 0;
  virtual  HRESULT STDMETHODCALLTYPE DisplayUI(HWND, LPCWSTR, LPCWSTR, VOID*, ULONG) = 0;
};

MIDL_INTERFACE("14056581-E16C-11D2-BB90-00C04F8EE6C0")
ISpDataKey : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetData(LPCWSTR, ULONG, const BYTE*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetData(LPCWSTR, ULONG*, BYTE*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetStringValue(LPCWSTR, LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetStringValue(LPCWSTR, LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetDWORD(LPCWSTR, DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDWORD(LPCWSTR, DWORD*) = 0;
  virtual HRESULT STDMETHODCALLTYPE OpenKey(LPCWSTR, ISpDataKey**) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateKey(LPCWSTR, ISpDataKey**) = 0;
  virtual HRESULT STDMETHODCALLTYPE DeleteKey(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE DeleteValue(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE EnumKeys(ULONG, LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE EnumValues(ULONG, LPWSTR*) = 0;
};

typedef interface ISpObjectTokenCategory ISpObjectTokenCategory;
MIDL_INTERFACE("14056589-E16C-11D2-BB90-00C04F8EE6C0")
ISpObjectToken : public ISpDataKey
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetId(LPCWSTR, LPCWSTR, BOOL) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetId(LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCategory(ISpObjectTokenCategory**) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown*, DWORD, REFIID, void**) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetStorageFileName(REFCLSID, LPCWSTR, LPCWSTR, ULONG, LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE RemoveStorageFileName(REFCLSID clsidCaller, LPCWSTR, BOOL) = 0;
  virtual HRESULT STDMETHODCALLTYPE Remove(const CLSID*) = 0;
  virtual HRESULT STDMETHODCALLTYPE IsUISupported(LPCWSTR, void*, ULONG, IUnknown*, BOOL*) = 0;
  virtual HRESULT STDMETHODCALLTYPE DisplayUI(HWND, LPCWSTR, LPCWSTR, void*, ULONG, IUnknown*) = 0;
  virtual HRESULT STDMETHODCALLTYPE MatchesAttributes(LPCWSTR, BOOL*) = 0;
};

MIDL_INTERFACE("06B64F9E-7FDA-11D2-B4F2-00C04F797396")
IEnumSpObjectTokens : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE Next(ULONG, ISpObjectToken**, ULONG*) = 0;
  virtual HRESULT STDMETHODCALLTYPE Skip(ULONG) = 0;
  virtual HRESULT STDMETHODCALLTYPE Reset(void) = 0;
  virtual HRESULT STDMETHODCALLTYPE Clone(IEnumSpObjectTokens**) = 0;
  virtual HRESULT STDMETHODCALLTYPE Item(ULONG, ISpObjectToken**) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCount(ULONG*) = 0;
};

MIDL_INTERFACE("2D3D3845-39AF-4850-BBF9-40B49780011D")
ISpObjectTokenCategory : public ISpDataKey
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetId(LPCWSTR, BOOL) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetId(LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDataKey(SPDATAKEYLOCATION, ISpDataKey**) = 0;
  virtual HRESULT STDMETHODCALLTYPE EnumTokens(LPCWSTR, LPCWSTR, IEnumSpObjectTokens**) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetDefaultTokenId(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDefaultTokenId(LPWSTR*) = 0;
};

class DECLSPEC_UUID("96749377-3391-11D2-9EE3-00C04F797396") SpVoice;
class DECLSPEC_UUID("A910187F-0C7A-45AC-92CC-59EDAFB77B53") SpObjectTokenCategory;

#endif // __ISpVoice_INTERFACE_DEFINED__

/////////////////////////////////////////////////////////////////////
// Glk interface to the speech engine
/////////////////////////////////////////////////////////////////////

namespace {

// Get details of all installed voices
void GetAllVoices(std::map<CString,CComPtr<ISpObjectToken> >& voices, CString& defaultVoice)
{
  CComPtr<ISpObjectTokenCategory> category;
  if (SUCCEEDED(category.CoCreateInstance(__uuidof(SpObjectTokenCategory))))
  {
    if (SUCCEEDED(category->SetId(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices",FALSE)))
    {
      CStringW defaultId;
      {
        LPWSTR id = NULL;
        if (SUCCEEDED(category->GetDefaultTokenId(&id)))
        {
          defaultId = id;
          ::CoTaskMemFree(id);
        }
      }

      CComPtr<IEnumSpObjectTokens> tokens;
      if (SUCCEEDED(category->EnumTokens(NULL,NULL,&tokens)))
      {
        ULONG count = 0;
        if (SUCCEEDED(tokens->GetCount(&count)))
        {
          for (ULONG i = 0; i < count; i++)
          {
            CComPtr<ISpObjectToken> token;
            if (SUCCEEDED(tokens->Item(i,&token)))
            {
              CStringW tokenId;
              {
                LPWSTR id = NULL;
                if (SUCCEEDED(token->GetId(&id)))
                {
                  tokenId = id;
                  ::CoTaskMemFree(id);
                }
              }

              LPWSTR name = NULL;
              if (SUCCEEDED(token->GetStringValue(NULL,&name)))
              {
                CString tokenName(name);
                voices[tokenName] = token;
                if (defaultId.Compare(tokenId) == 0)
                  defaultVoice = tokenName;
                ::CoTaskMemFree(name);
              }
            }
          }
        }
      }
    }
  }
}

} // unnamed namespace

// The only instance of the speech object
TextToSpeech TextToSpeech::SpeechEngine;

// Get the only instance of the speech engine
TextToSpeech& TextToSpeech::GetSpeechEngine(void)
{
  return SpeechEngine;
}

// Constructor
TextToSpeech::TextToSpeech()
{
  m_Available = NotTested;
  m_bInitialized = false;
}

// Destructor
TextToSpeech::~TextToSpeech()
{
  Destroy();
}

// Check if the engine is available
bool TextToSpeech::IsAvailable(void)
{
  if (m_Available == NotTested)
  {
    m_Available = NotAvailable;

    CComPtr<ISpVoice> Voice;
    if (SUCCEEDED(Voice.CoCreateInstance(__uuidof(SpVoice))))
      m_Available = SAPI5;
  }

  if (m_Available == SAPI5)
    return true;
  return false;
}

// Enumerate the voices that can be used by the speech engine
void TextToSpeech::GetVoices(CStringArray& names, CString& defaultName)
{
  if (IsAvailable())
  {
    std::map<CString,CComPtr<ISpObjectToken> > voices;
    GetAllVoices(voices,defaultName);

    for (std::map<CString,CComPtr<ISpObjectToken> >::iterator it = voices.begin(); it != voices.end(); ++it)
      names.Add(it->first);
  }
}

// Initialize the speech engine
void TextToSpeech::Initialize(LPCSTR Voice, int Speed)
{
  if (m_bInitialized)
    return;
  m_bInitialized = true;

  // Create an object to interface with the speech engine
  if ((m_Available == SAPI5) && (m_Voice == NULL))
  {
    m_Voice.CoCreateInstance(__uuidof(SpVoice));
    Update(Voice,Speed);
  }
}

// Close the speech engine
void TextToSpeech::Destroy(void)
{
  if (m_Voice != NULL)
    m_Voice.Release();
  m_bInitialized = false;
}

// Update the speech engine settings
void TextToSpeech::Update(LPCSTR Voice, int Speed)
{
  if (m_Voice != NULL)
  {
    m_Voice->SetRate(Speed);

    std::map<CString,CComPtr<ISpObjectToken> > voices;
    CString defaultVoice;
    GetAllVoices(voices,defaultVoice);
    std::map<CString,CComPtr<ISpObjectToken> >::iterator it = voices.find(Voice);
    if (it != voices.end())
      m_Voice->SetVoice(it->second);
    else
    {
      it = voices.find(defaultVoice);
      if (it != voices.end())
        m_Voice->SetVoice(it->second);
    }
  }
}

// Speak some text
void TextToSpeech::Speak(LPCSTR speech)
{
  if (m_Voice != NULL)
  {
    // Convert the text (in Latin-1) to Unicode
    int length = strlen(speech);
    LPWSTR unicode = new WCHAR[length+1];
    for (int i = 0; i <= length; i++)
      unicode[i] = speech[i];

    m_Voice->Speak(unicode,SPF_ASYNC,NULL);

    // Free the Unicode buffer
    delete[] unicode;
  }
}

void TextToSpeech::Speak(LPCWSTR speech)
{
  if (m_Voice != NULL)
    m_Voice->Speak(speech,SPF_ASYNC,NULL);
}
