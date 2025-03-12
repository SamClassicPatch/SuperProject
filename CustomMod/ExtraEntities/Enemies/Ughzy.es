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

// [Cecil] Rev: Recreation of an entity from Revolution
// [Cecil] NOTE: Some logic has been reworked for the better compared to Revolution and is commented as such
356
%{
#include "StdH.h"
#include "Models/Enemies/Devil/Devil.h"
#include "Effects/Effector.h"
%}

uses "Enemies/EnemyBase";
uses "Weapons/Projectile";
uses "Weapons/Bullet";

enum UghzyType {
  0 UZT_SMALL  "Small",
  1 UZT_NORMAL "Normal",
  2 UZT_BIG    "Big",
};

%{
#define ATT_LASER           (FLOAT3D( 0.63626f,  1.13841f, -0.033062f) * m_fSize)
#define ATT_PROJECTILE_GUN  (FLOAT3D(-0.703544f, 1.12582f, -0.329834f) * m_fSize)
#define ATT_ELECTRICITYGUN  (FLOAT3D(-0.764868f, 1.27992f, -0.311084f) * m_fSize)
#define ATT_ROCKETLAUNCHER  (FLOAT3D( 0.654788f, 1.30318f, -0.259358f) * m_fSize)

#define LASER_PIPE          (FLOAT3D( 0.0172566f,   -0.123152f,    -0.232228f) * m_fSize)
#define PROJECTILEGUN_PIPE  (FLOAT3D( 0.0359023f,   -0.000490744f, -0.394403f) * m_fSize)
#define ELECTROGUN_PIPE     (FLOAT3D(-0.00423616f,  -0.0216781f,   -0.506613f) * m_fSize)
#define ROCKETLAUNCHER_PIPE (FLOAT3D( 4.68194e-005f, 0.0483391f,   -0.475134f) * m_fSize)

// [Cecil] Projectile type per game
#define UGHZY_PROJECTILE_TYPE CHOOSE_FOR_GAME(PRT_DRAGONMAN_STRONG_FIRE, PRT_SHOOTER_FIREBALL, PRT_SHOOTER_FIREBALL)

static EntityInfo eiUghzy = {
  EIBT_FLESH, 1000.0f,
  0.0f, 2.0f, 0.0f,
  0.0f, 1.5f, 0.0f,
};
%}

class CUghzy : CEnemyBase {
name      "Ughzy";
thumbnail "Thumbnails\\Ughzy.tbn";

properties:
  1 enum UghzyType m_uztType "Type" 'T' = UZT_NORMAL,
  2 INDEX m_iFiredProjectiles = 0,
  3 INDEX m_iMeleeAnimation = 0,
  4 BOOL m_bRenderElectricity = FALSE,
  // [Cecil] Source property is deprecated because it's not being updated when the enemy is moving
  5 FLOAT3D m_vElectricitySource = FLOAT3D(0, 0, 0),
  6 FLOAT3D m_vElectricityTarget = FLOAT3D(0, 0, 0),
  7 FLOAT m_tmTemp = 0.0f,
  8 FLOAT m_tmNextFXTime = 0.0f,
  9 FLOAT m_fSize = 1.0f,
 10 INDEX m_iAttID = 0,
 20 BOOL m_bDoesMelee       "Uses melee attacks" = TRUE,
 21 BOOL m_bDoesRockets     "Uses rocket launcher" = TRUE,
 22 BOOL m_bDoesLasers      "Uses laser" = TRUE,
 23 BOOL m_bDoesLavabomb    "Uses lava bomb" = TRUE,
 24 BOOL m_bDoesGhostbuster "Uses ghostbuster" = TRUE,

components:
  0 class   CLASS_BASE        "Classes\\EnemyBase.ecl",
  1 class   CLASS_PROJECTILE  "Classes\\Projectile.ecl",
  2 class   CLASS_EFFECTOR    "Classes\\Effector.ecl",

