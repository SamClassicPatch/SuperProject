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

#include "StdH.h"

#if _PATCHCONFIG_NEW_QUERY

#include "QueryManager.h"

#include <errno.h>

extern unsigned char *gsseckey(u_char *secure, u_char *key, int enctype);
extern u_int resolv(char *host);

// Buffer lengths
static const ULONG _ulBufferLength = 8192;
static const ULONG _ulStringLength = 4096;

// Buffer of online IP addresses
static char *_pOnlineAddressBuffer = NULL;
static INDEX _ctOnlineBuffer = 0;

// Socket for interacting with the master server
static SOCKET _iSocket = INVALID_SOCKET;

// Buffer of local IP addresses
static char *_pLocalAddressBuffer = NULL;
static INDEX _ctLocalBuffer = 0;

// Enumeration activation
static BOOL _bActivated = FALSE;
static BOOL _bActivatedLocal = FALSE;

// Start local server search
static void StartLocalSearch(void) {
  // Reset requests
  IQuery::aRequests.Clear();

  // Not a server
  IQuery::bServer = FALSE;
  _pNetwork->ga_strEnumerationStatus = ".";

  // Buffer already exists
  if (_pLocalAddressBuffer != NULL) {
    return;
  }

  // Create a buffer for packet reading
  _pLocalAddressBuffer = new char[1024];

  // Start socket address
  WSADATA wsaData;

  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    // Something went wrong
    CPutString("Error initializing winsock!\n");

    // Delete local buffer
    if (_pLocalAddressBuffer != NULL) {
      delete[] _pLocalAddressBuffer;
    }
    _pLocalAddressBuffer = NULL;

    // Stop enumeration
    IQuery::CloseWinsock();
    IQuery::bInitialized = FALSE;

    _pNetwork->ga_bEnumerationChange = FALSE;
    _pNetwork->ga_strEnumerationStatus = "";

    WSACleanup();
    return;
  }

  // Buffer and its length
  char *pBuffer = _pLocalAddressBuffer;
  INDEX iLength = 0;

  char strName[256];
  in_addr addr;

  // Get host by its name
  if (gethostname(strName, sizeof(strName)) == 0) {
    PHOSTENT phHostinfo = gethostbyname(strName);

    if (phHostinfo != NULL) {
      INDEX ct = 0;

      // Go through the address list
      while (phHostinfo->h_addr_list[ct] != NULL) {
        // Get address IP
        addr.s_addr = *(u_long *)phHostinfo->h_addr_list[ct++];
        ULONG ulIP = htonl(addr.s_addr);

        // Write 20 IP addresses with different ports
        for (INDEX iPort = 25601; iPort <= 25621; iPort++) {
          UWORD uwPort = htons(iPort);

          memcpy(pBuffer, &ulIP, 4);
          pBuffer += 4;
          iLength += 4;

          memcpy(pBuffer, &uwPort, 2);
          pBuffer += 2;
          iLength += 2;
        }
      }

      // Packet end
      memcpy(pBuffer, "\\final\\", 7);
      pBuffer += 7;
      iLength += 7;

      // End with a null terminator
      pBuffer[iLength] = '\0';
    }
  }

  // Start local search
  _ctLocalBuffer = iLength;
  _bActivatedLocal = TRUE;

  IQuery::bInitialized = TRUE;
  IQuery::InitWinsock();
};

// Abort online server search
static void AbortSearch(char *pFreeBuffer = NULL) {
  // Free some buffer and close the socket
  if (pFreeBuffer != NULL) {
    free(pFreeBuffer);
  }

  closesocket(_iSocket);
  WSACleanup();
};

// Select with a timeout to not get stuck indefinitely
static int TimeoutSelect(SOCKET iSocket, int iSeconds) {
  timeval timeout;
  timeout.tv_sec = iSeconds;
  timeout.tv_usec = 0;

  fd_set fdsRead;
  FD_ZERO(&fdsRead);
  FD_SET(iSocket, &fdsRead);

  if (select(iSocket + 1, &fdsRead, NULL, NULL, &timeout) <= 0) {
    return -1;
  }

  return 0;
};

