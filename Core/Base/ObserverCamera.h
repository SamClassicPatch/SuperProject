/* Copyright (c) 2024 Dreamy Cecil
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

#ifndef CECIL_INCL_OBSERVERCAMERA_H
#define CECIL_INCL_OBSERVERCAMERA_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Interface for freecam observing during a game
class CORE_API CObserverCamera {
  public:
    // Camera control
    struct CameraControl {
      INDEX bMoveF, bMoveB, bMoveL, bMoveR, bMoveU, bMoveD;
      INDEX bBankingL, bBankingR, bZoomIn, bZoomOut;
      INDEX bResetToPlayer, bFollowPlayer, bSnapshot, bScreenshot;
      FLOAT fFOV;

      CameraControl() {
        Reset();
      };

      // Reset all controls
      void Reset(void) {
        bMoveF = bMoveB = bMoveL = bMoveR = bMoveU = bMoveD = FALSE;
        bBankingL = bBankingR = bZoomIn = bZoomOut = FALSE;
        bResetToPlayer = bFollowPlayer = bSnapshot = bScreenshot = FALSE;
        fFOV = 90.0f;
      };
    };

    // Camera properties
    struct CameraProps {
      BOOL bActive; // Dynamic camera toggle
      INDEX iShowInfo; // Display current camera properties and default controls for free fly mode
      BOOL bDefaultControls; // Use internal controls instead of manually binding the commands
      BOOL bPlaybackSpeedControl; // Let camera playback control demo speed
      BOOL bSmoothPlayback; // Smooth camera movement during playback
      FLOAT fSmoothTension; // Camera movement tension during smooth playback
      FLOAT fSpeed; // Movement speed multiplier
      FLOAT fTiltAngleMul; // Multiplier for banking rotation speed/angle
      FLOAT fFOVChangeMul; // Speed multiplier for changing the FOV
      FLOAT fSmoothMovement; // Factor for smooth camera movement
      FLOAT fSmoothRotation; // Factor for smooth camera rotation
      FLOAT fFollowDist; // Close in on the player if they're far enough from the camera

      INDEX iScreenshotW, iScreenshotH; // Screenshot resolution (limited to 1x1 .. 20000x20000)

      CameraProps() {
        Reset();
      };

      void Reset(void) {
        bActive = FALSE;
        iShowInfo = 2;
        bDefaultControls = TRUE;
        bPlaybackSpeedControl = FALSE;
        bSmoothPlayback = FALSE;
        fSmoothTension = 0.0f;
        fSpeed = 1.0f;
        fTiltAngleMul = 1.0f;
        fFOVChangeMul = 1.0f;
        fSmoothMovement = 1.0f;
        fSmoothRotation = 1.0f;
        fFollowDist = -1.0f;

        iScreenshotW = 1920;
        iScreenshotH = 1080;
      };
    };

    // Camera position
    struct CameraPos {
      TIME tmTick;
      FLOAT fSpeed;
      FLOAT3D vPos;
      ANGLE3D aRot;
      FLOAT fFOV;
      BOOL bLast; // Internal flag

      CameraPos() : vPos(0, 0, 0), aRot(0, 0, 0), fFOV(90), fSpeed(1), tmTick(0), bLast(FALSE)
      {
      };

      inline CPlacement3D GetPlacement(void) const {
        return CPlacement3D(vPos, aRot);
      };
    };

  public:
    // Global controls and properties for the observer camera
    static CameraControl cam_ctl;
    static CameraProps cam_props;

    BOOL cam_bPlayback; // Currently playing back the recording
    CTFileName cam_fnmDemo; // Currently playing demo
    CTFileStream cam_strmScript;

    TIME cam_tmStartTime; // Simulation time when camera started
    CTimerValue cam_tvDelta; // Time since last render frame

    CameraPos cam_acpCurve[4]; // Camera positions for a curve (playback mode)
    CameraPos cam_cpCurrent; // Current camera position (freecam mode)
    CameraPos cam_cpView; // Camera position for the current frame

    // Absolute movement & rotation speed
    FLOAT3D cam_vMovement;
    ANGLE3D cam_aRotation;

    CSoundListener cam_sliWorld; // Listener for world sounds
    BOOL cam_bExternalUsage; // Check whether or not the camera is usable from the outside

  public:
    // Constructor
    CObserverCamera() {
      cam_bExternalUsage = FALSE;
      Reset();
    };

    // Initialize camera interface
    void Init(void);

    // Hook old camera commands for compatibility
    void HookOldCamCommands(void);

    void ReadPos(CameraPos &cp);
    void WritePos(CameraPos &cp);

    // Change demo playback speed
    void SetSpeed(FLOAT fSpeed);

    // Reset camera FOV and the banking angle
    void ResetCameraAngles(void);

  public:
    // Start camera for a game (or a currently playing demo)
    void Start(const CTFileName &fnmDemo);

    // Stop playback or camera altogether
    virtual void Reset(BOOL bPlayback = FALSE);

    // Check if camera is on
    virtual BOOL IsActive(void);

    // Start recording into a file
    virtual BOOL StartRecording(void);

  public:
    // Direct button input using default controls
    void UpdateControls(void);

    // Print info and default controls for the camera
    void PrintCameraInfo(CDrawPort *pdp);

    // Free fly camera movement during the game
    CameraPos &FreeFly(CPlayerEntity *penObserving);

    // Update the camera and render the world through it
    BOOL Update(CEntity *pen, CDrawPort *pdp);

    // Take a high quality screenshot of the current view
    void TakeScreenshot(void);
};

#endif
