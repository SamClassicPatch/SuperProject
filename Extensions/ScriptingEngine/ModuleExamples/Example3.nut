/* ================================================================
 * Script module example 3: Player death taunt
 * Compatibility: TFE (v1.05), TSE (v1.05, v1.07)
 *
 * Plays a laughing sound upon player's death and spawns five tiny
 * marsh-hoppers at player's death place.
 * ================================================================ */

function SpawnGizmo(place) {
  Entities.Create("Classes\\Gizmo.ecl", place);
  Entities.SetProperty(0, "Stretch multiplier", 0.3);
  Entities.Initialize(0);
};

function OnReceiveGenericEvent(en, event, args) {
  // Cannot execute if not currently running on a patch-exclusive server
  if (!Network.IsHost() || Network.IsVanillaCompatible()) return;

  // If a player entity has died
  if (en.IsOfClass("Player") && event == EVENTCODE.EDeath) {
    // Play laughing sound with randomized pitch
    local snd = Network.Sound("Models\\Enemies\\Devil\\Sounds\\Celebrate01.wav", 0, 2);
    snd.fPitch = 1.5 + (rand() % 100).tofloat() / 100.0;
    snd.Play();

    // Spawn five hoppers around the player's death place
    local gizmos = 5;

    for (local i = 0; i < gizmos; i++) {
      local pl = en.GetPlacement();
      local rad = Math.DegToRad(360.0 / gizmos * i);

      pl.pos += FLOAT3D(sin(rad), 1, cos(rad)) * 3 * en.GetRotationMatrix();
      SpawnGizmo(pl);
    }
  }
};
