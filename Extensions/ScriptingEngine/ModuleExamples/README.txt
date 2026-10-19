This directory contains examples of custom script modules that can be used when placed in "Scripts/Squirrel/Custom/" directory inside the game.

You can also alternatively load them by using the "scr_LoadCustomScript()" shell command from the repository, for example like this if this repository resides under the "SuperProject" directory inside the game folder:
  scr_LoadCustomScript("SuperProject\\Extensions\\ScriptingEngine\\ModuleExamples\\Example1.nut");

Or from other Squirrel scripts by creating a custom script module per example script:
  IncludeScript("SuperProject\\Extensions\\ScriptingEngine\\ModuleExamples\\Example1.nut");

Feel free to study them and see how certain server-side features may be implemented using custom scripts.
