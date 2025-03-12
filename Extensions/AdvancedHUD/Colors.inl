/* Copyright (c) 2023-2024 Dreamy Cecil
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

// Base colors
COLOR CHud::COL_Base(void) {
  // Custom color
  if (_psColorize.GetIndex()) {
    return _psColorBase.GetIndex() << 8;
  }

  return pColorSet->colBase;
};

COLOR CHud::COL_Icon(void) {
  // Custom color
  if (_psColorize.GetIndex()) {
    return _psColorIcon.GetIndex() << 8;
  }

  return pColorSet->colIcon;
};

COLOR CHud::COL_PlayerNames(void) {
  // Custom color
  if (_psColorize.GetIndex()) {
    return _psColorNames.GetIndex() << 8;
  }

  return pColorSet->colNames;
};

COLOR CHud::COL_SnoopingLight(void) {
  // Swap color channels for TFE and custom colors
  if (!_bTSEColors || _psColorize.GetIndex()) {
    UBYTE ubR, ubG, ubB;
    ColorToRGB(_colBorder, ubR, ubG, ubB);

    return RGBToColor(ubG, ubB, ubR);
  }

  // TSE colors
  return 0xEE9C0000;
};

COLOR CHud::COL_SnoopingDark(void) {
  // Shift for TFE and custom color
  if (!_bTSEColors || _psColorize.GetIndex()) {
    return (_colBorder >> 1) & 0x7F7F7F00;
  }

  // TSE colors
  return 0x9B4B0000;
};

COLOR CHud::COL_AmmoSelected(void) {
  // Custom color
  if (_psColorize.GetIndex()) {
    return _psColorSelect.GetIndex() << 8;
  }

  return pColorSet->colAmmoSelected;
};

COLOR CHud::COL_AmmoDepleted(void) {
  return pColorSet->colAmmoDepleted;
};

// Value colors
COLOR CHud::COL_ValueOverTop(void) {
  // Custom color
  if (_psColorize.GetIndex()) {
    return _psColorMax.GetIndex() << 8;
  }

  return pColorSet->colValueOverTop;
};

COLOR CHud::COL_ValueTop(void) {
  // Custom color
  if (_psColorize.GetIndex()) {
    return _psColorTop.GetIndex() << 8;
  }

  return pColorSet->colValueTop;
};

COLOR CHud::COL_ValueMid(void) {
  // Custom color
  if (_psColorize.GetIndex()) {
    return _psColorMid.GetIndex() << 8;
  }

  return pColorSet->colValueMid;
};

COLOR CHud::COL_ValueLow(void) {
  // Custom color
  if (_psColorize.GetIndex()) {
    return _psColorLow.GetIndex() << 8;
  }

  return pColorSet->colValueLow;
};

// Sniper scope
COLOR CHud::COL_ScopeMask(void) {
  // Custom color
  if (_psColorize.GetIndex()) {
    return _psColorBase.GetIndex() << 8;
  }

  return pColorSet->colScopeMask;
};

COLOR CHud::COL_ScopeDetails(void) {
  // Custom color
  if (_psColorize.GetIndex()) {
    return _psColorIcon.GetIndex() << 8;
  }

  return pColorSet->colScopeDetails;
};

COLOR CHud::COL_ScopeLedIdle(void) {
  return pColorSet->colScopeLedIdle;
};

COLOR CHud::COL_ScopeLedFire(void) {
  return pColorSet->colScopeLedFire;
};

// Weapon selection list
COLOR CHud::COL_WeaponBorder(void) {
  // Custom color
  if (_psColorize.GetIndex()) {
    return _psColorBase.GetIndex() << 8;
  }

  return pColorSet->colWeaponBorder;
};

COLOR CHud::COL_WeaponIcon(void) {
  // Custom color
  if (_psColorize.GetIndex()) {
    return _psColorWeapon.GetIndex() << 8;
  }

  return pColorSet->colWeaponIcon;
};

COLOR CHud::COL_WeaponNoAmmo(void) {
  return pColorSet->colWeaponNoAmmo;
};

COLOR CHud::COL_WeaponWanted(void) {
  // Custom color
  if (_psColorize.GetIndex()) {
    return _psColorSelect.GetIndex() << 8;
  }

  return pColorSet->colWeaponWanted;
};
