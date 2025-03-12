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

501
%{
#include "StdH.h"
#include "Models/Weapons/Laser/Projectile/LaserProjectile.h"
#include "Enemies/EnemyBase.h"
#include "Models/Enemies/Elementals/Projectile/IcePyramid.h"
#include "Models/Enemies/ElementalLava/Projectile/LavaStone.h"
#include "Models/Enemies/ElementalLava/Projectile/LavaBomb.h"
#include "Models/Enemies/Huanman/Projectile/Projectile.h"
#include "Models/Enemies/Cyborg/Projectile/LaserProjectile.h"

#include "ModelsMP/Enemies/ExotechLarva/Weapons/PlasmaGun.h"
%}

uses "Effects/BasicEffects";
uses "Weapons/Projectile";

enum ProjectileTypeRev {
 // [Cecil] Rev: New projectiles
 30 PRT_WATERMAN_OLD          "Waterman (Old)",
 31 PRT_WATERMAN_BIG          "Waterman Big",
 32 PRT_WATERMAN_LARGE        "Waterman Large",
 33 PRT_WATERMAN_SMALL        "Waterman Small",

 78 PRT_PLASMA                "Plasma",
 79 PRT_RAILBOLT              "Railbolt",
 80 PRT_RAILBOLT_SPRAY        "Railbolt spray",
 81 PRT_AIRMAN_LARGE          "Airman Large",
 82 PRT_AIRMAN_BIG            "Airman Big",
 83 PRT_AIRMAN_SMALL          "Airman Small",
 90 PRT_EARTHMAN_BIG_BOMB     "Earthman Big Bomb",
 91 PRT_EARTHMAN_BIG          "Earthman Bomb",
 92 PRT_EARTHMAN_STONE        "Earthman Stone",
};

%{
void CProjectileRev_OnInitClass(void)
{
}

void CProjectileRev_OnPrecache(CDLLEntityClass *pdec, INDEX iUser) 
{
  //CProjectile_OnPrecache(pdec, iUser);

  switch ((ProjectileType)iUser) {
    // [Cecil] Rev: New projectiles
    case PRT_PLASMA:
      pdec->PrecacheModel(MODEL_PLAYER_LASER);
    case PRT_RAILBOLT:
    case PRT_RAILBOLT_SPRAY:
      pdec->PrecacheModel(MODEL_LASER);
      pdec->PrecacheTexture(TEXTURE_RED_LASER);
      pdec->PrecacheClass(CLASS_BASIC_EFFECT, BET_GRENADE);
      pdec->PrecacheClass(CLASS_BASIC_EFFECT, BET_EXPLOSIONSTAIN);
      break;

    case PRT_EARTHMAN_STONE:
      pdec->PrecacheModel(MODEL_ELEM_STONE);
      pdec->PrecacheTexture(TEXTURE_ELEM_STONE); 
      break;

    case PRT_WATERMAN_BIG:
    case PRT_WATERMAN_LARGE:
    case PRT_WATERMAN_SMALL:
      pdec->PrecacheSound(SOUND_FLYING); // [Cecil] Rev: 'SOUND_DEMON_FLYING' -> 'SOUND_FLYING'
      pdec->PrecacheModel(MODEL_LARVA_PLASMA);
      pdec->PrecacheTexture(TEXTURE_LARVA_PLASMA);
      pdec->PrecacheModel(MODEL_LARVA_PLASMA_BALL); // [Cecil] Rev
      pdec->PrecacheTexture(TEXTURE_LARVA_PLASMA_BALL); // [Cecil] Rev
      pdec->PrecacheClass(CLASS_BASIC_EFFECT, BET_LIGHT_CANNON);    
      break;

    case PRT_AIRMAN_LARGE:
    case PRT_AIRMAN_BIG:
    case PRT_AIRMAN_SMALL:
      pdec->PrecacheModel(MODEL_WINDBLAST);
      pdec->PrecacheTexture(TEXTURE_WINDBLAST);    
      break;

    case PRT_EARTHMAN_BIG:
    case PRT_EARTHMAN_BIG_BOMB:
      pdec->PrecacheModel(MODEL_ELEM_STONE);
      pdec->PrecacheTexture(TEXTURE_ELEM_EARTH);
      break;

    default:
      ASSERT(FALSE);
  }
}
%}

