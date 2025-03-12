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
#include "VarList.h"
#include "MGVarButton.h"

extern PIX _pixCursorPosI;
extern PIX _pixCursorPosJ;

extern BOOL _bVarChanged;
extern CSoundData *_psdSelect;
extern CSoundData *_psdPress;

// [Cecil] Constructor
CMGVarButton::CMGVarButton() : CMGEdit() {
  mg_pvsVar = NULL;
  mg_ctListValuesAtOnce = 0;
  mg_iValueListOffset = 0;
  mg_boxList = PIXaabbox2D(PIX2D(0, 0), PIX2D(0, 0));
  mg_iListValue = -1;
  mg_iLastListValue = -1;

  mg_vMouseInScrollbarArea = PIX2D(-1, -1);
  mg_boxScrollbarArea = PIXaabbox2D(PIX2D(0, 0), PIX2D(0, 0));

  mg_vMouseScrollbarDrag = PIX2D(-1, -1);
  mg_bMouseOnScrollbar = FALSE;
  mg_iLastValueListOffset = -1;
};

BOOL CMGVarButton::IsSeparator(void) {
  if (mg_pvsVar == NULL) {
    return FALSE;
  }

  // [Cecil] Check the separator type
  return (mg_pvsVar->vs_eType == CVarSetting::E_SEPARATOR);
}

BOOL CMGVarButton::IsEnabled(void) {
  return (_gmRunningGameMode == GM_NONE || mg_pvsVar == NULL || mg_pvsVar->vs_bCanChangeInGame);
}

// return slider position on scren
PIXaabbox2D CMGVarButton::GetSliderBox(CDrawPort *pdp, INDEX iSliderType) {
  // [Cecil] Big fill slider
  if (iSliderType == CVarSetting::SLD_BIGFILL) {
    extern PIXaabbox2D GetHorSliderBox(CDrawPort *pdp, FLOATaabbox2D boxOnScreen, BOOL bHasLabel);
    return GetHorSliderBox(pdp, mg_boxOnScreen, TRUE);
  }

  PIXaabbox2D box = FloatBoxToPixBox(pdp, mg_boxOnScreen);
  PIX pixIR = box.Min()(1) + box.Size()(1) * _fGadgetSideRatioR;
  PIX pixJ = box.Min()(2);
  PIX pixISize = box.Size()(1) * 0.13f;
  PIX pixJSize = box.Size()(2);

  return PIXaabbox2D(PIX2D(pixIR + 1, pixJ + 1), PIX2D(pixIR + pixISize - 4, pixJ + pixJSize - 6));
}