 10 model   MODEL_DEVIL             "Models\\Enemies\\Ughzy\\Devil.mdl",
 11 texture TEXTURE_DEVIL           "Models\\Enemies\\Ughzy\\Devil.tex",
 12 model   MODEL_LASER             "Models\\Enemies\\Ughzy\\Weapons\\Laser.mdl",
 13 texture TEXTURE_LASER           "Models\\Enemies\\Ughzy\\Weapons\\Laser.tex",
 14 model   MODEL_ROCKETLAUNCHER    "Models\\Enemies\\Ughzy\\Weapons\\RocketLauncher.mdl",
 15 texture TEXTURE_ROCKETLAUNCHER  "Models\\Enemies\\Ughzy\\Weapons\\RocketLauncher.tex",
 16 model   MODEL_PROJECTILEGUN     "Models\\Enemies\\Ughzy\\Weapons\\ProjectileGun.mdl",
 17 texture TEXTURE_PROJECTILEGUN   "Models\\Enemies\\Ughzy\\Weapons\\ProjectileGun.tex",
 18 model   MODEL_ELECTRICITYGUN    "Models\\Enemies\\Ughzy\\Weapons\\ElectricityGun.mdl",
 19 texture TEXTURE_ELECTRICITYGUN  "Models\\Enemies\\Ughzy\\Weapons\\ElectricityGun.tex",

