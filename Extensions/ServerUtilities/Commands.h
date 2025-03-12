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

// Maximum amount of weapons in the game
#define CT_WEAPONS INDEX(31)

// Maximum amount of items in the game
#define CT_ITEMS INDEX(15)

// Server settings
extern CPluginSymbol _psActionButtons;
extern CPluginSymbol _psThirdPerson;

// Difficulty settings
extern CPluginSymbol _psStartHP;
extern CPluginSymbol _psStartAR;
extern CPluginSymbol _psEnemyMul;

// Weapon settings
extern CPluginSymbol _psMaxAmmo;
extern CPluginSymbol _apsGiveWeapons[CT_WEAPONS];

// Item settings
extern CPluginSymbol _apsWeaponItems[CT_WEAPONS];
extern CPluginSymbol _apsAmmoItems[CT_WEAPONS];
extern CPluginSymbol _apsHealthItems[CT_ITEMS];
extern CPluginSymbol _apsArmorItems[CT_ITEMS];
extern CPluginSymbol _apsPowerUpItems[CT_ITEMS];
extern CPluginSymbol _psReplaceWeapons;
extern CPluginSymbol _psReplaceAmmo;
extern CPluginSymbol _psReplaceHealth;
extern CPluginSymbol _psReplaceArmor;