BOOL CMGVarButton::OnKeyDown(PressedMenuButton pmb)
{
  if (mg_pvsVar == NULL || mg_pvsVar->vs_eType == CVarSetting::E_SEPARATOR || !mg_pvsVar->Validate() || !mg_bEnabled) {
    // [Cecil] CMenuGadget::OnKeyDown() would call CMGEdit::OnActivate(), which shouldn't happen
    return pmb.Apply(TRUE);
  }

  CVarMenu &gmCurrent = _pGUIM->gmVarMenu;
  const BOOL bToggle = (mg_pvsVar->vs_eType == CVarSetting::E_TOGGLE);
  const BOOL bTextbox = (mg_pvsVar->vs_eType == CVarSetting::E_TEXTBOX);
  const BOOL bSlider = (mg_pvsVar->vs_eSlider != CVarSetting::SLD_NOSLIDER);

  if (mg_bEditing) {
    // [Cecil] Interact with the value list
    if (bToggle && !bSlider) {
      // Start dragging with left mouse button
      if (pmb.iKey == VK_LBUTTON && mg_vMouseInScrollbarArea != PIX2D(-1, -1)) {
        mg_vMouseScrollbarDrag = mg_vMouseInScrollbarArea;
        mg_bMouseOnScrollbar = (mg_boxScrollbarArea >= mg_vMouseInScrollbarArea);
        mg_iLastValueListOffset = mg_iValueListOffset;
      }

      return ListOnKeyDown(pmb);
    }

    // [Cecil] Editing the textbox
    return CMGEdit::OnKeyDown(pmb);
  }

  // [Cecil] Toggleable setting
  if (bToggle) {
    // handle slider
    if (bSlider && !mg_pvsVar->vs_bCustom) {
      // ignore RMB
      if (pmb.iKey == VK_RBUTTON) {
        return TRUE;
      }

      // handle LMB
      if (pmb.iKey == VK_LBUTTON) {
        // get position of slider box on screen
        PIXaabbox2D boxSlider = GetSliderBox(_pdpMenu, mg_pvsVar->vs_eSlider);
        // if mouse is within
        if (boxSlider >= PIX2D(_pixCursorPosI, _pixCursorPosJ)) {
          // set new position exactly where mouse pointer is
          mg_pvsVar->vs_iValue = (FLOAT)(_pixCursorPosI - boxSlider.Min()(1)) / boxSlider.Size()(1) * (mg_pvsVar->vs_ctValues);
          OnVarChanged(); // [Cecil]
        }

        // handled
        return TRUE;
      }
    }

  // [Cecil] Button setting
  } else if (mg_pvsVar->vs_eType == CVarSetting::E_BUTTON) {
    // Enter another option config on click
    if (pmb.Apply(TRUE)) {
      // Copy the string from the setting
      const CTString strConfig = mg_pvsVar->vs_strSchedule;

      FlushVarSettings(FALSE);
      gmCurrent.EndMenu();

      gmCurrent.gm_fnmMenuCFG = strConfig;
      gmCurrent.StartMenu();

      PlayMenuSound(_psdPress);
      return TRUE;
    }
  }

  if (pmb.Apply(FALSE)) {
    // [Cecil] Allow using controllers for textbox editing
    if (bTextbox && pmb.iCtrl != -1) {
      return CMGEdit::OnKeyDown(pmb);
    }

    // [Cecil] Emulate the action of clicking on "Apply"
    gmCurrent.gm_mgApply.OnActivate();
    return TRUE;
  }

  // [Cecil] Toggle values
  if (bToggle) {
    // [Cecil] Open a list of values for toggleable non-sliders
    if (sam_bConfigValueLists && pmb.Apply(TRUE) && !bSlider && mg_pvsVar->vs_ctValues > 2) {
      ListActivate();
      return TRUE;
    }

    // [Cecil] Increase/decrease the value
    INDEX iPower = pmb.ChangeValue();

    // Try previous/next value
    if (iPower == 0) {
      if (pmb.Prev()) iPower = -1;
      else
      if (pmb.Next()) iPower = +1;
    }

    if (iPower != 0) {
      // Change one value at a time for non-sliders
      if (!bSlider) iPower = SgnNZ(iPower);

      INDEX iOldValue = mg_pvsVar->vs_iValue;
      mg_pvsVar->vs_iValue += iPower;

      // Clamp sliders
      if (bSlider) {
        mg_pvsVar->vs_iValue = Clamp(mg_pvsVar->vs_iValue, (INDEX)0, INDEX(mg_pvsVar->vs_ctValues - 1));

      // Wrap to the beginning
      } else if (mg_pvsVar->vs_iValue >= mg_pvsVar->vs_ctValues) {
        mg_pvsVar->vs_iValue = 0;

      // Wrap to the end
      } else if (mg_pvsVar->vs_iValue < 0) {
        mg_pvsVar->vs_iValue = mg_pvsVar->vs_ctValues - 1;
      }

      if (iOldValue != mg_pvsVar->vs_iValue) {
        mg_pvsVar->vs_bCustom = FALSE;
        mg_pvsVar->Validate();
        OnVarChanged(); // [Cecil]
      }

      return TRUE;
    }

  // [Cecil] Reset editing value
  } else if (bTextbox) {
    OnStringCanceled();
  }

  // [Cecil] Pass to CMGEdit instead of CMenuGadget
  return CMGEdit::OnKeyDown(pmb);
}

// [Cecil] Stop dragging the scrollbar
BOOL CMGVarButton::OnKeyUp(PressedMenuButton pmb) {
  if (pmb.iKey == VK_LBUTTON) {
    mg_vMouseScrollbarDrag = PIX2D(-1, -1);
    return TRUE;
  }

  return CMGEdit::OnKeyUp(pmb);
};

// [Cecil] Pass character events only to textboxes
BOOL CMGVarButton::OnChar(MSG msg) {
  if (mg_pvsVar == NULL || mg_pvsVar->vs_eType != CVarSetting::E_TEXTBOX) {
    return CMenuGadget::OnChar(msg);
  }

  return CMGEdit::OnChar(msg);
};

