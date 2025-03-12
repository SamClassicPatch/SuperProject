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

#include "Sandbox.h"

// Define plugin
CLASSICSPATCH_DEFINE_PLUGIN(k_EPluginFlagGame | k_EPluginFlagServer, CORE_PATCH_VERSION,
  "Dreamy Cecil", "Server Utilities", "A collection of commands for enhanced world manipulation for hosting custom game servers.");

// Non-persistent and invisible mode switch symbol (-100 = ignore)
#define MSS CPluginSymbol(0, INDEX(-100))

// Server settings
CPluginSymbol _psActionButtons(0, INDEX(0xFFFFFFFF)); // Player action button mask
CPluginSymbol _psThirdPerson(0, INDEX(-1)); // Third person view button bit

// Difficulty settings
CPluginSymbol _psStartHP(SSF_PERSISTENT | SSF_USER, 100.0f);
CPluginSymbol _psStartAR(SSF_PERSISTENT | SSF_USER, 0.0f);
CPluginSymbol _psEnemyMul(SSF_PERSISTENT | SSF_USER, 1.0f);

// Weapon settings
CPluginSymbol _psMaxAmmo(SSF_PERSISTENT | SSF_USER, INDEX(FALSE));

// 0 = take away; 1 = give
CPluginSymbol _apsGiveWeapons[CT_WEAPONS] = {
  MSS, MSS, MSS, MSS, MSS, MSS, MSS, // Knife, Colt, DColt, SShotgun, DShotgun, Tommygun, Minigun
  MSS, MSS, MSS, MSS, MSS, MSS, MSS, // RLauncher, GLauncher, Chainsaw, Flamer, Laser, Sniper, Cannon
  MSS, MSS, MSS, MSS, MSS, MSS, MSS,
  MSS, MSS, MSS, MSS, MSS, MSS, MSS,
  MSS, MSS, MSS,
};

// 0+ = specific type
CPluginSymbol _apsWeaponItems[CT_WEAPONS] = {
  MSS, MSS, MSS, MSS, MSS, MSS, MSS, // <none>, Colt, SShotgun, DShotgun, Tommygun, Minigun, RLauncher
  MSS, MSS, MSS, MSS, MSS, MSS, MSS, // GLauncher, Sniper, Flamer, Laser, Chainsaw, Cannon, Ghostbuster
  MSS, MSS, MSS, MSS, MSS, MSS, MSS, // PlasmaThrower, MineLayer
  MSS, MSS, MSS, MSS, MSS, MSS, MSS,
  MSS, MSS, MSS,
};

// 0+ = specific type
CPluginSymbol _apsAmmoItems[CT_WEAPONS] = {
  MSS, MSS, MSS, MSS, MSS, MSS, MSS, // <none>, Shells, Bullets, Rockets, Grenades, Electricity, NukeBalls
  MSS, MSS, MSS, MSS, MSS, MSS, MSS, // IronBalls, SeriousPack, BackPack, Napalm, SnipBullets, Plasma, Mines
  MSS, MSS, MSS, MSS, MSS, MSS, MSS,
  MSS, MSS, MSS, MSS, MSS, MSS, MSS,
  MSS, MSS, MSS,
};

// 0+ = specific type
CPluginSymbol _apsHealthItems[CT_ITEMS] = {
  MSS, MSS, MSS, MSS, MSS, // Pill +1, Small +10, Medium +25, Large +50, Super +100
  MSS, MSS, MSS, MSS, MSS,
  MSS, MSS, MSS, MSS, MSS,
};

// 0+ = specific type
CPluginSymbol _apsArmorItems[CT_ITEMS] = {
  MSS, MSS, MSS, MSS, MSS, // Shard +1, Small +25, Medium +50, Strong +100, Super +200
  MSS, MSS, MSS, MSS, MSS, // Helm +5
  MSS, MSS, MSS, MSS, MSS,
};

// 0+ = specific type
CPluginSymbol _apsPowerUpItems[CT_ITEMS] = {
  MSS, MSS, MSS, MSS, MSS, // Invisibility, Invulnerability, Serious Damage, Serious Speed, Serious Bomb
  MSS, MSS, MSS, MSS, MSS,
  MSS, MSS, MSS, MSS, MSS,
};

