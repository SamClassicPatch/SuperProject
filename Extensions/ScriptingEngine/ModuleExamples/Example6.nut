/* ================================================================
 * Script module example 6: Playable server-side piano
 * Compatibility: TFE (v1.05), TSE (v1.05, v1.07)
 *
 * Displays a piano keyboard in the middle of the screen that the
 * host client can interact with in order to play global sounds to
 * every client on the server that are pitch-shifted to match their
 * respective note.
 *
 * Press TAB to switch between different sound samples and use the
 * main keyboard keys to play specific notes.
 * ================================================================ */

// Class for handling key states
class PianoKey {
  held = false;
  pressed = false;
  released = false;

  // Update the key to determine all of its states for the current step
  function Update(key, keyAlt) {
    local state = false;

    // Allow use of an alternative key ID for the same key
    if (keyAlt != -1) {
      state = Input.GetButtonState(key) || Input.GetButtonState(keyAlt);
    } else {
      state = Input.GetButtonState(key);
    }

    pressed = state && !held;
    released = !state && held;
    held = state;
  };
};

switchSample <- PianoKey();

// Maximum amount of Network.Sound channels
keyCount <- 32;

// 2 + 2/3 octaves worth of keys
pianoKeys <- [
  PianoKey(), PianoKey(), PianoKey(), PianoKey(), PianoKey(), PianoKey(),
  PianoKey(), PianoKey(), PianoKey(), PianoKey(), PianoKey(), PianoKey(),
  PianoKey(), PianoKey(), PianoKey(), PianoKey(), PianoKey(), PianoKey(),
  PianoKey(), PianoKey(), PianoKey(), PianoKey(), PianoKey(), PianoKey(),
  PianoKey(), PianoKey(), PianoKey(), PianoKey(), PianoKey(), PianoKey(),
  PianoKey(), PianoKey(),
];

// Primary buttons for the piano keys
keyButtons <- [
  // 1st octave: _^_^__^_^_^_
  Button.KID_Z, Button.KID_S, Button.KID_X, Button.KID_D, Button.KID_C, Button.KID_V, Button.KID_G, Button.KID_B, Button.KID_H, Button.KID_N, Button.KID_J, Button.KID_M,
  // 2nd octave: _^_^__^_^_^_
  Button.KID_Q, Button.KID_2, Button.KID_W, Button.KID_3, Button.KID_E, Button.KID_R, Button.KID_5, Button.KID_T, Button.KID_6, Button.KID_Y, Button.KID_7, Button.KID_U,
  // 3rd octave: _^_^__^_
  Button.KID_I, Button.KID_9, Button.KID_O, Button.KID_0, Button.KID_P, Button.KID_LBRACKET, Button.KID_EQUALS, Button.KID_RBRACKET,
];

// Secondary buttons for the piano keys
keyAltButtons <- [
  // 1st octave: _^_^__^_^_^_
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  // 2nd octave: _^_^__^_^_^_
  Button.KID_COMMA, Button.KID_L, Button.KID_PERIOD, Button.KID_SEMICOLON, Button.KID_SLASH, -1, -1, -1, -1, -1, -1, -1,
  // 3rd octave: _^_^__^_
  -1, -1, -1, -1, -1, -1, -1, -1,
];

// Primary button names
btnNames <- [
  // 1st octave: _^_^__^_^_^_
  "Z", "S", "X", "D", "C", "V", "G", "B", "H", "N", "J", "M",
  // 2nd octave: _^_^__^_^_^_
  "Q", "2", "W", "3", "E", "R", "5", "T", "6", "Y", "7", "U",
  // 3rd octave: _^_^__^_
  "I", "9", "O", "0", "P", "[", "=", "]",
];

// Piano key names
keyNames <- [
  // 1st octave: _^_^__^_^_^_
  "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4",
  // 2nd octave: _^_^__^_^_^_
  "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5", "A#5", "B5",
  // 3rd octave: _^_^__^_
  "C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6",
];

// Key pattern for one octave: 7 white keys (0), 5 black keys (1)
octave <- [0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0];

// Sound sample holder
class Sample {
  name = ""; // Display name
  sound = ""; // Sound file
  volume = 1; // Volume multiplier
  basePitch = 1; // Base pitch of the sound that matches C5
  offsets = [0, 0]; // For shortening samples by skipping a portion of the sound

  constructor(setName, setSound, setVolume, setPitch, setOffsets) {
    name = setName;
    sound = setSound;
    volume = setVolume;
    basePitch = setPitch;
    offsets = setOffsets;
  };
};

