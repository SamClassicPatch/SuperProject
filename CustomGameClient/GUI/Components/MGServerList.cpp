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
#include "MGServerList.h"
#include "MGEdit.h"

extern CSoundData *_psdSelect;
extern CSoundData *_psdPress;

FLOATaabbox2D GetBoxPartHoriz(const FLOATaabbox2D &box, FLOAT fMin, FLOAT fMax) {
  FLOAT fBoxMin = box.Min()(1);
  FLOAT fBoxSize = box.Size()(1);

  return FLOATaabbox2D(FLOAT2D(fBoxMin + fBoxSize * fMin, box.Min()(2)), FLOAT2D(fBoxMin + fBoxSize * fMax, box.Max()(2)));
}

void PrintInBox(CDrawPort *pdp, PIX pixI, PIX pixJ, PIX pixSizeI, CTString str, COLOR col, BOOL bUndecorated) {
  if (bUndecorated) {
    str = str.Undecorated();
  }

  // [Cecil] Decrease amount of characters until it fits within the width
  str.TrimRight(IData::TextFitsInWidth(pdp, pixSizeI, str));

  // print text
  pdp->PutText(str, pixI, pixJ, col);
}

CMGServerList::CMGServerList() {
  mg_iSelected = 0;
  mg_iFirstOnScreen = 0;
  mg_ctOnScreen = 10;
  mg_pixMinI = 0;
  mg_pixMaxI = 0;
  mg_pixListMinJ = 0;
  mg_pixListStepJ = 0;
  mg_pixDragJ = -1;
  mg_iDragLine = -1;
  mg_pixMouseDrag = -1;
  // by default, sort by ping, best on top
  mg_iSort = 2;
  mg_bSortDown = FALSE;
}
void CMGServerList::AdjustFirstOnScreen(void) {
  INDEX ctSessions = _lhServers.Count();
  mg_iSelected = Clamp(mg_iSelected, 0L, ClampDn(ctSessions - 1L, 0L));
  mg_iFirstOnScreen = Clamp(mg_iFirstOnScreen, 0L, ClampDn(ctSessions - mg_ctOnScreen, 0L));

  if (mg_iSelected < mg_iFirstOnScreen) {
    mg_iFirstOnScreen = ClampUp(mg_iSelected, ClampDn(ctSessions - mg_ctOnScreen - 1L, 0L));
  } else if (mg_iSelected >= mg_iFirstOnScreen + mg_ctOnScreen) {
    mg_iFirstOnScreen = ClampDn(mg_iSelected - mg_ctOnScreen + 1L, 0L);
  }
}

BOOL _iSort = 0;
BOOL _bSortDown = FALSE;

int CompareSessions(const void *pv0, const void *pv1) {
  const CNetworkSession &ns0 = **(const CNetworkSession **)pv0;
  const CNetworkSession &ns1 = **(const CNetworkSession **)pv1;

  int iResult = 0;
  switch (_iSort) {
    case 0: iResult = stricmp(ns0.ns_strSession, ns1.ns_strSession); break;
    case 1: iResult = stricmp(ns0.ns_strWorld, ns1.ns_strWorld); break;
    case 2: iResult = Sgn(ns0.ns_tmPing - ns1.ns_tmPing); break;
    case 3: iResult = Sgn(ns0.ns_ctPlayers - ns1.ns_ctPlayers); break;
    case 4: iResult = stricmp(ns0.ns_strGameType, ns1.ns_strGameType); break;
  #if SE1_GAME != SS_REV
    case 5: iResult = stricmp(ns0.ns_strMod, ns1.ns_strMod); break;
  #endif
    case 6: iResult = stricmp(ns0.ns_strVer, ns1.ns_strVer); break;
  }

  if (iResult == 0) { // make sure we always have unique order when resorting
    return stricmp(ns0.ns_strAddress, ns1.ns_strAddress);
    ;
  }

  return _bSortDown ? -iResult : iResult;
}

extern CMGButton mgServerColumn[7];
extern CMGEdit mgServerFilter[7];