 50 sound   SOUND_ANGER01               "Models\\Enemies\\Ughzy\\Sounds\\Anger01.wav",
 51 sound   SOUND_ANGER02               "Models\\Enemies\\Ughzy\\Sounds\\Anger02.wav",
 52 sound   SOUND_ATTACKCLOSE           "Models\\Enemies\\Ughzy\\Sounds\\AttackClose.wav",
 53 sound   SOUND_CELEBRATE01           "Models\\Enemies\\Ughzy\\Sounds\\Celebrate01.wav",
 55 sound   SOUND_DEATH                 "Models\\Enemies\\Ughzy\\Sounds\\Death.wav",
 56 sound   SOUND_DRAW_LOWER_WEAPONS    "Models\\Enemies\\Ughzy\\Sounds\\GrabWeaponsLower.wav",
 57 sound   SOUND_DRAW_UPPER_WEAPONS    "Models\\Enemies\\Ughzy\\Sounds\\GrabWeaponsUpper.wav",
 58 sound   SOUND_GETUP                 "Models\\Enemies\\Ughzy\\Sounds\\Getup.wav",
 59 sound   SOUND_IDLE                  "Models\\Enemies\\Ughzy\\Sounds\\Idle.wav",
 60 sound   SOUND_PUNCH                 "Models\\Enemies\\Ughzy\\Sounds\\Punch.wav",
 61 sound   SOUND_SMASH                 "Models\\Enemies\\Ughzy\\Sounds\\Smash.wav",
 62 sound   SOUND_WALK_LEFT             "Models\\Enemies\\Ughzy\\Sounds\\WalkL.wav",
 63 sound   SOUND_WALK_RIGHT            "Models\\Enemies\\Ughzy\\Sounds\\WalkR.wav",
 64 sound   SOUND_WOUND                 "Models\\Enemies\\Ughzy\\Sounds\\Wound.wav",
 65 sound   SOUND_ATTACK_BREATH_START   "Models\\Enemies\\Ughzy\\Sounds\\AttackBreathStart.wav",
 66 sound   SOUND_ATTACK_BREATH_FIRE    "Models\\Enemies\\Ughzy\\Sounds\\BreathProjectile.wav",
 67 sound   SOUND_ATTACK_BREATH_END     "Models\\Enemies\\Ughzy\\Sounds\\AttackBreathEnd.wav",
 68 sound   SOUND_HEAL                  "Models\\Enemies\\Ughzy\\Sounds\\Heal.wav",
 69 sound   SOUND_ROCKETLAUNCHER        "Models\\Enemies\\Ughzy\\Sounds\\RocketLauncher.wav",
 70 sound   SOUND_LASER                 "Models\\Enemies\\Ughzy\\Sounds\\Laser.wav",
 71 sound   SOUND_LAVABOMB              "Models\\Enemies\\Ughzy\\Sounds\\LavaBomb.wav",
 72 sound   SOUND_GHOSTBUSTER           "Models\\Enemies\\Ughzy\\Sounds\\Ghostbuster.wav",
 73 sound   SOUND_ATTACK_BREATH_LOOP    "Models\\Enemies\\Ughzy\\Sounds\\AttackBreath.wav",
 74 sound   SOUND_CLIMB                 "Models\\Enemies\\Ughzy\\Sounds\\Enter.wav",
 75 sound   SOUND_DEATHPARTICLES        "Models\\Enemies\\Ughzy\\Sounds\\DeathParticles.wav",
 76 sound   SOUND_DISAPPEAR             "Models\\Enemies\\Ughzy\\Sounds\\Disappear.wav",

functions:
  void Precache(void) {
    CEnemyBase::Precache();

    PrecacheModel(MODEL_DEVIL);
    PrecacheTexture(TEXTURE_DEVIL);

    PrecacheModel(MODEL_LASER);
    PrecacheTexture(TEXTURE_LASER);
    PrecacheModel(MODEL_ROCKETLAUNCHER);
    PrecacheTexture(TEXTURE_ROCKETLAUNCHER);
    PrecacheModel(MODEL_ELECTRICITYGUN);
    PrecacheTexture(TEXTURE_ELECTRICITYGUN);
    PrecacheModel(MODEL_PROJECTILEGUN);
    PrecacheTexture(TEXTURE_PROJECTILEGUN);

    PrecacheClass(CLASS_PROJECTILE, UGHZY_PROJECTILE_TYPE);
    PrecacheClass(CLASS_PROJECTILE, PRT_CYBORG_LASER);
    PrecacheClass(CLASS_PROJECTILE, PRT_WALKER_ROCKET);

    PrecacheSound(SOUND_ANGER01);
    PrecacheSound(SOUND_ANGER02);
    PrecacheSound(SOUND_ATTACKCLOSE);
    PrecacheSound(SOUND_CELEBRATE01);
    PrecacheSound(SOUND_DEATH);
    PrecacheSound(SOUND_DRAW_LOWER_WEAPONS);
    PrecacheSound(SOUND_DRAW_UPPER_WEAPONS);
    PrecacheSound(SOUND_GETUP);
    PrecacheSound(SOUND_IDLE);
    PrecacheSound(SOUND_PUNCH);
    PrecacheSound(SOUND_SMASH);
    PrecacheSound(SOUND_WALK_LEFT);
    PrecacheSound(SOUND_WALK_RIGHT);
    PrecacheSound(SOUND_WOUND);
    PrecacheSound(SOUND_ATTACK_BREATH_START);
    PrecacheSound(SOUND_ATTACK_BREATH_FIRE);
    PrecacheSound(SOUND_ATTACK_BREATH_END);
    PrecacheSound(SOUND_HEAL);
    PrecacheSound(SOUND_ROCKETLAUNCHER);
    PrecacheSound(SOUND_LASER);
    PrecacheSound(SOUND_LAVABOMB);
    PrecacheSound(SOUND_GHOSTBUSTER);
    PrecacheSound(SOUND_ATTACK_BREATH_LOOP);
    PrecacheSound(SOUND_CLIMB);
    PrecacheSound(SOUND_DEATHPARTICLES);    
    PrecacheSound(SOUND_DISAPPEAR);
  };

  virtual CTString GetPlayerKillDescription(const CTString &strPlayerName, const EDeath &eDeath) {
    CTString str;
    str.PrintF(TRANS("Ughzy killed %s"), strPlayerName);
    return str;
  };

  virtual const CTFileName &GetComputerMessageName(void) const {
    static DECLARE_CTFILENAME(fnm, "Data\\Messages\\Enemies\\DevilSmall.txt");
    return fnm;
  };

  void *GetEntityInfo(void) {
    return &eiUghzy;
  };

  BOOL ForcesCannonballToExplode(void) {
    return TRUE;
  };

  void RenderParticles(void) {
    if (m_bRenderElectricity) {
      // [Cecil] Get firing position relative to the current lerped position
      CPlacement3D plRelative(GetFiringPositionRelative(), ANGLE3D(0, 0, 0));
      plRelative.RelativeToAbsoluteSmooth(GetLerpedPlacement());

      Particles_Ghostbuster(plRelative.pl_PositionVector, m_vElectricityTarget, 24, 1.0f, 2.0f, 96.0f);
    }

    CEnemyBase::RenderParticles();
  };
  