// [Cecil] Adjust the slider by holding a button
BOOL CMGVarButton::OnMouseHeld(PressedMenuButton pmb)
{
  if (pmb.iKey != VK_LBUTTON) return FALSE;
  if (mg_pvsVar == NULL) return FALSE;

  // Not a toggleable setting
  if (mg_pvsVar->vs_eType != CVarSetting::E_TOGGLE) return FALSE;

  const BOOL bSlider = (mg_pvsVar->vs_eSlider != CVarSetting::SLD_NOSLIDER);

  // [Cecil] Scroll the value list
  if (mg_bEditing && !bSlider && mg_vMouseScrollbarDrag != PIX2D(-1, -1)) {
    return ListDragScrollbar();
  }

  // If it's a toggleable slider without a custom value that was pressed last
  if (_pmgLastPressedGadget == this && pmb.iKey == VK_LBUTTON && bSlider && !mg_pvsVar->vs_bCustom) {
    // Forward the key
    return OnKeyDown(pmb);
  }

  return FALSE;
};

void CMGVarButton::Render(CDrawPort *pdp) {
  if (mg_pvsVar == NULL) {
    return;
  }

  SetFontMedium(pdp, mg_fTextScale);

  PIXaabbox2D box = FloatBoxToPixBox(pdp, mg_boxOnScreen);
  PIX pixIL = box.Min()(1) + box.Size()(1) * _fGadgetSideRatioL;
  PIX pixIR = box.Min()(1) + box.Size()(1) * _fGadgetSideRatioR;
  PIX pixIC = box.Center()(1);
  PIX pixJ = box.Min()(2);

  // [Cecil] Different types
  switch (mg_pvsVar->vs_eType) {
    // Separator
    case CVarSetting::E_SEPARATOR: {
      mg_bEnabled = FALSE;
      COLOR col = _pGame->LCDGetColor(C_WHITE | 255, "separator");

      CTString strText = mg_pvsVar->vs_strName;
      pdp->PutTextC(strText, pixIC, pixJ, col);
    } break;

    // Toggleable
    case CVarSetting::E_TOGGLE: {
      if (mg_pvsVar->Validate()) {
        // check whether the variable is disabled
        if (mg_pvsVar->vs_strFilter != "") {
          mg_bEnabled = _pShell->GetINDEX(mg_pvsVar->vs_strFilter);
        }

        COLOR col = GetCurrentColor();
        pdp->PutTextR(mg_pvsVar->vs_strName, pixIL, pixJ, col);

        // custom is by default
        CTString strText = LOCALIZE("Custom");
        BOOL bCenteredText = FALSE;

        const CVarSetting::ESliderType eSlider = mg_pvsVar->vs_eSlider;

        // [Cecil] Render currently active value list
        if (mg_bEditing && eSlider == CVarSetting::SLD_NOSLIDER) {
          ListRender(pdp, PIX2D(pixIR, pixJ), strText);
          break;
        }

        // Not a custom value
        if (!mg_pvsVar->vs_bCustom) {
          strText = mg_pvsVar->vs_astrTexts[mg_pvsVar->vs_iValue];

          const FLOAT fFactor = FLOAT(mg_pvsVar->vs_iValue + 1) / (FLOAT)mg_pvsVar->vs_ctValues;

          // [Cecil] Use pre-calculated slider box for rendering
          PIXaabbox2D boxSlider = GetSliderBox(pdp, eSlider);
          const PIX pixSliderX = boxSlider.Min()(1) + 1;
          const PIX pixSliderY = boxSlider.Min()(2) + 1;
          const PIX2D vSliderSize = boxSlider.Size();

          // [Cecil] Unified both fill slider types
          if (eSlider == CVarSetting::SLD_FILL || eSlider == CVarSetting::SLD_BIGFILL) {
            // Draw box around the slider
            _pGame->LCDDrawBox(0, -1, boxSlider, _pGame->LCDGetColor(C_GREEN | 255, "slider box"));

            // Fill slider
            pdp->Fill(pixSliderX, pixSliderY, (vSliderSize(1) - 2) * fFactor, (vSliderSize(2) - 2), col);

            // [Cecil] Move text in the middle of the box
            if (eSlider == CVarSetting::SLD_BIGFILL) {
              pixIR += vSliderSize(1) / 2;
              pixJ += 2;
              bCenteredText = TRUE;

            } else {
              // Move text to the right of the box
              pixIR += vSliderSize(1) * 1.15f;
            }

          // Ratio slider
          } else if (eSlider == CVarSetting::SLD_RATIO) {
            // Draw box around the slider
            _pGame->LCDDrawBox(0, -1, boxSlider, _pGame->LCDGetColor(C_GREEN | 255, "slider box"));

            FLOAT fUnitWidth = FLOAT(vSliderSize(1) - 1) / mg_pvsVar->vs_ctValues;
            pdp->Fill(pixSliderX + (mg_pvsVar->vs_iValue * fUnitWidth), pixSliderY, fUnitWidth, (vSliderSize(2) - 2), col);
            
            // Move text to the right of the box
            pixIR += vSliderSize(1) * 1.15f;
          }
        }

        // Write value text
        if (bCenteredText) {
          pdp->PutTextC(strText, pixIR, pixJ, col);
        } else {
          pdp->PutText(strText, pixIR, pixJ, col);
        }
      }
    } break;

    // Textbox
    case CVarSetting::E_TEXTBOX: {
      CMGEdit::Render(pdp);
    } break;

    // Button
    case CVarSetting::E_BUTTON: {
      pdp->PutTextC(mg_pvsVar->vs_strName, pixIC, pixJ, GetCurrentColor());
    } break;
  }
}

