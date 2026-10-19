/* ================================================================
 * Script module example 5: Server-side enemy spawn menu
 * Compatibility: TSE (v1.05, v1.07)
 *
 * Displays a toggleable spawn menu on the left side of the screen
 * that the host client can interact with in order to spawn enemies
 * at their crosshair position, which is then synchronized with
 * every other client on the server.
 *
 * Press TAB to toggle the spawn menu and use the number row to
 * spawn the enemies and switch between menu pages.
 * ================================================================ */

// Class for handling key states
class ActionKey {
  held = false;
  pressed = false;
  released = false;

  // Update the key to determine all of its states for the current step
  function Update(key) {
    local state = Input.GetButtonState(key);
    pressed = state && !held;
    released = !state && held;
    held = state;
  };
};

// Menu toggling key
menuKey <- ActionKey();

// Ten keys of the number row
spawnKeys <- [
  ActionKey(), ActionKey(), ActionKey(), ActionKey(), ActionKey(),
  ActionKey(), ActionKey(), ActionKey(), ActionKey(), ActionKey(),
];

// Spawn menu
menu <- false;
page <- 0;

// Crosshair position variables
view <- CPlacement3D();
hit <- FLOAT3D();

// Prepare text font
fnt <- CFontData("Fonts\\Display3-narrow.fnt");

// Prepare class files
classes <- [
  "Classes\\Beast.ecl",
  "Classes\\Boneman.ecl",
  "Classes\\CannonRotating.ecl",
  "Classes\\CannonStatic.ecl",
  "Classes\\ChainsawFreak.ecl",
  "Classes\\Demon.ecl",
  "Classes\\Elemental.ecl",
  "Classes\\Eyeman.ecl",
  "Classes\\Fish.ecl",
  "Classes\\Gizmo.ecl",
  "Classes\\Grunt.ecl",
  "Classes\\Guffy.ecl",
  "Classes\\Headman.ecl",
  "Classes\\BigHead.ecl",
  "Classes\\Santa.ecl",
  "Classes\\Scorpman.ecl",
  "Classes\\Walker.ecl",
  "Classes\\Werebull.ecl",
  "Classes\\Woman.ecl",
];

// Prepare class icons
icons <- [
  CTextureObject("Thumbnails\\Beast.tbn"),
  CTextureObject("Thumbnails\\Boneman.tbn"),
  CTextureObject("Thumbnails\\CannonRotating.tbn"),
  CTextureObject("Thumbnails\\CannonStatic.tbn"),
  CTextureObject("Thumbnails\\ChainsawFreak.tbn"),
  CTextureObject("Thumbnails\\Demon.tbn"),
  CTextureObject("Thumbnails\\Elemental.tbn"),
  CTextureObject("Thumbnails\\Eyeman.tbn"),
  CTextureObject("Thumbnails\\Fish.tbn"),
  CTextureObject("Thumbnails\\Gizmo.tbn"),
  CTextureObject("Thumbnails\\Grunt.tbn"),
  CTextureObject("Thumbnails\\Guffy.tbn"),
  CTextureObject("Thumbnails\\Headman.tbn"),
  CTextureObject("Thumbnails\\Mental.tbn"),
  CTextureObject("Thumbnails\\Santa.tbn"),
  CTextureObject("Thumbnails\\Scorpman.tbn"),
  CTextureObject("Thumbnails\\Walker.tbn"),
  CTextureObject("Thumbnails\\Werebull.tbn"),
  CTextureObject("Thumbnails\\Woman.tbn"),
];

// Spawn entity at the crosshair position facing the player
function SpawnEntity(classFile) {
  // Create teleport effect
  Entities.Create("Classes\\BasicEffect.ecl", CPlacement3D(hit + FLOAT3D(0, 2, 0), FLOAT3D(0, 0, 0)));
  local ee = CEntityEvent(EVENTCODE.ESpawnEffect);
  // Event arguments: BET_TELEPORT, upward normal, upward direction, x3.0 stretch, white opaque color
  ee.SetArgs(17, FLOAT3D(0, 1, 0), FLOAT3D(0, 1, 0), FLOAT3D(3, 3, 3), 0xFFFFFFFF);
  Entities.Initialize(0, ee);

  local towardsPlayer = CPlacement3D(hit + FLOAT3D(0, 1, 0), FLOAT3D(view.rot.h + 180, 0, 0));
  Entities.Create(classFile, towardsPlayer);
  Entities.Initialize(0);
};