  FLOAT3D GetFiringPositionRelative(void) {
    FLOAT3D vWeaponPipe = FLOAT3D(0, 0, 0);
    FLOAT3D vAttachment = FLOAT3D(0, 0, 0);

    switch (m_iAttID) {
      case DEVIL_ATTACHMENT_LASER:
        vWeaponPipe = LASER_PIPE;
        vAttachment = ATT_LASER;
        break;

      case DEVIL_ATTACHMENT_PROJECTILEGUN:
        vWeaponPipe = PROJECTILEGUN_PIPE;
        vAttachment = ATT_PROJECTILE_GUN;
        break;

      case DEVIL_ATTACHMENT_ELETRICITYGUN:
        vWeaponPipe = ELECTROGUN_PIPE;
        vAttachment = ATT_ELECTRICITYGUN;
        break;

      case DEVIL_ATTACHMENT_ROCKETLAUNCHER:
        vWeaponPipe = ROCKETLAUNCHER_PIPE;
        vAttachment = ATT_ROCKETLAUNCHER;
        break;

      default: ASSERTALWAYS("Invalid attachment ID");
    }

    CAttachmentModelObject *pamo = GetModelObject()->GetAttachmentModel(m_iAttID);

    FLOATmatrix3D mWeapon;
    MakeRotationMatrixFast(mWeapon, pamo->amo_plRelative.pl_OrientationAngle);

    return (vAttachment + vWeaponPipe * mWeapon);
  };

  void ReceiveDamage(CEntity *penInflictor, enum DamageType dmtType,
    FLOAT fDamageAmmount, const FLOAT3D &vHitPoint, const FLOAT3D &vDirection) 
  {
    // Don't hurt each other
    if (IsOfClass(penInflictor, "Ughzy")) {
      return;
    }

    CEnemyBase::ReceiveDamage(penInflictor, dmtType, fDamageAmmount, vHitPoint, vDirection);
  };

  INDEX AnimForDamage(FLOAT fDamage) {
    StartModelAnim(DEVIL_ANIM_WOUNDEND, 0);
    return DEVIL_ANIM_WOUNDEND;
  };

  INDEX AnimForDeath(void) {
    StartModelAnim(DEVIL_ANIM_DEATH, 0);
    return DEVIL_ANIM_DEATH;
  };

  void DeathNotify(void) {
    // [Cecil] Remove electricity beam on death
    m_bRenderElectricity = FALSE;

    // Corpse becomes an obstacle
    SetCollisionFlags(ECF_MODEL);
  };

  // Animation functions
  void StandingAnim(void) {
    StartModelAnim(DEVIL_ANIM_IDLE, AOF_LOOPING|AOF_NORESTART);
  };

  void WalkingAnim(void) {
    StartModelAnim(DEVIL_ANIM_WALK, AOF_LOOPING|AOF_NORESTART);
  };

  void RunningAnim(void) {
    WalkingAnim();
  };

  void RotatingAnim(void) {
    WalkingAnim();
  };

  // Sound functions
  void IdleSound(void) {
    PlaySound(m_soSound, SOUND_IDLE, SOF_3D);
  };

  void SightSound(void) {
    //PlaySound(m_soSound, SOUND_SIGHT, SOF_3D);
  };

  void WoundSound(void) {
    PlaySound(m_soSound, SOUND_WOUND, SOF_3D);
  };

  void DeathSound(void) {
    PlaySound(m_soSound, SOUND_DEATH, SOF_3D);
  };

  // Check if using any weapons
  BOOL UsesWeapons(void) {
    return m_bDoesLasers || m_bDoesLavabomb || m_bDoesRockets || m_bDoesGhostbuster;
  };

  // Check which weapon can be used
  BOOL IsWeaponAvailable(INDEX iWeapon) {
    // [Cecil] Using attachments instead of arbitrary indices (0, 1, 2, 3 for Rockets, Lasers, Lavabomb, Ghostbuster)
    switch (iWeapon) {
      case DEVIL_ATTACHMENT_LASER:          return m_bDoesLasers;
      case DEVIL_ATTACHMENT_PROJECTILEGUN:  return m_bDoesLavabomb;
      case DEVIL_ATTACHMENT_ELETRICITYGUN:  return m_bDoesGhostbuster;
      case DEVIL_ATTACHMENT_ROCKETLAUNCHER: return m_bDoesRockets;
    }

    return FALSE;
  };