// [Cecil] Signal that some variable has been changed
void CMGVarButton::OnVarChanged(void) {
  // Apply new value immediately without having to press "Apply"
  if (mg_pvsVar->vs_bRealTime) {
    const BOOL bScheduled = mg_pvsVar->ApplyValue();

    // Execute scheduled command
    if (bScheduled && mg_pvsVar->vs_strSchedule != "") {
      _pShell->Execute(mg_pvsVar->vs_strSchedule);
    }

    mg_pvsVar->UpdateValue();
  }

  // Not necessary for the code above but good for the old reflexes of clicking "Apply" anyway
  _bVarChanged = TRUE;
};

// [Cecil] Change strings
void CMGVarButton::OnStringChanged(void) {
  // No textbox attached
  if (mg_pvsVar == NULL || mg_pvsVar->vs_eType != CVarSetting::E_TEXTBOX) {
    return;
  }

  // If new hash differs from the old one, mark as changed
  ULONG ulOldHash = static_cast<ULONG>(mg_pvsVar->vs_iOrgValue);

  if (mg_pvsVar->vs_strValue.GetHash() != ulOldHash) {
    OnVarChanged(); // [Cecil]
  }
};

void CMGVarButton::OnStringCanceled(void) {
  // Restore string from the setting
  SetText(mg_pvsVar->vs_strValue);
};

// [Cecil] Activate value list
void CMGVarButton::ListActivate(void) {
  mg_ctListValuesAtOnce = 0;
  mg_iValueListOffset = 0;
  mg_boxList = PIXaabbox2D(PIX2D(0, 0), PIX2D(0, 0));
  mg_iListValue = -1;
  mg_iLastListValue = -1;

  mg_bEditing = TRUE;
  _eEditingValue = VED_LIST;
  PlayMenuSound(_psdPress);
};

// [Cecil] Deactivate value list
void CMGVarButton::ListDeactivate(INDEX iSelectValue) {
  if (mg_pvsVar != NULL && iSelectValue != -1) {
    INDEX iOldValue = mg_pvsVar->vs_iValue;
    mg_pvsVar->vs_iValue = Clamp(iSelectValue, (INDEX)0, INDEX(mg_pvsVar->vs_ctValues - 1));

    if (iOldValue != mg_pvsVar->vs_iValue) {
      mg_pvsVar->vs_bCustom = FALSE;
      mg_pvsVar->Validate();
      OnVarChanged(); // [Cecil]
    }

    PlayMenuSound(_psdPress);
  }

  mg_bEditing = FALSE;
  _eEditingValue = VED_NONE;
};

// [Cecil] Value list interactions
BOOL CMGVarButton::ListOnKeyDown(PressedMenuButton pmb) {
  const INDEX ctList = mg_pvsVar->vs_ctValues;

  // Scroll the list
  if (mg_ctListValuesAtOnce < ctList) {
    INDEX iScroll = pmb.ScrollPower();
    const INDEX iLastOffset = (ctList - mg_ctListValuesAtOnce);

    if (iScroll != 0) {
      const INDEX iDir = SgnNZ(iScroll);

      if (Abs(iScroll) == 1) {
        iScroll = iDir * mg_ctListValuesAtOnce;
      } else {
        iScroll = iDir * 3;
      }

      mg_iValueListOffset = Clamp(mg_iValueListOffset + iScroll, (INDEX)0, iLastOffset);
      return TRUE;
    }

    // Immitate scrollbar dragging on the initial button press
    if (ListDragScrollbar()) return TRUE;
  }

  // Select value
  if (pmb.Apply(TRUE)) {
    ListDeactivate(mg_iListValue);
    return TRUE;
  }

  // Cancel value selection
  if (pmb.Back(TRUE)) {
    ListDeactivate(-1);
    return TRUE;
  }

  return TRUE;
};

