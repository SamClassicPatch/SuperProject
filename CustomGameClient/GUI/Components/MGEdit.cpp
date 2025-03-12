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
#include "MGEdit.h"

extern CSoundData *_psdPress;

CMGEdit::CMGEdit(void) {
  mg_pstrToChange = NULL;
  mg_ctMaxStringLen = 70;
  Clear();
}

void CMGEdit::Clear(void) {
  mg_iCursorPos = 0;
  mg_bEditing = FALSE;
  _eEditingValue = VED_NONE;
}

void CMGEdit::OnActivate(void) {
  if (!mg_bEnabled) {
    return;
  }
  ASSERT(mg_pstrToChange != NULL);
  PlayMenuSound(_psdPress);
  mg_iCursorPos = GetText().Length();
  mg_bEditing = TRUE;
  _eEditingValue = VED_STRING;
}

// focus lost
void CMGEdit::OnKillFocus(void) {
  // go out of editing mode
  if (mg_bEditing) {
    OnKeyDown(PressedMenuButton(VK_RETURN, -1));
    Clear();
  }
  // proceed
  CMenuGadget::OnKillFocus();
}

// [Cecil] Gadget should disappear
void CMGEdit::Disappear(void) {
  // Cancel string editing
  if (mg_bEditing) {
    OnKeyDown(PressedMenuButton(VK_RETURN, -1));
    Clear();
  }

  CMGButton::Disappear();
};

// helper function for deleting char(s) from string
static void Key_BackDel(CTString &str, INDEX &iPos, BOOL bShift, BOOL bRight) {
  // do nothing if string is empty
  INDEX ctChars = str.Length();

  if (ctChars == 0) {
    return;
  }

  if (bRight && iPos < ctChars) { // DELETE key
    if (bShift) {
      // delete to end of line
      str.TrimRight(iPos);
    } else {
      // delete only one char
      str.DeleteChar(iPos);
    }
  }
  if (!bRight && iPos > 0) { // BACKSPACE key
    if (bShift) {
      // delete to start of line
      str.TrimLeft(ctChars - iPos);
      iPos = 0;
    } else {
      // delete only one char
      str.DeleteChar(iPos - 1);
      iPos--;
    }
  }
}

// key/mouse button pressed
BOOL CMGEdit::OnKeyDown(PressedMenuButton pmb) {
  // if not in edit mode
  if (!mg_bEditing) {
    // [Cecil] Activate string editing inline
    if (pmb.Apply(TRUE)) {
      OnActivate();

      // [Cecil] Activate on-screen keyboard for controller input
      if (pmb.iCtrl != -1) {
        const PIXaabbox2D box = FloatBoxToPixBox(_pdpMenu, mg_boxOnScreen);
        GetSteamAPI()->ShowFloatingGamepadTextInput(box.Min()(1), box.Min()(2), box.Size()(1), box.Size()(2));
      }

      return TRUE;
    }

    return FALSE;
  }

  // [Cecil] Apply changes
  if (pmb.Apply(TRUE)) {
    *mg_pstrToChange = GetText();
    Clear();
    OnStringChanged();
    return TRUE;
  }

  // [Cecil] Discard changes
  if (pmb.Back(TRUE)) {
    SetText(*mg_pstrToChange);
    Clear();
    OnStringCanceled();
    return TRUE;
  }

  // [Cecil] Move left
  if (pmb.Up() || pmb.Left()) {
    if (mg_iCursorPos > 0) mg_iCursorPos--;
    return TRUE;
  }

  // [Cecil] Move right
  if (pmb.Down() || pmb.Right()) {
    if (mg_iCursorPos < GetText().Length()) mg_iCursorPos++;
    return TRUE;
  }

  // finish editing?
  BOOL bShift = GetKeyState(VK_SHIFT) & 0x8000;

  switch (pmb.iKey) {
    case VK_HOME: {
      mg_iCursorPos = 0;
    } break;

    case VK_END: {
      mg_iCursorPos = GetText().Length();
    } break;

    case VK_BACK: {
      CTString strText = GetText();
      Key_BackDel(strText, mg_iCursorPos, bShift, FALSE);

      SetText(strText);
    } break;

    case VK_DELETE: {
      CTString strText = GetText();
      Key_BackDel(strText, mg_iCursorPos, bShift, TRUE);

      SetText(strText);
    } break;
  }

  // Ignore all other special keys and mark them as handled
  return TRUE;
}

// char typed
BOOL CMGEdit::OnChar(MSG msg) {
  // if not in edit mode
  if (!mg_bEditing) {
    // behave like normal gadget
    return CMenuGadget::OnChar(msg);
  }
  // only chars are allowed
  CTString strText = GetText(); // [Cecil]
  const INDEX ctFullLen = strText.Length();
  const INDEX ctNakedLen = strText.LengthNaked();
  mg_iCursorPos = Clamp(mg_iCursorPos, 0L, ctFullLen);
  int iVKey = msg.wParam;
  if (isprint(iVKey) && ctNakedLen <= mg_ctMaxStringLen) {
    strText.InsertChar(mg_iCursorPos, (char)iVKey);
    SetText(strText);

    mg_iCursorPos++;
  }
  // key is handled
  return TRUE;
}

void CMGEdit::Render(CDrawPort *pdp) {
  if (mg_bEditing) {
    mg_iTextMode = -1;
  } else if (mg_bFocused) {
    mg_iTextMode = 0;
  } else {
    mg_iTextMode = 1;
  }

  if (GetText() == "" && !mg_bEditing) {
    // [Cecil] Don't hide empty string
    BOOL bHiddenState = mg_bHiddenText;
    mg_bHiddenText = FALSE;

    if (mg_bfsFontSize == BFS_SMALL) {
      SetText("*");
    } else {
      SetText(LOCALIZE("<none>"));
    }
    CMGButton::Render(pdp);

    SetText("");

    // [Cecil] Restore hidden state
    mg_bHiddenText = bHiddenState;

  } else {
    CMGButton::Render(pdp);
  }
}

void CMGEdit::OnStringChanged(void) {}

void CMGEdit::OnStringCanceled(void) {}