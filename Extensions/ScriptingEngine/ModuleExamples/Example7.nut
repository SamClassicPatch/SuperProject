/* ================================================================
 * Script module example 7: Chat recording for demos
 * Compatibility: TFE (v1.05), TSE (v1.05, v1.07)
 *
 * Allows recording chat messages into demos on the client's side
 * by writing them in local storage under paths to the demo files
 * as their keys.
 *
 * To migrate chat logs from one demo file to another, use the
 * MigrateChatLog() function included in this script. Example:
 *   MigrateChatLog("Demos\\Demo0001.dem", "Demos\\RenamedDemo123.dem");
 * ================================================================ */

// File of the currently recorded/played back demo
demoFile <- "";

// Controls whether the chat is being recorded or played back
// but only when 'demoFile' isn't an empty string, otherwise ignored
chatRec <- false;

// CPU time when the recording/playback started
startTime <- Timer.Value(0);

// Buffer of chat messages, either for recording or for playing them back
chatMessages <- [];
currentMsg <- 0;

// Special function for migrating chat logs from one demo file to another (unused in this example)
function MigrateChatLog(oldDemoFile, newDemoFile) {
  local chat = Core.LocalStorage("example_chat_recordings");

  // If there's an array of messages under the old demo file
  if (chat.IsArray(oldDemoFile)) {
    // Copy its contents under the new demo file
    chat.SetArray(newDemoFile, chat.GetArray(oldDemoFile));

    // And delete the old chat log
    chat.DeleteKey(oldDemoFile);

    chat.Save();
    return true;
  }

  // No array under the old demo file
  return false;
};

// Check if the chat is being recorded
function IsRecordingChat() {
  return demoFile != "" && chatRec;
};

// Check if the chat is being played back
function IsPlayingBackChat() {
  return demoFile != "" && !chatRec;
};

function OnDemoPlay(file) {
  // Prepare for playing back chat for this demo
  demoFile = file;
  chatRec = false;
  startTime <- Timer.GetHighPrecisionTimer();

  // Open local storage for chat recordings and read chat contents from it
  local chat = Core.LocalStorage("example_chat_recordings");

  if (chat.IsArray(demoFile)) {
    chatMessages = chat.GetArray(demoFile);
  } else {
    chatMessages = [];
  }

  currentMsg = 0;
};

function StopPlaying() {
  // If any demo is still playing back
  if (IsPlayingBackChat()) {
    // Discard the data
    chatMessages = [];
    demoFile = "";
  }
};

function OnDemoStart(file) {
  // Prepare for recording chat for this demo
  demoFile = file;
  chatRec = true;
  startTime <- Timer.GetHighPrecisionTimer();
  chatMessages = [];
};

function OnDemoStop() {
  // If any demo is still recording
  if (IsRecordingChat()) {
    // Open local storage for chat recordings and write chat contents to it
    local chat = Core.LocalStorage("example_chat_recordings");
    chat.SetArray(demoFile, chatMessages);
    chat.Save();
    demoFile = "";
  }
};

function OnFrame(dp) {
  // If no world is running anymore
  if (!World.IsActive()) {
    // Close any recorded or played back demo
    OnDemoStop();
    StopPlaying();
    return;
  }

  // Make sure playback is currently happening
  if (!IsPlayingBackChat()) return;

  // Make sure there are any messages left to play back
  if (currentMsg >= chatMessages.len()) return;

  // Get current message
  local msg = chatMessages[currentMsg];

  // If its cached time has reached the current second
  while (msg.time <= World.GetLerpedCurrentTick()) {
    // Display it in the console and increase chat message counter to play the chat sound
    print(msg.sender + ": ^o^cFFFFFF" + msg.message + "^r\n");
    Shell.SetIndex("net_ctChatMessages", Shell.GetIndex("net_ctChatMessages") + 1);

    // Move on to the next message if it exists
    currentMsg++;
    if (currentMsg >= chatMessages.len()) return;

    msg = chatMessages[currentMsg];
  }
};

function OnChatOutput(from_mask, from_name, sender, message) {
  // If currently recording chat and both the sender and the message exist
  if (IsRecordingChat() && sender != "" && message != "") {
    // Log the message as an object
    chatMessages.append({
      "time" : World.GetLerpedCurrentTick(),
      "sender" : sender,
      "message" : message,
    });
  }

  // Return unmodified sender and message
  return [sender, message];
};