BOOL CMGVarButton::ListDragScrollbar(void) {
  const INDEX ctList = mg_pvsVar->vs_ctValues;

  // All values are already visible
  if (mg_ctListValuesAtOnce >= ctList) return FALSE;

  // Can't grab the scrollbar
  if (mg_vMouseScrollbarDrag == PIX2D(-1, -1)) return FALSE;

  const INDEX iLastOffset = (ctList - mg_ctListValuesAtOnce);

  // Move scrollbar to a specific position
  if (!mg_bMouseOnScrollbar) {
    const FLOAT fRatio = Clamp(FLOAT(_pixCursorPosJ - mg_boxList.Min()(2)) / mg_boxList.Size()(2), 0.0f, 1.0f);

    // Round to the nearest value
    mg_iValueListOffset = (INDEX)floorf((FLOAT)iLastOffset * fRatio + 0.5f);
    return TRUE;
  }

  // Drag the scrollbar relative to the last offset
  const PIX pixDelta = _pixCursorPosJ - mg_vMouseScrollbarDrag(2);
  const INDEX iWantedValue = mg_iLastValueListOffset + (ctList * pixDelta) / mg_boxList.Size()(2);

  mg_iValueListOffset = Clamp(iWantedValue, (INDEX)0, iLastOffset);
  return TRUE;
};

