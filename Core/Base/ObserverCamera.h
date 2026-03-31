/* Copyright (c) 2024-2026 Dreamy Cecil
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
      INDEX bBankingL, bBankingR, bZoomIn, bZoomOut, bRotate;
      INDEX bResetToPlayer, bSnapshot, bSpeedUp;
      INDEX iFocusPlayer;
      FLOAT fFOV;

      CameraControl() {
        Reset();
      };

      // Reset all controls
      void Reset(void) {
        bMoveF = bMoveB = bMoveL = bMoveR = bMoveU = bMoveD = FALSE;
        bBankingL = bBankingR = bZoomIn = bZoomOut = bRotate = FALSE;
        bResetToPlayer = bSnapshot = bSpeedUp = FALSE;
        iFocusPlayer = 0;
        fFOV = 90.0f;
      };
    };

    // Camera properties
    struct CameraProps {
      BOOL bActive; // Dynamic camera toggle
      INDEX iShowInfo; // Display current camera properties and default controls for free fly mode
      BOOL bPlaybackSpeedControl; // Let camera playback control demo speed
      BOOL bSmoothPlayback; // Smooth camera movement during playback
      FLOAT fSmoothTension; // Camera movement tension during smooth playback
      FLOAT fSpeed; // Movement speed multiplier
      FLOAT fTiltAngleMul; // Multiplier for banking rotation speed/angle
      FLOAT fFOVChangeMul; // Speed multiplier for changing the FOV
      FLOAT fSmoothMovement; // Factor for smooth camera movement
      FLOAT fSmoothRotation; // Factor for smooth camera rotation
      FLOAT fFocusDist; // How close to keep the camera to the current player

      BOOL bGrid; // Rule of thirds grid rendering
      INDEX iScreenshotW, iScreenshotH; // Screenshot resolution (limited to 1x1 .. 20000x20000)
      BOOL bPosingMode; // Currently in the player posing mode instead of free fly

      CameraProps() {
        Reset();
      };

      void Reset(void) {
        bActive = FALSE;
        iShowInfo = 2;
        bPlaybackSpeedControl = FALSE;
        bSmoothPlayback = FALSE;
        fSmoothTension = 0.0f;
        fSpeed = 1.0f;
        fTiltAngleMul = 1.0f;
        fFOVChangeMul = 1.0f;
        fSmoothMovement = 1.0f;
        fSmoothRotation = 1.0f;
        fFocusDist = -1.0f;

        bGrid = FALSE;
        iScreenshotW = 1920;
        iScreenshotH = 1080;

        bPosingMode = FALSE;
      };
    };

    // Camera position
    struct CameraPos {
      TIME tmTick;
      FLOAT fSpeed;
      CPlacement3D plPos;
      FLOAT fFOV;
      BOOL bLast; // Internal flag

      CameraPos() : plPos(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0)), fFOV(90), fSpeed(1), tmTick(0), bLast(FALSE)
      {
      };

      inline FLOAT3D &Pos(void) { return plPos.pl_PositionVector; };
      inline FLOAT3D &Rot(void) { return plPos.pl_OrientationAngle; };

      inline const FLOAT3D &Pos(void) const { return plPos.pl_PositionVector; };
      inline const FLOAT3D &Rot(void) const { return plPos.pl_OrientationAngle; };
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
    CameraPos cam_cpCurrent; // Current camera position (free fly mode)
    CameraPos cam_cpView; // Camera position for the current frame

    // Absolute movement & rotation speed
    FLOAT3D cam_vMovement;
    ANGLE3D cam_aRotation;

    CEntity *cam_penViewer; // Which entity the world is currently being viewed from
    CSoundListener cam_sliWorld; // Listener for world sounds
    BOOL cam_bExternalUsage; // Check whether or not the camera is usable from the outside

    // Photo mode player posing
    CModelObject cam_moPose; // Player model for posing
    CEntity *cam_penPosingPlayer; // Player that needs to be posed

    FLOAT3D cam_vPoseOffset; // Player model offset (all from -1 to +1)
    FLOAT cam_fPoseRotation; // Player model rotation

  public:
    // Constructor
    CObserverCamera() {
      cam_bExternalUsage = FALSE;
      ResetPhotoModePose(FALSE);
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

    // Reset player for photo mode to reload the pose
    inline void ResetPhotoModePose(BOOL bOnlyReload) {
      cam_penPosingPlayer = NULL;

      if (!bOnlyReload) {
        cam_props.bPosingMode = FALSE;
        cam_vPoseOffset = FLOAT3D(0, 0, 0);
        cam_fPoseRotation = 0;
      }
    };

  public:
    // Start camera for a game (or a currently playing demo)
    void Start(const CTFileName &fnmDemo);

    // Stop playback or camera altogether
    void Reset(BOOL bPlayback = FALSE);

    // Retrieve camera state field
    BOOL &GetState(void);

    // Check if camera is on
    BOOL IsActive(BOOL bAlsoDemoPlayback = TRUE);

    // Toggle photo mode during the game or demos
    // Returns whether the camera was supposed to activate
    BOOL TogglePhotoMode(void);

    // Start recording into a file
    BOOL StartRecording(void);

    // Retrieve a photo mode model for some player entity
    CModelObject *GetPoseModel(CEntity *penPlayer, CModelObject *pmoOriginalAppearance);

  public:
    // Direct button input using default controls
    void UpdateControls(void);

    // Print camera info and controls
    void PrintCameraInfo(CDrawPort *pdp);

    // Free fly camera movement during the game
    CameraPos &FreeFly(CPlayerEntity *penObserving);

    // Update the camera and render the world through it
    BOOL Update(CEntity *pen, CDrawPort *pdp);

    // Take a high quality screenshot of the current view
    BOOL TakeScreenshot(CImageInfo &iiScreenshot);
};

#endif
