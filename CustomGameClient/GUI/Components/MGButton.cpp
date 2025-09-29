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
#include "LevelInfo.h"
#include "VarList.h"
#include "MGButton.h"

extern CSoundData *_psdPress;

CMGButton::CMGButton(void) {
  mg_pActivatedFunction = NULL;
  mg_iIndex = 0;
  mg_iCenterI = 0;
  mg_iCenterJ = -1; // [Cecil] Top
  mg_iTextMode = 1;
  mg_bfsFontSize = BFS_MEDIUM;
  mg_iCursorPos = -1;
  mg_fTextScale = 1.0f; // [Cecil]
  mg_bRectangle = FALSE;
  mg_bMental = FALSE;
  mg_bEditing = FALSE;
  mg_bHighlighted = FALSE;
  mg_bHiddenText = FALSE; // [Cecil]
}

void CMGButton::OnActivate(void) {
  if (mg_pActivatedFunction != NULL && mg_bEnabled) {
    PlayMenuSound(_psdPress);
    _pmgLastActivatedGadget = this;
    (*mg_pActivatedFunction)();
  }
}

void CMGButton::Render(CDrawPort *pdp) {
  // [Cecil] Replaced if-else blocks with switch-case
  switch (mg_bfsFontSize)
  {
    default:
    case BFS_SMALL: {
      ASSERT(mg_bfsFontSize == BFS_SMALL);
      SetFontSmall(pdp, mg_fTextScale);
    } break;

    case BFS_MEDIUM: {
      SetFontMedium(pdp, mg_fTextScale);
    } break;

    case BFS_LARGE: {
      SetFontBig(pdp, mg_fTextScale);
    } break;
  }

  pdp->SetTextMode(mg_iTextMode);

  // Convert to pixel box
  const PIXaabbox2D box = FloatBoxToPixBox(pdp, mg_boxOnScreen);

  // [Cecil] Check if there's a button label
  const BOOL bLabel = (GetName() != "");

  // Get current colors
  COLOR col = GetCurrentColor();

  if (mg_bEditing) {
    col = _pGame->LCDGetColor(C_GREEN | 0xFF, "editing");
  }

  COLOR colRectangle = col;

  // Get highlighted colors
  if (mg_bHighlighted) {
    col = _pGame->LCDGetColor(C_WHITE | 0xFF, "hilited");

    if (!mg_bFocused) {
      colRectangle = _pGame->LCDGetColor(C_WHITE | 0xFF, "hilited rectangle");
    }
  }

  // Fade in and out
  if (mg_bMental) {
    FLOAT tmIn = 0.2f;
    FLOAT tmOut = 1.0f;
    FLOAT tmFade = 0.1f;
    FLOAT tmExist = tmFade + tmIn + tmFade;
    FLOAT tmTotal = tmFade + tmIn + tmFade + tmOut;

    CTimerValue tvPatch(ClassicsCore_GetInitTime()); // [Cecil]
    DOUBLE tmTime = (_pTimer->GetHighPrecisionTimer() - tvPatch).GetSeconds();
    FLOAT fFactor = 1;

    if (tmTime > 0.1) {
      tmTime = fmod(tmTime, tmTotal);
      fFactor = CalculateRatio(tmTime, 0, tmExist, tmFade / tmExist, tmFade / tmExist);
    }

    col = (col & ~0xFF) | INDEX(0xFF * fFactor);
  }

  // Draw rectangular border
  if (mg_bRectangle) {
    const PIX pixLeft = box.Min()(1);
    const PIX pixUp = box.Min()(2) - 3;
    const PIX pixWidth = box.Size()(1) + 1;
    const PIX pixHeight = box.Size()(2);

    pdp->DrawBorder(pixLeft, pixUp, pixWidth, pixHeight, colRectangle);
  }

  CTString strText = GetText();

  // Draw border for textbox editing
  if (mg_bEditing) {
    PIX pixLeft = box.Min()(1);
    PIX pixUp = box.Min()(2) - 3;
    PIX pixWidth = box.Size()(1) + 1;
    PIX pixHeight = box.Size()(2);

    // Shift to the right side if there's a button label
    if (bLabel) {
      pixLeft = box.Min()(1) + box.Size()(1) * _fGadgetSideRatioR;
      pixWidth = box.Size()(1) * _fGadgetSideRatioL + 1;
    }

    pdp->Fill(pixLeft, pixUp, pixWidth, pixHeight, _pGame->LCDGetColor(C_dGREEN | 0x40, "edit fill"));

  // [Cecil] Hide the button text if not editing
  } else if (mg_bHiddenText) {
    INDEX iLength = strText.Length();

    // Create a string as long as the original text
    char *strHidden = new char[iLength + 1];

    // Mask all the characters instead of copying them
    memset(strHidden, '*', iLength);
    strHidden[iLength] = '\0';

    // Replace original text with the hidden string
    strText = strHidden;
    delete[] strHidden;
  }

  INDEX iCursor = mg_iCursorPos;

  // If there's a button label
  if (bLabel) {
    PIX pixIL = box.Min()(1) + box.Size()(1) * _fGadgetSideRatioL;
    PIX pixIR = box.Max()(1) - box.Size()(1) * _fGadgetSideRatioL;
    PIX pixJ = box.Min()(2);

    // Put label on the left and text on the right
    pdp->PutTextR(GetName(), pixIL, pixJ, col);
    pdp->PutText(strText, pixIR, pixJ, col);

  // If no button label
  } else {
    // If using monospace font
    if (pdp->dp_FontData->fd_bFixedWidth) {
      // Undecorate the string
      strText = strText.Undecorated();

      const INDEX iMaxLen = ClampDn(box.Size()(1) / (pdp->dp_pixTextCharSpacing + pdp->dp_FontData->fd_pixCharWidth), 1L);

      // Trim the string if the cursor is past the maximum length
      if (iCursor >= iMaxLen) {
        strText.TrimRight(iCursor);
        strText.TrimLeft(iMaxLen);
        iCursor = iMaxLen;

      // Trim the string to the maximum length
      } else {
        strText.TrimRight(iMaxLen);
      }
    }

    // [Cecil] Align text vertically
    const PIX pixTextHeight = pdp->dp_FontData->GetHeight() * pdp->dp_fTextScaling;
    PIX pixTextY;

    if (mg_iCenterJ == -1) {
      pixTextY = box.Min()(2);

    } else if (mg_iCenterJ == +1) {
      pixTextY = box.Max()(2) - pixTextHeight;

    } else {
      pixTextY = box.Center()(2) - pixTextHeight * 0.5f;
    }

    // Align text horizontally
    if (mg_iCenterI == -1) {
      pdp->PutText(strText, box.Min()(1), pixTextY, col);

    } else if (mg_iCenterI == +1) {
      pdp->PutTextR(strText, box.Max()(1), pixTextY, col);

    } else {
      pdp->PutTextC(strText, box.Center()(1), pixTextY, col);
    }
  }

  // Add blinking cursor to the string if editing
  if (mg_bEditing && ULONG(_pTimer->GetRealTimeTick() * 2.0f) & 1) {
    PIX pixX = box.Min()(1) + GetCharOffset(pdp, iCursor);
    PIX pixY = box.Min()(2);

    // Shift to the right side if there's a button label
    if (bLabel) {
      pixX += box.Size()(1) * _fGadgetSideRatioR;
    }

    if (!pdp->dp_FontData->fd_bFixedWidth) {
      pixY -= pdp->dp_fTextScaling * 2;
    }

    pdp->PutText("|", pixX, pixY, _pGame->LCDGetColor(C_WHITE | 0xFF, "editing cursor"));
  }
}

PIX CMGButton::GetCharOffset(CDrawPort *pdp, INDEX iCharNo) {
  if (pdp->dp_FontData->fd_bFixedWidth) {
    return (pdp->dp_FontData->fd_pixCharWidth + pdp->dp_pixTextCharSpacing) * (iCharNo - 0.5f);
  }
  CTString strCut(GetText());
  strCut.TrimLeft(GetText().Length() - iCharNo);
  PIX pixFullWidth = IRender::GetTextWidth(pdp, GetText());
  PIX pixCutWidth = IRender::GetTextWidth(pdp, strCut);
  // !!!! not implemented for different centering
  return pixFullWidth - pixCutWidth;
}