void SortAndFilterServers(void) {
  {FORDELETELIST(CNetworkSession, ns_lnNode, _lhServers, itns) {
    delete &*itns;
  }}

  {FOREACHINLIST(CNetworkSession, ns_lnNode, _pNetwork->ga_lhEnumeratedSessions, itns) {
    CNetworkSession &ns = *itns;
    extern CTString _strServerFilter[7];
    if (_strServerFilter[0] != "" && !ns.ns_strSession.Matches("*" + _strServerFilter[0] + "*")) continue;
    if (_strServerFilter[1] != "" && !ns.ns_strWorld.Matches("*" + _strServerFilter[1] + "*")) continue;
    if (_strServerFilter[2] != "") {
      char strCompare[3] = { 0, 0, 0 };
      int iPing = 0;
      _strServerFilter[2].ScanF("%2[< >= ]%d", strCompare, &iPing);
      if (strcmp(strCompare, "<") == 0 && !(int(ns.ns_tmPing * 1000) < iPing)) continue;
      if (strcmp(strCompare, " <= ") == 0 && !(int(ns.ns_tmPing * 1000) <= iPing)) continue;
      if (strcmp(strCompare, ">") == 0 && !(int(ns.ns_tmPing * 1000) > iPing)) continue;
      if (strcmp(strCompare, " >= ") == 0 && !(int(ns.ns_tmPing * 1000) >= iPing)) continue;
      if (strcmp(strCompare, "=") == 0 && !(int(ns.ns_tmPing * 1000) == iPing)) continue;
    }
    if (_strServerFilter[3] != "") {
      char strCompare[3] = { 0, 0, 0 };
      int iPlayers = 0;
      _strServerFilter[3].ScanF("%2[< >= ]%d", strCompare, &iPlayers);
      if (strcmp(strCompare, "<") == 0 && !(ns.ns_ctPlayers < iPlayers)) continue;
      if (strcmp(strCompare, " <= ") == 0 && !(ns.ns_ctPlayers <= iPlayers)) continue;
      if (strcmp(strCompare, ">") == 0 && !(ns.ns_ctPlayers > iPlayers)) continue;
      if (strcmp(strCompare, " >= ") == 0 && !(ns.ns_ctPlayers >= iPlayers)) continue;
      if (strcmp(strCompare, "=") == 0 && !(ns.ns_ctPlayers == iPlayers)) continue;
    }
    if (_strServerFilter[4] != "" && !ns.ns_strGameType.Matches("*" + _strServerFilter[4] + "*")) continue;
  #if SE1_GAME != SS_REV
    if (_strServerFilter[5] != "" && !ns.ns_strMod.Matches("*" + _strServerFilter[5] + "*")) continue;
  #endif
    if (_strServerFilter[6] != "" && !ns.ns_strVer.Matches("*" + _strServerFilter[6] + "*")) continue;

    CNetworkSession *pnsNew = new CNetworkSession;
    pnsNew->Copy(*itns);
    _lhServers.AddTail(pnsNew->ns_lnNode);
  }}

  _lhServers.Sort(CompareSessions, offsetof(CNetworkSession, ns_lnNode));
}