// Selection of sound samples
samples <- [
  Sample("Yodel", "SoundsMP\\Misc\\yoyuhu.wav", 4, 1, [2.2, 1.85]),
  Sample("Key", "Sounds\\Items\\Key.wav", 1, 0.707107, [35.2*1.5, 7.4*1.5]),
  Sample("Menu", "Sounds\\Menu\\Press.wav", 4, 0.7937, [0, 0]),
],

selectedSample <- 0;

// Prepare text fonts
fntKeys <- CFontData("Fonts\\Display3-caps.fnt");
fntText <- CFontData("Fonts\\Display3-narrow.fnt");

function OnStep() {
  switchSample.Update(Button.KID_TAB, -1);

  if (switchSample.pressed) {
    selectedSample = (selectedSample + 1) % samples.len();
  }

  // Prepare selected sound sample
  local sample = samples[selectedSample];

  for (local i = 0; i < keyCount; i++) {
    pianoKeys[i].Update(keyButtons[i], keyAltButtons[i]);

    if (pianoKeys[i].pressed) {
      // Play the sample sound on a sound channel unique for the piano key 
      local snd = Network.Sound(sample.sound, i, sample.volume);

      snd.fOffset = sample.offsets[1]; // Regular sample offset

      // Adjust note pitch based on the octave
      if (i < 12) {
        snd.fOffset = sample.offsets[0]; // Shorter offset for lower pitched keys to reduce their length
        snd.fPitch = pow(1.059463, i)*0.5; // 1st octave
      } else if (i < 24) {
        snd.fPitch = pow(1.059463, i-12); // 2nd octave
      } else if (i < 32) {
        snd.fPitch = pow(1.059463, i-24)*2; // 3rd octave
      }

      snd.fPitch *= sample.basePitch; // Adjust by base pitch
      snd.Play();
    }
  }
};

function OnPlayerAction(iClient, iPlayer, pa, iResent) {
  if (iResent != -1) return; // Ignore resent actions

  // Suppress controls for the host player that's playing the sounds
  if (iPlayer != 0) return;
  pa.vTranslation = FLOAT3D(0, 0, 0);
  pa.ulButtons = 0;
};

function OnPostDraw(dp) {
  if (!World.AnyLocalPlayers()) return;

  // Determine render scaling and set default text font
  local scaling = dp.GetHeight() / 480.0; // Based on the default 480p resolution
  dp.SetFont(fntKeys);
  dp.SetTextScaling(16.0 / fntKeys.GetHeight().tofloat() * scaling); // 16px height at default resolution

  // Calculate piano width based on the amount of white keys
  local keyW = 32 * scaling;
  local pianoHalfW = keyW * ceil(keyCount * (7.0/12.0)) / 2;
  local pianoH = keyW * 4;
  local blackH = pianoH * 0.6;

  // Render white keys first
  local x = dp.GetWidth() / 2 - pianoHalfW;
  local y = dp.GetHeight() * 0.6;

  for (local i = 0; i < keyCount; i++) {
    local held = pianoKeys[i].held;

    if (!octave[i % octave.len()]) {
      dp.Fill      (x, y, keyW, pianoH, held ? 0xFFBF00DF : 0xFFFFFFDF);
      dp.DrawBorder(x, y, keyW, pianoH, 0xFF); // Opaque black border

      dp.PutTextC(btnNames[i], x + keyW/2, y + pianoH*0.72, 0xCFCFCFFF);
      dp.PutTextC(keyNames[i], x + keyW/2, y + pianoH*0.85, 0xCFCFCFFF);

      x += keyW;
    }
  }

  // Then render black keys
  x = dp.GetWidth() / 2 - pianoHalfW;

  for (local i = 1; i < keyCount; i++) {
    local held = pianoKeys[i].held;

    if (octave[i % octave.len()]) {
      local keyX = x + keyW/1.6;

      dp.Fill      (keyX, y, keyW*0.8, blackH, held ? 0x7F5F00FF : 0x3F3F3FFF);
      dp.DrawBorder(keyX, y, keyW*0.8, blackH, 0xFF); // Opaque black border

      dp.PutTextC(btnNames[i], keyX + keyW/2.4, y + blackH*0.6, 0xFFFFFFFF);
      dp.PutTextC(keyNames[i], keyX + keyW/2.4, y + blackH*0.8, 0xFFFFFFFF);

    } else {
      x += keyW;
    }
  }

  // Display currently selected sample
  dp.SetFont(fntText);
  dp.SetTextScaling(16.0 / fntText.GetHeight().tofloat() * scaling); // 16px height at default resolution

  x = dp.GetWidth() / 2 - pianoHalfW;
  dp.PutText("Press TAB to change sample: " + samples[selectedSample].name, x, y - 24*scaling, 0xFFFFFFFF);
};
