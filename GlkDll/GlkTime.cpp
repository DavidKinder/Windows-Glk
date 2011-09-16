/////////////////////////////////////////////////////////////////////////////
//
// Windows MFC GLK Libraries
//
// GlkTime
// GLK Time related functions. Ideas on how to improve the accuracy beyond
// the ~15ms provided by the internal Windows clock are taken from Mozilla.
//
/////////////////////////////////////////////////////////////////////////////

#define VC_EXTRALEAN
#include <windows.h>

#include "GlkTime.h"
#include <stdlib.h>

#ifdef _MSC_VER
#define LONGABS(x) _abs64(x)
#define LONGCONST(x) (x##ui64)
#else
#define LONGABS(x) llabs(x)
#define LONGCONST(x) (x##ULL)
#endif

namespace {

FILETIME BadTime = { 0xFFFFFFFF,0xFFFFFFFF };

inline bool IsBadTime(const FILETIME& ft)
{
  if (ft.dwLowDateTime != BadTime.dwLowDateTime)
    return false;
  if (ft.dwHighDateTime != BadTime.dwHighDateTime)
    return false;
  return true;
}

inline ULONGLONG FileTimeToValue(const FILETIME& ft)
{
  return ((ULONGLONG)ft.dwHighDateTime << 32) + ft.dwLowDateTime;
}

inline FILETIME ValueToFileTime(ULONGLONG v)
{
  FILETIME ft;
  ft.dwHighDateTime = (DWORD)(v >> 32);
  ft.dwLowDateTime = (DWORD)(v & 0xFFFFFFFF);
  return ft;
}

glsi32 NormalizeField(glsi32 value, glsi32 range, glsi32& carry)
{
  carry = (value / range);
  if (value < 0)
    carry--;
  return value - (carry*range);
}

glsi32 DaysInMonth(WORD month, WORD year)
{
  switch (month)
  {
  case 9: // September
  case 4: // April
  case 6: // June
  case 11: // November
    return 30;
  case 1: // January
  case 3: // March
  case 5: // May
  case 7: // July
  case 8: // August
  case 10: // October
  case 12: // December
    return 31;
  case 2: // February
    if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
      return 29;
    else
      return 28;
  default:
    return 0;
  }
}

glsi32 NextMonth(WORD& month, WORD& year)
{
  glsi32 days = DaysInMonth(month,year);
  month++;
  if (month == 13)
  {
    month = 1;
    year++;
  }
  return days;
}

glsi32 LastMonth(WORD& month, WORD& year)
{
  month--;
  if (month == 0)
  {
    month = 12;
    year--;
  }
  return DaysInMonth(month,year);
}

bool NeedCalibration = true;
LONGLONG CalibrationFreq = 0;
LONGLONG CalibrationCountBase = 0;
ULONGLONG CalibrationTimeBase = 0;

void CalibrateNow(void)
{
  // If the timer frequency is not known, try to get it
  if (CalibrationFreq == 0)
  {
    LARGE_INTEGER freq;
    if (::QueryPerformanceFrequency(&freq) == 0)
      CalibrationFreq = -1;
    else
      CalibrationFreq = freq.QuadPart;
  }

  if (CalibrationFreq > 0)
  {
    // Get the current system time, accurate to ~1ms
    FILETIME ft1, ft2;
    ::timeBeginPeriod(1);
    ::GetSystemTimeAsFileTime(&ft1);
    do
    {
      // Loop until the value changes, so that the timeBeginPeriod() call has had an effect
      ::GetSystemTimeAsFileTime(&ft2);
    }
    while (FileTimeToValue(ft1) == FileTimeToValue(ft2));
    ::timeEndPeriod(1);

    // Get the current timer value
    LARGE_INTEGER counter;
    ::QueryPerformanceCounter(&counter);

    // Save calibration values
    CalibrationCountBase = counter.QuadPart;
    CalibrationTimeBase = FileTimeToValue(ft2);
    NeedCalibration = false;
  }
}

LONG GetTimeIncrement(void)
{
  typedef BOOL(__stdcall *GETSYSTEMTIMEADJUSTMENT)(PDWORD,PDWORD,PBOOL);
  static GETSYSTEMTIMEADJUSTMENT getSystemTimeAdjustment = 0;

  if (getSystemTimeAdjustment == 0)
  {
    static HMODULE kernel32 = 0;

    if (kernel32 == 0)
      kernel32 = ::LoadLibrary("kernel32.dll");
    if (kernel32 != 0)
    {
      getSystemTimeAdjustment = (GETSYSTEMTIMEADJUSTMENT)
        ::GetProcAddress(kernel32,"GetSystemTimeAdjustment");
    }
  }

  if (getSystemTimeAdjustment != 0)
  {
    DWORD timeAdjust, timeInc;
    BOOL timeAdjustDisabled;

    if ((*getSystemTimeAdjustment)(&timeAdjust,&timeInc,&timeAdjustDisabled))
      return timeAdjustDisabled ? timeAdjust : timeInc;
  }

  // Use a default value of 15.625ms
  return 156250;
}

} // unnamed namespace