void CMGServerList::Render(CDrawPort *pdp) {
  _iSort = mg_iSort;
  _bSortDown = mg_bSortDown;
  SortAndFilterServers();

  // [Cecil] Medium text scale
  const FLOAT fTextScale = 0.65f;
  const FLOAT fScaling = HEIGHT_SCALING(pdp);

  // [Cecil] Set medium font if scaling is larger than 150%
  if (fScaling >= 1.5f) {
    SetFontMedium(pdp, fTextScale);
  } else {
    SetFontSmall(pdp, 1.0f);
  }

  BOOL bFocusedBefore = mg_bFocused;
  mg_bFocused = FALSE;

  PIXaabbox2D box = FloatBoxToPixBox(pdp, mg_boxOnScreen);
  COLOR col = GetCurrentColor();

  PIX pixDPSizeI = pdp->GetWidth();
  PIX pixDPSizeJ = pdp->GetHeight();
  PIX pixCharSizeI = (pdp->dp_pixTextCharSpacing + pdp->dp_FontData->fd_pixCharWidth) * pdp->dp_fTextScaling;
  PIX pixCharSizeJ = (pdp->dp_pixTextLineSpacing + pdp->dp_FontData->fd_pixCharHeight + 1) * pdp->dp_fTextScaling;
  PIX pixLineSize = 1;
  PIX pixSliderSizeI = 10 * fScaling;
  PIX pixOuterMargin = 20;

  INDEX ctSessions = _lhServers.Count();
  INDEX iSession = 0;

  INDEX ctColumns[7];
  {for (INDEX i = 0; i < ARRAYCOUNT(ctColumns); i++) {
    ctColumns[i] = mgServerColumn[i].GetText().Length() + 1;
  }}

  PIX pixSizeMap, pixSizePing, pixSizePlrs, pixSizeMode, pixSizeMod, pixSizeVer;

  // [Cecil] Adjust sizes for the medium font
  if (fScaling >= 1.5f) {
    pixSizeMap  = PIX(pixDPSizeI * 0.25f) + pixLineSize * 2;
    pixSizePing = PIX(pixCharSizeI * 4)   + pixLineSize * 2;
    pixSizePlrs = PIX(pixCharSizeI * 4)   + pixLineSize * 2;
    pixSizeMode = PIX(pixCharSizeI * 7)   + pixLineSize * 2;
    pixSizeMod  = PIX(pixCharSizeI * 7)   + pixLineSize * 2;
    pixSizeVer  = PIX(pixCharSizeI * 4)   + pixLineSize * 2;

  } else {
    pixSizeMap  = Max(    PIX(pixDPSizeI * 0.25f),                         pixCharSizeI * ctColumns[1]) + pixLineSize * 2;
    pixSizePing = Max(    PIX(pixCharSizeI * 5),                           pixCharSizeI * ctColumns[2]) + pixLineSize * 2;
    pixSizePlrs = Max(    PIX(pixCharSizeI * 5),                           pixCharSizeI * ctColumns[3]) + pixLineSize * 2;
    pixSizeMode = Max(Min(PIX(pixCharSizeI * 20), PIX(pixDPSizeI * 0.2f)), pixCharSizeI * ctColumns[4]) + pixLineSize * 2;
    pixSizeMod  = Max(Min(PIX(pixCharSizeI * 11), PIX(pixDPSizeI * 0.2f)), pixCharSizeI * ctColumns[5]) + pixLineSize * 2;
    pixSizeVer  = Max(    PIX(pixCharSizeI * 7),                           pixCharSizeI * ctColumns[6]) + pixLineSize * 2;
  }

  PIX apixSeparatorI[9];
  apixSeparatorI[8] = pixDPSizeI - pixOuterMargin - pixLineSize;
  apixSeparatorI[7] = apixSeparatorI[8] - pixSliderSizeI - pixLineSize;
  apixSeparatorI[6] = apixSeparatorI[7] - pixSizeVer  - pixLineSize;
  apixSeparatorI[5] = apixSeparatorI[6] - pixSizeMod  - pixLineSize;
  apixSeparatorI[4] = apixSeparatorI[5] - pixSizeMode - pixLineSize;
  apixSeparatorI[3] = apixSeparatorI[4] - pixSizePlrs - pixLineSize;
  apixSeparatorI[2] = apixSeparatorI[3] - pixSizePing - pixLineSize;
  apixSeparatorI[1] = apixSeparatorI[2] - pixSizeMap  - pixLineSize;
  apixSeparatorI[0] = pixOuterMargin;

  PIX pixTopJ = pixDPSizeJ * 0.15f;
  PIX pixBottomJ = pixDPSizeJ * 0.82f;

  PIX pixFilterTopJ = pixTopJ + pixLineSize * 3 + pixCharSizeJ + pixLineSize * 3;
  PIX pixListTopJ = pixFilterTopJ + pixLineSize + pixCharSizeJ + pixLineSize;
  INDEX ctSessionsOnScreen = (pixBottomJ - pixListTopJ) / pixCharSizeJ;
  pixBottomJ = pixListTopJ + pixCharSizeJ * ctSessionsOnScreen + pixLineSize * 2;

  mg_pixMinI = apixSeparatorI[0];
  mg_pixMaxI = apixSeparatorI[5];
  mg_pixListMinJ = pixListTopJ;
  mg_pixListStepJ = pixCharSizeJ;
  mg_pixSBMinI = apixSeparatorI[7];
  mg_pixSBMaxI = apixSeparatorI[8];
  mg_pixSBMinJ = pixListTopJ;
  mg_pixSBMaxJ = pixBottomJ;
  mg_pixHeaderMinJ = pixTopJ;
  mg_pixHeaderMidJ = pixTopJ + pixLineSize + pixCharSizeJ;
  mg_pixHeaderMaxJ = pixTopJ + (pixLineSize + pixCharSizeJ) * 2;
  memcpy(mg_pixHeaderI, apixSeparatorI, sizeof(mg_pixHeaderI));

  {for (INDEX i = 0; i < ARRAYCOUNT(mgServerFilter); i++) {
    mgServerColumn[i].mg_boxOnScreen = PixBoxToFloatBox(pdp,
      PIXaabbox2D(PIX2D(apixSeparatorI[i] + pixCharSizeI / 2, pixTopJ + pixLineSize * 4), PIX2D(apixSeparatorI[i + 1] - pixCharSizeI / 2, pixTopJ + pixLineSize * 4 + pixCharSizeJ)));
    mgServerFilter[i].mg_boxOnScreen = PixBoxToFloatBox(pdp,
      PIXaabbox2D(PIX2D(apixSeparatorI[i] + pixCharSizeI / 2, pixFilterTopJ), PIX2D(apixSeparatorI[i + 1] - pixCharSizeI / 2, pixFilterTopJ + pixCharSizeJ)));

    // [Cecil] Set medium font if scaling is larger than 150%
    if (fScaling >= 1.5f) {
      mgServerColumn[i].mg_bfsFontSize = BFS_MEDIUM;
      mgServerColumn[i].mg_fTextScale = fTextScale;
      mgServerFilter[i].mg_bfsFontSize = BFS_MEDIUM;
      mgServerFilter[i].mg_fTextScale = fTextScale;

    } else {
      mgServerColumn[i].mg_bfsFontSize = BFS_SMALL;
      mgServerColumn[i].mg_fTextScale = 1.0f;
      mgServerFilter[i].mg_bfsFontSize = BFS_SMALL;
      mgServerFilter[i].mg_fTextScale = 1.0f;
    }
  }}

  for (INDEX i = 0; i < ARRAYCOUNT(apixSeparatorI); i++) {
    pdp->DrawLine(apixSeparatorI[i], pixTopJ, apixSeparatorI[i], pixBottomJ, col | CT_OPAQUE);
  }
  pdp->DrawLine(apixSeparatorI[0], pixTopJ, apixSeparatorI[8], pixTopJ, col | CT_OPAQUE);
  pdp->DrawLine(apixSeparatorI[0], pixListTopJ - pixLineSize, apixSeparatorI[8], pixListTopJ - pixLineSize, col | CT_OPAQUE);
  pdp->DrawLine(apixSeparatorI[0], pixBottomJ, apixSeparatorI[8], pixBottomJ, col | CT_OPAQUE);

  PIXaabbox2D boxHandle = GetScrollBarHandleBox();
  pdp->Fill(boxHandle.Min()(1) + 2, boxHandle.Min()(2) + 2, boxHandle.Size()(1) - 3, boxHandle.Size()(2) - 3, col | CT_OPAQUE);

  PIX pixJ = pixTopJ + pixLineSize * 2 + 1;

  mg_ctOnScreen = ctSessionsOnScreen;
  AdjustFirstOnScreen();

  if (_lhServers.Count() == 0) {
    if (_pNetwork->ga_strEnumerationStatus != "") {
      mg_bFocused = TRUE;
      COLOR colItem = GetCurrentColor();
      PrintInBox(pdp, apixSeparatorI[0] + pixCharSizeI, pixListTopJ + pixCharSizeJ + pixLineSize + 1,
                 apixSeparatorI[1] - apixSeparatorI[0], LOCALIZE("searching..."), colItem, TRUE);
    }
  } else {
    FOREACHINLIST(CNetworkSession, ns_lnNode, _lhServers, itns) {
      CNetworkSession &ns = *itns;

      if (iSession < mg_iFirstOnScreen || iSession >= mg_iFirstOnScreen + ctSessionsOnScreen) {
        iSession++;
        continue;
      }

      PIX pixJ = pixListTopJ + (iSession - mg_iFirstOnScreen) * pixCharSizeJ + pixLineSize + 1;

      mg_bFocused = bFocusedBefore && iSession == mg_iSelected;
      COLOR colItem = GetCurrentColor();

      if (ns.ns_strVer != _SE_VER_STRING) {
        colItem = MulColors(colItem, 0xA0A0A0FF);
      }

      CTString strPing(0, "%4d", INDEX(ns.ns_tmPing * 1000));
      CTString strPlayersCt(0, "%2d/%2d", ns.ns_ctPlayers, ns.ns_ctMaxPlayers);
      CTString strMod = sam_strGameName;

    #if SE1_GAME != SS_REV
      if (ns.ns_strMod != "") strMod = ns.ns_strMod;
    #endif

      // [Cecil] Arranged in an array
      const CTString astrEntries[7] = {
        ns.ns_strSession,
        TRANSV(ns.ns_strWorld),
        strPing,
        strPlayersCt,
        TRANSV(ns.ns_strGameType),
        TRANSV(strMod),
        ns.ns_strVer,
      };

      // [Cecil] Under which conditions to undecorate tab entries
      const BOOL abUndecorate[7] = {
        mg_bFocused || !sam_bDecoratedServerNames,
        mg_bFocused,
        TRUE,
        TRUE,
        mg_bFocused,
        TRUE,
        TRUE,
      };

      // [Cecil] Print entry in each tab
      for (INDEX iTab = 0; iTab < 7; iTab++) {
        PrintInBox(pdp, apixSeparatorI[iTab] + pixCharSizeI / 2, pixJ,
                   apixSeparatorI[iTab + 1] - apixSeparatorI[iTab] - pixCharSizeI,
                   astrEntries[iTab], colItem, abUndecorate[iTab]);
      }

      iSession++;
    }
  }

  mg_bFocused = bFocusedBefore;
}

