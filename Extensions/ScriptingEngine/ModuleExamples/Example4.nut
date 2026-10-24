/* ================================================================
 * Script module example 4: Kill feed via chat messages
 * Compatibility: TSE (v1.05, v1.07)
 *
 * Any time an enemy or a player is killed, a message is displayed
 * in chat about which player killed which enemy and with which
 * weapon and plays an additional "photo snapping" sound.
 * During demo playback, this is realized by locally printing out
 * messages in the console with a separate chat sound to simulate
 * chat message output, since it's not recorded with the demos.
 *
 * Due to entity limitations, it's impossible to tell which weapon
 * has been used to kill an enemy with explosion/projectile damage,
 * so a simple caching mechanism has been put in place that remembers
 * last selected explosive/projectile-based weapon and reports its
 * name if the enemy has been killed with that type of damage, even
 * if the weapon has since been changed.
 * But this may still be unreliable in cases when a weapon of some
 * type is switched to another weapon of the same type (e.g. from
 * Rocket Launcher to Grenade Launcher, both of which are explosive),
 * which will report the name of the current weapon instead of the
 * one that launched the killing projectile.
 * ================================================================ */

// Weapon types from The Second Encounter (mirroring WeaponType enum from PlayerWeapons)
enum WEAPON {
  KNIFE = 1,
  COLT = 2,
  DOUBLECOLT = 3,
  SINGLESHOTGUN = 4,
  DOUBLESHOTGUN = 5,
  TOMMYGUN = 6,
  MINIGUN = 7,
  ROCKETLAUNCHER = 8,
  GRENADELAUNCHER = 9,
  CHAINSAW = 10,
  FLAMER = 11,
  LASER = 12,
  SNIPER = 13,
  IRONCANNON = 14,
};

currentWeapon <- {}; // Currently selected weapon per player entity
lastExplosion <- {}; // Last selected explosive weapon per player entity
lastProjectile <- {}; // Last selected projectile-based weapon per player entity

weaponsProp <- Entities.Property(); // CPlayer::m_penWeapons property info
currentProp <- Entities.Property(); // CPlayerWeapons::m_iCurrentWeapon property info

// Clear cached weapons upon starting new world simulation
function OnGameStart() {
  currentWeapon = {};
  lastExplosion = {};
  lastProjectile = {};
};

function OnGameLoad(file) {
  currentWeapon = {};
  lastExplosion = {};
  lastProjectile = {};
};

function OnDemoPlay(file) {
  currentWeapon = {};
  lastExplosion = {};
  lastProjectile = {};
};

// Convert weapon type to its name
function WeaponToName(weapon) {
  switch (weapon) {
    case WEAPON.KNIFE:           return "Knife";
    case WEAPON.COLT:            return "Colt";
    case WEAPON.DOUBLECOLT:      return "Double Colts";
    case WEAPON.SINGLESHOTGUN:   return "Single Shotgun";
    case WEAPON.DOUBLESHOTGUN:   return "Double Shotgun";
    case WEAPON.TOMMYGUN:        return "Tommygun";
    case WEAPON.MINIGUN:         return "Minigun";
    case WEAPON.ROCKETLAUNCHER:  return "Rocket Launcher";
    case WEAPON.GRENADELAUNCHER: return "Grenade Launcher";
    case WEAPON.CHAINSAW:        return "Chainsaw";
    case WEAPON.FLAMER:          return "Flamethrower";
    case WEAPON.LASER:           return "Laser Rifle";
    case WEAPON.SNIPER:          return "Sniper Rifle";
    case WEAPON.IRONCANNON:      return "Cannon";
  }

  return "???";
};

// Convert damage type to a weapon of the killer entity
function WeaponForDamage(killer, damageType) {
  switch (damageType) {
    case DMT.EXPLOSION: {
      // Use name of the last cached explosive weapon, if it has been cached for this entity
      if (killer.GetID() in lastExplosion) {
        return lastExplosion[killer.GetID()];
      }
    } break;

    case DMT.PROJECTILE: {
      // Use name of the last cached projectile weapon, if it has been cached for this entity
      if (killer.GetID() in lastProjectile) {
        return lastProjectile[killer.GetID()];
      }
    } break;

    case DMT.BURNING: return WEAPON.FLAMER;
    case DMT.CANNONBALL: return WEAPON.IRONCANNON;
    case DMT.CANNONBALL_EXPLOSION: return WEAPON.IRONCANNON;
  }

  // Use name of the current weapon of this entity
  if (killer.GetID() in currentWeapon) {
    return currentWeapon[killer.GetID()];
  }

  return -1; // Unknown weapon
};

function OnStep() {
  // Quit if not in a world or currently playing on a remote server
  if (!World.IsActive() || Network.IsPlayingOnline()) return;

  // Go through every player entity in the world
  for (local i = 0; i < World.GetMaxPlayers(); i++) {
    // Get current player entity
    local player = World.GetPlayerEntity(i);
    if (player.Equal(null)) continue;

    // Determine PlayerWeapons property relative to the Player class
    if (weaponsProp.Equal(null)) {
      weaponsProp = player.GetPropInfoByVariable("CPlayer", "m_penWeapons");
    }

    // Determine current weapon property relative to the PlayerWeapons class
    if (!weaponsProp.Equal(null) && currentProp.Equal(null)) {
      local entPlayerWeapons = weaponsProp.GetValue(player);

      if (!entPlayerWeapons.Equal(null)) {
        currentProp = entPlayerWeapons.GetPropInfoByVariable("CPlayerWeapons", "m_iCurrentWeapon");
      }
    }

    // Cache last selected weapons of specific types based on the current weapon property
    if (!currentProp.Equal(null)) {
      local entPlayerWeapons = weaponsProp.GetValue(player);
      if (entPlayerWeapons.Equal(null)) continue;

      local weapon = currentProp.GetValue(entPlayerWeapons);
      local entityID = player.GetID();
      currentWeapon[entityID] <- weapon;

      switch (weapon) {
        case WEAPON.ROCKETLAUNCHER:
        case WEAPON.GRENADELAUNCHER:
          lastExplosion[entityID] <- weapon;
          lastProjectile[entityID] <- weapon;
          break;

        case WEAPON.LASER:
          lastProjectile[entityID] <- weapon;
          break;
      }
    }
  }
};

function LogKill(killer, died, damageType) {
  // Killer must be a player entity
  if (killer.Equal(null) || !killer.IsOfClass("Player")) return;

  local weapon = WeaponForDamage(killer, damageType);
  local message = "^cff7f7f" + killer.GetName() + "^r used ^cffbf00^b" + WeaponToName(weapon) + "^r to kill ^cff7f7f" + died.GetName() + "^r!";

  // Report kill in the console
  if (Network.IsHost()) {
    if (!Network.IsVanillaCompatible()) {
      local snd = Network.Sound("ModelsMP\\Weapons\\Sniper\\Sounds\\QuickZoom.wav", 1, 2);
      snd.Play();
    }

    Network.SendChatToEveryone("^a00", "^r>>>>> " + message);

  } else {
    print("  >>>>> " + message + "\n");

    // Increase chat message counter to play the chat sound
    Shell.SetIndex("net_ctChatMessages", Shell.GetIndex("net_ctChatMessages") + 1);
  }
};

function OnReceiveGenericEvent(en, event, args) {
  // If a player or an enemy has died
  if (en.IsOfClass("Player") || en.IsDerivedFromClass("Enemy Base")) {
    if (event == EVENTCODE.EDeath) {
      // Log the death based on the last entity that dealt the damage and the received damage type
      LogKill(args[0], en, args[4]);
    }
  }
};