// Get last socket error as a string
CTString GetSocketError(void) {
  // Get the error code
  DWORD dwMessageId = WSAGetLastError();

  LPVOID lpMsgBuf;
  DWORD dwSuccess = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, dwMessageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPSTR)&lpMsgBuf, 0, NULL);

  CTString strWinError;

  // If formatting succeeds
  if (dwSuccess != 0) {
    // Copy the result
    strWinError = (char *)lpMsgBuf;

    // Free the Windows message buffer
    LocalFree(lpMsgBuf);

  // Otherwise report failure
  } else {
    strWinError.PrintF(TRANS(
      "Cannot format error message!\nOriginal error code: %d\nFormatting error code: %d\n"),
      dwMessageId, GetLastError());
  }

  return strWinError;
};

// Start internet server search
static void StartInternetSearch(void) {
  // Reset requests
  IQuery::aRequests.Clear();

  // Not a server
  IQuery::bServer = FALSE;
  _pNetwork->ga_strEnumerationStatus = ".";

  // Start socket address
  WSADATA wsaData;

  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    // Something went wrong
    CPutString("Error initializing winsock!\n");
    return;
  }

  // Open a socket
  _iSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (_iSocket < 0) {
    CPutString("Error creating TCP socket!\n");
    WSACleanup();
    return;
  }

  // Connect to the master server
  char *strMasterServer = _aProtocols[E_MS_LEGACY]->GetMS().str_String;

  sockaddr_in addr;
  addr.sin_addr.s_addr = resolv(strMasterServer);
  addr.sin_port = htons(28900);
  addr.sin_family = AF_INET;

  // Try to connect with timeout
  int iRes = connect(_iSocket, (sockaddr *)&addr, sizeof(addr));

  if (iRes < 0) {
    if (WSAGetLastError() == WSAEINPROGRESS) {
      if (ms_bDebugOutput) {
        CPutString("WSAEINPROGRESS in connect() - selecting...\n");
      }

      while (true) {
        timeval tvConnection;
        tvConnection.tv_sec = 2;
        tvConnection.tv_usec = 0;

        fd_set fdsConnect;
        FD_ZERO(&fdsConnect);
        FD_SET(_iSocket, &fdsConnect);

        iRes = select(_iSocket + 1, NULL, &fdsConnect, NULL, &tvConnection);

        if (iRes < 0 && WSAGetLastError() != WSAEINTR) {
          if (ms_bDebugOutput) {
            CPrintF("Error connecting: %s\n", GetSocketError());
          }
          AbortSearch();
          return;

        } else if (iRes > 0) {
          int iOpt;
          socklen_t ctOptLen = sizeof(iOpt);

          // Socket selected for write
          if (getsockopt(_iSocket, SOL_SOCKET, SO_ERROR, (char *)&iOpt, &ctOptLen) < 0) {
            if (ms_bDebugOutput) {
              CPrintF("Error in getsockopt(): %s\n", GetSocketError());
            }
            AbortSearch();
            return;
          }

          // Check the returned value
          if (iOpt) {
            if (ms_bDebugOutput) {
              CPrintF("Error in delayed connection (%d): %s\n", iOpt, strerror(iOpt));
            }
            AbortSearch();
            return;
          }
          break;

        } else {
          if (ms_bDebugOutput) {
            CPutString("Timeout in select() - aborting search...\n");
          }
          AbortSearch();
          return;
        }
      }

    } else {
      if (ms_bDebugOutput) {
        CPrintF("Error connecting: %s\n", GetSocketError());
      }
      AbortSearch();
      return;
    }
  }

  INDEX iLength;
  INDEX iReceived;

  {
    // Allocate memory for receiving a response
    char *strResponse = (char *)malloc(_ulStringLength + 1);

    if (strResponse == NULL) {
      CPutString("Error allocating memory buffer!\n");
      AbortSearch();
      return;
    }

    // Receive response from the master server with the secret key
    iReceived = recv(_iSocket, strResponse, _ulStringLength, 0);

    if (iReceived < 0) {
      CPutString("Error reading from TCP socket!\n");
      AbortSearch(strResponse);
      return;
    }

    // Terminate the response
    iLength = iReceived;
    strResponse[iLength] = '\0';

    // Check for a secure key
    if (strstr(strResponse, "\\secure\\") == NULL) {
      CPutString("Invalid master server response!\n");
      AbortSearch(strResponse);
      return;
    }

    // Get secret key for validation (skip '\basic\\secure\')
    UBYTE *pSecretKey = gsseckey((UBYTE *)strResponse + 15, (UBYTE *)SAM_MS_KEY, 0);

    // Reset the string for sending a response
    memset(strResponse, 0, _ulStringLength + 1);

    iLength = _snprintf(strResponse, _ulStringLength,
      "\\gamename\\%s\\enctype\\%d\\validate\\%s\\final\\"
      "\\queryid\\1.1\\list\\cmp\\gamename\\%s\\gamever\\1.05\\final\\",
      SAM_MS_NAME, 0, pSecretKey, SAM_MS_NAME);

    // Check the buffer
    if (iLength < 0 || iLength > _ulStringLength) {
      CPrintF("\nError composing a response to the master server (length: %d/%d)\n\n", iLength, _ulStringLength);
      AbortSearch(strResponse);
      return;
    }

    // Send response to the master server
    if (send(_iSocket, strResponse, iLength, 0) < 0) {
      CPutString("Error reading from TCP socket!\n");
      AbortSearch(strResponse);
      return;
    }

    // Delete response string
    if (strResponse != NULL) {
      free(strResponse);
    }
  }

  // Buffer already exists
  if (_pOnlineAddressBuffer != NULL) {
    AbortSearch();
    return;
  }

  // Allocate memory for a buffer
  _pOnlineAddressBuffer = (char *)malloc(_ulBufferLength + 1);

  if (_pOnlineAddressBuffer == NULL) {
    CPutString("Error reading from TCP socket!\n");
    AbortSearch();
    return;
  }

  INDEX iNewLength = _ulBufferLength;
  iLength = 0;

  // Receive encoded data after sending the validation key
  while (!TimeoutSelect(_iSocket, 1)) {
    iReceived = recv(_iSocket, _pOnlineAddressBuffer + iLength, iNewLength - iLength, 0);

    if (iReceived <= 0) break;

    // Advance
    iLength += iReceived;

    // If current length exceeds the buffer size
    if (iLength >= iNewLength) {
      // Reallocate the buffer
      iNewLength += _ulBufferLength;
      _pOnlineAddressBuffer = (char *)realloc(_pOnlineAddressBuffer, iNewLength);

      if (_pOnlineAddressBuffer == NULL) {
        // Couldn't reallocate the buffer
        CPutString("Error reallocating memory buffer!\n");
        AbortSearch(_pOnlineAddressBuffer);
        return;
      }
    }
  }

  AbortSearch();

  // Start internet search
  _ctOnlineBuffer = iLength;
  _bActivated = TRUE;

  IQuery::bInitialized = TRUE;
  IQuery::InitWinsock();
};