static INDEX SliderPixToIndex(PIX pixOffset, INDEX iVisible, INDEX iTotal, PIXaabbox2D boxFull) {
  FLOAT fSize = ClampUp(FLOAT(iVisible) / iTotal, 1.0f);
  PIX pixFull = boxFull.Size()(2);
  PIX pixSize = PIX(pixFull * fSize);
  if (pixSize >= boxFull.Size()(2)) {
    return 0;
  }
  return (iTotal * pixOffset) / pixFull;
}

static PIXaabbox2D GetSliderBox(INDEX iFirst, INDEX iVisible, INDEX iTotal, PIXaabbox2D boxFull) {
  if (iTotal <= 0) {
    return boxFull;
  }
  FLOAT fSize = ClampUp(FLOAT(iVisible) / iTotal, 1.0f);
  PIX pixFull = boxFull.Size()(2);
  PIX pixSize = PIX(pixFull * fSize);
  pixSize = ClampDn(pixSize, boxFull.Size()(1));
  PIX pixTop = pixFull * (FLOAT(iFirst) / iTotal) + boxFull.Min()(2);
  PIX pixI0 = boxFull.Min()(1);
  PIX pixI1 = boxFull.Max()(1);
  return PIXaabbox2D(PIX2D(pixI0, pixTop), PIX2D(pixI1, pixTop + pixSize));
}

