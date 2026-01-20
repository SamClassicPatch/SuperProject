/* Copyright (c) 2023-2026 Dreamy Cecil
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

#include "HttpRequests.h"

#if SE1_VER == SE1_110
  // Link library directly
  #include <winhttp.h>
  #pragma comment(lib, "Winhttp.lib")

  static __forceinline BOOL LinkWinHttp(void) {
    return TRUE;
  };

#else
  // Get functions from the library
  #include <WinInet.h>

  #define WINHTTP_ACCESS_TYPE_DEFAULT_PROXY 0
  #define WINHTTP_NO_PROXY_NAME   NULL
  #define WINHTTP_NO_PROXY_BYPASS NULL
  #define WINHTTP_FLAG_SECURE 0x00800000

  #define WINHTTP_NO_ADDITIONAL_HEADERS NULL
  #define WINHTTP_NO_REQUEST_DATA       NULL

  typedef ULONG DWORD_PTR;

  static HINTERNET (__stdcall *WinHttpOpen)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);
  static HINTERNET (__stdcall *WinHttpConnect)(HINTERNET, LPCWSTR, INTERNET_PORT, DWORD);
  static HINTERNET (__stdcall *WinHttpOpenRequest)(HINTERNET, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR *, DWORD);
  static BOOL (__stdcall *WinHttpSendRequest)(HINTERNET, LPCWSTR, DWORD, LPVOID, DWORD, DWORD, DWORD_PTR);
  static BOOL (__stdcall *WinHttpReceiveResponse)(HINTERNET, LPVOID);
  static BOOL (__stdcall *WinHttpQueryDataAvailable)(HINTERNET, LPDWORD);
  static BOOL (__stdcall *WinHttpReadData)(HINTERNET, LPVOID, DWORD, LPDWORD);
  static BOOL (__stdcall *WinHttpCloseHandle)(HINTERNET);

  static __forceinline BOOL LinkWinHttp(void) {
    // Load methods from the library dynamically
    HINSTANCE hWinHttp = LoadLibraryA("Winhttp.dll");

    // Couldn't initialize
    if (hWinHttp == NULL) return FALSE;

    typedef HINTERNET (__stdcall *COpen)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);
    typedef HINTERNET (__stdcall *CConnect)(HINTERNET, LPCWSTR, INTERNET_PORT, DWORD);
    typedef HINTERNET (__stdcall *COpenRequest)(HINTERNET, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR *, DWORD);
    typedef BOOL (__stdcall *CSendRequest)(HINTERNET, LPCWSTR, DWORD, LPVOID, DWORD, DWORD, DWORD_PTR);
    typedef BOOL (__stdcall *CResponse)(HINTERNET, LPVOID);
    typedef BOOL (__stdcall *CDataAvailable)(HINTERNET, LPDWORD);
    typedef BOOL (__stdcall *CReadData)(HINTERNET, LPVOID, DWORD, LPDWORD);
    typedef BOOL (__stdcall *CCloseHandle)(HINTERNET);

    WinHttpOpen               = (COpen)GetProcAddress(hWinHttp, "WinHttpOpen");
    WinHttpConnect            = (CConnect)GetProcAddress(hWinHttp, "WinHttpConnect");
    WinHttpOpenRequest        = (COpenRequest)GetProcAddress(hWinHttp, "WinHttpOpenRequest");
    WinHttpSendRequest        = (CSendRequest)GetProcAddress(hWinHttp, "WinHttpSendRequest");
    WinHttpReceiveResponse    = (CResponse)GetProcAddress(hWinHttp, "WinHttpReceiveResponse");
    WinHttpQueryDataAvailable = (CDataAvailable)GetProcAddress(hWinHttp, "WinHttpQueryDataAvailable");
    WinHttpReadData           = (CReadData)GetProcAddress(hWinHttp, "WinHttpReadData");
    WinHttpCloseHandle        = (CCloseHandle)GetProcAddress(hWinHttp, "WinHttpCloseHandle");

    // Couldn't initialize
    if (WinHttpOpen == NULL || WinHttpConnect == NULL || WinHttpOpenRequest == NULL || WinHttpSendRequest == NULL
     || WinHttpReceiveResponse == NULL || WinHttpQueryDataAvailable == NULL || WinHttpReadData == NULL || WinHttpCloseHandle == NULL) {
      return FALSE;
    }

    return TRUE;
  };
#endif

// HTTP module has been initialized
static BOOL _bHttpInitialized = FALSE;

// Check if HTTP module is available
BOOL IsHttpInitialized(void) {
  return _bHttpInitialized;
};

// Make an HTTP request and get the reply via a callback function
CHttpResponse HttpRequest(LPCWSTR wstrServer, LPCWSTR wstrVerb, LPCWSTR wstrObject, BOOL bSecure, CHttpCallback pCallback) {
  if (!_bHttpInitialized) return CHttpResponse();

  HINTERNET hSession = NULL;
  HINTERNET hConnect = NULL;
  HINTERNET hRequest = NULL;

  // Convert user agent string into a wide string
  const CTString strUserAgent(0, "Serious Sam Classics Patch WinHTTP/%s", ClassicsCore_GetVersionName());

  const INDEX ctLength = strUserAgent.Length() + 1;
  wchar_t *wstrAgent = new wchar_t[ctLength];
  mbstowcs(wstrAgent, strUserAgent.str_String, ctLength);

  // Use WinHttpOpen to obtain a session handle
  hSession = WinHttpOpen(wstrAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  delete[] wstrAgent;

  // Specify an HTTP server
  if (hSession) {
    hConnect = WinHttpConnect(hSession, wstrServer, bSecure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, 0);
  }

  // Create an HTTP request handle
  if (hConnect) {
    hRequest = WinHttpOpenRequest(hConnect, wstrVerb, wstrObject, NULL, NULL, NULL, bSecure ? WINHTTP_FLAG_SECURE : 0);
  }

  BOOL bResults = FALSE;

  // Send a request and wait for it
  if (hRequest != NULL && WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
    bResults = WinHttpReceiveResponse(hRequest, NULL);
  }

  char *aResponse = (char *)calloc(1, sizeof(char));

  // Keep checking for data until there is nothing left
  if (bResults) {
    ULONG ulSize = 0;
    ULONG ulTotalSize = 0;

    do {
      ulSize = 0;

      // Check for available data
      if (!WinHttpQueryDataAvailable(hRequest, &ulSize)) {
        CPrintF(TRANS("Error %u in WinHttpQueryDataAvailable()\n"), GetLastError());

        if (ulSize == 0) break;
      }

      // Allocate space for new data
      size_t iCount = strlen(aResponse);
      aResponse = (char *)realloc(aResponse, iCount + ulSize);
      memset(&aResponse[iCount], 0, ulSize);

      // Read the data
      ULONG ulDownloaded = 0;

      if (!WinHttpReadData(hRequest, (LPVOID)(&aResponse[iCount]), ulSize, &ulDownloaded)) {
        CPrintF(TRANS("Error %u in WinHttpReadData()\n"), GetLastError());

      } else if (ulSize > 0) {
        ulTotalSize += ulDownloaded;
      }

    } while (ulSize > 0);

  // Report any errors
  } else {
    CPrintF(TRANS("Error %u in HttpRequest()\n"), GetLastError());
  }

  // Close any open handles
  if (hRequest != NULL) WinHttpCloseHandle(hRequest);
  if (hConnect != NULL) WinHttpCloseHandle(hConnect);
  if (hSession != NULL) WinHttpCloseHandle(hSession);

  if (pCallback != NULL) {
    pCallback(aResponse);
  }

  CHttpResponse strReturn(aResponse);
  free(aResponse);

  return strReturn;
};

void InitHttp(void) {
  // Make sure WinHttp in linked
  if (!LinkWinHttp()) return;

  _bHttpInitialized = TRUE;
};
