/* Copyright (c) 2024-2025 Dreamy Cecil
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
357
%{
#include "StdH.h"
#include "ModelsMP/Enemies/Demon/Demon.h"
%}

uses "Enemies/EnemyBase";
uses "Effects/BasicEffects";
uses "Weapons/Projectile";

%{
#define CLOSE_ATTACK_RANGE 10.0f
#define DEMON_STRETCH 25.2f

static EntityInfo eiDemon = {
  EIBT_FLESH, 1600.0f,
  0.0f, 2.0f, 0.0f,
  0.0f, 1.5f, 0.0f,
};
%}

class CDestroyer : CEnemyBase {
name      "Destroyer";
thumbnail "Thumbnails\\Demon.tbn";

properties:
  2 INDEX m_iCounter = 0,
  3 CEntityPointer m_penFireFX, // [Cecil] Unused

 10 INDEX m_iAttackRandPrNumber = 0,
 11 INDEX m_iAttackLoopCounter = 0,
 12 INDEX m_iAttackLoopAngle = 0,
 13 INDEX m_iAttackSeries = 0,
 14 INDEX m_iAttackIsCircle = 0,
 15 FLOAT m_fWaitTime1 = 0.0f,
 16 FLOAT m_fWaitTime2 = 0.0f,
 17 FLOAT m_fAttackLoopAngleForPP = 0.0f,
 18 BOOL m_bTextureTurnOn = TRUE,
 20 enum ProjectileType m_ptAttackProjectile = PRT_WOMAN_FIRE,

components:
  0 class   CLASS_BASE          "Classes\\EnemyBase.ecl",
  1 class   CLASS_PROJECTILE    "Classes\\Projectile.ecl",
  2 class   CLASS_BASIC_EFFECT  "Classes\\BasicEffect.ecl",

 10 model   MODEL_DEMON         "ModelsMP\\Enemies\\Demon\\Demon.mdl",
 11 texture TEXTURE_BASE        "Models\\Enemies\\Destroyer\\DestroyerBase.tex",
 12 texture TEXTURE_WOUNDED     "Models\\Enemies\\Destroyer\\DestroyerWounded.tex",

 // ************** SOUNDS **************
 50 sound   SOUND_IDLE      "ModelsMP\\Enemies\\Demon\\Sounds\\Idle.wav",
 51 sound   SOUND_SIGHT     "ModelsMP\\Enemies\\Demon\\Sounds\\Sight.wav",
 52 sound   SOUND_WOUND     "ModelsMP\\Enemies\\Demon\\Sounds\\Wound.wav",
 55 sound   SOUND_DEATH     "ModelsMP\\Enemies\\Demon\\Sounds\\Death.wav",
 57 sound   SOUND_CAST      "ModelsMP\\Enemies\\Demon\\Sounds\\Cast.wav",

functions:
  virtual CTString GetPlayerKillDescription(const CTString &strPlayerName, const EDeath &eDeath) {
    CTString str;
    str.PrintF(TRANS("%s got oblitirated by a Destroyer"), strPlayerName);
    return str;
  };

  virtual const CTFileName &GetComputerMessageName(void) const {
    static DECLARE_CTFILENAME(fnmDemon, "Data\\Messages\\Enemies\\Destroyer.txt");
    return fnmDemon;
  };

  void Precache(void) {
    CEnemyBase::Precache();

    PrecacheModel(MODEL_DEMON);
    PrecacheTexture(TEXTURE_BASE);
    PrecacheTexture(TEXTURE_WOUNDED);

    PrecacheSound(SOUND_IDLE);
    PrecacheSound(SOUND_SIGHT);
    PrecacheSound(SOUND_WOUND);
    PrecacheSound(SOUND_DEATH);
    PrecacheSound(SOUND_CAST);

    PrecacheClass(CLASS_PROJECTILE, PRT_HEADMAN_ROCKETMAN);
    PrecacheClass(CLASS_PROJECTILE, PRT_HEADMAN_FIRECRACKER);
    PrecacheClass(CLASS_PROJECTILE, PRT_CYBORG_LASER);
    PrecacheClass(CLASS_PROJECTILE, PRT_WOMAN_FIRE);
  #if SE1_GAME != SS_TFE
    PrecacheClass(CLASS_PROJECTILE, PRT_GRUNT_PROJECTILE_COM);
    PrecacheClass(CLASS_PROJECTILE, PRT_GRUNT_PROJECTILE_SOL);
  #endif
  };

  void *GetEntityInfo(void) {
    return &eiDemon;
  };

  BOOL ForcesCannonballToExplode(void) {
    return TRUE;
  };

  void ReceiveDamage(CEntity *penInflictor, enum DamageType dmtType,
    FLOAT fDamageAmmount, const FLOAT3D &vHitPoint, const FLOAT3D &vDirection)
  {
    // [Cecil] Wounded texture at half the health
    if (m_fMaxHealth * 0.5f > GetHealth() && m_bTextureTurnOn) {
      SetModelMainTexture(TEXTURE_WOUNDED);
      m_bTextureTurnOn = FALSE;
    }

    // take less damage from heavy bullets (e.g. sniper)
    if (dmtType == DMT_BULLET && fDamageAmmount > 100.0f) {
      fDamageAmmount *= 0.5f;
    }

    // [Cecil] NOTE: It probably meant to be "Destroyer" but it isn't
    // can't harm own class
    if (!IsOfClass(penInflictor, "Demon")) {
      CEnemyBase::ReceiveDamage(penInflictor, dmtType, fDamageAmmount, vHitPoint, vDirection);
    }
  };

  // Damage anim
  INDEX AnimForDamage(FLOAT fDamage) {
    StartModelAnim(DEMON_ANIM_WOUND, 0);
    return DEMON_ANIM_WOUND;
  };

  // Death anim
  INDEX AnimForDeath(void) {
    StartModelAnim(DEMON_ANIM_DEATHFORWARD, 0);
    return DEMON_ANIM_DEATHFORWARD;
  };

  FLOAT WaitForDust(FLOAT3D &vStretch) {
    vStretch = FLOAT3D(3, 3, 6);
    return 1.1f;
  };

  void DeathNotify(void) {
    ChangeCollisionBoxIndexWhenPossible(DEMON_COLLISION_BOX_DEATH);
    en_fDensity = 500.0f;
  };

  // Animations
  void StandingAnim(void) {
    StartModelAnim(DEMON_ANIM_IDLE, AOF_LOOPING|AOF_NORESTART);
  };

  void WalkingAnim(void) {
    RunningAnim();
  };

  void RunningAnim(void) {
    StartModelAnim(DEMON_ANIM_RUN, AOF_LOOPING|AOF_NORESTART);
  };

  void RotatingAnim(void) {
    StartModelAnim(DEMON_ANIM_RUN, AOF_LOOPING|AOF_NORESTART);
  };

  // Sounds
  void IdleSound(void) {
    PlaySound(m_soSound, SOUND_IDLE, SOF_3D);
  };

  void SightSound(void) {
    PlaySound(m_soSound, SOUND_SIGHT, SOF_3D);
  };

  void WoundSound(void) {
    PlaySound(m_soSound, SOUND_WOUND, SOF_3D);
  };

  void DeathSound(void) {
    PlaySound(m_soSound, SOUND_DEATH, SOF_3D);
  };

  void EnemyPostInit(void) {
    m_soSound.Set3DParameters(160.0f, 50.0f, 2.0f, 1.0f);
  };

  // [Cecil] Fire a full circle of projectiles that used to be inline
  void ShootFullCircle(BOOL bHalfOffset) {
    m_iAttackLoopAngle = 360;
    INDEX ct = 45;

    while (--ct >= 0) {
      FLOAT fOffset = m_iAttackLoopAngle - (bHalfOffset ? 4 : 0);
      ShootProjectile(m_ptAttackProjectile, FLOAT3D(0.0f, 0.5f, 0.0f), FLOAT3D(fOffset, 0.0f, 0.0f));

      m_iAttackLoopAngle -= 8;
    }
  };

procedures:
/************************************************************
 *                A T T A C K   E N E M Y                   *
 ************************************************************/
  Fire(EVoid) : CEnemyBase::Fire {
    if (m_fMoveSpeed > 0.0f) {
      SetDesiredTranslation(FLOAT3D(0.0f, 0.0f, -m_fMoveSpeed));
    }

    StartModelAnim(DEMON_ANIM_ATTACK, 0);
    autocall CMovableModelEntity::WaitUntilScheduledAnimStarts() EReturn;    

    SetDesiredTranslation(FLOAT3D(0.0f, 0.0f, 0.0f));
    PlaySound(m_soSound, SOUND_CAST, SOF_3D);

    autowait(2.94f);
    MaybeSwitchToAnotherPlayer();

    // Prepare a random attack
  #if SE1_GAME != SS_TFE
    m_iAttackRandPrNumber = IRnd() % 6;
  #else
    m_iAttackRandPrNumber = IRnd() % 4;
  #endif
    m_iAttackIsCircle = IRnd() % 2;
    m_iAttackSeries = IRnd() % 3;

    if (GetHealth() >= m_fMaxHealth * 0.5f) {
      m_fWaitTime1 = 0.5f;
      m_fWaitTime2 = 0.05f;
    } else {
      m_fWaitTime1 = 0.25f;
      m_fWaitTime2 = 0.01f;
    }

    switch (m_iAttackRandPrNumber) {
      case 0: m_ptAttackProjectile = PRT_HEADMAN_ROCKETMAN; break;
      case 1: m_ptAttackProjectile = PRT_HEADMAN_FIRECRACKER; break;
      case 2: m_ptAttackProjectile = PRT_CYBORG_LASER; break;
      case 3: m_ptAttackProjectile = PRT_WOMAN_FIRE; break;
    #if SE1_GAME != SS_TFE
      case 4: m_ptAttackProjectile = PRT_GRUNT_PROJECTILE_COM; break;
      case 5: m_ptAttackProjectile = PRT_GRUNT_PROJECTILE_SOL; break;
    #endif
    }

    // [Cecil] Perform attack in a circle
    if (m_iAttackIsCircle) {
      jump CircularAttack();
    }

    jump AngularAttack();
  };

  // [Cecil] These procedure are a hassle to reverse-engineer, so they're mostly rewritten from scratch
  CircularAttack() {
    // Shoot 1-3 times
    m_iAttackLoopCounter = 0;

    while (m_iAttackLoopCounter <= m_iAttackSeries) {
      ShootFullCircle(m_iAttackLoopCounter % 2);
      autowait(m_fWaitTime1);

      m_iAttackLoopCounter++;
    }

    autowait(1.0f);
    return EReturn();
  };

  AngularAttack() {
    // [Cecil] Dynamic calculation: 45 per each loop + 1
    m_iAttackLoopCounter = (m_iAttackSeries + 1) * 45 + 1;
    m_iAttackLoopAngle = 360;

    do {
      ShootProjectile(m_ptAttackProjectile, FLOAT3D(0.0f, 0.5f, 0.0f), FLOAT3D(m_iAttackLoopAngle, 0.0f, 0.0f));
      autowait(m_fWaitTime2);

      m_iAttackLoopCounter--;
      m_iAttackLoopAngle -= 8;
    } while (m_iAttackLoopCounter > 0);

    autowait(1.0f);
    return EReturn();
  };

  Hit(EVoid) : CEnemyBase::Hit {
    // Close attack
    if (CalcDist(m_penEnemy) < 6.0f) {
      StartModelAnim(DEMON_ANIM_WOUND, 0);
      autowait(0.45f);
      PlaySound(m_soSound, SOUND_WOUND, SOF_3D);

      if (CalcDist(m_penEnemy) < CLOSE_ATTACK_RANGE && IsInPlaneFrustum(m_penEnemy, CosFast(60.0f)))
      {
        FLOAT3D vEnemy = m_penEnemy->GetPlacement().pl_PositionVector;
        FLOAT3D vDirection = (vEnemy - GetPlacement().pl_PositionVector);
        vDirection.SafeNormalize();

        InflictDirectDamage(m_penEnemy, this, DMT_CLOSERANGE, 50.0f, vEnemy, vDirection);
      }

      autowait(1.5f);
      MaybeSwitchToAnotherPlayer();

    // Run towards the enemy
    } else {
      m_fShootTime = _pTimer->CurrentTick() + 0.5f;
    }

    return EReturn();
  };