class export CProjectileRev : CProjectile {
name      "Projectile";
thumbnail "";
features "ImplementsOnInitClass", "ImplementsOnPrecache", "CanBePredictable";

properties:

components:
  1 class   CLASS_BASIC_EFFECT  "Classes\\BasicEffect.ecl",
  2 class   CLASS_LIGHT         "Classes\\Light.ecl",
  3 class   CLASS_PROJECTILE    "Classes\\Projectile.ecl",
  4 class   CLASS_BLOOD_SPRAY   "Classes\\BloodSpray.ecl",

  8 sound   SOUND_FLYING              "Sounds\\Weapons\\RocketFly.wav",
 17 model   MODEL_PLAYER_LASER        "Models\\Weapons\\Laser\\Projectile\\LaserProjectile.mdl",
 50 model   MODEL_LASER               "Models\\Weapons\\Laser\\Projectile\\LaserProjectile.mdl",
 53 texture TEXTURE_RED_LASER         "Models\\Weapons\\Laser\\Projectile\\LaserProjectileRed.tex",

 77 texture TEXTURE_ELEM_EARTH        "Models\\Enemies\\ElementalMan\\Old\\Elementals\\Projectile\\Earth.tex",
 78 model   MODEL_ELEM_STONE          "Models\\Enemies\\ElementalMan\\Old\\Elementals\\Projectile\\Stone.mdl",
 90 texture TEXTURE_ELEM_STONE        "Models\\Enemies\\ElementalMan\\Old\\Elementals\\Projectile\\Stone.tex",

 98 model   MODEL_ELEM_WATER_DROP     "Models\\Enemies\\ElementalMan\\Old\\ElementalWater\\Projectile\\WaterDrop.mdl",
 99 texture TEXTURE_ELEM_WATERMAN     "Models\\Enemies\\ElementalMan\\Old\\ElementalWater\\Projectile\\WaterManFX.tex",

172 model   MODEL_LARVA_PLASMA        "ModelsMP\\Enemies\\ExotechLarva\\Projectile\\Projectile.mdl",
173 texture TEXTURE_LARVA_PLASMA      "ModelsMP\\Enemies\\ExotechLarva\\Projectile\\Projectile.tex",
174 model   MODEL_LARVA_PLASMA_BALL   "ModelsMP\\Enemies\\ExotechLarva\\Weapons\\PlasmaGun.mdl",
175 texture TEXTURE_LARVA_PLASMA_BALL "ModelsMP\\Enemies\\ExotechLarva\\Weapons\\PlasmaGun.tex",

180 model   MODEL_WINDBLAST           "ModelsMP\\Enemies\\AirElemental\\Projectile\\WindBlast.mdl",
181 texture TEXTURE_WINDBLAST         "ModelsMP\\Enemies\\AirElemental\\Projectile\\WindBlast.tex",

190 model   MODEL_BANANA              "Models\\Effects\\Debris\\Fruits\\Banana.mdl",
191 texture TEXTURE_BANANA            "Models\\Effects\\Debris\\Fruits\\Banana.tex",

functions:
  void SetupLightSource(BOOL bLive) {
    CLightSource lsNew;

    switch (m_prtType) {
      // [Cecil] Rev: New projectiles
      case PRT_WATERMAN_BIG:
      case PRT_WATERMAN_LARGE:
      case PRT_WATERMAN_SMALL:
        lsNew.ls_colColor = C_dBLUE;
        lsNew.ls_rFallOff = 5.0f;
        lsNew.ls_plftLensFlare = &_lftCatmanFireGlow;
        break;

      case PRT_PLASMA:
      case PRT_RAILBOLT:
      case PRT_RAILBOLT_SPRAY:
        lsNew.ls_colColor = C_dRED;
        lsNew.ls_rFallOff = 1.5f;
        lsNew.ls_plftLensFlare = NULL;
        break;

      // Vanilla projectiles
      default:
        CProjectile::SetupLightSource(bLive);
        return;
    }

    lsNew.ls_ulFlags = LSF_NONPERSISTENT|LSF_DYNAMIC;
    lsNew.ls_rHotSpot = 0.0f;
    lsNew.ls_ubPolygonalMask = 0;
    lsNew.ls_paoLightAnimation = NULL;

    m_lsLightSource.ls_penEntity = this;
    m_lsLightSource.SetLightSource(lsNew);
  };

