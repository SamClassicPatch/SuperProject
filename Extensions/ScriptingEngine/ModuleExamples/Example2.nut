/* ================================================================
 * Script module example 2: Hard mode
 * Compatibility: TFE (v1.05), TSE (v1.05, v1.07)
 *
 * Forces the strongest variant of each possible enemy to make the
 * game harder.
 * ================================================================ */

// Prevent enemies from falling through the floor because of the bigger size
function TeleportEnemy(enemy) {
  // Only if the enemy isn't a template
  if (!enemy.GetPropValueByName(EPT.BOOL, "Template")) {
    Entities.Teleport(enemy.GetID(), CPlacement3D(FLOAT3D(0, 0.5, 0), FLOAT3D(0, 0, 0)), true);
  }
};

function ReplaceEnemies() {
  // Not a server
  if (!Network.IsHost()) return;

  if (Network.IsVanillaCompatible()) {
    print("^cff0000Enemies may only be replaced in Classics Patch exclusive games and servers!\n");
    return;
  }

  // Find all entities in the world that are derived from CEnemyBase
  local enemies = World.FindClassesByID(310);
  local count = enemies.len();

  for (local i = 0; i < count; i++) {
    local enemy = enemies[i];
    local classID = enemy.GetClassID();

    switch (classID) {
      // CBeast - Replace Highlanders with Highlander's Brides (only in TSE)
      case 336: {
        if (!Core.IsTSE()) break;
        local char = enemy.GetPropValueByName(EPT.ENUM, "Character");

        if (char == 1) {
          Entities.SetProperty(enemy.GetID(), "Character", 2);
          Entities.Initialize(enemy.GetID());
        }
      } break;

      // CEyeman - Replace Soldiers with Sergeants
      case 323: {
        local char = enemy.GetPropValueByName(EPT.ENUM, "Character");

        if (char == 0) {
          Entities.SetProperty(enemy.GetID(), "Character", 1);
          Entities.Initialize(enemy.GetID());
          TeleportEnemy(enemy);
        }
      } break;

      // CGizmo - Make them half the regular size
      case 335: {
        local stretch = enemy.GetPropValueByName(EPT.FLOAT, "Stretch multiplier");

        if (stretch > 0.5) {
          Entities.SetProperty(enemy.GetID(), "Stretch multiplier", 0.5);
          Entities.Initialize(enemy.GetID());
        }
      } break;

      // CGrunt - Replace Soldiers with Commanders
      case 343: {
        local type = enemy.GetPropValueByName(EPT.ENUM, "Type");

        if (type == 0) {
          Entities.SetProperty(enemy.GetID(), "Type", 1);
          Entities.Initialize(enemy.GetID());
          TeleportEnemy(enemy);
        }
      } break;

      // CHeadman - Replace Rocketeers with Bombermen
      case 303: {
        local type = enemy.GetPropValueByName(EPT.ENUM, "Type");

        if (type == 1) {
          Entities.SetProperty(enemy.GetID(), "Type", 2);
          Entities.Initialize(enemy.GetID());
        }
      } break;

      // CScorpman - Replace Soldiers with Generals
      case 306: {
        local type = enemy.GetPropValueByName(EPT.ENUM, "Type");

        if (type == 0) {
          Entities.SetProperty(enemy.GetID(), "Type", 1);
          Entities.Initialize(enemy.GetID());
          TeleportEnemy(enemy);
        }
      } break;

      // CWalker - Replace Soldiers with Sergeants
      case 324: {
        local char = enemy.GetPropValueByName(EPT.ENUM, "Character");

        if (char == 0) {
          Entities.SetProperty(enemy.GetID(), "Character", 1);
          Entities.Initialize(enemy.GetID());
          TeleportEnemy(enemy);
        }
      } break;
    }
  }
};

// Replace enemies just once on game start and level change
function OnGameStart() {
  ReplaceEnemies();
};

function OnChangeLevel() {
  ReplaceEnemies();
};