  // [Cecil] Projectile shooting for a specific weapon that used to be inline for each attack
  void UghzyProjectile(ProjectileType eType, INDEX iAnim, INDEX iSound) {
    StartModelAnim(iAnim, 0);

    ShootProjectile(eType, GetFiringPositionRelative(), ANGLE3D(0, 0, 0));
    PlaySound(m_soSound, iSound, SOF_3D);

    m_iFiredProjectiles++;
  };

  // [Cecil] Calculate target position from the current enemy that used to be done inline for ghostbuster beam
  FLOAT3D GetEnemyTargetPos(void) {
    FLOAT3D vTarget(0, 0, 0);

    if (m_penEnemy != NULL) {
      EntityInfo *peiTarget = (EntityInfo *)m_penEnemy->GetEntityInfo();
      GetEntityInfoPosition(m_penEnemy, peiTarget->vTargetCenter, vTarget);
    }

    return vTarget;
  };

  // [Cecil] Fire ghostbuster ray once that used to be done inline
  void SpawnGhostbusterRay(void) {
    const FLOAT3D vSource = GetPlacement().pl_PositionVector + GetFiringPositionRelative() * GetRotationMatrix();

    // Shoot a ray at some target position
    CCastRay crRay(this, vSource, m_vElectricityTarget);
    crRay.cr_bHitTranslucentPortals = FALSE;
    crRay.cr_bPhysical = FALSE;
    crRay.cr_ttHitModels = CCastRay::TT_COLLISIONBOX;
    GetWorld()->CastRay(crRay);

    // Apply damage to detected entity
    if (crRay.cr_penHit != NULL) {
      InflictDirectDamage(crRay.cr_penHit, this, DMT_BULLET, 50.0f * _pTimer->TickQuantum / 0.5f,
        crRay.cr_vHit, (vSource - m_vElectricityTarget).SafeNormalize());
    }

    // If time has passed until the next effect
    if (_pTimer->CurrentTick() > m_tmNextFXTime) {
      // Schedule the next ground hit
      m_tmNextFXTime = _pTimer->CurrentTick() + 0.125f + FRnd() * 0.125f;

      // Create the current one
      CEntity *penEffector = CreateEntity(CPlacement3D(m_vElectricityTarget, ANGLE3D(0, 0, 0)), CLASS_EFFECTOR);

      ESpawnEffector eSpawnEffector;
      eSpawnEffector.tmLifeTime = 6.0f;
      eSpawnEffector.fSize = 0.025f;
      eSpawnEffector.eetType = ET_HIT_GROUND;
      eSpawnEffector.vDamageDir = FLOAT3D(0.0f, 2.0f, 0.0f);

      penEffector->Initialize(eSpawnEffector);
    }
  };

procedures:
/************************************************************
 *                PROCEDURES WHEN HARMED                    *
 ************************************************************/

  // [Cecil] Remove electricity beam on damage
  BeWounded(EDamage eDamage) : CEnemyBase::BeWounded {
    m_bRenderElectricity = FALSE;
    jump CEnemyBase::BeWounded();
  };

/************************************************************
 *                A T T A C K   E N E M Y                   *
 ************************************************************/

  // Fire various projectiles
  FireRockets() {
    while (m_iFiredProjectiles < 3) {
      UghzyProjectile(PRT_WALKER_ROCKET, DEVIL_ANIM_ATTACK02, SOUND_ROCKETLAUNCHER);
      autowait((FRnd() * 0.2f) + 0.2f);
    }

    MaybeSwitchToAnotherPlayer();
    return EReturn();
  };

  FireLasers() {
    while (m_iFiredProjectiles < 3) {
      UghzyProjectile(PRT_CYBORG_LASER, DEVIL_ANIM_ATTACK01, SOUND_LASER);
      autowait((FRnd() * 0.2f) + 0.2f);
    }

    MaybeSwitchToAnotherPlayer();
    return EReturn();
  };