// -1 = leave as is; 0+ = item index
CPluginSymbol _psReplaceWeapons(SSF_PERSISTENT | SSF_USER, INDEX(-1));
CPluginSymbol _psReplaceAmmo   (SSF_PERSISTENT | SSF_USER, INDEX(-1));
CPluginSymbol _psReplaceHealth (SSF_PERSISTENT | SSF_USER, INDEX(-1));
CPluginSymbol _psReplaceArmor  (SSF_PERSISTENT | SSF_USER, INDEX(-1));

// Module entry point
CLASSICSPATCH_PLUGIN_STARTUP(CIniConfig &props, PluginEvents_t &events)
{
  // Register plugin events
  events.m_game->OnGameStart   = &IGameEvents_OnGameStart;
  events.m_game->OnChangeLevel = &IGameEvents_OnChangeLevel;

  events.m_packet->OnCharacterConnect = &IPacketEvents_OnCharacterConnect;
  events.m_packet->OnCharacterChange  = &IPacketEvents_OnCharacterChange;
  events.m_packet->OnPlayerAction     = &IPacketEvents_OnPlayerAction;

  // Custom symbols
  {
    // Server settings
    _psActionButtons.Register("sutl_iActionButtons");
    _psThirdPerson.Register("sutl_iThirdPersonViewButton");

    // Difficulty settings
    _psStartHP.Register("sutl_fStartHealth");
    _psStartAR.Register("sutl_fStartArmor");
    _psEnemyMul.Register("sutl_fEnemyMultiplier");

    // Weapon settings
    _psMaxAmmo.Register("sutl_bMaxAmmo");

    CTString strCommand;

    for (INDEX iWeapon = 0; iWeapon < CT_WEAPONS; iWeapon++)
    {
      strCommand.PrintF("sutl_iGiveWeapon%d", iWeapon);
      _apsGiveWeapons[iWeapon].Register(strCommand);

      strCommand.PrintF("sutl_iWeaponType%d", iWeapon);
      _apsWeaponItems[iWeapon].Register(strCommand);

      strCommand.PrintF("sutl_iAmmoType%d", iWeapon);
      _apsAmmoItems[iWeapon].Register(strCommand);
    }

    // Item settings
    for (INDEX iItemType = 0; iItemType < CT_ITEMS; iItemType++)
    {
      strCommand.PrintF("sutl_iHealthType%d", iItemType);
      _apsHealthItems[iItemType].Register(strCommand);

      strCommand.PrintF("sutl_iArmorType%d", iItemType);
      _apsArmorItems[iItemType].Register(strCommand);

      strCommand.PrintF("sutl_iPowerUpType%d", iItemType);
      _apsPowerUpItems[iItemType].Register(strCommand);
    }

    _psReplaceWeapons.Register("sutl_iReplaceWeapons");
    _psReplaceAmmo.Register("sutl_iReplaceAmmo");
    _psReplaceHealth.Register("sutl_iReplaceHealth");
    _psReplaceArmor.Register("sutl_iReplaceArmor");

    // Server sandbox commands
    GetPluginAPI()->RegisterMethod(TRUE, "void", "sutl_ListScheduledCommands",  "void", &IServerSandbox::ListScheduledCommands);
    GetPluginAPI()->RegisterMethod(TRUE, "void", "sutl_ClearScheduledCommands", "void", &IServerSandbox::ClearScheduledCommands);

    GetPluginAPI()->RegisterMethod(TRUE, "void", "sutl_DeleteEntity",      "INDEX",                      &IServerSandbox::DeleteEntity);
    GetPluginAPI()->RegisterMethod(TRUE, "void", "sutl_InitEntity",        "INDEX",                      &IServerSandbox::InitEntity);
    GetPluginAPI()->RegisterMethod(TRUE, "void", "sutl_SetEntityPosition", "INDEX, FLOAT, FLOAT, FLOAT", &IServerSandbox::SetEntityPosition);
    GetPluginAPI()->RegisterMethod(TRUE, "void", "sutl_SetEntityRotation", "INDEX, FLOAT, FLOAT, FLOAT", &IServerSandbox::SetEntityRotation);
    GetPluginAPI()->RegisterMethod(TRUE, "void", "sutl_SetEntityProperty", "INDEX, CTString, CTString",  &IServerSandbox::SetEntityProperty);
    GetPluginAPI()->RegisterMethod(TRUE, "void", "sutl_ParentEntity",      "INDEX, INDEX",               &IServerSandbox::ParentEntity);
  }
};

// Module cleanup
CLASSICSPATCH_PLUGIN_SHUTDOWN(CIniConfig &props)
{
};