/************************************************************
 *                       M  A  I  N                         *
 ************************************************************/
  Main(EVoid) {
    InitAsModel();
    SetPhysicsFlags(EPF_MODEL_WALKING);
    SetCollisionFlags(ECF_MODEL);
    SetFlags(GetFlags()|ENF_ALIVE);

    // Set appearance
    SetModel(MODEL_DEMON);
    SetModelMainTexture(TEXTURE_BASE);
    StandingAnim();

    GetModelObject()->StretchModel(FLOAT3D(DEMON_STRETCH, DEMON_STRETCH, DEMON_STRETCH));
    ModelChangeNotify();

    // Setup moving speed
    m_fWalkSpeed = 0.0f;
    m_aWalkRotateSpeed = (FRnd() * 20.0f) + 50.0f;
    m_fCloseRunSpeed = 0.0f;
    m_aCloseRotateSpeed = (FRnd() * 100.0f) + 900.0f;
    m_fAttackRunSpeed = 0.0f;
    m_aAttackRotateSpeed = (FRnd() * 100.0f) + 900.0f;

    // Setup attack distances
    m_fAttackDistance = 650.0f;
    m_fCloseDistance = 12.0f;
    m_fStopDistance = 5.0f;
    m_fAttackFireTime = 5.0f;
    m_fCloseFireTime = 1.0f;
    m_fIgnoreRange = 800.0f;
    m_tmGiveUp = Max(m_tmGiveUp, 10.0f);

    // Physical properties
    SetHealth(15000.0f);
    m_fMaxHealth = GetHealth();
    en_fDensity = 1100.0f;

    m_fBlowUpAmount = 10000.0f;
    m_fBodyParts = 4;
    m_fDamageWounded = 1e6;
    m_iScore = 1000000;
    m_fLockOnEnemyTime = 3.0f;

    jump CEnemyBase::MainLoop();
  };
};