  FireLavaBomb() {
    while (m_iFiredProjectiles < 3) {
      UghzyProjectile(UGHZY_PROJECTILE_TYPE, DEVIL_ANIM_ATTACK01, SOUND_LAVABOMB);
      autowait((FRnd() * 0.2f) + 0.2f);
    }

    MaybeSwitchToAnotherPlayer();
    return EReturn();
  };

  // Fire a ghostbuster beam
  FireGhostbuster() {
    StartModelAnim(DEVIL_ANIM_FROMIDLETOATTACK02, 0);
    autowait(GetModelObject()->GetAnimLength(DEVIL_ANIM_FROMIDLETOATTACK02) - 0.1f);

    while (m_iFiredProjectiles < 3) {
      StartModelAnim(DEVIL_ANIM_ATTACK02, AOF_LOOPING|AOF_NORESTART);

      // Ray target position at the beginning
      m_vElectricityTarget = GetEnemyTargetPos();

      autowait(0.4f);

      // Start shooting the beam
      m_bRenderElectricity = TRUE;
      m_tmTemp = _pTimer->CurrentTick();
      m_tmNextFXTime = m_tmTemp - _pTimer->TickQuantum;
      PlaySound(m_soSound, SOUND_GHOSTBUSTER, SOF_3D);

      while (_pTimer->CurrentTick() < m_tmTemp + 0.75f) {
        // New ray target position for this tick
        FLOAT3D vNewTarget = GetEnemyTargetPos();

        // Adjust the ray position to slowly move towards the target
        const FLOAT3D vDiff = vNewTarget - m_vElectricityTarget;
        const FLOAT fDiffLen = vDiff.Length();

        if (fDiffLen > 1.0f) {
          m_vElectricityTarget += (vDiff / fDiffLen) * 10.0f * _pTimer->TickQuantum;
        }

        // Shoot the physical ray
        SpawnGhostbusterRay();

        // [Cecil] Reworked wait() with ETimer into just autowait(), which is what it really is
        autowait(_pTimer->TickQuantum);
      }

      // Stop shooting the beam
      m_soSound.Stop();
      m_bRenderElectricity = FALSE;

      m_iFiredProjectiles++;
      autowait(0.2f);

      // [Cecil] Stop shooting if the enemy is out of view angle
      if (!IsInFrustum(m_penEnemy, CosFast(30.0f))) {
        m_iFiredProjectiles = 10000;
      }
    }

    StartModelAnim(DEVIL_ANIM_FROMATTACK02TOIDLE, 0);
    autowait(0.4f);

    MaybeSwitchToAnotherPlayer();
    return EReturn();
  };

  MeleeAttack() {
    // Punch or smash
    m_iMeleeAnimation = DEVIL_ANIM_PUNCH + (IRnd() % 2);
    StartModelAnim(m_iMeleeAnimation, 0);

    StopMoving();
    autowait(0.35f);

    FLOAT3D vEnemy = m_penEnemy->GetPlacement().pl_PositionVector;
    FLOAT3D vDirection = (vEnemy - GetPlacement().pl_PositionVector);
    vDirection.SafeNormalize();

    InflictDirectDamage(m_penEnemy, this, DMT_CLOSERANGE, 10.0f, vEnemy, vDirection);
    PlaySound(m_soSound, (m_iMeleeAnimation == DEVIL_ANIM_PUNCH) ? SOUND_PUNCH : SOUND_SMASH, SOF_3D);

    autowait(0.35f);
    return EReturn();
  };

