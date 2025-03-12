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

324
%{
#include "StdH.h"
#include "Models/Enemies/Walker/Walker.h"

// [Cecil] Rev: New attachments
#define WALKER_ATTACHMENT_CANNON_LT WALKER_ATTACHMENT_ROCKETLAUNCHER_LT
#define WALKER_ATTACHMENT_CANNON_RT WALKER_ATTACHMENT_ROCKETLAUNCHER_RT
%}

uses "Enemies/EnemyBase";
uses "Weapons/Projectile";
uses "Weapons/CannonBall";
uses "Enemies/Gizmo";

enum WalkerChar {
  0 WLC_SOLDIER   "Soldier",    // soldier
  1 WLC_SERGEANT  "Sergeant",   // sergeant
  // [Cecil] Rev: New types
  2 WLC_MONSTER   "Artillery",
  3 WLC_SPAWNER   "Spawner",
};

%{
// info structure
static EntityInfo eiWalker = {
  EIBT_FLESH, 1000.0f,
  0.0f, 5.4f, 0.0f,
  0.0f, 4.5f, 0.0f,
};

#define SIZE_SOLDIER   (0.5f)
#define SIZE_SERGEANT  (1.0f)
// [Cecil] Rev: New types
#define SIZE_MONSTER   (1.5f)
#define SIZE_SPAWNER   (0.7f)

#define FIRE_LEFT_ARM   FLOAT3D(-2.5f, 5.0f, 0.0f)
#define FIRE_RIGHT_ARM  FLOAT3D(+2.5f, 5.0f, 0.0f)
#define FIRE_DEATH_LEFT   FLOAT3D( 0.0f, 7.0f, -2.0f)
#define FIRE_DEATH_RIGHT  FLOAT3D(3.75f, 4.2f, -2.5f)

// [Cecil] Rev: Pick sound for Sergeant, Monster or Soldier/Spawner (same sounds)
#define WALKERSOUND(soundname) \
  ((m_EwcChar == WLC_SERGEANT) ? SOUND_SERGEANT_##soundname : \
  ((m_EwcChar == WLC_MONSTER)  ? SOUND_MONSTER_##soundname : SOUND_SOLDIER_##soundname))
%}


class CWalker : CEnemyBase {
name      "Walker";
thumbnail "Thumbnails\\Walker.tbn";

properties:
  1 enum WalkerChar m_EwcChar   "Character" 'C' = WLC_SOLDIER,
  2 INDEX m_iLoopCounter = 0,
  3 FLOAT m_fSize = 1.0f,
  4 BOOL m_bWalkSoundPlaying = FALSE,
  5 FLOAT m_fThreatDistance = 5.0f,

  // [Cecil] Rev: New properties
  20 INDEX m_iCustomFireCounter "Custom fire counter" = -1,
  21 INDEX m_ctMaxGizmos "Max gizmos count" = 10,
  22 INDEX m_ctLiveGizmos = 0,

  10 CSoundObject m_soFeet,
  11 CSoundObject m_soFire1,
  12 CSoundObject m_soFire2,
  13 CSoundObject m_soFire3,
  14 CSoundObject m_soFire4,
  
components:
  0 class   CLASS_BASE          "Classes\\EnemyBase.ecl",
  1 class   CLASS_PROJECTILE    "Classes\\Projectile.ecl",
  2 class   CLASS_BASIC_EFFECT  "Classes\\BasicEffect.ecl",
  3 class   CLASS_CANNONBALL    "Classes\\CannonBall.ecl",
  4 class   CLASS_GIZMO         "Classes\\Gizmo.ecl",

 10 model   MODEL_WALKER              "Models\\Enemies\\Walker\\Walker.mdl",
 11 texture TEXTURE_WALKER_SOLDIER    "Models\\Enemies\\Walker\\Walker02.tex",
 12 texture TEXTURE_WALKER_SERGEANT   "Models\\Enemies\\Walker\\Walker01.tex",
 13 texture TEXTURE_WALKER_MONSTER    "Models\\Enemies\\Walker\\Walker03.tex", // [Cecil] Rev
 14 model   MODEL_LASER               "Models\\Enemies\\Walker\\Laser.mdl",
 15 texture TEXTURE_LASER             "Models\\Enemies\\Walker\\Laser.tex",
 16 model   MODEL_ROCKETLAUNCHER      "Models\\Enemies\\Walker\\RocketLauncher.mdl",
 17 texture TEXTURE_ROCKETLAUNCHER    "Models\\Enemies\\Walker\\RocketLauncher.tex",

 // [Cecil] Rev: New models
 18 model   MODEL_CANNON              "Models\\Weapons\\Cannon\\Body.mdl",
 19 texture TEXTURE_CANNON            "Models\\Weapons\\Cannon\\Body.tex",
 20 texture TEXTURE_WALKER_SPAWNER    "Models\\Enemies\\Walker\\Walker04.tex",
 21 model   MODEL_GIZMO               "Models\\Enemies\\Gizmo\\Gizmo.mdl",
 22 texture TEXTURE_GIZMO             "Models\\Enemies\\Gizmo\\Gizmo.tex",
 23 model   MODEL_SPAWNERGUN          "Models\\Enemies\\Walker\\SpawnerGun.mdl",
 24 texture TEXTURE_SPAWNERGUN        "Models\\Enemies\\Walker\\SpawnerGun.tex",

// ************** SOUNDS **************
 50 sound   SOUND_SOLDIER_IDLE        "Models\\Enemies\\Walker\\Sounds\\Soldier\\Idle.wav",
 51 sound   SOUND_SOLDIER_SIGHT       "Models\\Enemies\\Walker\\Sounds\\Soldier\\Sight.wav",
 53 sound   SOUND_SOLDIER_FIRE_LASER  "Models\\Enemies\\Walker\\Sounds\\Soldier\\Fire.wav",
 54 sound   SOUND_SOLDIER_DEATH       "Models\\Enemies\\Walker\\Sounds\\Soldier\\Death.wav",
 55 sound   SOUND_SOLDIER_WALK        "Models\\Enemies\\Walker\\Sounds\\Soldier\\Walk.wav",
 56 sound   SOUND_SOLDIER_TELEPORT    "Models\\Enemies\\Walker\\Sounds\\Soldier\\Teleport.wav", // [Cecil] Rev

 60 sound   SOUND_SERGEANT_IDLE        "Models\\Enemies\\Walker\\Sounds\\Sergeant\\Idle.wav",
 61 sound   SOUND_SERGEANT_SIGHT       "Models\\Enemies\\Walker\\Sounds\\Sergeant\\Sight.wav",
 63 sound   SOUND_SERGEANT_FIRE_ROCKET "Models\\Enemies\\Walker\\Sounds\\Sergeant\\Fire.wav",
 64 sound   SOUND_SERGEANT_DEATH       "Models\\Enemies\\Walker\\Sounds\\Sergeant\\Death.wav",
 65 sound   SOUND_SERGEANT_WALK        "Models\\Enemies\\Walker\\Sounds\\Sergeant\\Walk.wav",

 // [Cecil] Rev: New sounds
 70 sound   SOUND_MONSTER_IDLE        "Models\\Enemies\\Walker\\Sounds\\Monster\\Idle.wav",
 71 sound   SOUND_MONSTER_SIGHT       "Models\\Enemies\\Walker\\Sounds\\Monster\\Sight.wav",
 72 sound   SOUND_MONSTER_FIRE        "Models\\Enemies\\Walker\\Sounds\\Monster\\Fire.wav",
 73 sound   SOUND_MONSTER_DEATH       "Models\\Enemies\\Walker\\Sounds\\Monster\\Death.wav",
 74 sound   SOUND_MONSTER_WALK        "Models\\Enemies\\Walker\\Sounds\\Monster\\Walk.wav",
 75 sound   SOUND_GIZMO_IDLE          "Models\\Enemies\\Gizmo\\Sounds\\Idle.wav",
 76 sound   SOUND_GIZMO_JUMP          "Models\\Enemies\\Gizmo\\Sounds\\Jump.wav",
 77 sound   SOUND_GIZMO_DEATH_JUMP    "Models\\Enemies\\Gizmo\\Sounds\\JumpDeath.wav",
 78 sound   SOUND_GIZMO_SIGHT         "Models\\Enemies\\Gizmo\\Sounds\\Sight.wav",

 /*
 70 model   MODEL_WALKER_HEAD1   "Models\\Enemies\\Walker\\Debris\\Head.mdl",
 71 model   MODEL_WALKER_HEAD2   "Models\\Enemies\\Walker\\Debris\\Head2.mdl",
 72 model   MODEL_WALKER_LEG     "Models\\Enemies\\Walker\\Debris\\Leg.mdl",
 */

functions:
  // describe how this enemy killed player
  virtual CTString GetPlayerKillDescription(const CTString &strPlayerName, const EDeath &eDeath)
  {
    CTString str;
    str.PrintF(LOCALIZE("A Biomech blew %s away"), strPlayerName);
    return str;
  }

  virtual const CTFileName &GetComputerMessageName(void) const {
    static DECLARE_CTFILENAME(fnmSoldier,  "Data\\Messages\\Enemies\\WalkerSmall.txt");
    static DECLARE_CTFILENAME(fnmSergeant, "Data\\Messages\\Enemies\\WalkerBig.txt");
    static DECLARE_CTFILENAME(fnmMonster,  "Data\\Messages\\Enemies\\WalkerLarge.txt");
    static DECLARE_CTFILENAME(fnmSpawner,  "Data\\Messages\\Enemies\\WalkerSpawner.txt");
    switch(m_EwcChar) {
    default: ASSERT(FALSE);
    case WLC_SOLDIER:  return fnmSoldier;
    case WLC_SERGEANT: return fnmSergeant;
    // [Cecil] Rev: New types
    case WLC_MONSTER:  return fnmMonster;
    case WLC_SPAWNER:  return fnmSpawner;
    }
  }
  // overridable function to get range for switching to another player
  FLOAT GetThreatDistance(void)
  {
    return m_fThreatDistance;
  }

  BOOL ForcesCannonballToExplode(void)
  {
    // [Cecil] Rev: For Monster too
    if (m_EwcChar == WLC_SERGEANT || m_EwcChar == WLC_MONSTER) {
      return TRUE;
    }
    return CEnemyBase::ForcesCannonballToExplode();
  }

  void Precache(void) {
    CEnemyBase::Precache();

    PrecacheModel(MODEL_WALKER);

    switch (m_EwcChar) {
      case WLC_SOLDIER:
        PrecacheSound(SOUND_SOLDIER_IDLE);
        PrecacheSound(SOUND_SOLDIER_SIGHT);
        PrecacheSound(SOUND_SOLDIER_DEATH);
        PrecacheSound(SOUND_SOLDIER_FIRE_LASER);
        PrecacheSound(SOUND_SOLDIER_WALK);

        PrecacheTexture(TEXTURE_WALKER_SOLDIER);
        PrecacheModel(MODEL_LASER);
        PrecacheTexture(TEXTURE_LASER);

        PrecacheClass(CLASS_PROJECTILE, PRT_CYBORG_LASER);
        break;

      case WLC_SERGEANT:
        PrecacheSound(SOUND_SERGEANT_IDLE);
        PrecacheSound(SOUND_SERGEANT_SIGHT);
        PrecacheSound(SOUND_SERGEANT_DEATH);
        PrecacheSound(SOUND_SERGEANT_FIRE_ROCKET);
        PrecacheSound(SOUND_SERGEANT_WALK);

        PrecacheTexture(TEXTURE_WALKER_SERGEANT);
        PrecacheModel(MODEL_ROCKETLAUNCHER);
        PrecacheTexture(TEXTURE_ROCKETLAUNCHER);

        PrecacheClass(CLASS_PROJECTILE, PRT_WALKER_ROCKET);
        break;

      // [Cecil] Rev: New types
      case WLC_MONSTER:
        PrecacheSound(SOUND_MONSTER_IDLE);
        PrecacheSound(SOUND_MONSTER_SIGHT);
        PrecacheSound(SOUND_MONSTER_DEATH);
        PrecacheSound(SOUND_MONSTER_FIRE);
        PrecacheSound(SOUND_MONSTER_WALK);

        PrecacheTexture(TEXTURE_WALKER_MONSTER);
        PrecacheModel(MODEL_CANNON);
        PrecacheTexture(TEXTURE_CANNON);

        PrecacheClass(CLASS_CANNONBALL, CBT_IRON);
        break;

      case WLC_SPAWNER:
        PrecacheSound(SOUND_SOLDIER_IDLE);
        PrecacheSound(SOUND_SOLDIER_SIGHT);
        PrecacheSound(SOUND_SOLDIER_DEATH);
        PrecacheSound(SOUND_SOLDIER_TELEPORT);
        PrecacheSound(SOUND_SOLDIER_WALK);

        PrecacheSound(SOUND_GIZMO_JUMP);
        PrecacheSound(SOUND_GIZMO_IDLE);
        PrecacheSound(SOUND_GIZMO_DEATH_JUMP);
        PrecacheSound(SOUND_GIZMO_SIGHT);

        PrecacheTexture(TEXTURE_WALKER_SPAWNER);
        PrecacheModel(MODEL_SPAWNERGUN);
        PrecacheTexture(TEXTURE_SPAWNERGUN);
        PrecacheModel(MODEL_GIZMO);
        PrecacheTexture(TEXTURE_GIZMO);
        break;
    }
  };

  /* Entity info */
  void *GetEntityInfo(void) {
    return &eiWalker;
  };

  FLOAT GetCrushHealth(void)
  {
    // [Cecil] Rev: For Monster too
    if (m_EwcChar == WLC_SERGEANT || m_EwcChar == WLC_MONSTER) {
      return 100.0f;
    }
    return 0.0f;
  }

  /* Receive damage */
  void ReceiveDamage(CEntity *penInflictor, enum DamageType dmtType,
    FLOAT fDamageAmmount, const FLOAT3D &vHitPoint, const FLOAT3D &vDirection) 
  {

    // take less damage from heavy bullets (e.g. sniper)
    if(dmtType==DMT_BULLET && fDamageAmmount>100.0f)
    {
      fDamageAmmount*=0.666f;
    }

    // walker can't harm walker
    if (!IsOfClass(penInflictor, "Walker") ||
      ((CWalker*)penInflictor)->m_EwcChar!=m_EwcChar) {
      CEnemyBase::ReceiveDamage(penInflictor, dmtType, fDamageAmmount, vHitPoint, vDirection);
    }
  };

  // [Cecil] Rev: Receive notifications from dead Gizmos
  BOOL HandleEvent(const CEntityEvent &ee) {
    if (ee.ee_slEvent == EVENTCODE_EStopAttack) {
      m_ctLiveGizmos = ClampDn(INDEX(m_ctLiveGizmos - 1), (INDEX)0);
      return TRUE;
    }

    return CEnemyBase::HandleEvent(ee);
  };

  // virtual anim functions
  void StandingAnim(void) {
    DeactivateWalkingSound();
    StartModelAnim(WALKER_ANIM_STAND01, AOF_LOOPING|AOF_NORESTART);
  };
  void StandingAnimFight(void)
  {
    DeactivateWalkingSound();
    StartModelAnim(WALKER_ANIM_IDLEFIGHT, AOF_LOOPING|AOF_NORESTART);
  }
  void WalkingAnim(void) {
    ActivateWalkingSound();

    // [Cecil] Rev: For Monster too
    if (m_EwcChar == WLC_SERGEANT || m_EwcChar == WLC_MONSTER) {
      StartModelAnim(WALKER_ANIM_WALKBIG, AOF_LOOPING|AOF_NORESTART);
    } else {
      StartModelAnim(WALKER_ANIM_WALK, AOF_LOOPING|AOF_NORESTART);
    }
  };
  void RunningAnim(void) {
    WalkingAnim();
  };
  void RotatingAnim(void) {
    WalkingAnim();
  };

  // virtual sound functions
  void IdleSound(void) {
    PlaySound(m_soSound, WALKERSOUND(IDLE), SOF_3D);
  };
  void SightSound(void) {
    PlaySound(m_soSound, WALKERSOUND(SIGHT), SOF_3D);
  };
  void DeathSound(void) {
    PlaySound(m_soSound, WALKERSOUND(DEATH), SOF_3D);
  };

  // walking sounds
  void ActivateWalkingSound(void)
  {
    if (!m_bWalkSoundPlaying) {
      PlaySound(m_soFeet, WALKERSOUND(WALK), SOF_3D|SOF_LOOP);
      m_bWalkSoundPlaying = TRUE;
    }
  }
  void DeactivateWalkingSound(void)
  {
    m_soFeet.Stop();
    m_bWalkSoundPlaying = FALSE;
  }

  // fire death rocket
  void FireDeathRocket(const FLOAT3D &vPos) {
    CPlacement3D plRocket;
    plRocket.pl_PositionVector = vPos;
    plRocket.pl_OrientationAngle = ANGLE3D(0, -5.0f-FRnd()*10.0f, 0);
    plRocket.RelativeToAbsolute(GetPlacement());
    CEntityPointer penProjectile = CreateEntity(plRocket, CLASS_PROJECTILE);
    ELaunchProjectile eLaunch;
    eLaunch.penLauncher = this;
    eLaunch.prtType = PRT_WALKER_ROCKET;
    penProjectile->Initialize(eLaunch);
  };

  // fire death laser
  void FireDeathLaser(const FLOAT3D &vPos) {
    CPlacement3D plLaser;
    plLaser.pl_PositionVector = vPos;
    plLaser.pl_OrientationAngle = ANGLE3D(0, -5.0f-FRnd()*10.0f, 0);
    plLaser.RelativeToAbsolute(GetPlacement());
    CEntityPointer penProjectile = CreateEntity(plLaser, CLASS_PROJECTILE);
    ELaunchProjectile eLaunch;
    eLaunch.penLauncher = this;
    eLaunch.prtType = PRT_CYBORG_LASER;
    penProjectile->Initialize(eLaunch);
  };

  // [Cecil] Rev: Launch cannonball on death
  void FireDeathCannonball(const FLOAT3D &vPos) {
    CPlacement3D pl(vPos, ANGLE3D(0, -5.0f - FRnd() * 10.0f, 0));
    pl.RelativeToAbsolute(GetPlacement());

    CEntityPointer penBall = CreateEntity(pl, CLASS_CANNONBALL);
    ELaunchCannonBall eLaunch;
    eLaunch.penLauncher = this;
    eLaunch.fLaunchPower = 140.0f;
    eLaunch.cbtType = CBT_IRON;
  #if SE1_GAME != SS_TFE
    eLaunch.fSize = 3.0f;
  #endif
    penBall->Initialize(eLaunch);
  };

  // [Cecil] Rev: Shoot cannonball at the enemy
  void ShootCannonball(FLOAT3D vOffset) {
    FLOAT3D vShooting = GetPlacement().pl_PositionVector + vOffset * GetRotationMatrix();
    FLOAT3D vTarget = m_penEnemy->GetPlacement().pl_PositionVector;
    FLOAT3D vSpeedDest = FLOAT3D(0.0f, 0.0f, 0.0f);
    FLOAT fLaunchSpeed;
    FLOAT fRelativeHdg;

    FLOAT fDistanceFactor = ClampUp((vShooting - vTarget).Length() / 150.0f, 1.0f) - 0.75f;
    FLOAT fPitch = Clamp(fDistanceFactor * 45.0f, 0.0f, 45.0f);

    // calculate parameters for predicted angular launch curve
    EntityInfo *peiTarget = (EntityInfo *)m_penEnemy->GetEntityInfo();
    CalculateAngularLaunchParams(vShooting, peiTarget->vTargetCenter[1] - 6.0f / 3.0f, vTarget, 
      vSpeedDest, fPitch, fLaunchSpeed, fRelativeHdg);

    // target enemy body
    FLOAT3D vShootTarget;
    GetEntityInfoPosition(m_penEnemy, peiTarget->vTargetCenter, vShootTarget);
    // launch
    CPlacement3D pl;
    PrepareFreeFlyingProjectile(pl, vShootTarget, vOffset, ANGLE3D(fRelativeHdg, fPitch, 0));

    // create cannon ball
    CEntityPointer penBall = CreateEntity(pl, CLASS_CANNONBALL);
    // init and launch cannon ball
    ELaunchCannonBall eLaunch;
    eLaunch.penLauncher = this;
    eLaunch.fLaunchPower = fLaunchSpeed;
    eLaunch.cbtType = CBT_IRON;
  #if SE1_GAME != SS_TFE
    eLaunch.fSize = 3.0f;
  #endif
    penBall->Initialize(eLaunch);
  };

  // [Cecil] Rev: Spawn Gizmos on death
  void FireDeathGizmo(const FLOAT3D &vPos) {
    INDEX ctGizmos = 10;

    if (GetSP()->sp_gdGameDifficulty <= CSessionProperties::GD_EASY) {
      ctGizmos = 5;
    }

    while (--ctGizmos >= 0) {
      CEntity *pen = CreateEntity(GetPlacement(), CLASS_GIZMO);
      CGizmo &enGizmo = (CGizmo &)*pen;
      pen->Initialize();

      // [Cecil] Don't give score
      enGizmo.m_iScore = 0;
    }
  };

  // [Cecil] Rev: Spawn one Gizmo
  CEntity *SpawnGizmo(const FLOAT3D &vPos) {
    CPlacement3D pl(vPos, ANGLE3D(0, 0, 0));
    pl.RelativeToAbsolute(GetPlacement());

    CEntity *pen = CreateEntity(pl, CLASS_GIZMO);
    CGizmo &enGizmo = (CGizmo &)*pen;

    enGizmo.m_penDeathTarget = this;
    enGizmo.m_eetDeathType = EET_STOPATTACK;
    pen->Initialize();

    // [Cecil] Don't give score
    enGizmo.m_iScore = 0;

    // Count spawned Gizmo
    m_ctLiveGizmos++;
    return pen;
  };

  // [Cecil] Rev: Set new lock-on-enemy time before firing again (used to be inline for each attack)
  void SetLockOnEnemyTime(void) {
    if (GetSP()->sp_gdGameDifficulty <= CSessionProperties::GD_EASY) {
      switch (m_EwcChar) {
        case WLC_SOLDIER:  m_fLockOnEnemyTime = 0.4f; break;
        case WLC_SERGEANT: m_fLockOnEnemyTime = 1.0f; break;
        case WLC_MONSTER:  m_fLockOnEnemyTime = 0.5f; break;
        case WLC_SPAWNER:  m_fLockOnEnemyTime = 0.5f; break;
      }

    } else {
      switch (m_EwcChar) {
        case WLC_SOLDIER:  m_fLockOnEnemyTime = 0.1f; break;
        case WLC_SERGEANT: m_fLockOnEnemyTime = 0.5f; break;
        case WLC_MONSTER:  m_fLockOnEnemyTime = 0.2f; break;
        case WLC_SPAWNER:  m_fLockOnEnemyTime = 0.2f; break;
      }
    }
  };

  // adjust sound and watcher parameters here if needed
  void EnemyPostInit(void) 
  {
    // set sound default parameters
    m_soSound.Set3DParameters(160.0f, 50.0f, 1.0f, 1.0f);
    m_soFeet.Set3DParameters(160.0f, 50.0f, 1.0f, 1.0f);
    m_soFire1.Set3DParameters(160.0f, 50.0f, 1.0f, 1.0f);
    m_soFire2.Set3DParameters(160.0f, 50.0f, 1.0f, 1.0f);
    m_soFire3.Set3DParameters(160.0f, 50.0f, 1.0f, 1.0f);
    m_soFire4.Set3DParameters(160.0f, 50.0f, 1.0f, 1.0f);
  };

/************************************************************
 *                 BLOW UP FUNCTIONS                        *
 ************************************************************/
  // spawn body parts
/*  void BlowUp(void)
  {
    // get your size
    FLOATaabbox3D box;
    GetBoundingBox(box);
    FLOAT fEntitySize = box.Size().MaxNorm();

    FLOAT3D vNormalizedDamage = m_vDamage-m_vDamage*(m_fBlowUpAmount/m_vDamage.Length());
    vNormalizedDamage /= Sqrt(vNormalizedDamage.Length());

    vNormalizedDamage *= 0.75f;
    FLOAT3D vBodySpeed = en_vCurrentTranslationAbsolute-en_vGravityDir*(en_vGravityDir%en_vCurrentTranslationAbsolute);

    // spawn debris
    Debris_Begin(EIBT_FLESH, DPT_NONE, BET_NONE, fEntitySize, vNormalizedDamage, vBodySpeed, 5.0f, 2.0f);
    Debris_Spawn(this, this, MODEL_WALKER_HEAD1, TEXTURE_WALKER_SOLDIER, 0, 0, 0, 0, 0.1250f,
      FLOAT3D(FRnd()*0.6f+0.2f, FRnd()*0.6f+0.2f, FRnd()*0.6f+0.2f));
    Debris_Spawn(this, this, MODEL_WALKER_HEAD2, TEXTURE_WALKER_SOLDIER, 0, 0, 0, 0, 0.125f,
      FLOAT3D(FRnd()*0.6f+0.2f, FRnd()*0.6f+0.2f, FRnd()*0.6f+0.2f));
    Debris_Spawn(this, this, MODEL_WALKER_LEG, TEXTURE_WALKER_SOLDIER, 0, 0, 0, 0, 0.125f,
      FLOAT3D(FRnd()*0.6f+0.2f, FRnd()*0.6f+0.2f, FRnd()*0.6f+0.2f));
    Debris_Spawn(this, this, MODEL_WALKER_LEG, TEXTURE_WALKER_SOLDIER, 0, 0, 0, 0, 0.125f,
      FLOAT3D(FRnd()*0.6f+0.2f, FRnd()*0.6f+0.2f, FRnd()*0.6f+0.2f));

    // hide yourself (must do this after spawning debris)
    SwitchToEditorModel();
    SetPhysicsFlags(EPF_MODEL_IMMATERIAL);
    SetCollisionFlags(ECF_IMMATERIAL);
  };*/

procedures:
/************************************************************
 *                A T T A C K   E N E M Y                   *
 ************************************************************/
  Fire(EVoid) : CEnemyBase::Fire {
    DeactivateWalkingSound();
    // to fire
    StartModelAnim(WALKER_ANIM_TOFIRE, 0);
    m_fLockOnEnemyTime = GetModelObject()->GetAnimLength(WALKER_ANIM_TOFIRE);
    autocall CEnemyBase::LockOnEnemy() EReturn;

    if (m_EwcChar == WLC_SERGEANT) {
      // [Cecil] Rev: Custom amount
      if (m_iCustomFireCounter >= 0) {
        m_iLoopCounter = m_iCustomFireCounter;
      } else {
        m_iLoopCounter = 2;
      }

      while (m_iLoopCounter > 0) {
        if (m_iLoopCounter % 2) {
          StartModelAnim(WALKER_ANIM_FIRELEFT, AOF_LOOPING);
          ShootProjectile(PRT_WALKER_ROCKET, FIRE_LEFT_ARM * m_fSize, ANGLE3D(0, 0, 0));
        } else {
          StartModelAnim(WALKER_ANIM_FIRERIGHT, AOF_LOOPING);
          ShootProjectile(PRT_WALKER_ROCKET, FIRE_RIGHT_ARM * m_fSize, ANGLE3D(0, 0, 0));
        }

        INDEX iChannel = m_iLoopCounter % 4;
        PlaySound((&m_soFire1)[iChannel], SOUND_SERGEANT_FIRE_ROCKET, SOF_3D);

        if (m_iLoopCounter > 1) {
          SetLockOnEnemyTime(); // [Cecil]
          autocall CEnemyBase::LockOnEnemy() EReturn;
        }

        m_iLoopCounter--;
      }

    } else if (m_EwcChar == WLC_SOLDIER) {
      // [Cecil] Rev: Custom amount
      if (m_iCustomFireCounter >= 0) {
        m_iLoopCounter = m_iCustomFireCounter;
      } else {
        m_iLoopCounter = 8;
      }

      // [Cecil] Rev: Half any amount
      if (GetSP()->sp_gdGameDifficulty <= CSessionProperties::GD_EASY) {
        m_iLoopCounter /= 2;
      }

      while (m_iLoopCounter > 0) {
        if (m_iLoopCounter % 2) {
          StartModelAnim(WALKER_ANIM_FIRELEFT, AOF_LOOPING);
          ShootProjectile(PRT_CYBORG_LASER, FIRE_LEFT_ARM * m_fSize, ANGLE3D(0, 0, 0));
        } else {
          StartModelAnim(WALKER_ANIM_FIRERIGHT, AOF_LOOPING);
          ShootProjectile(PRT_CYBORG_LASER, FIRE_RIGHT_ARM * m_fSize, ANGLE3D(0, 0, 0));
        }

        INDEX iChannel = m_iLoopCounter % 4;
        PlaySound((&m_soFire1)[iChannel], SOUND_SOLDIER_FIRE_LASER, SOF_3D);

        if (m_iLoopCounter > 1) {
          SetLockOnEnemyTime(); // [Cecil]
          autocall CEnemyBase::LockOnEnemy() EReturn;
        }

        m_iLoopCounter--;
      }

    // [Cecil] Rev: Shoot cannonballs
    } else if (m_EwcChar == WLC_MONSTER) {
      m_iLoopCounter = 2;

      while (m_iLoopCounter > 0) {
        if (m_iLoopCounter % 2) {
          StartModelAnim(WALKER_ANIM_FIRELEFT, AOF_LOOPING);
          ShootCannonball(FIRE_LEFT_ARM * m_fSize);
        } else {
          StartModelAnim(WALKER_ANIM_FIRERIGHT, AOF_LOOPING);
          ShootCannonball(FIRE_RIGHT_ARM * m_fSize);
        }

        INDEX iChannel = m_iLoopCounter % 4;
        PlaySound((&m_soFire1)[iChannel], SOUND_MONSTER_FIRE, SOF_3D);

        if (m_iLoopCounter > 1) {
          SetLockOnEnemyTime(); // [Cecil]
          autocall CEnemyBase::LockOnEnemy() EReturn;
        }

        m_iLoopCounter--;
      }

    // [Cecil] Rev: Spawn Gizmos
    } else if (m_EwcChar == WLC_SPAWNER) {
      if (GetSP()->sp_gdGameDifficulty <= CSessionProperties::GD_EASY) {
        m_iLoopCounter = 4;
      } else {
        m_iLoopCounter = 6;
      }

      while (m_ctLiveGizmos < m_ctMaxGizmos && m_iLoopCounter > 0) {
        if (m_iLoopCounter % 2) {
          StartModelAnim(WALKER_ANIM_FIRELEFT, AOF_LOOPING);
          SpawnGizmo(FIRE_LEFT_ARM * m_fSize);
        } else {
          StartModelAnim(WALKER_ANIM_FIRERIGHT, AOF_LOOPING);
          SpawnGizmo(FIRE_RIGHT_ARM * m_fSize);
        }

        INDEX iChannel = m_iLoopCounter % 4;
        PlaySound((&m_soFire1)[iChannel], SOUND_SOLDIER_TELEPORT, SOF_3D);

        if (m_iLoopCounter > 1) {
          SetLockOnEnemyTime(); // [Cecil]
          autocall CEnemyBase::LockOnEnemy() EReturn;
        }

        m_iLoopCounter--;
      }
    }

    StopMoving();

    MaybeSwitchToAnotherPlayer();

    // from fire
    StartModelAnim(WALKER_ANIM_FROMFIRE, 0);
    autowait(GetModelObject()->GetAnimLength(WALKER_ANIM_FROMFIRE));

    // wait for a while
    StandingAnimFight();
    autowait(FRnd()*0.1f+0.1f);

    return EReturn();
  };



/************************************************************
 *                    D  E  A  T  H                         *
 ************************************************************/
  Death(EVoid) : CEnemyBase::Death {
    // stop moving
    StopMoving();
    DeathSound();     // death sound
    DeactivateWalkingSound();

    // set physic flags
    SetCollisionFlags(ECF_MODEL);
    SetFlags(GetFlags() | ENF_SEETHROUGH);

    // death notify (change collision box)
    ChangeCollisionBoxIndexWhenPossible(WALKER_COLLISION_BOX_DEATH);

    // start death anim
    StartModelAnim(WALKER_ANIM_DEATH, 0);
    autowait(0.9f);

    // [Cecil] Rev: Pick left or right position at random
    const FLOAT3D vFire = ((IRnd() % 2 == 0) ? FIRE_DEATH_RIGHT : FIRE_DEATH_LEFT) * m_fSize;

    // [Cecil] Rev: Take the last shot
    switch (m_EwcChar) {
      case WLC_SOLDIER:
        FireDeathLaser(vFire);
        PlaySound(m_soFire2, SOUND_SOLDIER_FIRE_LASER, SOF_3D);
        break;

      case WLC_SERGEANT:
        FireDeathRocket(vFire);
        PlaySound(m_soSound, SOUND_SERGEANT_FIRE_ROCKET, SOF_3D);
        break;

      case WLC_MONSTER:
        FireDeathCannonball(vFire);
        PlaySound(m_soFire2, SOUND_MONSTER_FIRE, SOF_3D);
        break;

      case WLC_SPAWNER:
        FireDeathGizmo(vFire);
        PlaySound(m_soFire2, SOUND_SOLDIER_TELEPORT, SOF_3D);
        break;
    }

    autowait(0.25f);

  #if SE1_GAME != SS_TFE
    // [Cecil] Four times the size for each type
    FLOAT fStretch = m_fSize * 4;

    // spawn dust effect
    CPlacement3D plFX=GetPlacement();
    ESpawnEffect ese;
    ese.colMuliplier = C_WHITE|CT_OPAQUE;
    ese.vStretch = FLOAT3D(1.5,1,1)*fStretch;
    ese.vNormal = FLOAT3D(0,1,0);
    ese.betType = BET_DUST_FALL;
    CPlacement3D plSmoke=plFX;
    plSmoke.pl_PositionVector+=FLOAT3D(0,0.35f*ese.vStretch(2),0);
    CEntityPointer penFX = CreateEntity(plSmoke, CLASS_BASIC_EFFECT);
    penFX->Initialize(ese);
  #endif

    autowait(0.35f);

    // [Cecil] Stop making fuss
    RemoveFromFuss();

    return EEnd();
  };

/************************************************************
 *                       M  A  I  N                         *
 ************************************************************/
  Main(EVoid) {
    // declare yourself as a model
    InitAsModel();
    SetPhysicsFlags(EPF_MODEL_WALKING);
    SetCollisionFlags(ECF_MODEL);
    SetFlags(GetFlags()|ENF_ALIVE);
    en_fDensity = 3000.0f;

    m_sptType = SPT_ELECTRICITY_SPARKS;

    // set your appearance
    SetModel(MODEL_WALKER);

    // [Cecil] For attachment setup
    INDEX iAttL = -1;
    INDEX iAttR = -1;
    FLOAT fAttSize = 1.0f;

    // [Cecil] Simplified setup
    switch (m_EwcChar) {
      case WLC_SOLDIER: {
        m_fSize = 0.5f;
        fAttSize = 0.5f;
        iAttL = WALKER_ATTACHMENT_LASER_LT;
        iAttR = WALKER_ATTACHMENT_LASER_RT;
        AddAttachment(iAttL, MODEL_LASER, TEXTURE_LASER);
        AddAttachment(iAttR, MODEL_LASER, TEXTURE_LASER);

        SetHealth(150.0f);
        m_iScore = 2000;
        m_fThreatDistance = 5;
      } break;

      case WLC_SERGEANT: {
        m_fSize = 1.0f;
        fAttSize = 1.0f;
        iAttL = WALKER_ATTACHMENT_ROCKETLAUNCHER_LT;
        iAttR = WALKER_ATTACHMENT_ROCKETLAUNCHER_RT;
        AddAttachment(iAttL, MODEL_ROCKETLAUNCHER, TEXTURE_ROCKETLAUNCHER);
        AddAttachment(iAttR, MODEL_ROCKETLAUNCHER, TEXTURE_ROCKETLAUNCHER);

        SetHealth(750.0f);
        m_iScore = 7500;
        m_fThreatDistance = 15;
      } break;

      // [Cecil] Rev: New types
      case WLC_MONSTER: {
        m_fSize = 1.5f;
        fAttSize = 6.0f;
        iAttL = WALKER_ATTACHMENT_CANNON_LT;
        iAttR = WALKER_ATTACHMENT_CANNON_RT;
        AddAttachment(iAttL, MODEL_CANNON, TEXTURE_CANNON);
        AddAttachment(iAttR, MODEL_CANNON, TEXTURE_CANNON);

        SetHealth(1500.0f);
        m_iScore = 10000;
        m_fThreatDistance = 30;
      } break;

      case WLC_SPAWNER: {
        m_fSize = 0.7f;
        fAttSize = 3.0f;
        iAttL = WALKER_ATTACHMENT_ROCKETLAUNCHER_LT;
        iAttR = WALKER_ATTACHMENT_ROCKETLAUNCHER_RT;
        AddAttachment(iAttL, MODEL_SPAWNERGUN, TEXTURE_SPAWNERGUN);
        AddAttachment(iAttR, MODEL_SPAWNERGUN, TEXTURE_SPAWNERGUN);

        SetHealth(500.0f);
        m_iScore = 4500;
        m_fThreatDistance = 10;
      } break;
    }

    // [Cecil] Resize model
    GetModelObject()->StretchModel(FLOAT3D(m_fSize, m_fSize, m_fSize));
    ModelChangeNotify();

    // [Cecil] Resize attachments
    if (iAttL != -1) {
      CModelObject &moLeft = GetModelObject()->GetAttachmentModel(iAttL)->amo_moModelObject;
      moLeft.StretchModel(FLOAT3D(1, 1, 1) * fAttSize);
    }

    if (iAttR != -1) {
      CModelObject &moRight = GetModelObject()->GetAttachmentModel(iAttR)->amo_moModelObject;
      moRight.StretchModel(FLOAT3D(-1, 1, 1) * fAttSize);
    }

    // [Cecil] Texture per type
    switch (m_EwcChar) {
      case WLC_SOLDIER:  SetModelMainTexture(TEXTURE_WALKER_SOLDIER); break;
      case WLC_SERGEANT: SetModelMainTexture(TEXTURE_WALKER_SERGEANT); break;
      case WLC_MONSTER:  SetModelMainTexture(TEXTURE_WALKER_MONSTER); break;
      case WLC_SPAWNER:  SetModelMainTexture(TEXTURE_WALKER_SPAWNER); break;
    }

    if (m_fStepHeight==-1) {
      m_fStepHeight = 4.0f;
    }

    StandingAnim();
    // setup moving speed
    m_fWalkSpeed = FRnd()*1.5f + 9.0f;
    m_aWalkRotateSpeed = AngleDeg(FRnd()*50.0f + 500.0f);
    m_fAttackRunSpeed = m_fWalkSpeed;
    m_aAttackRotateSpeed = m_aWalkRotateSpeed/2;
    m_fCloseRunSpeed = m_fWalkSpeed;
    m_aCloseRotateSpeed = m_aWalkRotateSpeed/2;
    m_fWalkSpeed/=2.0f;
    // setup attack distances
    m_fAttackDistance = 150.0f;
    m_fCloseDistance = 0.0f;
    m_fStopDistance = 15.0f;
    m_fAttackFireTime = 3.0f;
    m_fCloseFireTime = 1.0f;
    m_fIgnoreRange = 300.0f;
    // damage/explode properties
    m_fBodyParts = 8;
    m_fDamageWounded = 100000.0f;

    // [Cecil] Same setup for all types
    m_fBlowUpAmount = 1E10f;
    m_fMaxHealth = GetHealth();

    // continue behavior in base class
    jump CEnemyBase::MainLoop();
  };
};