static void ParseStatusResponse(sockaddr_in &sinClient, BOOL bIgnorePing) {
  // Skip separator at the beginning
  const char *strData = IQuery::pBuffer + 1;

  // Key and value strings
  CTString strKey;
  CTString strValue;

  // Values for reading
  CTString strGameName, strGameVer, strLocation, strHostName, strHostPort, strMapName, strGameType, strActiveMod;
  INDEX ctPlayers, ctMaxPlayers;

  BOOL bReadValue = FALSE;

  while (*strData != '\0')
  {
    switch (*strData) {
      // Data separator
      case '\\': {
        if (bReadValue && strKey != "gamemode") {
          if (strKey == "gamename") {
            strGameName = strValue;

          } else if (strKey == "gamever") {
            strGameVer = strValue;

          } else if (strKey == "location") {
            strLocation = strValue;

          } else if (strKey == "hostname") {
            strHostName = strValue;

          } else if (strKey == "hostport") {
            strHostPort = strValue;

          } else if (strKey == "mapname") {
            strMapName = strValue;

          } else if (strKey == "gametype") {
            strGameType = strValue;

          } else if (strKey == "activemod") {
            strActiveMod = strValue;

          } else if (strKey == "numplayers") {
            ctPlayers = atoi(strValue);

          } else if (strKey == "maxplayers") {
            ctMaxPlayers = atoi(strValue);
          }

          // Reset key and value
          strKey = "";
          strValue = "";
        }

        // Toggle between the key and the value
        bReadValue = !bReadValue;

        // Proceed
        strData++;
      } break;

      // Data
      default: {
        // Extract substring until a separator or the end
        ULONG ulSep = IData::FindChar(strData, '\\');
        CTString strExtracted = IData::ExtractSubstr(strData, 0, ulSep);

        // Set new key or value
        if (bReadValue) {
          strValue = strExtracted;
        } else {
          strKey = strExtracted;
        }

        // Advance the packet data
        strData += strExtracted.Length();
      } break;
    }
  }

  // Set active mod as the game name
  if (strActiveMod != "") {
    strGameName = strActiveMod;
  }

  // Get request time from some server request
  CTimerValue tvPingTime = SServerRequest::PopRequestTime(sinClient);

  // Get ping in milliseconds
  __int64 llPingTime = 0;

  if (!bIgnorePing && tvPingTime.tv_llValue != -1) {
    llPingTime = (_pTimer->GetHighPrecisionTimer() - tvPingTime).GetMilliseconds();
  }

  if (bIgnorePing || (llPingTime > 0 && llPingTime < 2500000)) {
    // Check for sessions with the same address
    const CTString strSessionAddress(0, "%s:%d", inet_ntoa(sinClient.sin_addr), htons(sinClient.sin_port) - 1);

    FOREACHINLIST(CNetworkSession, ns_lnNode, _pNetwork->ga_lhEnumeratedSessions, itns) {
      // Don't add a new session if it leads to the exact same server
      if (itns->ns_strAddress == strSessionAddress) {
        if (ms_bDebugOutput) {
          CPrintF("'%s' is already listed, skipping duplicate...\n", itns->ns_strSession.Undecorated());
        }
        return;
      }
    }

    // Create a new server listing
    CNetworkSession &ns = *new CNetworkSession;
    _pNetwork->ga_lhEnumeratedSessions.AddTail(ns.ns_lnNode);

    ns.ns_strAddress = strSessionAddress;
    ns.ns_tmPing = FLOAT(llPingTime) / 1000.0f;

    ns.ns_strSession = strHostName;
    ns.ns_strWorld = strMapName;
    ns.ns_ctPlayers = ctPlayers;
    ns.ns_ctMaxPlayers = ctMaxPlayers;

    ns.ns_strGameType = strGameType;
    ns.ns_strMod = strGameName;
    ns.ns_strVer = strGameVer;
  }
};

