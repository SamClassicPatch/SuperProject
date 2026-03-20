/* Copyright (c) 2022-2026 Dreamy Cecil
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

#include "StdH.h"

// Temporarily suppress timestamp printing
BOOL _bTempIgnoreTimestamps = FALSE;

#if _PATCHCONFIG_ENGINEPATCHES

#include "Strings.h"

#if _PATCHCONFIG_FIX_STRINGS

// Original function pointer
void (*pPutString)(const char *) = NULL;

// Patched function
void P_CPrintF(const char *strFormat, ...) {
  if (_pConsole == NULL) return;

  va_list arg;
  va_start(arg, strFormat);
  CTString strBuffer;
  strBuffer.VPrintF(strFormat, arg);
  va_end(arg);

  // Pass the formatted string into the regular printing function
  P_CPutString(strBuffer);
};

// Patched function
void P_CPutString(const char *strString) {
  // Don't need any timestamps
  if (_bTempIgnoreTimestamps || !_EnginePatches._bLogTimestamps) {
    // Proceed to the original function
    (*pPutString)(strString);
    return;
  }

  if (_pConsole == NULL) return;

  // Prepare the current timestamp
  time_t iLongTime;
  time(&iLongTime);
  tm *tmNow = localtime(&iLongTime);

  const CTString strTime(0, "[%02d:%02d:%02d] ", tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);

  // Separate the string by lines
  CStringStack astr;
  IData::GetStrings(astr, strString, '\n');

  const INDEX ct = astr.Count();
  if (ct == 0) return; // Nothing to print

  CTString strResult = "";

  // Add a timestamp in the beginning if the last print ended with a newline
  if (_pConsole->con_strCurrent[-1] == '\n') {
    strResult += strTime;
  }

  // Add the first line as is
  strResult += astr[0];

  // Add the rest of the lines with timestamps in the beginning
  for (INDEX i = 1; i < ct; i++) {
    // Ignore the timestamp if the last line is empty
    if (i == ct - 1 && astr[i] == "") {
      strResult += "\n";
      break;
    }

    strResult += "\n" + strTime + astr[i];
  }

  // Proceed to the original function
  (*pPutString)(strResult);
};

INDEX CStringPatch::P_VPrintF(const char *strFormat, va_list arg)
{
  // [Cecil] Resize 4 times more than vanilla
  static const ULONG ulAddSize = 1024;

  // [Cecil] Local variables instead of static
  INDEX ctBufferSize = ulAddSize;
  char *pchBuffer = (char *)AllocMemory(ulAddSize);

  INDEX iLen;

  FOREVER {
    // Print to the buffer
    iLen = _vsnprintf(pchBuffer, ctBufferSize, strFormat, arg);

    // Stop if printed okay
    if (iLen != -1) {
      break;
    }

    // Increase the buffer size
    ctBufferSize += ulAddSize;
    GrowMemory((void **)&pchBuffer, ctBufferSize);
  }

  ((CTString &)*this) = pchBuffer;

  // [Cecil] Free local buffer memory
  FreeMemory(pchBuffer);

  return iLen;
};

CTString CStringPatch::P_Undecorated(void) const {
  CTString strResult = *this;
  const char *pchSrc = str_String;
  char *pchDst = strResult.str_String;

  while (pchSrc[0] != 0)
  {
    if (pchSrc[0] != '^') {
      *pchDst++ = *pchSrc++;
      continue;
    }

    switch (pchSrc[1]) {
      case 'c': pchSrc += 2 + FindZero((UBYTE *)pchSrc + 2, 6); break;
      case 'a': pchSrc += 2 + FindZero((UBYTE *)pchSrc + 2, 2); break;

      // [Cecil] Skip 1 byte instead of 2
      case 'f': pchSrc += 2 + FindZero((UBYTE *)pchSrc + 2, 1); break;

      case 'b': case 'i': case 'r': case 'o':
      case 'C': case 'A': case 'F': case 'B': case 'I': pchSrc += 2; break;
      case '^': pchSrc++; *pchDst++ = *pchSrc++; break;

      default:
        *pchDst++ = *pchSrc++;
        break;
    }
  }

  *pchDst++ = 0;

  ASSERT(strResult.Length() <= Length());
  return strResult;
};

#endif // _PATCHCONFIG_FIX_STRINGS

#endif // _PATCHCONFIG_ENGINEPATCHES
