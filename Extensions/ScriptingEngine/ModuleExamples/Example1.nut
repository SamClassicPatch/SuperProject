/* ================================================================
 * Script module example 1: Powered-up weapons
 * Compatibility: TSE (v1.05, v1.07)
 *
 * When a player picks up an Invulnerability power-up, their cannon
 * balls turn into unused "nuke balls" that are much more powerful
 * than regular iron balls.
 *
 * When a player picks up a Serious Damage power-up, their
 * flamethrower starts shooting fireballs of a cut Dragonman enemy,
 * turning the flamethrower into a long-range projectile weapon.
 *
 * When a player picks up a Serious Speed power-up, their chainsaw
 * deals high amount of additional damage, which insta-kills most
 * enemies at the cost of having to get close and personal.
 *
 * Adds a new console command "test_bRequirePowerUps" that toggles
 * whether the players need the respective power-ups to be active
 * in order to use new powered-up weapons.
 * ================================================================ */

// Register a new console command for toggling requirement of active power-ups
if (!Shell.SymbolExists("test_bRequirePowerUps")) {
  Shell.Execute("extern user INDEX test_bRequirePowerUps = 1;");
}

function OnGameStart() {
  if (Network.IsVanillaCompatible()) {
    print("^cff0000Powered-up weapons can only work in Classics Patch exclusive games and servers!\n");
    return;
  }
};

const CPlayer_ClassID = 401;

function OnStep() {
  // Cannot execute if not in any world or not currently running on a patch-exclusive server
  if (!World.IsActive() || !Network.IsHost() || Network.IsVanillaCompatible()) return;

  // Find all CCannonBall entities in the world
  local cannonballs = World.FindClassesByID(506);

  for (local i = 0; i < cannonballs.len(); i++) {
    local ball = cannonballs[i];

    // Skip the ones that are already nuke balls
    local ballType = ball.GetPropValueByID(EPT.ENUM, (506 << 8) + 30);
    if (ballType == 1) continue;

    // Find the ball launcher and make sure they exist
    local launcher = ball.GetPropValueByVariable("CCannonBall", "m_penLauncher");
    if (launcher == null || launcher.Equal(null)) continue;

    // Make sure the cannonball launcher is a CPlayer entity
    if (!launcher.IsOfClassID(CPlayer_ClassID)) continue;

    // If requiring power-ups
    if (Shell.GetIndex("test_bRequirePowerUps")) {
      // Get player's current invulnerability time and see how much is left
      local invul = launcher.GetPropValueByVariable("CPlayer", "m_tmInvulnerability");
      local timeLeft = (invul != null) ? invul - Timer.CurrentTick() : -1000.0;

      // Make sure invulnerability is active
      if (timeLeft <= 0) continue;
    }

    local ballSpeed = ball.GetPropValueByVariable("CCannonBall", "m_fLaunchPower");
    local ballSize = ball.GetPropValueByVariable("CCannonBall", "m_fCannonBallSize");

    // Reinitialize the cannonball as a "nuke ball" with half the speed of a regular cannonball by cancelling out half of it
    local ballEvent = CEntityEvent(EVENTCODE.ELaunchCannonBall);
    ballEvent.SetArgs(launcher, 1, -ballSpeed/2, ballSize);
    Entities.Initialize(ball.GetID(), ballEvent);
  }

  // Find all CProjectile entities in the world
  local projectiles = World.FindClassesByID(501);

  for (local i = 0; i < projectiles.len(); i++) {
    local proj = projectiles[i];

    // Skip the ones that aren't flames or are already dragonman fireballs
    local projType = proj.GetPropValueByVariable("CProjectile", "m_prtType");
    if (projType != 2 || projType == 17) continue;

    // Find the projectile launcher and make sure they exist
    local launcher = proj.GetPropValueByVariable("CProjectile", "m_penLauncher");
    if (launcher == null || launcher.Equal(null)) continue;

    // Make sure the projectile launcher is a CPlayer entity
    if (!launcher.IsOfClassID(CPlayer_ClassID)) continue;

    // If requiring power-ups
    if (Shell.GetIndex("test_bRequirePowerUps")) {
      // Get player's current serious damage time and see how much is left
      local damage = launcher.GetPropValueByVariable("CPlayer", "m_tmSeriousDamage");
      local timeLeft = (damage != null) ? damage - Timer.CurrentTick() : -1000.0;

      // Make sure serious damage is active
      if (timeLeft <= 0) continue;
    }

    local projSpeed = proj.GetPropValueByVariable("CProjectile", "m_fSpeed");
    local projSize = proj.GetPropValueByVariable("CProjectile", "m_fStretch");

    // Reinitialize the projectile as a dragonman fireball
    local projEvent = CEntityEvent(EVENTCODE.ELaunchProjectile);
    projEvent.SetArgs(launcher, 17, projSpeed, projSize);
    Entities.Initialize(proj.GetID(), projEvent);
  }
};

weaponsProp <- Entities.Property(); // CPlayer::m_penWeapons property info
currentProp <- Entities.Property(); // CPlayerWeapons::m_iCurrentWeapon property info

function DeterminePlayerWeapon(player) {
  // Determine PlayerWeapons property relative to the Player class
  if (weaponsProp.Equal(null)) {
    weaponsProp = player.GetPropInfoByVariable("CPlayer", "m_penWeapons");
    if (weaponsProp.Equal(null)) return -1;
  }

  local entPlayerWeapons = weaponsProp.GetValue(player);
  if (entPlayerWeapons.Equal(null)) return -1;

  // Determine current weapon property relative to the PlayerWeapons class
  if (currentProp.Equal(null)) {
    currentProp = entPlayerWeapons.GetPropInfoByVariable("CPlayerWeapons", "m_iCurrentWeapon");
    if (currentProp.Equal(null)) return -1;
  }

  return currentProp.GetValue(entPlayerWeapons);
};

function OnDamage(caller, target, inflictor, damageType, damage, hitPoint, direction) {
  // Cannot execute if not in any world or not currently running on a patch-exclusive server
  if (!World.IsActive() || !Network.IsHost() || Network.IsVanillaCompatible()) return;

  // Make sure it's chainsaw damage
  if (damageType != DMT.CHAINSAW) return;

  // Make sure it was a player entity that dealt it
  local player = inflictor;
  if (player.Equal(null) || !player.IsOfClassID(CPlayer_ClassID)) return;

  // If requiring power-ups
  if (Shell.GetIndex("test_bRequirePowerUps")) {
    // Get player's current serious speed time and see how much is left
    local speed = player.GetPropValueByVariable("CPlayer", "m_tmSeriousSpeed");
    local timeLeft = (speed != null) ? speed - Timer.CurrentTick() : -1000.0;

    // Make sure serious speed is active
    if (timeLeft <= 0) return;
  }

  // If player's weapon was determined to be a chainsaw (WEAPON_CHAINSAW = 10)
  if (DeterminePlayerWeapon(player) == 10) {
    // Deal additional explosive damage (because it's crazy and also to avoid recursion)
    local dir = (target.GetPlacement().pos - player.GetPlacement().pos).Normalize();
    Entities.InflictDirectDamage(player.GetID(), target.GetID(), DMT.EXPLOSION, 150, target.GetPlacement().pos, dir);
  }
};