// Receive server data for enumeration
static BOOL ReceiveServerData(SOCKET &iSocketUDP, BOOL bLocal) {
  // Define empty read set with a socket
  fd_set fdsReadUDP;
  FD_ZERO(&fdsReadUDP);
  FD_SET(iSocketUDP, &fdsReadUDP);

  // Define a time value
  timeval timeoutUDP;
  timeoutUDP.tv_sec = 0; // 0 second timeout
  timeoutUDP.tv_usec = (bLocal ? 250000 : 50000); // Add 0.25 or 0.05 seconds

  int iNumber = select(iSocketUDP + 1, &fdsReadUDP, NULL, NULL, &timeoutUDP);

  if (iNumber <= 0) {
    return FALSE;
  }

  // Receive data
  sockaddr_in sinClient;
  socklen_t iClientLength = sizeof(sinClient);

  INDEX iReceived = recvfrom(iSocketUDP, IQuery::pBuffer, 2048, 0, (sockaddr *)&sinClient, &iClientLength);
  FD_CLR(iSocketUDP, &fdsReadUDP);

  // If received enough data
  if (iReceived != SOCKET_ERROR && iReceived > 100) {
    // End with a null terminator
    IQuery::pBuffer[iReceived] = '\0';

    // Check game name
    if (strstr(IQuery::pBuffer, "\\gamename\\" SAM_MS_NAME "\\") == NULL) {
      // Terminate upon unknown response
      if (ms_bDebugOutput) {
        CPrintF("Unknown query server response!\n"
                "Data (%d bytes): %s\n", iReceived, IQuery::pBuffer);
      }

      // [Cecil] TEMP: Don't terminate enumeration upon receiving random packets with players
      if (strstr(IQuery::pBuffer, "\\player_0\\") == NULL)
      {
        if (bLocal) {
          if (_pLocalAddressBuffer != NULL) {
            delete[] _pLocalAddressBuffer;
          }
          _pLocalAddressBuffer = NULL;

          WSACleanup();
        }
        return TRUE;
      }

    } else {
      // Ignore ping in local networks
      ParseStatusResponse(sinClient, bLocal);
    }

  } else {
    // Search for a request to clear it
    SServerRequest *preq = SServerRequest::Find(sinClient);

    if (preq != NULL) {
      preq->Clear();
    }
  }

  return FALSE;
};