function OnStep() {
  menuKey.Update(Button.KID_TAB);

  // Numbers row
  spawnKeys[1].Update(Button.KID_1);
  spawnKeys[2].Update(Button.KID_2);
  spawnKeys[3].Update(Button.KID_3);
  spawnKeys[4].Update(Button.KID_4);
  spawnKeys[5].Update(Button.KID_5);
  spawnKeys[6].Update(Button.KID_6);
  spawnKeys[7].Update(Button.KID_7);
  spawnKeys[8].Update(Button.KID_8);
  spawnKeys[9].Update(Button.KID_9);
  spawnKeys[0].Update(Button.KID_0);

  // Make sure there are any local players
  if (!World.AnyLocalPlayers()) return;

  // Toggle spawn menu and ignore any actions if it's closed
  if (menuKey.pressed) menu = !menu;
  if (!menu) return;

  // Spawn entities on any of the 8 menu buttons
  for (local i = 0; i < 8; i++) {
    local element = i + 8*page;

    if (spawnKeys[i + 1].pressed && element < classes.len()) {
      SpawnEntity(classes[element]);
    }
  }

  // Switch menu pages
  if (spawnKeys[9].pressed && page > 0) page--;
  if (spawnKeys[0].pressed && page < 2) page++;
};

function OnPlayerAction(iClient, iPlayer, pa, iResent) {
  if (iResent != -1) return; // Ignore resent actions

  // Make sure there are any local players
  local localPlayers = World.GetLocalPlayers();
  if (localPlayers.len() == 0) return;

  // Make sure this player is the first local one
  local player = Network.GetPlayerEntity(iPlayer);
  if (!player.Equal(localPlayers[0])) return;

  // Suppress weapon selection while the spawn menu is open
  local PLACT_SELECT_WEAPON_MASK = 0x1F << 14;
  if (menu) pa.ulButtons = pa.ulButtons & ~(PLACT_SELECT_WEAPON_MASK);
};

function OnPostDraw(dp) {
  // Make sure there are any local players
  local players = World.GetLocalPlayers();
  if (players.len() == 0) return;

  // Determine render scaling and set default text font
  local scaling = dp.GetHeight() / 480.0; // Based on the default 480p resolution
  dp.SetFont(fnt);
  dp.SetTextScaling(16.0 / fnt.GetHeight().tofloat() * scaling); // 16px height at default resolution

  if (!menu) {
    dp.PutText("Press TAB to open the spawn menu", 32*scaling, 64*scaling, 0xFFFFFFFF);
    return;
  }

  // Remember player's viewpoint
  local me = players[0];
  view = me.GetViewpoint(true);

  // Determine crosshair position by casting a ray from the viewpoint
  local cr = CCastRay(me, view, 100);
  cr.bHitTranslucentPortals = false;
  cr.bPhysical = false;
  cr.ttHitModels = TestType.COLLISIONBOX;
  cr.Cast();
  hit = cr.GetHitPoint();

  // Display basic info about the first local player
  local str = "" + me.GetName() + "^r info:";

  local pos = me.GetPlacement().pos;
  str += "\n XYZ: " + format("[%.4f; %.4f; %.4f]", pos.x, pos.y, pos.z);
  str += "\n Crosshair: " + format("[%.4f; %.4f; %.4f]", hit.x, hit.y, hit.z);

  local textW = dp.GetTextWidth(str) + 6*scaling;
  dp.Fill(32*scaling, 64*scaling, textW, (3 * fnt.GetHeight() + fnt.pixLineSpacing) * scaling, 0x7F);
  dp.PutText(str, 35*scaling, 67*scaling, 0xFFFFFFFF);

  // Spawn menu
  local cellW = 48*scaling;
  local menuW = cellW*2;
  local menuH = cellW*5;
  local gapSize = 4*scaling;
  local iconW = cellW - gapSize;
  local keyOffset = 2*scaling;

  // Menu background
  local baseX = 32*scaling;
  local baseY = dp.GetHeight()*0.5 - menuH/2;
  dp.Fill(baseX, baseY, menuW, menuH, 0x3F3F7F7F);

  // Render each button of the spawn menu
  for (local i = 0; i < 10; i++) {
    local col = i % 2;
    local row = i / 2;
    local element = i + 8*page;

    local x = baseX + col * cellW + gapSize / 2;
    local y = baseY + row * cellW + gapSize / 2;
    
    // Detect held keys for coloring the buttons red
    local keyNum = (i + 1) % 10;
    local keyHeld = spawnKeys[keyNum].held;

    // Let the first 8 buttons be occupied by class icons, as long as they exist
    if (i < 8 && element < icons.len()) {
      dp.PutTexture(icons[element], [x, y], [x + iconW, y + iconW], keyHeld ? 0xFF7F7FFF : 0xFFFFFFFF);
    } else {
      dp.Fill(x, y, iconW, iconW, keyHeld ? 0x7F3F3FBF : 0x7F7F9FBF);
    }

    // Display key number on each menu button
    dp.PutText(keyNum.tostring(), x + keyOffset, y + keyOffset, 0xFFFFFFFF);

    // The last two buttons are for changing menu pages
    if (i == 8) dp.PutTextC("PREV", x + iconW/2, y + iconW/2, 0xFF7F7FFF);
    if (i == 9) dp.PutTextC("NEXT", x + iconW/2, y + iconW/2, 0x7FFF7FFF);
  }
};