// [Cecil] Render value list
void CMGVarButton::ListRender(CDrawPort *pdp, PIX2D vListBox, CTString strValue) {
  mg_iListValue = -1;
  mg_vMouseInScrollbarArea = PIX2D(-1, -1);
  mg_boxScrollbarArea = PIXaabbox2D(PIX2D(0, 0), PIX2D(0, 0));

  extern CFontData _fdMedium;
  const PIX pixTextHeight = _fdMedium.GetHeight() * pdp->dp_fTextScaling;

  INDEX iList;
  const INDEX ctList = mg_pvsVar->vs_ctValues;

  const COLOR colCurrent = GetCurrentColor();
  const COLOR colListBox = _pGame->LCDGetColor(C_GREEN | 0xFF, "slider box");
  const COLOR colHighlighted = _pGame->LCDGetColor(C_WHITE | 0xFF, "hilited");
  const COLOR colUnselected = _pGame->LCDGetColor(C_GREEN, "unselected");

  // Figure out the longest string length
  const BOOL bCustom = mg_pvsVar->vs_bCustom;
  PIX pixBoxWidth = (bCustom ? IRender::GetTextWidth(pdp, strValue) : 0);

  for (iList = 0; iList < ctList; iList++) {
    PIX pixLen = IRender::GetTextWidth(pdp, mg_pvsVar->vs_astrTexts[iList]);

    if (pixLen > pixBoxWidth) {
      pixBoxWidth = pixLen;
    }
  }

  // Add space for a scrollbar
  const PIX pixScrollBar = 10 * HEIGHT_SCALING(pdp);
  const PIX pixBoxNoScrollBarWidth = pixBoxWidth;
  pixBoxWidth += pixScrollBar + 4 * HEIGHT_SCALING(pdp);

  // Create box for the value list
  const PIX pixNegateMargin = pixTextHeight * 0.15f;

  mg_boxList = PIXaabbox2D(vListBox, vListBox + PIX2D(pixBoxWidth, pixTextHeight - pixNegateMargin));
  mg_boxList.Expand(5);

  // Render current value within its own box
  _pGame->LCDDrawBox(1, 1, mg_boxList, colListBox);
  pdp->Fill(mg_boxList.Min()(1), mg_boxList.Min()(2), mg_boxList.Size()(1) + 1, mg_boxList.Size()(2) + 1, 0xBF);

  if (!bCustom) {
    strValue = mg_pvsVar->vs_astrTexts[mg_pvsVar->vs_iValue];
  }

  pdp->PutText(strValue, vListBox(1), vListBox(2), colHighlighted);

  // Shift box down and expand size for all values
  const PIX2D vListOffset(0, mg_boxList.Size()(2) * 1.2f);

  {
    // Start at one value
    INDEX ctValuesAtOnceDown = 1;
    INDEX ctValuesAtOnceUp = 1;

    // Count how many values fit upwards
    for (iList = 0; iList < ctList; iList++) {
      // Stop counting if the height is lower than the limit
      if (FLOAT(mg_boxList.Min()(2) - pixTextHeight * iList) / pdp->GetHeight() < 0.2f) {
        break;
      }

      ctValuesAtOnceUp = iList + 1;
    }

    // Count how many values fit downwards
    for (iList = 0; iList < ctList; iList++) {
      // Stop counting if the height is lower than the limit
      if (FLOAT(mg_boxList.Min()(2) + pixTextHeight * iList) / pdp->GetHeight() > 0.8f) {
        break;
      }

      ctValuesAtOnceDown = iList + 1;
    }

    // Shift value list down
    if (ctValuesAtOnceDown >= ctValuesAtOnceUp) {
      mg_ctListValuesAtOnce = ctValuesAtOnceDown;
      PIX pixValuesOffset = pixTextHeight * (mg_ctListValuesAtOnce - 1);

      vListBox += vListOffset;

      mg_boxList += vListOffset;
      mg_boxList.maxvect(2) += pixValuesOffset;

    // Shift value list up
    } else {
      mg_ctListValuesAtOnce = ctValuesAtOnceUp;
      PIX pixValuesOffset = pixTextHeight * (mg_ctListValuesAtOnce - 1);
      
      vListBox -= vListOffset;
      vListBox(2) -= pixValuesOffset;

      mg_boxList -= vListOffset;
      mg_boxList.minvect(2) -= pixValuesOffset;
    }
  }

  const PIX2D vCursorPos(_pixCursorPosI, _pixCursorPosJ);

  // Render all values in the list
  _pGame->LCDDrawBox(1, 1, mg_boxList, colListBox);
  pdp->Fill(mg_boxList.Min()(1), mg_boxList.Min()(2), mg_boxList.Size()(1) + 1, mg_boxList.Size()(2) + 1, 0xBF);

  for (iList = 0; iList < mg_ctListValuesAtOnce; iList++) {
    COLOR colList = colUnselected;
    const PIX pixListValueY = (vListBox(2) + pixTextHeight * iList);

    // Check if the mouse is hovering over the value
    const PIX2D vValueStart(vListBox(1), pixListValueY);
    const PIX2D vValueSize(pixBoxNoScrollBarWidth, pixTextHeight - 1);
    const PIXaabbox2D boxValue(vValueStart, vValueStart + vValueSize);

    const INDEX iActualValue = iList + mg_iValueListOffset;

    if (boxValue >= vCursorPos) {
      colList = colCurrent;

      // Play selection sound
      if (mg_iLastListValue != iActualValue) {
        mg_iLastListValue = iActualValue;
        PlayMenuSound(_psdSelect, FALSE);
      }

      mg_iListValue = iActualValue;

    } else if (!bCustom && iActualValue == mg_pvsVar->vs_iValue) {
      colList = colHighlighted;
    }

    pdp->PutText(mg_pvsVar->vs_astrTexts[iActualValue], vListBox(1), pixListValueY, colList);
  }

  // No value selected
  if (mg_iListValue == -1) {
    mg_iLastListValue = -1;
  }

  // Render scrollbar
  if (mg_ctListValuesAtOnce < ctList) {
    PIX2D vScrollStart(mg_boxList.Max()(1) - pixScrollBar, mg_boxList.Min()(2));
    PIX2D vScrollSize(pixScrollBar, mg_boxList.Size()(2));

    PIXaabbox2D boxScroll(vScrollStart, vScrollStart + vScrollSize);
    _pGame->LCDDrawBox(1, 1, boxScroll, colListBox);

    COLOR colScroll = colUnselected;

    vScrollStart += PIX2D(2, 2);
    vScrollSize  -= PIX2D(3, 3);

    // Adjust slider for the current values on screen
    const FLOAT fScrollH = (FLOAT)vScrollSize(2) / (FLOAT)ctList;
    vScrollStart(2) += fScrollH * mg_iValueListOffset;

    vScrollSize(2) *= (FLOAT)mg_ctListValuesAtOnce / (FLOAT)ctList;
    vScrollSize(2) = ClampDn(vScrollSize(2), (PIX)1);

    // Grab the scrollbar
    if (boxScroll >= vCursorPos) {
      mg_iListValue = -1;
      mg_vMouseInScrollbarArea = vCursorPos;
      mg_boxScrollbarArea = PIXaabbox2D(vScrollStart, vScrollStart + vScrollSize);
      colScroll = colCurrent;
    }

    pdp->Fill(vScrollStart(1), vScrollStart(2), vScrollSize(1), vScrollSize(2), colScroll);
  }
};
