/////////////////////////////////////////////////////////////////////
// Simple speech interface for Glk
/////////////////////////////////////////////////////////////////////

#ifndef GLKTALK_H_
#define GLKTALK_H_

#ifndef __ISpVoice_FWD_DEFINED__
#define __ISpVoice_FWD_DEFINED__
typedef interface ISpVoice ISpVoice;
#endif

class TextToSpeech
{
public:
  static TextToSpeech& GetSpeechEngine(void);

  bool IsAvailable(void);
  void GetVoices(CStringArray& names, CString& defaultName);

  void Initialize(LPCSTR Voice, int Speed);
  void Destroy(void);
  void Update(LPCSTR Voice, int Speed);
  void Speak(LPCSTR speech, UINT codePage);
  void Speak(LPCWSTR speech);

protected:
  TextToSpeech();
  ~TextToSpeech();

  static TextToSpeech SpeechEngine;

  enum Available
  {
    NotTested,
    NotAvailable,
    SAPI5
  };

  Available m_Available;
  bool m_bInitialized;

  CComPtr<ISpVoice> m_Voice;
};

#endif // GLKTALK_H_