FILETIME GetNow(void)
{
  for (int i = 0; i < 4; i++)
  {
    // Calibrate if needed, and give up if this fails
    if (NeedCalibration)
      CalibrateNow();
    if (NeedCalibration)
      break;

    // Get the current timer value and use it to compute now
    FILETIME ft;
    ::GetSystemTimeAsFileTime(&ft);
    LARGE_INTEGER counter;
    ::QueryPerformanceCounter(&counter);
    LONGLONG elapsed = ((counter.QuadPart - CalibrationCountBase) * 10000000) / CalibrationFreq;
    ULONGLONG now = CalibrationTimeBase + elapsed;

    // Don't let time go back
    static ULONGLONG lastNow = 0;
    now = max(now,lastNow);
    lastNow = now;

    // Check for clock skew
    if (LONGABS(FileTimeToValue(ft) - now) > 2 * GetTimeIncrement())
    {
      NeedCalibration = true;
      lastNow = 0;
    }

    if (!NeedCalibration)
      return ValueToFileTime(now);
  }

  // Calibration has failed to stabilize, so just use the system time
  FILETIME ft;
  ::GetSystemTimeAsFileTime(&ft);
  return ft;
}

FILETIME ToLocal(const FILETIME& ft)
{
  if (!IsBadTime(ft))
  {
    FILETIME lft;
    if (::FileTimeToLocalFileTime(&ft,&lft))
      return lft;
  }
  return BadTime;
}

void ToGlkTime(const FILETIME& ft, glktimeval_t* time)
{
  if (IsBadTime(ft))
  {
    time->high_sec = time->low_sec = 0xFFFFFFFF;
    return;
  }

  // Convert to seconds since the start of 1970
  ULONGLONG ticks = FileTimeToValue(ft);
  LONGLONG secs = (ticks / 10000000) - LONGCONST(11644473600);

  time->high_sec = (glsi32)(secs >> 32);
  time->low_sec = (glsi32)(secs & 0xFFFFFFFF);
  time->microsec = (glsi32)(ticks % 10000000) / 10;
}

FILETIME FromGlkTime(const glktimeval_t* time)
{
  LONGLONG secs = ((LONGLONG)time->high_sec << 32) + time->low_sec;
  ULONGLONG ticks = ((secs + LONGCONST(11644473600)) * 10000000) + (time->microsec * 10);
  return ValueToFileTime(ticks);
}

glsi32 ToSimpleTime(const FILETIME& ft, glui32 factor)
{
  if (IsBadTime(ft))
    return -1;

  // Convert to seconds since the start of 1970
  ULONGLONG ticks = FileTimeToValue(ft);
  LONGLONG secs = (ticks / 10000000) - LONGCONST(11644473600);

  // Round towards negative infinity
  if (secs < 0)
    return (glsi32)(-1 - (((LONGLONG)-1 - secs) / (LONGLONG)factor));
  return (glsi32)(secs / (LONGLONG)factor);
}

FILETIME FromSimpleTime(glsi32 time, glui32 factor)
{
  LONGLONG secs = time * factor;
  ULONGLONG ticks = (secs + LONGCONST(11644473600)) * 10000000;
  return ValueToFileTime(ticks);
}

void ToGlkDate(const FILETIME& ft, glkdate_t* date)
{
  SYSTEMTIME st;
  ::FileTimeToSystemTime(&ft,&st);

  date->year = st.wYear;
  date->month = st.wMonth;
  date->day = st.wDay;
  date->weekday = st.wDayOfWeek;
  date->hour = st.wHour;
  date->minute = st.wMinute;
  date->second = st.wSecond;

  LONGLONG ticks = FileTimeToValue(ft);
  date->microsec = (glsi32)((ticks / 10) % 1000000);
}

FILETIME FromGlkDate(const glkdate_t* date)
{
  glsi32 carry = 0;
  glsi32 micros = NormalizeField(date->microsec,1000000,carry);

  SYSTEMTIME st;
  ::ZeroMemory(&st,sizeof st);
  st.wSecond = (WORD)NormalizeField(date->second + carry,60,carry);
  st.wMinute = (WORD)NormalizeField(date->minute + carry,60,carry);
  st.wHour = (WORD)NormalizeField(date->hour + carry,24,carry);

  glsi32 days = date->day + carry;
  st.wMonth = 1 + (WORD)NormalizeField(date->month - 1,12,carry);
  st.wYear = (WORD)(date->year + carry);

  while (days <= 0)
    days += LastMonth(st.wMonth,st.wYear);
  while (days > DaysInMonth(st.wMonth,st.wYear))
    days -= NextMonth(st.wMonth,st.wYear);
  st.wDay = (WORD)days;

  FILETIME ft;
  if (::SystemTimeToFileTime(&st,&ft))
  {
    ULONGLONG ticks = FileTimeToValue(ft);
    ticks += (micros * 10);
    return ValueToFileTime(ticks);
  }
  return BadTime;
}

DWORD TickCountDiff(DWORD later, DWORD earlier)
{
  if (later < earlier)
  {
    // The timer must have wrapped round
    return ((MAXDWORD - earlier) + later);
  }
  return (later - earlier);
}