PIXaabbox2D CMGServerList::GetScrollBarFullBox(void) {
  return PIXaabbox2D(PIX2D(mg_pixSBMinI, mg_pixSBMinJ), PIX2D(mg_pixSBMaxI, mg_pixSBMaxJ));
}

PIXaabbox2D CMGServerList::GetScrollBarHandleBox(void) {
  return GetSliderBox(mg_iFirstOnScreen, mg_ctOnScreen, _lhServers.Count(), GetScrollBarFullBox());
}

void CMGServerList::OnMouseOver(PIX pixI, PIX pixJ) {
  mg_pixMouseI = pixI;
  mg_pixMouseJ = pixJ;

  BOOL bInSlider = (pixI >= mg_pixSBMinI && pixI <= mg_pixSBMaxI && pixJ >= mg_pixSBMinJ && pixJ <= mg_pixSBMaxJ);
  if (mg_pixDragJ >= 0 && bInSlider) {
    PIX pixDelta = pixJ - mg_pixDragJ;
    INDEX ctSessions = _lhServers.Count();
    INDEX iWantedLine = mg_iDragLine + SliderPixToIndex(pixDelta, mg_ctOnScreen, ctSessions, GetScrollBarFullBox());
    mg_iFirstOnScreen = Clamp(iWantedLine, 0L, ClampDn(ctSessions - mg_ctOnScreen, 0L));
    mg_iSelected = Clamp(mg_iSelected, mg_iFirstOnScreen, mg_iFirstOnScreen + mg_ctOnScreen - 1L);
    //    AdjustFirstOnScreen();
    return;
  }

  // if some server is selected
  if (pixI >= mg_pixMinI && pixI <= mg_pixMaxI) {
    INDEX iOnScreen = (pixJ - mg_pixListMinJ) / mg_pixListStepJ;
    if (iOnScreen >= 0 && iOnScreen < mg_ctOnScreen) {
      // put focus on it
      mg_iSelected = mg_iFirstOnScreen + iOnScreen;
      AdjustFirstOnScreen();
      mg_pixMouseDrag = -1;
    }
  } else if (bInSlider) {
    mg_pixMouseDrag = pixJ;
  }
}

