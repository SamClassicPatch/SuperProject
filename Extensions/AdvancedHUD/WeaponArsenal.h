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

#ifndef CECIL_INCL_WEAPONARSENAL_H
#define CECIL_INCL_WEAPONARSENAL_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Ammo info
struct HudAmmo {
  struct SIconTexture *ptoAmmo;
  INDEX iAmmo;
  INDEX iMaxAmmo;
  INDEX iLastAmmo;
  FLOAT tmAmmoChanged;
  BOOL bHasWeapon;

  HudAmmo() : ptoAmmo(NULL),
    iAmmo(0), iMaxAmmo(0), iLastAmmo(0),
    tmAmmoChanged(-9), bHasWeapon(FALSE)
  {
  };

  inline void Setup(SIconTexture *ptoSet) {
    ptoAmmo = ptoSet;
  };
};

// Weapon info
struct HudWeapon {
  INDEX iWeapon;
  SIconTexture *ptoWeapon;
  HudAmmo *paiAmmo;
  BOOL bHasWeapon;

  HudWeapon() : iWeapon(0),
    ptoWeapon(NULL), paiAmmo(NULL), bHasWeapon(FALSE)
  {
  };

  inline void Setup(INDEX iSetWeapon, SIconTexture *ptoSet, HudAmmo *pSetAmmo = NULL) {
    iWeapon = iSetWeapon;
    ptoWeapon = ptoSet;
    paiAmmo = pSetAmmo;
  };
};

struct HudArsenal {
  // Stack types
  typedef CStaticStackArray<HudAmmo> AmmoStack;
  typedef CStaticStackArray<HudWeapon> WeaponStack;

  AmmoStack aAmmo;
  WeaponStack aWeapons;
};

#endif
