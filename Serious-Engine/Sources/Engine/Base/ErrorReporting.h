/* Copyright (c) 2002-2012 Croteam Ltd. 
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef SE_INCL_ERRORREPORTING_H
#define SE_INCL_ERRORREPORTING_H
#ifdef PRAGMA_ONCE
  #pragma once
#endif

// LogCat
extern void AndroidLogPrintI(CTString log);
extern void AndroidLogPrintW(CTString log);
extern void AndroidLogPrintE(CTString log);
extern void AndroidCloseApplication();

/* Throw an exception of formatted string. */
//ENGINE_API extern void ThrowF_t(char *strFormat, ...); // throws char *
template<typename ... Types>
void ThrowF_t(const char *strPattern, Types... t) {
  const SLONG slBufferSize = 1024;
  static thread_local char strBuffer[slBufferSize + 1];
  CTString result = stringFormatter::format(strPattern, t...);
  AndroidLogPrintE(result);
  strncpy(strBuffer, result.str_String, slBufferSize);
  throw strBuffer;
}

/* Report error and terminate program. */
//ENGINE_API extern void FatalError(const char *strFormat, ...);
template<typename ... Types>
void FatalError(const char *strPattern, Types... t) {
  AndroidLogPrintE(stringFormatter::format(strPattern, t...));
  AndroidCloseApplication();
}

/* Report warning without terminating program (stops program until user responds). */
//ENGINE_API extern void WarningMessage(const char *strFormat, ...);
template<typename ... Types>
void WarningMessage(const char *strPattern, Types... t) {
  AndroidLogPrintW(stringFormatter::format(strPattern, t...));
}

/* Report information message to user (stops program until user responds). */
//ENGINE_API extern void InfoMessage(const char *strFormat, ...);
template<typename ... Types>
void InfoMessage(const char *strPattern, Types... t) {
  AndroidLogPrintI(stringFormatter::format(strPattern, t...));
}

/* Ask user for yes/no answer(stops program until user responds). */
//ENGINE_API extern BOOL YesNoMessage(const char *strFormat, ...);
template<typename ... Types>
void YesNoMessage(const char *strPattern, Types... t) {
  // TODO: show dialog to user, for the moment error
  AndroidLogPrintE(stringFormatter::format(strPattern, t...));
  AndroidCloseApplication();
}

/* Get the description string for windows error code. */
ENGINE_API extern const CTString GetWindowsError(DWORD dwWindowsErrorCode);


#endif  /* include-once check. */