BOOL CMGServerList::OnKeyDown(PressedMenuButton pmb) {
  // [Cecil] Start dragging with left mouse button
  if (pmb.iKey == VK_LBUTTON)
  {
    if (mg_pixMouseDrag >= 0) {
      mg_pixDragJ = mg_pixMouseDrag;
      mg_iDragLine = mg_iFirstOnScreen;
      return TRUE;
    }
  }

  // [Cecil] Scroll in some direction
  const INDEX iScroll = pmb.ScrollPower();

  if (iScroll != 0) {
    const INDEX iPower = Abs(iScroll);
    INDEX ctScroll = 0;

    // Entire page at a time
    if (iPower == 1) {
      ctScroll = (mg_ctOnScreen - 1);

    // A few at a time
    } else if (iPower == 2) {
      ctScroll = 3;
    }

    if (ctScroll != 0) {
      const INDEX iDir = SgnNZ(iScroll);
      mg_iSelected      += ctScroll * iDir;
      mg_iFirstOnScreen += ctScroll * iDir;
      AdjustFirstOnScreen();
    }
  }

  // [Cecil] Go up one listing
  if (pmb.Up()) {
    mg_iSelected -= 1;
    AdjustFirstOnScreen();
    return TRUE;
  }

  // [Cecil] Go down one listing
  if (pmb.Down()) {
    mg_iSelected += 1;
    AdjustFirstOnScreen();
    return TRUE;
  }

  // [Cecil] Select the listing
  if (pmb.Apply(TRUE)) {
    PlayMenuSound(_psdPress);
    IFeel_PlayEffect("Menu_press");

    INDEX i = 0;

    FOREACHINLIST(CNetworkSession, ns_lnNode, _lhServers, itns) {
      if (i == mg_iSelected) {
        char strAddress[256];
        int iPort;
        itns->ns_strAddress.ScanF("%200[^:]:%d", &strAddress, &iPort);
        _pGame->gam_strJoinAddress = strAddress;
        _pShell->SetINDEX("net_iPort", iPort);

        extern void StartJoinServerMenu(void);
        StartJoinServerMenu();
        return TRUE;
      }

      i++;
    }

    // [Cecil] NOTE: It skipped to the end with TRUE before, so not sure
    return TRUE;
  }

  return FALSE;
}

// [Cecil] Stop dragging the scrollbar
BOOL CMGServerList::OnKeyUp(PressedMenuButton pmb) {
  if (pmb.iKey == VK_LBUTTON) {
    mg_pixDragJ = -1;
    return TRUE;
  }

  return CMGButton::OnKeyUp(pmb);
};

void CMGServerList::OnSetFocus(void) {
  mg_bFocused = TRUE;
}

void CMGServerList::OnKillFocus(void) {
  mg_bFocused = FALSE;
}