  Fire() : CEnemyBase::Fire {
    // [Cecil] NOTE: No idea why this needs to be here instead of the Hit() procedure
    if ((m_bDoesMelee || !UsesWeapons()) && CalcDist(m_penEnemy) <= 3.5f) {
      StandingAnim();
      autowait(FRnd() * 0.25f + 0.2f);

      jump MeleeAttack();
    }

    // Fire a weapon
    if (UsesWeapons()) {
      StandingAnim();
      autowait(FRnd() * 0.25f + 0.2f);

      // Reset fire counter
      m_iFiredProjectiles = 0;

      // [Cecil] Find a weapon that's available and immediately store it as the attachment index
      do {
        m_iAttID = IRnd() % 4;
      } while (!IsWeaponAvailable(m_iAttID));

      switch (m_iAttID) {
        case DEVIL_ATTACHMENT_LASER:          jump FireLasers();
        case DEVIL_ATTACHMENT_PROJECTILEGUN:  jump FireLavaBomb();
        case DEVIL_ATTACHMENT_ELETRICITYGUN:  jump FireGhostbuster();
        case DEVIL_ATTACHMENT_ROCKETLAUNCHER: jump FireRockets();
      }
    }

    return EReturn();
  };

/************************************************************
 *                       M  A  I  N                         *
 ************************************************************/
  Main() {
    InitAsModel();
    SetPhysicsFlags(EPF_MODEL_WALKING);
    SetCollisionFlags(ECF_MODEL);
    SetFlags(GetFlags()|ENF_ALIVE);

    // Set appearance
    SetComponents(this, *GetModelObject(), MODEL_DEVIL, TEXTURE_DEVIL, 0, 0, 0);

    if (m_bDoesLasers) {
      AddAttachmentToModel(this, *GetModelObject(), DEVIL_ATTACHMENT_LASER, MODEL_LASER, TEXTURE_LASER, 0, 0, 0);
    }
    if (m_bDoesLavabomb) {
      AddAttachmentToModel(this, *GetModelObject(), DEVIL_ATTACHMENT_PROJECTILEGUN, MODEL_PROJECTILEGUN, TEXTURE_PROJECTILEGUN, 0, 0, 0);
    }
    if (m_bDoesRockets) {
      AddAttachmentToModel(this, *GetModelObject(), DEVIL_ATTACHMENT_ROCKETLAUNCHER, MODEL_ROCKETLAUNCHER, TEXTURE_ROCKETLAUNCHER, 0, 0, 0);
    }
    if (m_bDoesGhostbuster) {
      AddAttachmentToModel(this, *GetModelObject(), DEVIL_ATTACHMENT_ELETRICITYGUN, MODEL_ELECTRICITYGUN, TEXTURE_ELECTRICITYGUN, 0, 0, 0);
    }

    // Setup speeds
    m_fWalkSpeed         = (FRnd() * 1.0f) + 1.5f;
    m_aWalkRotateSpeed   = (FRnd() * 10.0f) + 35.0f;
    m_fAttackRunSpeed    = (FRnd() * 2.0f) + 4.0f;
    m_aAttackRotateSpeed = (FRnd() * 50.0f) + 245.0f;
    m_fCloseRunSpeed     = (FRnd() * 2.0f) + 4.0f;
    m_aCloseRotateSpeed  = (FRnd() * 50.0f) + 245.0f;

    // Adjust stop distance if only using weapons
    if (!m_bDoesMelee && UsesWeapons()) {
      m_fStopDistance = (FRnd() * 20.0f) + 10.0f;
    } else {
      m_fStopDistance = 2.5f;
    }

    // Setup attack distances
    m_fAttackDistance = 100.0f;
    m_fCloseDistance = 0.0f;
    m_fAttackFireTime = 0.5f;
    m_fCloseFireTime = 0.5f;
    m_fIgnoreRange = 200.0f;

    switch (m_uztType) {
      case UZT_SMALL:
        SetHealth(250);
        m_fDamageWounded = 150.0f;
        m_iScore = 5000;
        m_fSize = 1.0f;
        break;

      case UZT_NORMAL:
        SetHealth(500);
        m_fDamageWounded = 250.0f;
        m_iScore = 10000;
        m_fSize = 1.5f;
        break;

      case UZT_BIG:
        SetHealth(1250);
        m_fDamageWounded = 800.0f;
        m_iScore = 15000;
        m_fSize = 2.5f;
        break;
    }

    m_fBlowUpAmount = 1e10;
    m_fBodyParts = 6;
    en_fDensity = 3000.0f;
    m_fMaxHealth = GetHealth();

    StandingAnim();

    GetModelObject()->StretchModel(FLOAT3D(m_fSize, m_fSize, m_fSize));
    ModelChangeNotify();

    jump CEnemyBase::MainLoop();
  };
};
