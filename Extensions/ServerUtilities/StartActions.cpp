/* Copyright (c) 2022-2024 Dreamy Cecil
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

#include <CoreLib/Networking/ExtPackets.h>

// Notify about not being able to retrieve some entity property
static void ReportPropError(CEntity *pen, const char *strPropertyName) {
  CPrintF(TRANS("'%s' (%u) : Cannot retrieve '%s' property!\n"), pen->GetName(), pen->en_ulID, strPropertyName);
};

// Verify item type and reset it to default value if it's invalid
static INDEX VerifyItemType(CEntity *pen, const CPropertyPtr &pptrType, INDEX iType) {
  CTString strType = pptrType._pep->ep_pepetEnumType->NameForValue(iType);

  // Set default valid type if invalid
  if (strType == "") {
    CPrintF(TRANS("'%s' (%u) : Item type %d is invalid!\n"), pen->GetName(), pen->en_ulID, iType);
    return 1;
  }

  return iType;
};

// Destroy some entity
static inline void DestroyEntity(CEntity *pen) {
#if _PATCHCONFIG_EXT_PACKETS
  // Send packet to destroy the entity
  CExtEntityDelete pck;
  pck("ulEntity", (int)pen->en_ulID);
  pck("bSameClass", false);
  pck.SendToClients();

#else
  // Destroy entity immediately
  pen->Destroy();
#endif
};

// Reinitialize some entity
static inline void ReinitEntity(CEntity *pen) {
#if _PATCHCONFIG_EXT_PACKETS
  // Send packet to reinitialize the entity
  CExtEntityInit pck;
  pck("ulEntity", (int)pen->en_ulID);
  pck.SetEvent(EVoid(), sizeof(EVoid));
  pck.SendToClients();

#else
  // Reinitialize entity immediately
  pen->Reinitialize();
#endif
};

// Change property of some entity
static inline void ChangeEntityProp(CEntity *pen, CPropertyPtr &pptr, DOUBLE fValue) {
#if _PATCHCONFIG_EXT_PACKETS
  // Send packet to change the property
  CExtEntityProp pck;
  pck("ulEntity", (int)pen->en_ulID);
  pck.SetProperty(pptr._pep->ep_ulID);
  pck.SetValue(fValue);
  pck.SendToClients();

#else
  // Change values of float and index properties
  INDEX iType = IProperties::ConvertType(pptr._pep->ep_eptType);

  if (iType == CEntityProperty::EPT_FLOAT) {
    FLOAT fFloatProp = fValue;
    IProperties::SetPropValue(pen, pptr._pep, &fFloatProp);

  } else if (iType == CEntityProperty::EPT_INDEX) {
    INDEX iIntProp = fValue;
    IProperties::SetPropValue(pen, pptr._pep, &iIntProp);
  }
#endif
};

// Affect weapon item at the beginning of the game
void AffectWeaponItem(CEntity *pen) {
  // Retrieve CWeaponItem::m_EwitType
  static CPropertyPtr pptr(pen);

  if (!pptr.ByVariable("CWeaponItem", "m_EwitType")) {
    ReportPropError(pen, "CWeaponItem::m_EwitType");
    return;
  }

  INDEX iCurrType = ENTITYPROPERTY(pen, pptr.Offset(), INDEX);
  INDEX iSetType = _apsWeaponItems[iCurrType].GetIndex();

  // Remove weapon item
  if (iSetType == -1) {
    DestroyEntity(pen);

  // Set specific type
  } else if (iSetType >= 0) {
    iCurrType = VerifyItemType(pen, pptr, iSetType);

    ChangeEntityProp(pen, pptr, iCurrType);
    ReinitEntity(pen);

  // Replace with another weapon
  } else if (_psReplaceWeapons.GetIndex() >= 0) {
    iCurrType = VerifyItemType(pen, pptr, _psReplaceWeapons.GetIndex());

    ChangeEntityProp(pen, pptr, iCurrType);
    ReinitEntity(pen);
  }
};

// Affect ammo item at the beginning of the game
void AffectAmmoItem(CEntity *pen) {
  // Retrieve CAmmoItem::m_EaitType
  static CPropertyPtr pptr(pen);

  if (!pptr.ByVariable("CAmmoItem", "m_EaitType")) {
    ReportPropError(pen, "CAmmoItem::m_EaitType");
    return;
  }

  INDEX iCurrType = ENTITYPROPERTY(pen, pptr.Offset(), INDEX);
  INDEX iSetType = _apsAmmoItems[iCurrType].GetIndex();

  // Remove ammo item
  if (iSetType == -1) {
    DestroyEntity(pen);

  // Set specific type
  } else if (iSetType >= 0) {
    iCurrType = VerifyItemType(pen, pptr, iSetType);

    ChangeEntityProp(pen, pptr, iCurrType);
    ReinitEntity(pen);

  // Replace with another weapon
  } else if (_psReplaceAmmo.GetIndex() >= 0) {
    iCurrType = VerifyItemType(pen, pptr, _psReplaceAmmo.GetIndex());

    ChangeEntityProp(pen, pptr, iCurrType);
    ReinitEntity(pen);
  }
};

// Affect health item at the beginning of the game
void AffectHealthItem(CEntity *pen) {
  // Retrieve CHealthItem::m_EhitType
  static CPropertyPtr pptr(pen);

  if (!pptr.ByVariable("CHealthItem", "m_EhitType")) {
    ReportPropError(pen, "CHealthItem::m_EhitType");
    return;
  }

  INDEX iCurrType = ENTITYPROPERTY(pen, pptr.Offset(), INDEX);
  INDEX iSetType = _apsHealthItems[iCurrType].GetIndex();

  // Remove health item
  if (iSetType == -1) {
    DestroyEntity(pen);

  // Set specific type
  } else if (iSetType >= 0) {
    iCurrType = VerifyItemType(pen, pptr, iSetType);

    ChangeEntityProp(pen, pptr, iCurrType);
    ReinitEntity(pen);

  // Replace with another health item
  } else if (_psReplaceHealth.GetIndex() >= 0) {
    iCurrType = VerifyItemType(pen, pptr, _psReplaceHealth.GetIndex());

    ChangeEntityProp(pen, pptr, iCurrType);
    ReinitEntity(pen);
  }
};

// Affect armor item at the beginning of the game
void AffectArmorItem(CEntity *pen) {
  // Retrieve CArmorItem::m_EaitType
  static CPropertyPtr pptr(pen);

  if (!pptr.ByVariable("CArmorItem", "m_EaitType")) {
    ReportPropError(pen, "CArmorItem::m_EaitType");
    return;
  }

  INDEX iCurrType = ENTITYPROPERTY(pen, pptr.Offset(), INDEX);
  INDEX iSetType = _apsArmorItems[iCurrType].GetIndex();

  // Remove armor item
  if (iSetType == -1) {
    DestroyEntity(pen);

  // Set specific type
  } else if (iSetType >= 0) {
    iCurrType = VerifyItemType(pen, pptr, iSetType);

    ChangeEntityProp(pen, pptr, iCurrType);
    ReinitEntity(pen);

  // Replace with another armor item
  } else if (_psReplaceArmor.GetIndex() >= 0) {
    iCurrType = VerifyItemType(pen, pptr, _psReplaceArmor.GetIndex());

    ChangeEntityProp(pen, pptr, iCurrType);
    ReinitEntity(pen);
  }
};

// Affect power up item at the beginning of the game
void AffectPowerUpItem(CEntity *pen) {
  // Retrieve CPowerUpItem::m_puitType
  static CPropertyPtr pptr(pen);

  if (!pptr.ByVariable("CPowerUpItem", "m_puitType")) {
    ReportPropError(pen, "CPowerUpItem::m_puitType");
    return;
  }

  INDEX iCurrType = ENTITYPROPERTY(pen, pptr.Offset(), INDEX);
  INDEX iSetType = _apsPowerUpItems[iCurrType].GetIndex();

  // Remove power up
  if (iSetType == -1) {
    DestroyEntity(pen);

  // Set specific type
  } else if (iSetType >= 0) {
    iCurrType = VerifyItemType(pen, pptr, iSetType);

    ChangeEntityProp(pen, pptr, iCurrType);
    ReinitEntity(pen);
  }
};

// Affect player start marker at the beginning of the game
void AffectPlayerMarker(CEntity *pen) {
  // Retrieve CPlayerMarker::m_iGiveWeapons
  static CPropertyPtr pptrGive(pen);

  if (!pptrGive.ByVariable("CPlayerMarker", "m_iGiveWeapons")) {
    ReportPropError(pen, "CPlayerMarker::m_iGiveWeapons");

  } else {
    INDEX iSet = ENTITYPROPERTY(pen, pptrGive.Offset(), INDEX);

    // Give out specific weapons
    for (INDEX iWeapon = 0; iWeapon < CT_WEAPONS; iWeapon++) {
      INDEX iGive = _apsGiveWeapons[iWeapon].GetIndex();

      if (iGive == 1) {
        iSet |= (1 << iWeapon); // Give the weapon
      } else if (iGive == 0) {
        iSet &= ~(1 << iWeapon); // Take away the weapon
      }
    }

    ChangeEntityProp(pen, pptrGive, iSet);
  }

  // Retrieve CPlayerMarker::m_fMaxAmmoRatio
  static CPropertyPtr pptrMaxAmmo(pen);

  if (!pptrMaxAmmo.ByVariable("CPlayerMarker", "m_fMaxAmmoRatio")) {
    ReportPropError(pen, "CPlayerMarker::m_fMaxAmmoRatio");

  // Set maximum ammo
  } else if (_psMaxAmmo.GetIndex() != 0) {
    ChangeEntityProp(pen, pptrMaxAmmo, 1.0f);
  }

  // Retrieve CPlayerMarker::m_iTakeAmmo
  static CPropertyPtr pptrTakeAmmo(pen);

  if (!pptrTakeAmmo.ByVariable("CPlayerMarker", "m_iTakeAmmo")) {
    ReportPropError(pen, "CPlayerMarker::m_iTakeAmmo");

  // Allow maximum ammo for all available weapons
  } else if (_psMaxAmmo.GetIndex() != 0) {
    ChangeEntityProp(pen, pptrTakeAmmo, 0);
  }

  // Retrieve CPlayerMarker::m_fHealth
  static CPropertyPtr pptrHealth(pen);

  if (!pptrHealth.ByVariable("CPlayerMarker", "m_fHealth")) {
    ReportPropError(pen, "CPlayerMarker::m_fHealth");

  // Set custom health
  } else if (_psStartHP.GetFloat() != 100.0f) {
    ChangeEntityProp(pen, pptrHealth, _psStartHP.GetFloat());
  }

  // Retrieve CPlayerMarker::m_fShield
  static CPropertyPtr pptrArmor(pen);

  if (!pptrArmor.ByVariable("CPlayerMarker", "m_fShield")) {
    ReportPropError(pen, "CPlayerMarker::m_fShield");

  // Set custom armor
  } else if (_psStartAR.GetFloat() != 0.0f) {
    ChangeEntityProp(pen, pptrArmor, _psStartAR.GetFloat());
  }
};

// Affect enemy spawner at the beginning of the game
void AffectEnemySpawner(CEntity *pen) {
  const FLOAT fEnemyMul = _psEnemyMul.GetFloat();

  // No multiplier set
  if (fEnemyMul <= 1.0f) return;

  // Retrieve CEnemySpawner::m_ctTotal
  static CPropertyPtr pptrTotal(pen);

  if (!pptrTotal.ByVariable("CEnemySpawner", "m_ctTotal")) {
    ReportPropError(pen, "CEnemySpawner::m_ctTotal");

  // Multiply total amount
  } else {
    INDEX &iCount = ENTITYPROPERTY(pen, pptrTotal.Offset(), INDEX);
    ChangeEntityProp(pen, pptrTotal, ClampDn(INDEX(iCount * fEnemyMul), (INDEX)1));
  }

  // Retrieve CEnemySpawner::m_ctGroupSize
  static CPropertyPtr pptrGroup(pen);

  if (!pptrGroup.ByVariable("CEnemySpawner", "m_ctGroupSize")) {
    ReportPropError(pen, "CEnemySpawner::m_ctGroupSize");

  // Multiply group size
  } else {
    INDEX &iGroup = ENTITYPROPERTY(pen, pptrGroup.Offset(), INDEX);
    ChangeEntityProp(pen, pptrGroup, ClampDn(INDEX(iGroup * fEnemyMul), (INDEX)1));
  }

  // Retrieve CEnemySpawner::m_tmSingleWait
  static CPropertyPtr pptrDelaySingle(pen);

  if (!pptrDelaySingle.ByVariable("CEnemySpawner", "m_tmSingleWait")) {
    ReportPropError(pen, "CEnemySpawner::m_tmSingleWait");

  // Decrease delay between single enemies
  } else {
    FLOAT &fTime = ENTITYPROPERTY(pen, pptrDelaySingle.Offset(), FLOAT);
    ChangeEntityProp(pen, pptrDelaySingle, ClampDn(fTime / fEnemyMul, 0.05f));
  }

  // Retrieve CEnemySpawner::m_tmGroupWait
  static CPropertyPtr pptrDelayGroup(pen);

  if (!pptrDelayGroup.ByVariable("CEnemySpawner", "m_tmGroupWait")) {
    ReportPropError(pen, "CEnemySpawner::m_tmGroupWait");

  // Decrease delay between groups of enemies
  } else {
    FLOAT &fTime = ENTITYPROPERTY(pen, pptrDelayGroup.Offset(), FLOAT);
    ChangeEntityProp(pen, pptrDelayGroup, ClampDn(fTime / fEnemyMul, 0.05f));
  }
};
