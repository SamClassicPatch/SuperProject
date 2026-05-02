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

// Undecorate a string except for color tags, which are converted to extended colors for Windows terminal
inline void ConvertColorsForTerminal(CTString &str) {
  // Simple undecorated text
  if (_EnginePatches._iColoredTextInServerLog == 1) {
    str = str.Undecorated();
    return;
  }

  CTString strResult = "";
  const char *pch = str.str_String;

  // Temporary variables for setting colors
  char strTemp[7];
  char *pchDummy;
  COLOR col;
  UBYTE ubR, ubG, ubB;

  while (*pch != '\0') {
    // Print regular characters as is
    if (*pch != '^') {
      strResult += CTString(0, "%c", *pch);
      pch++;
      continue;
    }

    switch (pch[1]) {
      // Set color
      case 'c': {
        const INDEX ctColor = FindZero((UBYTE *)pch + 2, 6);

        // If it's a full color code
        if (ctColor == 6) {
          // Convert code string to color
          strncpy(strTemp, pch + 2, 6);
          col = strtoul(strTemp, &pchDummy, 16) << 8;

          // Then set extended foreground color in the terminal
          ColorToRGB(col, ubR, ubG, ubB);
          strResult += CTString(0, "\033[38;2;%d;%d;%dm", ubR, ubG, ubB);

          // Set background color based on the overall brightness of the color
          if (_EnginePatches._iColoredTextInServerLog > 2) {
            ULONG iGray = (ubR * 0.299 + ubG * 0.587 + ubB * 0.114);
            strResult += (iGray < 0x18) ? "\033[107m" : "\033[40m"; // Bright white or non-bright black
          }
        }

        pch += 2 + ctColor;
      } break;

      // Reset color
      case 'r': case 'C': {
        strResult += "\033[0m";
        pch += 2;
      } break;

      // Skip codes
      case 'a': pch += 2 + FindZero((UBYTE *)pch + 2, 2); break;
      case 'f': pch += 2 + FindZero((UBYTE *)pch + 2, 1); break;
      case 'b': case 'i': case 'o':
      case 'A': case 'F': case 'B': case 'I': pch += 2; break;

      // Print tag character as is
      case '^':
        strResult += "^";
        pch += 2;
        break;

      // Print unrecognized codes as is
      default:
        strResult += CTString(0, "%c", pch[1]);
        pch += 2;
        break;
    }
  }

  str = strResult;
};

// Special wrapper for the original function
inline void PutStringWrapper(const CTString &str) {
  const BOOL bServer = _bDedicatedServer;

  // Set output encoding for dedicated servers according to the system locale to support localized strings
  const UINT uLastEncoding = GetConsoleOutputCP();
  if (bServer) SetConsoleOutputCP(GetACP());

  // If running through a dedicated server and want colored text
  if (bServer && _EnginePatches._iColoredTextInServerLog > 0) {
    // Print to the console application our own way
    CTString strTerminal = str;
    ConvertColorsForTerminal(strTerminal);
    printf("%s", strTerminal.str_String);

    // And then temporarily skip printing via printf() in the original function
    _bDedicatedServer = FALSE;
    (*pPutString)(str);
    _bDedicatedServer = bServer;

  } else {
    (*pPutString)(str);
  }

  // Restore encoding
  if (bServer) SetConsoleOutputCP(uLastEncoding);
};

// Patched function
void P_CPutString(const char *strString) {
  // Don't need any timestamps
  if (_bTempIgnoreTimestamps || !_EnginePatches._bLogTimestamps) {
    // Proceed to the original function
    PutStringWrapper(strString);
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
  PutStringWrapper(strResult);
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