  void RenderParticles(void) {
    switch (m_prtType) {
      // [Cecil] Rev: New projectiles
      case PRT_EARTHMAN_BIG_BOMB: Particles_LavaBombTrail(this, 4.0f); break;
      case PRT_EARTHMAN_BIG: Particles_LavaBombTrail(this, 1.0f); break;

      case PRT_AIRMAN_LARGE:
      case PRT_AIRMAN_BIG:
        Particles_Windblast(this, m_fStretch/4.0f, m_fStartTime+3.0f);
        break;

      case PRT_AIRMAN_SMALL:
        Particles_Windblast(this, 0.125f, m_fStartTime + 3.0f);
        break;

      case PRT_PLASMA: case PRT_RAILBOLT: case PRT_RAILBOLT_SPRAY:
        Particles_FirecrackerTrail(this);
        break;

      // Vanilla projectiles
      default: CProjectile::RenderParticles();
    }
  };

// ----- PROJECTILE SETUP METHODS -----

// [Cecil] Rev: New player projectiles from here
void PlayerPlasma(void) {
  InitAsModel();
  SetPhysicsFlags(EPF_PROJECTILE_FLYING);
  SetCollisionFlags(ECF_PROJECTILE_SOLID);
  SetFlags(GetFlags() | ENF_SEETHROUGH);
  SetModel(MODEL_PLAYER_LASER);

  CModelObject *pmo = GetModelObject();

  if (pmo != NULL) {
    pmo->PlayAnim(LASERPROJECTILE_ANIM_GROW, 0);
  }

  SetModelMainTexture(TEXTURE_RED_LASER);

  LaunchAsPropelledProjectile(FLOAT3D(0, 0, -m_fSpeed), (CMovableEntity *)&*m_penLauncher);
  SetDesiredRotation(ANGLE3D(0, 0, 0));

  m_fFlyTime = 4.0f;
  m_fDamageHotSpotRange = 2.0f;
  m_fRangeDamageAmount = m_fDamageAmount * 0.5f;
  m_fDamageFallOffRange = 4.0f;
  m_fSoundRange = 0.0f;
  m_bExplode = TRUE;
  m_bLightSource = TRUE;
  m_bCanHitHimself = TRUE;
  m_bCanBeDestroyed = FALSE;
  m_fWaitAfterDeath = 0.0f;
  m_tmExpandBox = 0.1f;
  m_tmInvisibility = 0.05f;
  m_pmtMove = PMT_FLYING;
};

void PlayerPlasmaExplosion(void) {
  ESpawnEffect ese;
  ese.colMuliplier = C_WHITE | CT_OPAQUE;
  ese.betType = BET_GRENADE;//BET_PLASMA;
  ese.vStretch = FLOAT3D(1, 1, 1);
  SpawnEffect(GetPlacement(), ese);

  // Spawn sound event in range
  if (IsDerivedFromClass(m_penLauncher, "Player")) {
    SpawnRangeSound(m_penLauncher, this, SNDT_PLAYER, m_fSoundRange);
  }

  FLOAT3D vPoint;
  FLOATplane3D vPlaneNormal;
  FLOAT fDistanceToEdge;

  if (GetNearestPolygon(vPoint, vPlaneNormal, fDistanceToEdge)) {
    if ((vPoint - GetPlacement().pl_PositionVector).Length() < 3.5f) {
      ese.betType = BET_EXPLOSIONSTAIN;
      ese.vNormal = FLOAT3D(vPlaneNormal);
      SpawnEffect(CPlacement3D(vPoint, ANGLE3D(0, 0, 0)), ese);
    }
  }
};

void PlayerRailBolt(void) {
  InitAsModel();
  SetPhysicsFlags(EPF_PROJECTILE_FLYING);
  SetCollisionFlags(ECF_PROJECTILE_SOLID);
  SetFlags(GetFlags() | ENF_SEETHROUGH);
  SetModel(MODEL_LASER);

  CModelObject *pmo = GetModelObject();

  if (pmo != NULL) {
    pmo->PlayAnim(LASERPROJECTILE_ANIM_GROW, 0);
    pmo->StretchModel(FLOAT3D(0.4f, 0.4f, 0.4f));
    ModelChangeNotify();
  }

  SetModelMainTexture(TEXTURE_RED_LASER);

  LaunchAsPropelledProjectile(FLOAT3D(0, 0, -m_fSpeed), (CMovableEntity *)&*m_penLauncher);
  SetDesiredRotation(ANGLE3D(0, 0, 0));

  m_fFlyTime = 4.0f;
  m_fDamageAmount = 30.0f;
  m_fRangeDamageAmount = 15.0f;
  m_fDamageHotSpotRange = 1.0f;
  m_fDamageFallOffRange = 3.0f;
  m_fSoundRange = 0.0f;
  m_bExplode = TRUE;
  m_bLightSource = TRUE;
  m_bCanHitHimself = TRUE;
  m_bCanBeDestroyed = FALSE;
  m_fWaitAfterDeath = 0.0f;
  m_tmExpandBox = 0.1f;
  m_tmInvisibility = 0.0001f;
  m_pmtMove = PMT_FLYING;
};

void PlayerRailBoltSpray(void) {
  InitAsModel();
  SetPhysicsFlags(EPF_PROJECTILE_FLYING);
  SetCollisionFlags(ECF_PROJECTILE_SOLID);
  SetFlags(GetFlags() | ENF_SEETHROUGH);
  SetModel(MODEL_LASER);

  CModelObject *pmo = GetModelObject();

  if (pmo != NULL) {
    pmo->PlayAnim(LASERPROJECTILE_ANIM_GROW, 0);
    pmo->StretchModel(FLOAT3D(0.4f, 0.4f, 0.4f));
    ModelChangeNotify();
  }

  SetModelMainTexture(TEXTURE_RED_LASER);

  LaunchAsPropelledProjectile(FLOAT3D(FRnd() + 2.0f, FRnd() + 5.0f, -m_fSpeed), (CMovableEntity *)&*m_penLauncher);
  SetDesiredRotation(ANGLE3D((FRnd() * 180.0f) - 90.0f, 0, 0));

  m_fFlyTime = 4.0f;
  m_fDamageAmount = 30.0f;
  m_fRangeDamageAmount = 15.0f;
  m_fDamageHotSpotRange = 1.0f;
  m_fDamageFallOffRange = 3.0f;
  m_fSoundRange = 0.0;
  m_bExplode = TRUE;
  m_bLightSource = TRUE;
  m_bCanHitHimself = TRUE;
  m_bCanBeDestroyed = FALSE;
  m_fWaitAfterDeath = 0.0f;
  m_tmExpandBox = 0.1f;
  m_tmInvisibility = 0.0001f;
  m_pmtMove = PMT_FLYING;
};

// [Cecil] Rev: New elemental projectiles from here
void WaterDropProjectile(void) {
  InitAsModel();
  SetPhysicsFlags(EPF_PROJECTILE_FLYING);
  SetCollisionFlags(ECF_PROJECTILE_MAGIC);
  SetFlags(GetFlags() | ENF_SEETHROUGH);
  SetModel(MODEL_ELEM_WATER_DROP);
  SetModelMainTexture(TEXTURE_ELEM_WATERMAN);

  LaunchAsPropelledProjectile(FLOAT3D(0, 0, -30), (CMovableEntity *)&*m_penLauncher);
  SetDesiredRotation(ANGLE3D(0, 0, 0));

  m_fFlyTime = 5.0f;
  m_fDamageAmount = 8.0f;
  m_fSoundRange = 0.0f;
  m_bExplode = FALSE;
  m_bLightSource = TRUE;
  m_bCanHitHimself = FALSE;
  m_bCanBeDestroyed = FALSE;
  m_fWaitAfterDeath = 0.0f;
  m_pmtMove = PMT_FLYING;
};

// [Cecil] Common setup for all sizes
void WaterManPlasma(FLOAT fStretch, FLOAT fDamageCoop, FLOAT fDamageSP, BOOL bBig) {
  InitAsModel();
  SetPhysicsFlags(EPF_MODEL_SLIDING & ~EPF_ORIENTEDBYGRAVITY);
  SetCollisionFlags(ECF_PROJECTILE_MAGIC);
  SetModel(MODEL_LARVA_PLASMA_BALL);
  SetModelMainTexture(TEXTURE_LARVA_PLASMA_BALL);
  AddAttachmentToModel(this, *GetModelObject(), PLASMAGUN_ATTACHMENT_PROJECTILE, MODEL_LARVA_PLASMA, TEXTURE_LARVA_PLASMA, 0, 0, 0);

  GetModelObject()->StretchModel(FLOAT3D(fStretch, fStretch, fStretch));
  ModelChangeNotify();

  if (bBig) {
    LaunchAsPropelledProjectile(FLOAT3D(0, 0, -25), (CMovableEntity *)&*m_penLauncher);
    SetDesiredRotation(ANGLE3D(0, 0, (FRnd() * 20.0f) - 10.0f));
  } else {
    LaunchAsPropelledProjectile(FLOAT3D(0, 0, -60), (CMovableEntity *)&*m_penLauncher);
    SetDesiredRotation(ANGLE3D(0, 0, 0));
  }

  // Flying sound
  m_soEffect.Set3DParameters(20.0f, 2.0f, 1.0f, 1.0f);
  PlaySound(m_soEffect, SOUND_FLYING, SOF_3D | SOF_LOOP);

  m_fFlyTime = 30.0f;

  if (GetSP()->sp_bCooperative) {
    m_fDamageAmount = fDamageCoop;
    m_fRangeDamageAmount = fDamageCoop;
  } else {
    m_fDamageAmount = fDamageSP;
    m_fRangeDamageAmount = fDamageSP;
  }

  m_fDamageHotSpotRange = 4.0f;
  m_fDamageFallOffRange = 8.0f;
  m_fSoundRange = 50.0f;
  m_bExplode = TRUE;
  m_bLightSource = TRUE;
  m_bCanHitHimself = FALSE;
  m_bCanBeDestroyed = FALSE;
  m_fWaitAfterDeath = 0.05f;
  m_tmExpandBox = 0.1f;
  m_tmInvisibility = 0.05f;
  SetHealth(100.0f);

  m_iRebounds = 4;
  m_pmtMove = PMT_FLYING_REBOUNDING;
};

void WaterManPlasmaSmall(void) {
  WaterManPlasma(1.0f, 12.0f, 8.0f, FALSE);
};

void WaterManPlasmaBig(void) {
  WaterManPlasma(2.5f, 20.0f, 18.0f, TRUE);
};

void WaterManPlasmaLarge(void) {
  WaterManPlasma(5.0f, 50.0f, 45.0f, TRUE);
};

// [Cecil] Common setup for all sizes
void WaterManPlasmaExplosion(FLOAT fStretch, FLOAT fParticleSize) {
  // Explosion
  ESpawnEffect ese;
  ese.colMuliplier = C_WHITE | CT_OPAQUE;
  ese.betType = BET_LIGHT_CANNON;
  ese.vStretch = FLOAT3D(fStretch, fStretch, fStretch);
  SpawnEffect(GetPlacement(), ese);

  // Particles
  CEntityPointer penSpray = CreateEntity(GetPlacement(), CLASS_BLOOD_SPRAY);
  penSpray->SetParent(this);

  ESpawnSpray eSpawnSpray;
  eSpawnSpray.colBurnColor = C_WHITE | CT_OPAQUE;
  eSpawnSpray.fDamagePower = 1.0f;
  eSpawnSpray.fSizeMultiplier = fParticleSize;
  eSpawnSpray.sptType = SPT_PLASMA;
  eSpawnSpray.vDirection = FLOAT3D(0.0f, 2.5f, 0.0f);
  eSpawnSpray.penOwner = this;
  penSpray->Initialize(eSpawnSpray);
};

void WaterManPlasmaSmallExplosion(void) {
  WaterManPlasmaExplosion(1.0f, 0.15f);
};

void WaterManPlasmaBigExplosion(void) {
  WaterManPlasmaExplosion(2.0f, 0.25f);
};

void WaterManPlasmaLargeExplosion(void) {
  WaterManPlasmaExplosion(2.0f, 0.25f);
};

void EarthManBomb(void) {
  InitAsModel();
  SetPhysicsFlags(EPF_MODEL_BOUNCING);
  SetCollisionFlags(ECF_PROJECTILE_SOLID);

  SetModel(MODEL_ELEM_STONE);
  SetModelMainTexture(TEXTURE_ELEM_EARTH);

  if (m_prtType == PRT_EARTHMAN_BIG_BOMB) {
    GetModelObject()->StretchModel(FLOAT3D(6.0f, 6.0f, 6.0f));
    m_fDamageAmount = 10.0f;
    m_fRangeDamageAmount = 5.0f;
    m_fDamageHotSpotRange = 3.75f;
    m_fDamageFallOffRange = 7.5f;
    SetHealth(30.0f);

  } else if (m_prtType == PRT_EARTHMAN_BIG) {
    GetModelObject()->StretchModel(FLOAT3D(1.5f, 1.5f, 1.5f));
    m_fDamageAmount = 5.0f;
    m_fRangeDamageAmount =  2.5f;
    m_fDamageHotSpotRange = 2.5f;
    m_fDamageFallOffRange = 5.0f;
    SetHealth(10.0f);
  }

  ModelChangeNotify();
  
  LaunchAsFreeProjectile(FLOAT3D(0.0f, 0.0f, -m_fSpeed), (CMovableEntity *)&*m_penLauncher);
  SetDesiredRotation(ANGLE3D(0, FRnd() * 360.0f - 180.0f, 0.0f));

  m_fFlyTime = 20.0f;
  m_fSoundRange = 50.0f;
  m_bExplode = TRUE;
  m_bLightSource = FALSE;
  m_bCanHitHimself = FALSE;
  m_bCanBeDestroyed = TRUE;
  m_pmtMove = PMT_FLYING;
  m_fWaitAfterDeath = 4.0f;

  if (m_prtType == PRT_EARTHMAN_BIG_BOMB) {
    // Spawn particle debris
    CEntityPointer penSpray = CreateEntity(GetPlacement(), CLASS_BLOOD_SPRAY);
    penSpray->SetParent(this);

    ESpawnSpray eSpawnSpray;
    eSpawnSpray.colBurnColor = C_WHITE | CT_OPAQUE;
    eSpawnSpray.fDamagePower = 4.0f;
    eSpawnSpray.fSizeMultiplier = 0.5f;
    eSpawnSpray.sptType = SPT_STONES;
    eSpawnSpray.vDirection = FLOAT3D(0.0f, -0.5f, 0.0f);
    eSpawnSpray.penOwner = this;
    penSpray->Initialize(eSpawnSpray);
  }
};

void EarthanBombExplosion(void) {
  ESpawnEffect ese;
  FLOAT3D vPoint;
  FLOATplane3D vPlaneNormal;
  FLOAT fDistanceToEdge;

  // Spawn shock wave
  if (GetNearestPolygon(vPoint, vPlaneNormal, fDistanceToEdge))
  {
    if ((vPoint - GetPlacement().pl_PositionVector).Length() < 3.5f)
    {
      ese.colMuliplier = C_WHITE | CT_OPAQUE;
      ese.betType = BET_SHOCKWAVE;
      ese.vNormal = FLOAT3D(vPlaneNormal);
      SpawnEffect(CPlacement3D(vPoint, ANGLE3D(0, 0, 0)), ese);
    }
  }

  // Spawn particle debris
  CPlacement3D plSpray = GetPlacement();
  CEntityPointer penSpray = CreateEntity(plSpray, CLASS_BLOOD_SPRAY);
  penSpray->SetParent(this);

  ESpawnSpray eSpawnSpray;
  eSpawnSpray.colBurnColor = C_WHITE | CT_OPAQUE;
  eSpawnSpray.fDamagePower = 4.0f;
  eSpawnSpray.fSizeMultiplier = 0.5f;
  eSpawnSpray.sptType = SPT_STONES;
  eSpawnSpray.vDirection = en_vCurrentTranslationAbsolute / 32.0f;
  eSpawnSpray.penOwner = this;
  penSpray->Initialize(eSpawnSpray);

  // Spawn smaller earthman bombs
  for (INDEX iDebris = 0; iDebris < 3 + IRnd() % 3; iDebris++)
  {
    FLOAT fHeading = (FRnd() - 0.5f) * 180.0f;
    FLOAT fPitch = 10.0f + FRnd() * 40.0f;
    FLOAT fSpeed = 10.0f + FRnd() * 50.0f;

    // Launch
    CPlacement3D pl = GetPlacement();
    pl.pl_PositionVector(2) += 2.0f;
    pl.pl_OrientationAngle = m_penLauncher->GetPlacement().pl_OrientationAngle;
    pl.pl_OrientationAngle(1) += AngleDeg(fHeading);
    pl.pl_OrientationAngle(2) = AngleDeg(fPitch);

    CEntityPointer penProjectile = CreateEntity(pl, CLASS_PROJECTILE);
    ELaunchProjectile eLaunch;
    eLaunch.penLauncher = this;
    eLaunch.prtType = (ProjectileType)PRT_EARTHMAN_BIG;
    eLaunch.fSpeed = fSpeed;
    penProjectile->Initialize(eLaunch);

    // Spawn particle debris
    CPlacement3D plSpray = pl;
    CEntityPointer penSpray = CreateEntity(plSpray, CLASS_BLOOD_SPRAY);
    penSpray->SetParent(penProjectile);

    ESpawnSpray eSpawnSpray;
    eSpawnSpray.colBurnColor = C_WHITE | CT_OPAQUE;
    eSpawnSpray.fDamagePower = 1.0f;
    eSpawnSpray.fSizeMultiplier = 0.5f;
    eSpawnSpray.sptType = SPT_STONES;
    eSpawnSpray.vDirection = FLOAT3D(0, -0.5f, 0);
    eSpawnSpray.penOwner = penProjectile;
    penSpray->Initialize(eSpawnSpray);
  }
};

void EarthmanBombDebrisExplosion(void) {
  ESpawnEffect ese;
  FLOAT3D vPoint;
  FLOATplane3D vPlaneNormal;
  FLOAT fDistanceToEdge;

  // Spawn shock wave
  if (GetNearestPolygon(vPoint, vPlaneNormal, fDistanceToEdge))
  {
    if ((vPoint - GetPlacement().pl_PositionVector).Length() < 3.5f)
    {
      ese.colMuliplier = C_WHITE | CT_OPAQUE;
      ese.betType = BET_SHOCKWAVE;
      ese.vNormal = FLOAT3D(vPlaneNormal);
      SpawnEffect(CPlacement3D(vPoint, ANGLE3D(0, 0, 0)), ese);
    }
  }

  // Spawn particle debris
  CPlacement3D plSpray = GetPlacement();
  CEntityPointer penSpray = CreateEntity(plSpray, CLASS_BLOOD_SPRAY);
  penSpray->SetParent(this);

  ESpawnSpray eSpawnSpray;
  eSpawnSpray.colBurnColor = C_WHITE | CT_OPAQUE;
  eSpawnSpray.fSizeMultiplier = 4.0f;
  eSpawnSpray.fDamagePower = 2.0f;
  eSpawnSpray.sptType = SPT_STONES;
  eSpawnSpray.vDirection = en_vCurrentTranslationAbsolute / 16.0f;
  eSpawnSpray.penOwner = this;
  penSpray->Initialize(eSpawnSpray);
};

// [Cecil] Common setup for all sizes
void WindBlast(FLOAT fStretch, FLOAT fSpeed, FLOAT fSpin, FLOAT fDamage) {
  // set appearance
  InitAsEditorModel();
  SetPhysicsFlags(EPF_MODEL_SLIDING);
  SetCollisionFlags(ECF_PROJECTILE_MAGIC);
  SetFlags(GetFlags() | ENF_SEETHROUGH);
  SetModel(MODEL_WINDBLAST);
  SetModelMainTexture(TEXTURE_WINDBLAST);
  GetModelObject()->StretchModel(FLOAT3D(fStretch, fStretch, fStretch));
  ModelChangeNotify();
  // start moving
  LaunchAsPropelledProjectile(FLOAT3D(0, 0, -fSpeed), (CMovableEntity *)&*m_penLauncher);
  SetDesiredRotation(ANGLE3D(0, 0, fSpin));
  m_fFlyTime = 5.0f;
  m_fDamageAmount = fDamage;
  m_fSoundRange = 0.0f;
  m_bExplode = FALSE;
  m_bLightSource = FALSE;
  m_bCanHitHimself = FALSE;
  m_bCanBeDestroyed = FALSE;
  m_fWaitAfterDeath = 0.0f;
  m_pmtMove = PMT_SLIDING;
};

void WindBlastLarge(void) {
  WindBlast(3.0f, 25.0f, (FRnd() * 20.0f) - 10.0f, 20.0f);
};

void WindBlastBig(void) {
  WindBlast(2.0f, 25.0f, (FRnd() * 20.0f) - 10.0f, 10.0f);
};

void WindBlastSmall(void) {
  WindBlast(1.0f, 25.0f, (FRnd() * 20.0f) - 10.0f, 5.0f);
};

procedures:
  Main(ELaunchProjectile eLaunch) {
    // [Cecil] Process vanilla projectiles
    switch (eLaunch.prtType) {
      case PRT_WATERMAN_OLD: case PRT_WATERMAN_BIG: case PRT_WATERMAN_LARGE: case PRT_WATERMAN_SMALL:
      case PRT_PLASMA: case PRT_RAILBOLT: case PRT_RAILBOLT_SPRAY:
      case PRT_AIRMAN_LARGE: case PRT_AIRMAN_BIG: case PRT_AIRMAN_SMALL:
      case PRT_EARTHMAN_BIG_BOMB: case PRT_EARTHMAN_BIG:
        break;

      // PRT_EARTHMAN_STONE is essentially PRT_STONEMAN_FIRE
      case PRT_EARTHMAN_STONE:
        m_prtType = PRT_STONEMAN_FIRE;
      default:
        // [Cecil] NOTE: Procedures are essentially regular functions
        CProjectile::Main(eLaunch);
        return;
    }

    // Remember the initial parameters
    ASSERT(eLaunch.penLauncher != NULL);
    m_penLauncher = eLaunch.penLauncher;
    m_prtType = eLaunch.prtType;
    m_fSpeed = eLaunch.fSpeed;
    m_fStretch = eLaunch.fStretch;
    SetPredictable(TRUE);

    // Remember lauching time
    m_fIgnoreTime = _pTimer->CurrentTick() + 1.0f;
    m_penLastDamaged = NULL;
    
    // [Cecil] Rev: Prepare particles for new projectiles
    switch (m_prtType) {
      case PRT_PLASMA: case PRT_RAILBOLT: case PRT_RAILBOLT_SPRAY:
        Particles_FirecrackerTrail_Prepare(this);
        break;
    }

    // [Cecil] Rev: Initialize new projectiles
    switch (m_prtType) {
      case PRT_WATERMAN_OLD: WaterDropProjectile(); break;
      case PRT_WATERMAN_BIG: WaterManPlasmaBig(); break;
      case PRT_WATERMAN_LARGE: WaterManPlasmaLarge(); break;
      case PRT_WATERMAN_SMALL: WaterManPlasmaSmall(); break;
      case PRT_PLASMA: PlayerPlasma(); break;
      case PRT_RAILBOLT: PlayerRailBolt(); break;
      case PRT_RAILBOLT_SPRAY: PlayerRailBoltSpray(); break;
      case PRT_AIRMAN_LARGE: WindBlastLarge(); break;
      case PRT_AIRMAN_BIG: WindBlastBig(); break;
      case PRT_AIRMAN_SMALL: WindBlastSmall(); break;
      case PRT_EARTHMAN_BIG_BOMB: EarthManBomb(); break;
      case PRT_EARTHMAN_BIG: EarthManBomb(); break;
      default: ASSERTALWAYS("Unknown projectile type");
    }

    // Setup light source
    if (m_bLightSource) {
      SetupLightSource(TRUE);
    }

    // [Cecil] Replace air projectiles in order to apply the same logic inside ProjectileTouch() procedure
    switch (m_prtType) {
      case PRT_AIRMAN_LARGE: case PRT_AIRMAN_BIG: case PRT_AIRMAN_SMALL:
        m_prtType = PRT_AIRELEMENTAL_WIND;
        break;
    }

    // Start flying
    m_fStartTime = _pTimer->CurrentTick();

    if (m_pmtMove == PMT_GUIDED) {
      autocall CProjectile::ProjectileGuidedFly() EEnd;

    } else if (m_pmtMove == PMT_GUIDED_FAST) {
      autocall CProjectile::ProjectileGuidedFastFly() EEnd;

    } else if (m_pmtMove == PMT_FLYING) {
      autocall CProjectile::ProjectileFly() EEnd;

    } else if (m_pmtMove == PMT_SLIDING) {
      autocall CProjectile::ProjectileSlide() EEnd;

    } else if (m_pmtMove == PMT_FLYING_REBOUNDING) {
      autocall CProjectile::ProjectileFlyRebounding() EEnd;

    } else if (m_pmtMove == PMT_GUIDED_SLIDING) {
      autocall CProjectile::ProjectileGuidedSlide() EEnd;
    }

    // [Cecil] Rev: Explode new projectiles
    switch (m_prtType) {
      case PRT_WATERMAN_BIG: WaterManPlasmaBigExplosion(); break;
      case PRT_WATERMAN_LARGE: WaterManPlasmaLargeExplosion(); break;
      case PRT_WATERMAN_SMALL: WaterManPlasmaSmallExplosion(); break;
      case PRT_PLASMA:
      case PRT_RAILBOLT:
      case PRT_RAILBOLT_SPRAY: PlayerPlasmaExplosion(); break;
      case PRT_EARTHMAN_BIG_BOMB: EarthanBombExplosion(); break;
      case PRT_EARTHMAN_BIG: EarthmanBombDebrisExplosion(); break;
    }

    // Wait after death
    if (m_fWaitAfterDeath > 0.0f) {
      SwitchToEditorModel();
      ForceFullStop();
      SetCollisionFlags(ECF_IMMATERIAL);

      // Kill light source
      if (m_bLightSource) {
        SetupLightSource(FALSE);
      }

      autowait(m_fWaitAfterDeath);
    }

    Destroy();
    return;
  };
};
