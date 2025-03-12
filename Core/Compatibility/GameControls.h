/* Copyright (c) 2002-2012 Croteam Ltd.
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

// [Cecil] NOTE: Structures in here are moved from Game.h to avoid unnecessary clutter

#ifndef CORE_COMPATIBILITY_INCL_GAMECONTROLS_H
#define CORE_COMPATIBILITY_INCL_GAMECONTROLS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#define AXIS_ACTIONS_CT 9

// One axis in the controls
class CAxisAction {
  public:
    INDEX aa_iAxisAction;       // To which axis this object refers to
    FLOAT aa_fSensitivity;      // Percentage of maximum sensitivity (0..100)
    FLOAT aa_fDeadZone;         // Percentage of deadzone (0..100)
    BOOL aa_bInvert;            // If controller axis should be inverted
    BOOL aa_bRelativeControler; // If this is controller of relative type
    BOOL aa_bSmooth;            // If controller axis should be smoothed

    // This value is appled to view angle or movement and it's calculated from the invert flag and sensitivity
    // E.g. rotation could be: AXIS_ROTATION_SPEED * sensitivity * -1 (if should be inverted)
    FLOAT aa_fAxisInfluence;

    FLOAT aa_fLastReading; // Last axis value (for smoothing)
    FLOAT aa_fAbsolute;    // Absolute axis value (integrated from previous readings)
};

// One action in the controls
class CButtonAction {
  public:
    CListNode ba_lnNode;

    INDEX ba_iFirstKey;
    BOOL ba_bFirstKeyDown;
    INDEX ba_iSecondKey;
    BOOL ba_bSecondKeyDown;

    CTString ba_strName;
    CTString ba_strCommandLineWhenPressed;
    CTString ba_strCommandLineWhenReleased;

  public:
    // Assignment operator
    virtual CButtonAction &operator=(CButtonAction &baOriginal);

    // Serialization
    virtual void Read_t(CTStream &istrm);
    virtual void Write_t(CTStream &ostrm);
};

// Individual player controls setup
class CControls {
  public:
    // Mounted button actions
    CListHead ctrl_lhButtonActions;

    // Mounted controller axes and their properties
    CAxisAction ctrl_aaAxisActions[AXIS_ACTIONS_CT];

    FLOAT ctrl_fSensitivity; // Global sensitivity for all axes
    BOOL ctrl_bInvertLook;   // Invert vertical view axis
    BOOL ctrl_bSmoothAxes;   // Smooths axes movements

  public:
    // Destructor
    virtual ~CControls(void);

    // Assignment operator
    virtual CControls &operator=(CControls &ctrlOriginal);

    // Calculate influence factors for all axis actions depending on axis properties
    virtual void CalculateInfluencesForAllAxis(void);

    // Get current reading of an axis
    virtual FLOAT GetAxisValue(INDEX iAxis);

    // Check if these controls use any joystick
    virtual BOOL UsesJoystick(void);

    // Switch button and axis action mounters to defaults
    virtual void SwitchAxesToDefaults(void);
    virtual void SwitchToDefaults(void);

    virtual void DoButtonActions(void);
    virtual void CreateAction(const CPlayerCharacter &pc, CPlayerAction &paAction, BOOL bPreScan);
    virtual CButtonAction &AddButtonAction(void);
    virtual void RemoveButtonAction(CButtonAction &baButtonAction);
    virtual void Load_t(CTFileName fnFile);
    virtual void Save_t(CTFileName fnFile);
};

#endif