static DWORD WINAPI MasterServerThread(LPVOID lpParam) {
  IQuery::SetStatus("");

  // Create a UDP socket
  SOCKET iSocketUDP = socket(AF_INET, SOCK_DGRAM, 0);

  if (iSocketUDP == INVALID_SOCKET) {
    WSACleanup();
    return 0;
  }

  const INDEX iAddrLength = 6;
  const char *pServers = _pOnlineAddressBuffer;

  while (_ctOnlineBuffer >= iAddrLength) {
    // Doesn't end with a final tag
    if (strncmp(pServers, "\\final\\", 7) == 0) {
      break;
    }

    // Add server request from the address at the current position
    IQuery::Address addr = *(IQuery::Address *)pServers;
    addr.AddServerRequest(&pServers, _ctOnlineBuffer, addr.uwPort, "\\status\\", iSocketUDP);

    // Receive server data for enumeration
    if (ReceiveServerData(iSocketUDP, FALSE)) {
      return 0;
    }
  }

  // Delete buffer
  if (_pOnlineAddressBuffer) {
    free(_pOnlineAddressBuffer);
  }
  _pOnlineAddressBuffer = NULL;

  // Stop enumeration
  closesocket(iSocketUDP);

  IQuery::CloseWinsock();
  IQuery::bInitialized = FALSE;

  _pNetwork->ga_bEnumerationChange = FALSE;
  WSACleanup();

  return 0;
};

static DWORD WINAPI LocalNetworkThread(LPVOID lpParam) {
  // Create a UDP socket
  SOCKET iSocketUDP = socket(AF_INET, SOCK_DGRAM, 0);

  if (iSocketUDP == INVALID_SOCKET) {
    WSACleanup();
    _pNetwork->ga_strEnumerationStatus = "";

    // Delete local buffer
    if (_pLocalAddressBuffer != NULL) {
      delete[] _pLocalAddressBuffer;
    }
    _pLocalAddressBuffer = NULL;

    return 0;
  }

  // Allow receiving UDP broadcast
  BOOL bOpt = TRUE;

  if (setsockopt(iSocketUDP, SOL_SOCKET, SO_BROADCAST, (char *)&bOpt, sizeof(bOpt)) != 0) {
    return 0;
  }

  const INDEX iAddrLength = 6;
  const char *pServers = _pLocalAddressBuffer;

  {
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = 0xFFFFFFFF;

    for (INDEX i = 25601 ; i <= 25621; i++) {
      saddr.sin_port = htons(i);
      IQuery::SendPacketTo(&saddr, "\\status\\", 8, iSocketUDP);
    }
  }

  while (_ctLocalBuffer >= iAddrLength) {
    // Doesn't end with a final tag
    if (strncmp(pServers, "\\final\\", 7) == 0) {
      break;
    }

    // Add server request from the address at the current position
    IQuery::Address addr = *(IQuery::Address *)pServers;
    addr.AddServerRequest(&pServers, _ctLocalBuffer, addr.uwPort, "\\status\\", iSocketUDP);

    // Receive server data for enumeration
    if (ReceiveServerData(iSocketUDP, TRUE)) {
      return 0;
    }
  }

  // Delete local buffer
  if (_pLocalAddressBuffer != NULL) {
    delete[] _pLocalAddressBuffer;
  }
  _pLocalAddressBuffer = NULL;

  // Stop enumeration
  closesocket(iSocketUDP);

  IQuery::CloseWinsock();
  IQuery::bInitialized = FALSE;

  _pNetwork->ga_bEnumerationChange = FALSE;
  _pNetwork->ga_strEnumerationStatus = "";

  WSACleanup();
  return 0;
};

void ILegacy::EnumTrigger(BOOL bInternet)
{
  if (bInternet) {
    StartInternetSearch();
  } else {
    StartLocalSearch();
  }
};

void ILegacy::EnumUpdate(void) {
  DWORD dwThreadId;

  // Start master server thread
  if (_bActivated) {
    HANDLE hThread = CreateThread(NULL, 0, MasterServerThread, 0, 0, &dwThreadId);

    if (hThread != NULL) {
      CloseHandle(hThread);
    }

    _bActivated = FALSE;
  }

  // Start local network thread
  if (_bActivatedLocal) {
    HANDLE hThread = CreateThread(NULL, 0, LocalNetworkThread, 0, 0, &dwThreadId);

    if (hThread != NULL) {
      CloseHandle(hThread);
    }

    _bActivatedLocal = FALSE;
  }
};

#endif // _PATCHCONFIG_NEW_QUERY
