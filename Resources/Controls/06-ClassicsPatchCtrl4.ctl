Button
 Name: TTRS Move Forward
 Key1: Joy 4 POV N
 Key2: None
 Pressed:  ctl_bMoveForward = 1;
 Released: ctl_bMoveForward = 0;
Button
 Name: TTRS Move Backward
 Key1: Joy 4 POV S
 Key2: None
 Pressed:  ctl_bMoveBackward = 1;
 Released: ctl_bMoveBackward = 0;
Button
 Name: TTRS Strafe Left
 Key1: Joy 4 POV W
 Key2: None
 Pressed:  ctl_bMoveLeft = 1;
 Released: ctl_bMoveLeft = 0;
Button
 Name: TTRS Strafe Right
 Key1: Joy 4 POV E
 Key2: None
 Pressed:  ctl_bMoveRight = 1;
 Released: ctl_bMoveRight = 0;
Button
 Name: TTRS Up/Jump
 Key1: Joy 4 Button 25
 Key2: None
 Pressed:  ctl_bMoveUp = 1;
 Released: ctl_bMoveUp = 0;
Button
 Name: TTRS Down/Duck
 Key1: Joy 4 Button 7
 Key2: None
 Pressed:  ctl_bMoveDown = 1;
 Released: ctl_bMoveDown = 0;
Button
 Name: TTRS Turn Left
 Key1: None
 Key2: None
 Pressed:  ctl_bTurnLeft = 1;
 Released: ctl_bTurnLeft = 0;
Button
 Name: TTRS Turn Right
 Key1: None
 Key2: None
 Pressed:  ctl_bTurnRight = 1;
 Released: ctl_bTurnRight = 0;
Button
 Name: TTRS Look Up
 Key1: None
 Key2: None
 Pressed:  ctl_bTurnUp = 1;
 Released: ctl_bTurnUp = 0;
Button
 Name: TTRS Look Down
 Key1: None
 Key2: None
 Pressed:  ctl_bTurnDown = 1;
 Released: ctl_bTurnDown = 0;
Button
 Name: TTRS Center View
 Key1: Joy 4 Button 8
 Key2: None
 Pressed:  ctl_bCenterView = 1;
 Released: ctl_bCenterView = 0;
Button
 Name: TTRS Walk
 Key1: None
 Key2: None
 Pressed:  ctl_bWalk = !ctl_bWalk;
 Released: ctl_bWalk = !ctl_bWalk;
Button
 Name: TTRS Walk/Run Toggle
 Key1: None
 Key2: None
 Pressed:  ctl_bWalk = !ctl_bWalk;
 Released: ;
Button
 Name: TTRS Strafe
 Key1: None
 Key2: None
 Pressed:  ctl_bStrafe = 1;
 Released: ctl_bStrafe = 0;
Button
 Name: TTRS Fire
 Key1: Joy 4 Button 26
 Key2: None
 Pressed:  ctl_bFire = 1;
 Released: ctl_bFire = 0;
Button
 Name: TTRS Reload
 Key1: Joy 4 Button 2
 Key2: None
 Pressed:  ctl_bReload = 1;
 Released: ctl_bReload = 0;
Button
 Name: TTRS 3rd Person View
 Key1: None
 Key2: None
 Pressed:  ctl_b3rdPersonView = 1;
 Released: ctl_b3rdPersonView = 0;
Button
 Name: TTRS Use/Invoke NETRICSA
 Key1: None
 Key2: None
 Pressed:  ctl_bUseOrComputer = 1;
 Released: ctl_bUseOrComputer = 0;
Button
 Name: TTRS Use
 Key1: Joy 4 Button 0
 Key2: None
 Pressed:  ctl_bUse = 1;
 Released: ctl_bUse = 0;
Button
 Name: TTRS Invoke NETRICSA
 Key1: Joy 4 Button 3
 Key2: None
 Pressed:  ctl_bComputer = 1;
 Released: ctl_bComputer = 0;
Button
 Name: TTRS Talk
 Key1: None
 Key2: None
 Pressed:  con_bTalk=1;
 Released: 
Button
 Name: TTRS Previous Weapon
 Key1: Joy 4 Button 9
 Key2: None
 Pressed:  ctl_bWeaponPrev = 1;
 Released: ctl_bWeaponPrev = 0;
Button
 Name: TTRS Next Weapon
 Key1: Joy 4 Button 10
 Key2: None
 Pressed:  ctl_bWeaponNext = 1;
 Released: ctl_bWeaponNext = 0;
Button
 Name: TTRS Flip Weapon
 Key1: None
 Key2: None
 Pressed:  ctl_bWeaponFlip = 1;
 Released: ctl_bWeaponFlip = 0;
Button
 Name: TTRS Knife
 Key1: None
 Key2: None
 Pressed:  ctl_bSelectWeapon[1] = 1;
 Released: ctl_bSelectWeapon[1] = 0;
Button
 Name: TTRS Colt/Two Colts
 Key1: None
 Key2: None
 Pressed:  ctl_bSelectWeapon[2] = 1;
 Released: ctl_bSelectWeapon[2] = 0;
Button
 Name: TTRS Single/Double Shotgun
 Key1: None
 Key2: None
 Pressed:  ctl_bSelectWeapon[3] = 1;
 Released: ctl_bSelectWeapon[3] = 0;
Button
 Name: TTRS Tommygun/Minigun
 Key1: None
 Key2: None
 Pressed:  ctl_bSelectWeapon[4] = 1;
 Released: ctl_bSelectWeapon[4] = 0;
Button
 Name: TTRS Rocket Launcher
 Key1: None
 Key2: None
 Pressed:  ctl_bSelectWeapon[5] = 1;
 Released: ctl_bSelectWeapon[5] = 0;
Button
 Name: TTRS Grenade Launcher
 Key1: None
 Key2: None
 Pressed:  ctl_bSelectWeapon[6] = 1;
 Released: ctl_bSelectWeapon[6] = 0;
Button
 Name: TTRS Laser
 Key1: None
 Key2: None
 Pressed:  ctl_bSelectWeapon[7] = 1;
 Released: ctl_bSelectWeapon[7] = 0;
Button
 Name: TTRS Cannon
 Key1: None
 Key2: None
 Pressed:  ctl_bSelectWeapon[8] = 1;
 Released: ctl_bSelectWeapon[8] = 0;
Button
 Name: TTRS Serious Bomb
 Key1: Joy 4 Button 1
 Key2: None
 Pressed:  ctl_bFireBomb = 1;
 Released: ctl_bFireBomb = 0;
Axis "move u/d" "None" 50 0 NotInverted Relative NotSmooth
Axis "move l/r" "Joy 4 Axis X" 60 4 Inverted Relative NotSmooth
Axis "move f/b" "Joy 4 Axis Y" 60 4 Inverted Relative NotSmooth
Axis "look u/d" "Joy 4 Axis R" 70 4 Inverted Relative NotSmooth
Axis "turn l/r" "Joy 4 Axis Z" 70 4 NotInverted Relative NotSmooth
Axis "banking" "None" 50 0 NotInverted Absolute NotSmooth
Axis "view u/d" "None" 50 0 NotInverted Absolute NotSmooth
Axis "view l/r" "None" 50 0 NotInverted Absolute NotSmooth
Axis "view banking" "None" 50 0 NotInverted Absolute NotSmooth
GlobalDontInvertLook
GlobalSmoothAxes
GlobalSensitivity 40