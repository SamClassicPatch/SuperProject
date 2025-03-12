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

322
%{
#include "StdH.h"
#include "Models/Enemies/Elementals/Projectile/LavaStone.h"

#include "Models/Enemies/ElementalLava/ElementalLava.h"
#include "Effects/WorldSettingsController.h"
#include "Tools/BackgroundViewer.h"

// lava elemental definitions
#define LAVAMAN_SMALL_STRETCH (2.0f*0.75f)
#define LAVAMAN_BIG_STRETCH (4.0f*1.25f)
#define LAVAMAN_LARGE_STRETCH (16.0f*2.5f)

#define LAVAMAN_BOSS_FIRE_RIGHT FLOAT3D(1.01069f, 0.989616f, -1.39743f)
#define LAVAMAN_BOSS_FIRE_LEFT FLOAT3D(-0.39656f, 1.08619f, -1.34373f)
#define LAVAMAN_FIRE_LEFT FLOAT3D(-0.432948f, 1.51133f, -0.476662f)

#define LAVAMAN_FIRE_SMALL  (LAVAMAN_FIRE_LEFT*LAVAMAN_SMALL_STRETCH)
#define LAVAMAN_FIRE_BIG    (LAVAMAN_FIRE_LEFT*LAVAMAN_BIG_STRETCH)
#define LAVAMAN_FIRE_LARGE_LEFT  (LAVAMAN_BOSS_FIRE_LEFT*LAVAMAN_LARGE_STRETCH)
#define LAVAMAN_FIRE_LARGE_RIGHT (LAVAMAN_BOSS_FIRE_RIGHT*LAVAMAN_LARGE_STRETCH)

#define LAVAMAN_SPAWN_BIG   (FLOAT3D(0.0171274f, 1.78397f, -0.291414f)*LAVAMAN_BIG_STRETCH)
#define LAVAMAN_SPAWN_LARGE (FLOAT3D(0.0171274f, 1.78397f, -0.291414f)*LAVAMAN_LARGE_STRETCH)
#define DEATH_BURN_TIME 1.0f

%}

uses "Enemies/EnemyBase";

enum ElementalType {
  // [Cecil] Rev: Restored old types, removed ice, added omni
  0 ELT_AIR           "Air",        // air elemental
  2 ELT_LAVA          "Lava",       // lava elemental
  3 ELT_STONE         "Earth",      // stone elemental
  4 ELT_WATER         "Water",      // water elemental
  5 ELT_OMNI          "Omni",
};


enum ElementalCharacter {
  0 ELC_SMALL         "Small",      // small (fighter)
  1 ELC_BIG           "Big",        // big
  2 ELC_LARGE         "Large",      // large
};


enum ElementalState {
  0 ELS_NORMAL        "Normal",     // normal state
  1 ELS_BOX           "Box",        // in box
  2 ELS_PLANE         "Plane",      // as plane
};


%{
#define ECF_AIR ( \
  ((ECBI_BRUSH|ECBI_MODEL|ECBI_CORPSE|ECBI_ITEM|ECBI_PROJECTILE_MAGIC|ECBI_PROJECTILE_SOLID)<<ECB_TEST) |\
  ((ECBI_MODEL|ECBI_CORPSE|ECBI_ITEM|ECBI_PROJECTILE_MAGIC|ECBI_PROJECTILE_SOLID)<<ECB_PASS) |\
  ((ECBI_MODEL)<<ECB_IS))

// [Cecil] Rev: Same entity info for all types; body type is flesh
static EntityInfo eiElementalSmall = {
  EIBT_FLESH, 2000.0f,
  0.0f, 1.7f*LAVAMAN_SMALL_STRETCH, 0.0f,
  0.0f, LAVAMAN_SMALL_STRETCH, 0.0f,
};
static EntityInfo eiElementalBig = {
  EIBT_FLESH, 2800.0f,
  0.0f, 1.7f*LAVAMAN_BIG_STRETCH, 0.0f,
  0.0f, LAVAMAN_BIG_STRETCH, 0.0f,
};
static EntityInfo eiElementalLarge = {
  EIBT_FLESH, 11200.0f,
  0.0f, 1.7f*LAVAMAN_LARGE_STRETCH, 0.0f,
  0.0f, LAVAMAN_LARGE_STRETCH, 0.0f,
};

// [Cecil] Sad
#if SE1_GAME == SS_TFE
  #define PRT_AIRMAN_SMALL      PRT_LAVAMAN_STONE
  #define PRT_EARTHMAN_STONE    PRT_LAVAMAN_STONE
  #define PRT_WATERMAN_SMALL    PRT_LAVAMAN_STONE
  #define PRT_AIRMAN_BIG        PRT_LAVAMAN_BOMB
  #define PRT_EARTHMAN_BIG      PRT_LAVAMAN_BOMB
  #define PRT_WATERMAN_BIG      PRT_LAVAMAN_BOMB
  #define PRT_AIRMAN_LARGE      PRT_LAVAMAN_BIG_BOMB
  #define PRT_EARTHMAN_BIG_BOMB PRT_LAVAMAN_BIG_BOMB
  #define PRT_WATERMAN_LARGE    PRT_LAVAMAN_BIG_BOMB

  #define SPT_AIRSPOUTS   SPT_FEATHER
  #define SPT_SMALLPLASMA SPT_SLIME
#endif

// [Cecil] Rev: Unique projectiles for each elemental type
static const ProjectileType _aSmallProjectiles[4] = {
  PRT_AIRMAN_SMALL, PRT_LAVAMAN_STONE, PRT_EARTHMAN_STONE, PRT_WATERMAN_SMALL
};

static const ProjectileType _aBigProjectiles[4] = {
  PRT_AIRMAN_BIG, PRT_LAVAMAN_BOMB, PRT_EARTHMAN_BIG, PRT_WATERMAN_BIG
};

static const ProjectileType _aLargeProjectiles[4] = {
  PRT_AIRMAN_LARGE, PRT_LAVAMAN_BIG_BOMB, PRT_EARTHMAN_BIG_BOMB, PRT_WATERMAN_LARGE
};

// obsolete
#define EPF_BOX_PLANE_ELEMENTAL (EPF_ORIENTEDBYGRAVITY|EPF_MOVABLE)

%}


class CElemental : CEnemyBase {
name      "Elemental";
thumbnail "Thumbnails\\Elemental.tbn";

properties:
  1 enum ElementalType m_EetType          "Type" 'Y' = ELT_LAVA, // [Cecil] Rev: Lava by default
  2 enum ElementalCharacter m_EecChar     "Character" 'C' = ELC_SMALL,
  3 enum ElementalState m_EesStartState   "State" 'S' = ELS_NORMAL,
  4 BOOL m_bSpawnWhenHarmed               "Damage spawn" 'N' = TRUE,
  5 BOOL m_bSpawnOnBlowUp                 "Blowup spawn" 'B' = TRUE,
  6 enum ElementalState m_EesCurrentState = ELS_NORMAL,
  7 BOOL m_bSpawned = FALSE,
  8 BOOL m_bMovable                       "Movable" 'V' = TRUE,
  9 RANGE m_fLookRange                    "Look range" 'O' = 30.0f,
 10 INDEX m_iFireCount                    "Fire count" = 2,
 11 FLOAT m_fWaitTime = 0.0f,
 12 INDEX m_iCounter = 0,
 13 FLOAT m_fDensity                      "Density" 'D' = 10000.0f,

// placement for non movable elemental
 20 CEntityPointer m_penPosition1         "Position 1",
 21 CEntityPointer m_penPosition2         "Position 2",
 22 CEntityPointer m_penPosition3         "Position 3",
 23 CEntityPointer m_penPosition4         "Position 4",
 24 CEntityPointer m_penPosition5         "Position 5",
 25 CEntityPointer m_penPosition6         "Position 6",

 30 CSoundObject m_soBackground,  // sound channel for background noise
 31 INDEX m_ctSpawned = 0,
 32 FLOAT m_fSpawnDamage = 1e6f,
 33 BOOL m_bSpawnEnabled = FALSE,
 34 CSoundObject m_soFireL,
 35 CSoundObject m_soFireR,
 36 INDEX m_bCountAsKill = TRUE,
 
components:
  0 class   CLASS_BASE         "Classes\\EnemyBase.ecl",
  1 class   CLASS_PROJECTILE   "Classes\\Projectile.ecl",
  2 class   CLASS_BLOOD_SPRAY  "Classes\\BloodSpray.ecl",
  3 class   CLASS_BASIC_EFFECT "Classes\\BasicEffect.ecl",

// [Cecil] Rev: Models for each type
 7 model MODEL_EARTH      "Models\\Enemies\\ElementalMan\\ElementalEarth.mdl",
 8 model MODEL_AIR        "Models\\Enemies\\ElementalMan\\ElementalAir.mdl",
 9 model MODEL_GENERIC    "Models\\Enemies\\ElementalMan\\ElementalMan.mdl",
10 model MODEL_BODY_FLARE "Models\\Enemies\\ElementalMan\\BodyFlare.mdl",
11 model MODEL_HAND_FLARE "Models\\Enemies\\ElementalMan\\HandFlare.mdl",

// [Cecil] Rev: 'ElementalLava' -> 'ElementalMan'
12 sound SOUND_LAVA_IDLE     "Models\\Enemies\\ElementalMan\\Sounds\\Idle.wav",
13 sound SOUND_LAVA_WOUND    "Models\\Enemies\\ElementalMan\\Sounds\\Wound.wav",
14 sound SOUND_LAVA_FIRE     "Models\\Enemies\\ElementalMan\\Sounds\\Fire.wav",
15 sound SOUND_LAVA_KICK     "Models\\Enemies\\ElementalMan\\Sounds\\Kick.wav",
16 sound SOUND_LAVA_DEATH    "Models\\Enemies\\ElementalMan\\Sounds\\Death.wav",
17 sound SOUND_LAVA_LAVABURN "Models\\Enemies\\ElementalMan\\Sounds\\LavaBurn.wav",
18 sound SOUND_LAVA_ANGER    "Models\\Enemies\\ElementalMan\\Sounds\\Anger.wav",
19 sound SOUND_LAVA_GROW     "Models\\Enemies\\ElementalMan\\Sounds\\Grow.wav",

// [Cecil] Rev: Textures for each type
20 texture TEXTURE_AIR_MAN      "Models\\Enemies\\ElementalMan\\AirMan.tex",
22 texture TEXTURE_AIR_DETAIL   "Models\\Enemies\\ElementalMan\\AirDetail.tex",
23 texture TEXTURE_AIR_FLARE    "Models\\Enemies\\ElementalMan\\AirFlare.tex",
30 texture TEXTURE_LAVA_MAN     "Models\\Enemies\\ElementalMan\\LavaMan.tex",
31 texture TEXTURE_LAVA_DETAIL  "Models\\Enemies\\ElementalMan\\LavaDetail.tex",
32 texture TEXTURE_LAVA_FLARE   "Models\\Enemies\\ElementalMan\\LavaFlare.tex",
40 texture TEXTURE_EARTH_MAN    "Models\\Enemies\\ElementalMan\\EarthMan.tex",
42 texture TEXTURE_EARTH_DETAIL "Models\\Enemies\\ElementalMan\\EarthDetail.tex",
50 texture TEXTURE_WATER_MAN    "Models\\Enemies\\ElementalMan\\WaterMan.tex",
51 texture TEXTURE_WATER_FLARE  "Models\\Enemies\\ElementalMan\\WaterFlare.tex",
52 texture TEXTURE_WATER_DETAIL "Models\\Enemies\\ElementalMan\\WaterDetail.tex",

// [Cecil] Rev: Omni type
60 texture TEXTURE_OMNI_MAN      "Models\\Enemies\\ElementalMan\\OmniMan.tex",
61 texture TEXTURE_OMNI_DETAIL   "Models\\Enemies\\ElementalMan\\OmniDetail.tex",
62 model   MODEL_OMNI_SHOULDER1  "Models\\Enemies\\ElementalMan\\Attach\\Pleche1.mdl",
63 model   MODEL_OMNI_SHOULDER2  "Models\\Enemies\\ElementalMan\\Attach\\Pleche2.mdl",
64 model   MODEL_OMNI_ARM1       "Models\\Enemies\\ElementalMan\\Attach\\Ruka1.mdl",
65 model   MODEL_OMNI_ARM2       "Models\\Enemies\\ElementalMan\\Attach\\Ruka2.mdl",
66 model   MODEL_OMNI_BACK       "Models\\Enemies\\ElementalMan\\Attach\\Spina.mdl",
67 model   MODEL_OMNI_BODY       "Models\\Enemies\\ElementalMan\\Attach\\Telo.mdl",
68 texture TEXTURE_OMNI_CHEST    "Models\\Enemies\\ElementalMan\\Attach\\Nagrrudnik.tex",
69 texture TEXTURE_OMNI_SHOULDER "Models\\Enemies\\ElementalMan\\Attach\\Pleche.tex",
70 texture TEXTURE_OMNI_ARM      "Models\\Enemies\\ElementalMan\\Attach\\Ruka.tex",
71 texture TEXTURE_OMNI_BACK     "Models\\Enemies\\ElementalMan\\Attach\\Spina.tex",
72 model   MODEL_OMNI            "Models\\Enemies\\ElementalMan\\ElementalCK.mdl",

functions:
  // describe how this enemy killed player
  virtual CTString GetPlayerKillDescription(const CTString &strPlayerName, const EDeath &eDeath)
  {
    CTString str;
    str.PrintF(TRANS("%s was killed by an elemental"), strPlayerName);

    // [Cecil] Rev: Message per type
    switch (m_EetType) {
      case ELT_AIR:   str.PrintF(TRANS("%s was blown away by a Air Golem"), strPlayerName); break;
      default: ASSERT(FALSE);
      case ELT_LAVA:  str.PrintF(LOCALIZE("%s was killed by a Lava Golem"), strPlayerName); break;
      case ELT_STONE: str.PrintF(TRANS("Earth Golem buried %s in the ground"), strPlayerName); break;
      case ELT_WATER: str.PrintF(TRANS("%s was drowned by a Water Golem"), strPlayerName); break;
      case ELT_OMNI:  str.PrintF(TRANS("Omni Golem blew %s into quantum state"), strPlayerName); break;
    }

    return str;
  }
  virtual const CTFileName &GetComputerMessageName(void) const {
    // [Cecil] Rev: Message per type
    static DECLARE_CTFILENAME(fnmAir,   "Data\\Messages\\Enemies\\ElementalManAir.txt");
    static DECLARE_CTFILENAME(fnmLava,  "Data\\Messages\\Enemies\\ElementalManLava.txt");
    static DECLARE_CTFILENAME(fnmStone, "Data\\Messages\\Enemies\\ElementalManEarth.txt");
    static DECLARE_CTFILENAME(fnmWater, "Data\\Messages\\Enemies\\ElementalManWater.txt");
    static DECLARE_CTFILENAME(fnmOmni,  "Data\\Messages\\Enemies\\ElementalManOmni.txt");

    switch (m_EetType) {
      case ELT_AIR: return fnmAir;
      default: ASSERT(FALSE);
      case ELT_LAVA: return fnmLava;
      case ELT_STONE: return fnmStone;
      case ELT_WATER: return fnmWater;
      case ELT_OMNI: return fnmOmni;
    }
  };

  // render burning particles
  void RenderParticles(void)
  {
    FLOAT fTimeFactor=1.0f;
    FLOAT fPower=0.25f;
    if (m_EesCurrentState==ELS_NORMAL)
    {
      FLOAT fDeathFactor=1.0f;
      if( m_fSpiritStartTime!=0.0f)
      {
        fDeathFactor=1.0f-Clamp(FLOAT(_pTimer->CurrentTick()-m_fSpiritStartTime)/DEATH_BURN_TIME, 0.0f, 1.0f);
      }

      // [Cecil] Rev: Only lava ones burn naturally
      if (m_EetType == ELT_LAVA) {
        Particles_Burning(this, fPower, fTimeFactor * fDeathFactor);
      }
    }
  }

  void Precache(void)
  {
    CEnemyBase::Precache();

    // [Cecil] Rev: Precache all projectiles per character and all models
    if (m_EecChar == ELC_LARGE) {
      PrecacheClass(CLASS_PROJECTILE, PRT_AIRMAN_LARGE);
      PrecacheClass(CLASS_PROJECTILE, PRT_LAVAMAN_BIG_BOMB);
      PrecacheClass(CLASS_PROJECTILE, PRT_EARTHMAN_BIG_BOMB);
      PrecacheClass(CLASS_PROJECTILE, PRT_WATERMAN_LARGE);
    }

    if (m_EecChar == ELC_LARGE || m_EecChar == ELC_BIG) {
      PrecacheClass(CLASS_PROJECTILE, PRT_AIRMAN_BIG);
      PrecacheClass(CLASS_PROJECTILE, PRT_LAVAMAN_BOMB);
      PrecacheClass(CLASS_PROJECTILE, PRT_EARTHMAN_BIG);
      PrecacheClass(CLASS_PROJECTILE, PRT_WATERMAN_BIG);
    }

    PrecacheClass(CLASS_PROJECTILE, PRT_AIRMAN_SMALL);
    PrecacheClass(CLASS_PROJECTILE, PRT_LAVAMAN_STONE);
    PrecacheClass(CLASS_PROJECTILE, PRT_EARTHMAN_STONE);
    PrecacheClass(CLASS_PROJECTILE, PRT_WATERMAN_SMALL);

    PrecacheModel(MODEL_EARTH);
    PrecacheModel(MODEL_AIR);
    PrecacheModel(MODEL_GENERIC);
    PrecacheModel(MODEL_BODY_FLARE);
    PrecacheModel(MODEL_HAND_FLARE);

    PrecacheTexture(TEXTURE_AIR_MAN);
    PrecacheTexture(TEXTURE_AIR_DETAIL);
    PrecacheTexture(TEXTURE_AIR_FLARE);
    PrecacheTexture(TEXTURE_LAVA_MAN);
    PrecacheTexture(TEXTURE_LAVA_DETAIL);
    PrecacheTexture(TEXTURE_LAVA_FLARE);
    PrecacheTexture(TEXTURE_EARTH_MAN);
    PrecacheTexture(TEXTURE_EARTH_DETAIL);
    PrecacheTexture(TEXTURE_WATER_MAN);
    PrecacheTexture(TEXTURE_WATER_FLARE);
    PrecacheTexture(TEXTURE_WATER_DETAIL);

    PrecacheTexture(TEXTURE_OMNI_MAN);
    PrecacheTexture(TEXTURE_OMNI_DETAIL);
    PrecacheModel(MODEL_OMNI_SHOULDER1);
    PrecacheModel(MODEL_OMNI_SHOULDER2);
    PrecacheModel(MODEL_OMNI_ARM1);
    PrecacheModel(MODEL_OMNI_ARM2);
    PrecacheModel(MODEL_OMNI_BACK);
    PrecacheModel(MODEL_OMNI_BODY);
    PrecacheTexture(TEXTURE_OMNI_CHEST);
    PrecacheTexture(TEXTURE_OMNI_SHOULDER);
    PrecacheTexture(TEXTURE_OMNI_ARM);
    PrecacheTexture(TEXTURE_OMNI_BACK);
    PrecacheModel(MODEL_OMNI);

    PrecacheSound(SOUND_LAVA_IDLE);
    PrecacheSound(SOUND_LAVA_WOUND);
    PrecacheSound(SOUND_LAVA_FIRE);
    PrecacheSound(SOUND_LAVA_KICK);
    PrecacheSound(SOUND_LAVA_DEATH);
    PrecacheSound(SOUND_LAVA_ANGER);
    PrecacheSound(SOUND_LAVA_LAVABURN);
    PrecacheSound(SOUND_LAVA_GROW);
  };

  /* Entity info */
  void *GetEntityInfo(void) {
    // [Cecil] Rev: Same entity info for all types
    switch(m_EecChar) {
      case ELC_LARGE: return &eiElementalLarge;
      case ELC_BIG:   return &eiElementalBig;
    }
    return &eiElementalSmall;
  };

  /* Fill in entity statistics - for AI purposes only */
  BOOL FillEntityStatistics(EntityStats *pes)
  {
    CEnemyBase::FillEntityStatistics(pes);
    switch(m_EetType) {
    case ELT_WATER : { pes->es_strName+=" Water"; } break;
    case ELT_AIR   : { pes->es_strName+=" Air"; } break;
    case ELT_STONE : { pes->es_strName+=" Stone"; } break;
    case ELT_LAVA  : { pes->es_strName+=" Lava"; } break;
    case ELT_OMNI  : { pes->es_strName+=" Omni"; } break; // [Cecil] Rev
    }
    switch(m_EecChar) {
    case ELC_LARGE: pes->es_strName+=" Large"; break;
    case ELC_BIG:   pes->es_strName+=" Big"; break;
    case ELC_SMALL: pes->es_strName+=" Small"; break;
    }
    return TRUE;
  }

  /* Receive damage */
  void ReceiveDamage(CEntity *penInflictor, enum DamageType dmtType,
    FLOAT fDamageAmmount, const FLOAT3D &vHitPoint, const FLOAT3D &vDirection) 
  {
    // [Cecil] Rev: Take half of the close range damage
    if (dmtType == DMT_CLOSERANGE) {
      fDamageAmmount *= 0.5f;

    } else if (dmtType == DMT_BURNING) {
      // [Cecil] Rev: Take no flamethrower damage as Lava elemental
      if (m_EetType == ELT_LAVA) {
        fDamageAmmount = 0.0f;

      // [Cecil] Rev: Take half of the flamethrower damage as Water elemental
      } else if (m_EetType == ELT_WATER) {
        fDamageAmmount *= 0.5f;
      }
    }

    // elemental can't harm elemental
    if( IsOfClass(penInflictor, "Elemental")) {
      return;
    }

    // boss can't be telefragged
    if( m_EecChar==ELC_LARGE && dmtType==DMT_TELEPORT)
    {
      return;
    }

    // elementals take less damage from heavy bullets (e.g. sniper)
    if( m_EecChar==ELC_BIG && dmtType==DMT_BULLET && fDamageAmmount>100.0f)
    {
      fDamageAmmount/=2.5f;
    }

    // [Cecil] Rev: Air elementals ignore close range and bullet damage
    if (m_EetType == ELT_AIR && (dmtType == DMT_CLOSERANGE || dmtType == DMT_BULLET)) {
      return;
    }

    INDEX ctShouldSpawn = Clamp( INDEX((m_fMaxHealth-GetHealth())/m_fSpawnDamage), INDEX(0), INDEX(10));
    CTString strChar = ElementalCharacter_enum.NameForValue(INDEX(m_EecChar));
    //CPrintF( "Character: %s, MaxHlt = %g, Hlt = %g, SpwnDmg = %g, Spawned: %d, Should: %d\n",
    //  strChar, m_fMaxHealth, GetHealth(), m_fSpawnDamage, m_ctSpawned, ctShouldSpawn);

    if (m_bSpawnEnabled && m_bSpawnWhenHarmed && (m_EecChar==ELC_LARGE || m_EecChar==ELC_BIG))
    {
      INDEX ctShouldSpawn = Clamp( INDEX((m_fMaxHealth-GetHealth())/m_fSpawnDamage), INDEX(0), INDEX(10));
      if(m_ctSpawned<ctShouldSpawn)
      {
        SendEvent( EForceWound() );
      }
    }

    // if not in normal state can't be harmed
    if (m_EesCurrentState!=ELS_NORMAL) {
      return;
    }

    CEnemyBase::ReceiveDamage(penInflictor, dmtType, fDamageAmmount, vHitPoint, vDirection);
  };

  void LeaveStain( BOOL bGrow)
  {
    return;
  }

  // damage anim
  INDEX AnimForDamage(FLOAT fDamage) {
    INDEX iAnim;

    switch (IRnd() % 3) {
      case 0:  iAnim = ELEMENTALLAVA_ANIM_WOUND01; break;
      case 1:  iAnim = ELEMENTALLAVA_ANIM_WOUND02; break;
      default: iAnim = ELEMENTALLAVA_ANIM_WOUND03; break;
    }

    StartModelAnim(iAnim, 0);
    return iAnim;
  };

  void StandingAnimFight(void) {
    StartModelAnim(ELEMENTALLAVA_ANIM_STANDFIGHT, AOF_LOOPING|AOF_NORESTART);
  }

  // virtual anim functions
  void StandingAnim(void) {
    switch (m_EesCurrentState) {
      case ELS_NORMAL: StartModelAnim(ELEMENTALLAVA_ANIM_WALKBIG, AOF_LOOPING|AOF_NORESTART); break;
      case ELS_BOX: StartModelAnim(ELEMENTALLAVA_ANIM_MELTFLY, AOF_LOOPING|AOF_NORESTART); break;
      //case ELS_PLANE:
      default: StartModelAnim(ELEMENTALLAVA_ANIM_STANDPLANE, AOF_LOOPING|AOF_NORESTART); break;
    }
  };

  void WalkingAnim(void)
  {
    if (m_EecChar == ELC_LARGE) {
      StartModelAnim(ELEMENTALLAVA_ANIM_WALKBIG, AOF_LOOPING|AOF_NORESTART);
    } else if (m_EecChar == ELC_BIG) {
      StartModelAnim(ELEMENTALLAVA_ANIM_RUNMEDIUM, AOF_LOOPING|AOF_NORESTART);
    } else {
      StartModelAnim(ELEMENTALLAVA_ANIM_RUNSMALL, AOF_LOOPING|AOF_NORESTART);
    }
  };

  void RunningAnim(void) {
    WalkingAnim();
  };

  void RotatingAnim(void) {
    WalkingAnim();
  };

  INDEX AnimForDeath(void) {
    INDEX iAnim = ELEMENTALLAVA_ANIM_DEATH03;
    StartModelAnim(iAnim, 0);
    return iAnim;
  };

  // virtual sound functions
  void IdleSound(void) {
    PlaySound(m_soSound, SOUND_LAVA_IDLE, SOF_3D);
  };
  void SightSound(void) {
  };
  void WoundSound(void) {
    PlaySound(m_soSound, SOUND_LAVA_WOUND, SOF_3D);
  };
  void DeathSound(void) {
    PlaySound(m_soSound, SOUND_LAVA_DEATH, SOF_3D);
  };

  BOOL CountAsKill(void)
  {
    return m_bCountAsKill;
  }

  // spawn new elemental
  void SpawnNewElemental(void) 
  {
    INDEX ctShouldSpawn = Clamp( INDEX((m_fMaxHealth-GetHealth())/m_fSpawnDamage), INDEX(0), INDEX(10));
    // disable too much spawning
    if (m_bSpawnOnBlowUp && (m_EecChar==ELC_LARGE || m_EecChar==ELC_BIG) && (GetHealth()<=0.0f) )
    {
      ctShouldSpawn+=2;
    }

    ASSERT(m_ctSpawned<=ctShouldSpawn);
    if(m_ctSpawned>=ctShouldSpawn)
    {
      return;
    }

    CPlacement3D pl;
    // spawn placement
    if (m_EecChar==ELC_LARGE) {
      pl = CPlacement3D(LAVAMAN_SPAWN_LARGE, ANGLE3D(-90.0f+FRnd()*180.0f, 30+FRnd()*30, 0));
    } else {
      pl = CPlacement3D(LAVAMAN_SPAWN_BIG, ANGLE3D(-90.0f+FRnd()*180.0f, 40+FRnd()*20, 0));
    }
    pl.RelativeToAbsolute(GetPlacement());

    // create entity
    CEntityPointer pen = GetWorld()->CreateEntity(pl, GetClass());

    // [Cecil] Rev: Omni spawns random types
    if (m_EetType == ELT_OMNI) {
      switch (IRnd() % 4) {
        case 0: ((CElemental&)*pen).m_EetType = ELT_AIR; break;
        case 1: ((CElemental&)*pen).m_EetType = ELT_LAVA; break;
        case 2: ((CElemental&)*pen).m_EetType = ELT_STONE; break;
        case 3: ((CElemental&)*pen).m_EetType = ELT_WATER; break;
      }
    } else {
      ((CElemental&)*pen).m_EetType = m_EetType;
    }

    // elemental size
    if (m_EecChar==ELC_LARGE) {
      ((CElemental&)*pen).m_EecChar = ELC_BIG;
    } else {
      ((CElemental&)*pen).m_EecChar = ELC_SMALL;
    }
    // start properties
    ((CElemental&)*pen).m_EesStartState = ELS_BOX;
    ((CElemental&)*pen).m_fDensity = m_fDensity;
    ((CElemental&)*pen).m_colColor = m_colColor;
    ((CElemental&)*pen).m_penEnemy = m_penEnemy;
    ((CElemental&)*pen).m_ttTarget = m_ttTarget;
    ((CElemental&)*pen).m_bSpawned = TRUE;
    pen->Initialize(EVoid());
    // set moving
    if (m_EecChar==ELC_LARGE) {
      ((CElemental&)*pen).LaunchAsFreeProjectile(FLOAT3D(0, 0, -40.0f), this);
    } else {
      ((CElemental&)*pen).LaunchAsFreeProjectile(FLOAT3D(0, 0, -20.0f), this);
    }
    ((CElemental&)*pen).SetDesiredRotation(ANGLE3D(0, 0, FRnd()*360-180));

    // spawn particle debris explosion
    CEntity *penSpray = CreateEntity( pl, CLASS_BLOOD_SPRAY);
    penSpray->SetParent( pen);
    ESpawnSpray eSpawnSpray;
    eSpawnSpray.fDamagePower = 4.0f;
    eSpawnSpray.fSizeMultiplier = 0.5f;

    // [Cecil] Rev: Set spray particles per type
    switch (m_EetType) {
      case ELT_AIR:   eSpawnSpray.sptType = SPT_AIRSPOUTS; break;
      case ELT_LAVA:  eSpawnSpray.sptType = SPT_LAVA_STONES; break;
      case ELT_STONE: eSpawnSpray.sptType = SPT_STONES; break;
      case ELT_WATER: eSpawnSpray.sptType = SPT_SMALLPLASMA; break;
      case ELT_OMNI:  eSpawnSpray.sptType = SPT_SMALLPLASMA; break;
    }

    eSpawnSpray.vDirection = FLOAT3D(0,-0.5f,0);
  #if SE1_GAME != SS_TFE
    eSpawnSpray.colBurnColor=C_WHITE|CT_OPAQUE;
  #endif
    eSpawnSpray.penOwner = pen;
    penSpray->Initialize( eSpawnSpray);
    m_ctSpawned++;
  };

  // [Cecil] Rev: Modified to work with both Big and Large characters to eliminate code duplication
  void BossFirePredictedLavaRock(FLOAT3D vFireingRel, BOOL bLarge)
  {
    FLOAT3D vTarget = m_penEnemy->GetPlacement().pl_PositionVector;
    FLOAT3D vSpeedDest = ((CMovableEntity&) *m_penEnemy).en_vCurrentTranslationAbsolute;
    FLOAT fLaunchSpeed;
    FLOAT fRelativeHdg;

    // [Cecil] For Big characters
    FLOAT3D vShooting = GetPlacement().pl_PositionVector;
    FLOAT fPitch = 20.0f;

    // [Cecil] For Large characters
    if (bLarge) {
      vShooting += vFireingRel * GetRotationMatrix();

      FLOAT fDistanceFactor = ClampUp((vShooting - vTarget).Length() / 150.0f, 1.0f) - 0.75f;
      fPitch = fDistanceFactor * 45.0f;
    }

    // calculate parameters for predicted angular launch curve
    EntityInfo *peiTarget = (EntityInfo*) (m_penEnemy->GetEntityInfo());

    // [Cecil] Projectile to launch
    ProjectileType eProjectile = PRT_LAVAMAN_BOMB;
    
    // [Cecil] For Large characters
    if (bLarge) {
      CalculateAngularLaunchParams(vShooting, peiTarget->vTargetCenter[1] - 6.0f / 3.0f, vTarget, 
        vSpeedDest, fPitch, fLaunchSpeed, fRelativeHdg);

      // [Cecil] Rev: Select projectile based on type
      switch (m_EetType) {
        case ELT_AIR:   eProjectile = _aLargeProjectiles[0]; break;
        case ELT_LAVA:  eProjectile = _aLargeProjectiles[1]; break;
        case ELT_STONE: eProjectile = _aLargeProjectiles[2]; break;
        case ELT_WATER: eProjectile = _aLargeProjectiles[3]; break;
        case ELT_OMNI:  eProjectile = _aLargeProjectiles[IRnd() % 4]; break; // Random
      }

    // [Cecil] For Big characters
    } else {
      CalculateAngularLaunchParams(vShooting, vFireingRel(2) - peiTarget->vTargetCenter[1] - 1.5f / 3.0f, vTarget,
        vSpeedDest, fPitch, fLaunchSpeed, fRelativeHdg);

      // [Cecil] Rev: Select projectile based on type
      switch (m_EetType) {
        case ELT_AIR:   eProjectile = _aBigProjectiles[0]; break;
        case ELT_LAVA:  eProjectile = _aBigProjectiles[1]; break;
        case ELT_STONE: eProjectile = _aBigProjectiles[2]; break;
        case ELT_WATER: eProjectile = _aBigProjectiles[3]; break;
        case ELT_OMNI:  eProjectile = _aBigProjectiles[IRnd() % 4]; break; // Random
      }
    }

    // target enemy body
    FLOAT3D vShootTarget;
    GetEntityInfoPosition(m_penEnemy, peiTarget->vTargetCenter, vShootTarget);
    // launch
    CPlacement3D pl;
    PrepareFreeFlyingProjectile(pl, vShootTarget, vFireingRel, ANGLE3D( fRelativeHdg, fPitch, 0));
    CEntityPointer penProjectile = CreateEntity(pl, CLASS_PROJECTILE);
    ELaunchProjectile eLaunch;
    eLaunch.penLauncher = this;
    eLaunch.prtType = eProjectile;
    eLaunch.fSpeed = fLaunchSpeed;
    penProjectile->Initialize(eLaunch);
  }

  // [Cecil] Wrapper method for compatibility
  class CWorldSettingsController *GetWSC(void) {
  #if SE1_GAME != SS_TFE
    return ::GetWSC(this);

  #else
    // Obtain world settings controller from the background viewer
    CWorldSettingsController *pwsc = NULL;
    CBackgroundViewer *penBcgViewer = (CBackgroundViewer *)GetWorld()->GetBackgroundViewer();

    if (penBcgViewer != NULL) {
      pwsc = (CWorldSettingsController *)&*penBcgViewer->m_penWorldSettingsController;
    }

    return pwsc;
  #endif
  };

  /* Shake ground */
  void ShakeItBaby(FLOAT tmShaketime, FLOAT fPower)
  {
    CWorldSettingsController *pwsc = GetWSC();
    if (pwsc!=NULL) {
      pwsc->m_tmShakeStarted = tmShaketime;
      pwsc->m_vShakePos = GetPlacement().pl_PositionVector;
      pwsc->m_fShakeFalloff = 450.0f;
      pwsc->m_fShakeFade = 3.0f;

      pwsc->m_fShakeIntensityZ = 0;
      pwsc->m_tmShakeFrequencyZ = 5.0f;
      pwsc->m_fShakeIntensityY = 0.1f*fPower;
      pwsc->m_tmShakeFrequencyY = 5.0f;
      pwsc->m_fShakeIntensityB = 2.5f*fPower;
      pwsc->m_tmShakeFrequencyB = 7.2f;

    #if SE1_GAME != SS_TFE
      pwsc->m_bShakeFadeIn = FALSE;
    #endif
    }
  }

  void SpawnShockwave(FLOAT fSize)
  {
    CPlacement3D pl = GetPlacement();
    pl.pl_PositionVector(2) += 0.1f;
    CEntityPointer penShockwave = CreateEntity(pl, CLASS_BASIC_EFFECT);
        
    ESpawnEffect eSpawnEffect;
    eSpawnEffect.colMuliplier = C_WHITE|CT_OPAQUE;
    eSpawnEffect.betType = BET_CANNONSHOCKWAVE;
    eSpawnEffect.vStretch = FLOAT3D(fSize, fSize, fSize);
    penShockwave->Initialize(eSpawnEffect);
  }

  // hit ground
  void HitGround(void) {
    FLOAT3D vSource;
    if( m_penEnemy != NULL)
    {
      vSource = GetPlacement().pl_PositionVector +
      FLOAT3D(m_penEnemy->en_mRotation(1, 2), m_penEnemy->en_mRotation(2, 2), m_penEnemy->en_mRotation(3, 2));
    }
    else
    {
      vSource = GetPlacement().pl_PositionVector;
    }

    // damage
    if (m_EecChar==ELC_LARGE) {
      InflictRangeDamage(this, DMT_IMPACT, 150.0f, vSource, 7.5f, m_fCloseDistance);
      ShakeItBaby(_pTimer->CurrentTick(), 5.0f);
      SpawnShockwave(10.0f);
    } else if (m_EecChar==ELC_BIG) {
      InflictRangeDamage(this, DMT_IMPACT, 75.0f, vSource, 5.0f, m_fCloseDistance);
      ShakeItBaby(_pTimer->CurrentTick(), 2.0f);
      SpawnShockwave(3.0f);
    } else {
      InflictRangeDamage(this, DMT_IMPACT, 25.0f, vSource, 2.5f, m_fCloseDistance);
      SpawnShockwave(1.0f);
    }
  };

  // add attachments
  void AddAttachments(void) {
    switch (m_EetType) {
      case ELT_AIR: {
        if (GetModelObject()->GetAttachmentModel(ELEMENTALLAVA_ATTACHMENT_BODY_FLARE) == NULL) {
          AddAttachmentToModel(this, *GetModelObject(), ELEMENTALLAVA_ATTACHMENT_BODY_FLARE,       MODEL_BODY_FLARE, TEXTURE_AIR_FLARE, 0, 0, 0);
          AddAttachmentToModel(this, *GetModelObject(), ELEMENTALLAVA_ATTACHMENT_RIGHT_HAND_FLARE, MODEL_HAND_FLARE, TEXTURE_AIR_FLARE, 0, 0, 0);
          AddAttachmentToModel(this, *GetModelObject(), ELEMENTALLAVA_ATTACHMENT_LEFT_HAND_FLARE,  MODEL_HAND_FLARE, TEXTURE_AIR_FLARE, 0, 0, 0);
        }
      } break;

      case ELT_LAVA: {
        if (GetModelObject()->GetAttachmentModel(ELEMENTALLAVA_ATTACHMENT_BODY_FLARE) == NULL) {
          AddAttachmentToModel(this, *GetModelObject(), ELEMENTALLAVA_ATTACHMENT_BODY_FLARE,       MODEL_BODY_FLARE, TEXTURE_LAVA_FLARE, 0, 0, 0);
          AddAttachmentToModel(this, *GetModelObject(), ELEMENTALLAVA_ATTACHMENT_RIGHT_HAND_FLARE, MODEL_HAND_FLARE, TEXTURE_LAVA_FLARE, 0, 0, 0);
          AddAttachmentToModel(this, *GetModelObject(), ELEMENTALLAVA_ATTACHMENT_LEFT_HAND_FLARE,  MODEL_HAND_FLARE, TEXTURE_LAVA_FLARE, 0, 0, 0);
        }
      } break;

      case ELT_WATER: {
        if (GetModelObject()->GetAttachmentModel(ELEMENTALLAVA_ATTACHMENT_BODY_FLARE) == NULL) {
          AddAttachmentToModel(this, *GetModelObject(), ELEMENTALLAVA_ATTACHMENT_BODY_FLARE,       MODEL_BODY_FLARE, TEXTURE_WATER_FLARE, 0, 0, 0);
          AddAttachmentToModel(this, *GetModelObject(), ELEMENTALLAVA_ATTACHMENT_RIGHT_HAND_FLARE, MODEL_HAND_FLARE, TEXTURE_WATER_FLARE, 0, 0, 0);
          AddAttachmentToModel(this, *GetModelObject(), ELEMENTALLAVA_ATTACHMENT_LEFT_HAND_FLARE,  MODEL_HAND_FLARE, TEXTURE_WATER_FLARE, 0, 0, 0);
        }
      } break;

      case ELT_OMNI: {
        AddAttachmentToModel(this, *GetModelObject(), 0, MODEL_OMNI_BACK,      TEXTURE_OMNI_BACK, 0, 0, 0);
        AddAttachmentToModel(this, *GetModelObject(), 1, MODEL_OMNI_SHOULDER1, TEXTURE_OMNI_SHOULDER, 0, 0, 0);
        AddAttachmentToModel(this, *GetModelObject(), 2, MODEL_OMNI_SHOULDER2, TEXTURE_OMNI_SHOULDER, 0, 0, 0);
        AddAttachmentToModel(this, *GetModelObject(), 3, MODEL_OMNI_BODY,      TEXTURE_OMNI_CHEST, 0, 0, 0);
        AddAttachmentToModel(this, *GetModelObject(), 4, MODEL_OMNI_ARM1,      TEXTURE_OMNI_ARM, 0, 0, 0);
        AddAttachmentToModel(this, *GetModelObject(), 5, MODEL_OMNI_ARM2,      TEXTURE_OMNI_ARM, 0, 0, 0);
      } break;
    }

    GetModelObject()->StretchModel(GetModelObject()->mo_Stretch);
    ModelChangeNotify();
  };

  // remove attachments
  void RemoveAttachments(void) {
    // [Cecil] Rev: Remove flares for everyone
    RemoveAttachmentFromModel(*GetModelObject(), ELEMENTALLAVA_ATTACHMENT_BODY_FLARE);
    RemoveAttachmentFromModel(*GetModelObject(), ELEMENTALLAVA_ATTACHMENT_RIGHT_HAND_FLARE);
    RemoveAttachmentFromModel(*GetModelObject(), ELEMENTALLAVA_ATTACHMENT_LEFT_HAND_FLARE);

    if (m_EetType == ELT_OMNI) {
      RemoveAttachmentFromModel(*GetModelObject(), 0);
      RemoveAttachmentFromModel(*GetModelObject(), 1);
      RemoveAttachmentFromModel(*GetModelObject(), 2);
      RemoveAttachmentFromModel(*GetModelObject(), 3);
      RemoveAttachmentFromModel(*GetModelObject(), 4);
      RemoveAttachmentFromModel(*GetModelObject(), 5);
    }
  };

/************************************************************
 *                 BLOW UP FUNCTIONS                        *
 ************************************************************/
  // [Cecil] Spawn one armor debris
  void SpawnArmorDebris(SLONG idModel, SLONG idTexture, FLOAT fEntitySize, INDEX iAttachment) {
  #if SE1_GAME != SS_TFE
    FLOAT3D vSpeed(FRnd() * 0.6f + 0.2f, FRnd() * 0.6f + 0.2f, FRnd() * 0.6f + 0.2f);
    ANGLE3D vRotation(FRnd() * 180.0f - 90.0f, FRnd() * 180.0f - 90.0f, FRnd() * 180.0f - 90.0f);

    CPlacement3D plAttach = GetModelObject()->GetAttachmentPlacement(*GetModelObject()->GetAttachmentModel(iAttachment));
    plAttach.RelativeToAbsolute(GetPlacement());

    Debris_Spawn_Independent(this, this, idModel, idTexture, 0, 0, 0, 0, fEntitySize, plAttach, vSpeed, vRotation);
  #endif
  };

  // [Cecil] Rev: Make Omni elemental drop its armor
  void OmniBlowUp(void) {
    if (m_EetType == ELT_OMNI) {
      // Calculate model size
      FLOAT fEntitySize = GetModelObject()->mo_Stretch.ManhattanNorm() / 3.0f;

      // Spawn debris
      Debris_Begin(EIBT_FLESH, DPT_NONE, BET_NONE, fEntitySize, FLOAT3D(0, 1, 0), FLOAT3D(0, 0, 0), 5.0f, 2.0f);

      SpawnArmorDebris(MODEL_OMNI_BACK,      TEXTURE_OMNI_BACK,     fEntitySize, 0);
      SpawnArmorDebris(MODEL_OMNI_SHOULDER1, TEXTURE_OMNI_SHOULDER, fEntitySize, 1);
      SpawnArmorDebris(MODEL_OMNI_SHOULDER2, TEXTURE_OMNI_SHOULDER, fEntitySize, 2);
      SpawnArmorDebris(MODEL_OMNI_BODY,      TEXTURE_OMNI_CHEST,    fEntitySize, 3);
      SpawnArmorDebris(MODEL_OMNI_ARM1,      TEXTURE_OMNI_ARM,      fEntitySize, 4);
      SpawnArmorDebris(MODEL_OMNI_ARM2,      TEXTURE_OMNI_ARM,      fEntitySize, 5);

      // Remove armor attachments
      RemoveAttachments();
    }
  };


  // adjust sound and watcher parameters here if needed
  void EnemyPostInit(void) 
  {
    if (m_EecChar == ELC_LARGE)
    {
      m_soBackground.Set3DParameters(400.0f, 0.0f, 1.0f, 1.0f);
      m_soSound.Set3DParameters(400.0f, 50.0f, 1.0f, 1.0f);
      m_soFireL.Set3DParameters(400.0f, 50.0f, 1.0f, 1.0f);
      m_soFireR.Set3DParameters(400.0f, 50.0f, 1.0f, 1.0f);
    }
    else if (m_EecChar == ELC_BIG)
    {
      m_soBackground.Set3DParameters(150.0f, 15.0f, 0.5f, 1.0f);
      m_soSound.Set3DParameters(200.0f, 0.0f, 1.0f, 1.0f);
      m_soFireL.Set3DParameters(200.0f, 0.0f, 1.0f, 1.0f);
      m_soFireR.Set3DParameters(200.0f, 0.0f, 1.0f, 1.0f);
    }
  };

procedures:
/************************************************************
 *                    CLASS INTERNAL                        *
 ************************************************************/
  FallOnFloor(EVoid) {
    // drop to floor
    SetPhysicsFlags(EPF_MODEL_WALKING);
    // wait at most 10 seconds
    wait (10.0f) {
      on (ETimer) : { stop; }
      on (EBegin) : { resume; }
      // if a brush is touched
      on (ETouch et) : {
        if (et.penOther->GetRenderType()&RT_BRUSH) {
          // stop waiting
          StopMoving();
          stop;
        }
        resume;
      }
      otherwise() : { resume; }
    }
    StartModelAnim(ELEMENTALLAVA_ANIM_MELTUP, 0);
    return EReturn();
  };

/************************************************************
 *                PROCEDURES WHEN HARMED                    *
 ************************************************************/
  // Play wound animation and falling body part
  BeWounded(EDamage eDamage) : CEnemyBase::BeWounded {
    // spawn additional elemental
    if( m_bSpawnEnabled)
    {
      SpawnNewElemental();
    }
    jump CEnemyBase::BeWounded(eDamage);
  };

/************************************************************
 *                 CHANGE STATE PROCEDURES                  *
 ************************************************************/
   // box to normal
  BoxToNormal(EVoid) {
    m_EesCurrentState = ELS_NORMAL;
    SetPhysicsFlags(EPF_MODEL_WALKING);
    ChangeCollisionBoxIndexWhenPossible(ELEMENTALLAVA_COLLISION_BOX_NORMAL);
    PlaySound(m_soFireL, SOUND_LAVA_GROW, SOF_3D);
    StartModelAnim(ELEMENTALLAVA_ANIM_MELTUP, 0);
    AddAttachments();
    autowait(GetModelObject()->GetAnimLength(ELEMENTALLAVA_ANIM_MELTUP));
    return EReturn();
  };

  // normal to box
/*  NormalToBox(EVoid) {
    StartModelAnim(STONEMAN_ANIM_MORPHBOXDOWN, 0);
    autowait(GetModelObject()->GetAnimLength(STONEMAN_ANIM_MORPHBOXDOWN));
    m_EesCurrentState = ELS_BOX;
    SetPhysicsFlags(EPF_BOX_PLANE_ELEMENTAL);
    ChangeCollisionBoxIndexWhenPossible(STONEMAN_COLLISION_BOX_BOX);
    RemoveAttachments();
    return EReturn();
  };*/

  // plane to normal
  PlaneToNormal(EVoid) {
    m_EesCurrentState = ELS_NORMAL;
    SwitchToModel();
    SetPhysicsFlags(EPF_MODEL_WALKING);
    ChangeCollisionBoxIndexWhenPossible(ELEMENTALLAVA_COLLISION_BOX_NORMAL);
    PlaySound(m_soFireL, SOUND_LAVA_GROW, SOF_3D);
    StartModelAnim(ELEMENTALLAVA_ANIM_MELTUP, 0);
    AddAttachments();
    autowait(GetModelObject()->GetAnimLength(ELEMENTALLAVA_ANIM_MELTUP));
    return EReturn();
  };

/************************************************************
 *                A T T A C K   E N E M Y                   *
 ************************************************************/
  InitializeAttack(EVoid) : CEnemyBase::InitializeAttack {
    // change state from box to normal
    if (m_EesCurrentState==ELS_BOX)
    {
      autocall BoxToNormal() EReturn;
    }
    // change state from plane to normal
    else if (m_EesCurrentState==ELS_PLANE)
    {
      autocall PlaneToNormal() EReturn;
    }
    jump CEnemyBase::InitializeAttack();
  };

  Fire(EVoid) : CEnemyBase::Fire {
    // [Cecil] Rev: Moved LavamanFire() logic here since all types have the same attack pattern
    m_bSpawnEnabled = TRUE;

    // Shoot projectiles
    if (m_EecChar == ELC_LARGE) {
      CModelObject &mo = *GetModelObject();
      FLOAT tmWait = mo.GetAnimLength(mo.ao_iCurrentAnim) - mo.GetPassedTime();

      StartModelAnim(ELEMENTALLAVA_ANIM_ATTACKBOSS, AOF_SMOOTHCHANGE);

      autowait(tmWait + 0.95f);
      BossFirePredictedLavaRock(LAVAMAN_FIRE_LARGE_RIGHT, TRUE);
      PlaySound(m_soFireR, SOUND_LAVA_FIRE, SOF_3D);

      autowait(2.0150f - 0.95f);
      BossFirePredictedLavaRock(LAVAMAN_FIRE_LARGE_LEFT, TRUE);
      PlaySound(m_soFireL, SOUND_LAVA_FIRE, SOF_3D);

      StartModelAnim(ELEMENTALLAVA_ANIM_WALKBIG, AOF_SMOOTHCHANGE);
      autocall CMovableModelEntity::WaitUntilScheduledAnimStarts() EReturn;
      MaybeSwitchToAnotherPlayer();

      // Set next shoot time
      m_fShootTime = _pTimer->CurrentTick() + m_fAttackFireTime * (1.0f + FRnd() / 5.0f);
      return EReturn();

    } else if (m_EecChar == ELC_BIG) {
      CModelObject &mo = *GetModelObject();
      FLOAT tmWait = mo.GetAnimLength(mo.ao_iCurrentAnim) - mo.GetPassedTime();

      StartModelAnim(ELEMENTALLAVA_ANIM_ATTACKLEFTHAND, AOF_SMOOTHCHANGE);

      autowait(tmWait + 0.90f);
      BossFirePredictedLavaRock(LAVAMAN_FIRE_BIG, FALSE); // [Cecil]
      PlaySound(m_soSound, SOUND_LAVA_FIRE, SOF_3D);

    } else if (TRUE) {
      // Simple attack for small elementals
      CModelObject &mo = *GetModelObject();
      FLOAT tmWait = mo.GetAnimLength(mo.ao_iCurrentAnim) - mo.GetPassedTime();

      StartModelAnim(ELEMENTALLAVA_ANIM_ATTACKLEFTHAND, AOF_SMOOTHCHANGE);
      autowait(tmWait + 0.8f);

      // [Cecil] Rev: Select projectile based on type
      ProjectileType eProjectile = PRT_LAVAMAN_STONE;

      switch (m_EetType) {
        case ELT_AIR:   eProjectile = _aSmallProjectiles[0]; break;
        case ELT_LAVA:  eProjectile = _aSmallProjectiles[1]; break;
        case ELT_STONE: eProjectile = _aSmallProjectiles[2]; break;
        case ELT_WATER: eProjectile = _aSmallProjectiles[3]; break;
        case ELT_OMNI:  eProjectile = _aSmallProjectiles[IRnd() % 4]; break; // Random
      }

      ShootProjectile(eProjectile, LAVAMAN_FIRE_SMALL, ANGLE3D(0, 0, 0));
      PlaySound(m_soSound, SOUND_LAVA_FIRE, SOF_3D);
    }

    autowait(GetModelObject()->GetAnimLength(ELEMENTALLAVA_ANIM_ATTACKLEFTHAND) - 0.9f);

    StandingAnim();
    autowait(_pTimer->TickQuantum);

    if (m_EecChar != ELC_SMALL) {
      MaybeSwitchToAnotherPlayer();
    }

    // Set next shoot time
    m_fShootTime = _pTimer->CurrentTick() + m_fAttackFireTime * (1.0f + FRnd() / 5.0f);

    return EReturn();
  };

  Hit(EVoid) : CEnemyBase::Hit {
    // [Cecil] Rev: Moved LavamanHit() logic here
    StartModelAnim(ELEMENTALLAVA_ANIM_ATTACKTWOHANDS, 0);
    autowait(0.6f);

    HitGround();
    PlaySound(m_soFireL, SOUND_LAVA_KICK, SOF_3D);

    StartModelAnim(ELEMENTALLAVA_ANIM_WALKBIG, AOF_SMOOTHCHANGE);
    autocall CMovableModelEntity::WaitUntilScheduledAnimStarts() EReturn;

    return EReturn();
  };

/************************************************************
 *                    D  E  A  T  H                         *
 ************************************************************/
  Death(EVoid) : CEnemyBase::Death
  {
    if (m_bSpawnOnBlowUp && (m_EecChar==ELC_LARGE || m_EecChar==ELC_BIG)) {
      SpawnNewElemental();
      SpawnNewElemental();

      // [Cecil] Rev: Omni spawns two more
      if (m_EetType == ELT_OMNI) {
        SpawnNewElemental();
        SpawnNewElemental();
      }
    }

    OmniBlowUp(); // [Cecil] Rev

    autocall CEnemyBase::Death() EEnd;
    GetModelObject()->mo_toBump.SetData( NULL);
    return EEnd();
  };

  BossAppear(EVoid)
  {
    autowait(2.0f);
    m_fFadeStartTime = _pTimer->CurrentTick();
    GetModelObject()->PlayAnim(ELEMENTALLAVA_ANIM_ANGER, 0);
    PlaySound(m_soSound, SOUND_LAVA_ANGER, SOF_3D);
    autowait(GetModelObject()->GetAnimLength(ELEMENTALLAVA_ANIM_ANGER)-_pTimer->TickQuantum);

    StartModelAnim(ELEMENTALLAVA_ANIM_ATTACKTWOHANDS, AOF_SMOOTHCHANGE);
    autowait(0.7f);
    HitGround();
    PlaySound(m_soFireL, SOUND_LAVA_KICK, SOF_3D);
    autowait(GetModelObject()->GetAnimLength(ELEMENTALLAVA_ANIM_ATTACKTWOHANDS)-0.7f-_pTimer->TickQuantum);

    StartModelAnim(ELEMENTALLAVA_ANIM_ATTACKTWOHANDS, 0);
    autowait(0.6f);
    HitGround();
    PlaySound(m_soFireR, SOUND_LAVA_KICK, SOF_3D);
    autowait(GetModelObject()->GetAnimLength(ELEMENTALLAVA_ANIM_ATTACKTWOHANDS)-0.6f-_pTimer->TickQuantum);


    return EReturn();
  }

  // overridable called before main enemy loop actually begins
  PreMainLoop(EVoid) : CEnemyBase::PreMainLoop
  {
    // if spawned by other entity
    if (m_bSpawned) {
      m_bSpawned = FALSE;
      m_bCountAsKill = FALSE;
      // wait till touching the ground
      autocall FallOnFloor() EReturn;
    }

    if (m_EecChar == ELC_LARGE || m_EecChar == ELC_BIG) {
      PlaySound(m_soBackground, SOUND_LAVA_LAVABURN, SOF_3D|SOF_LOOP);
    }

    if( m_EecChar==ELC_LARGE)
    {
      autocall BossAppear() EReturn;
    }
    return EReturn();
  }

/************************************************************
 *                       M  A  I  N                         *
 ************************************************************/
  Main(EVoid) {
    // [Cecil] Non-lava elementals can only be used on Revolution levels
    if (_EnginePatches._eWorldFormat != E_LF_SSR && m_EetType != ELT_LAVA) {
      m_EetType = ELT_LAVA;
    }

    // declare yourself as a model
    InitAsModel();
    // movable
    if (m_bMovable) {
      SetPhysicsFlags(EPF_MODEL_WALKING);
    // non movable
    } else {
      SetPhysicsFlags(EPF_MODEL_IMMATERIAL|EPF_MOVABLE);
    }
    SetCollisionFlags(ECF_MODEL);
    SetFlags(GetFlags()|ENF_ALIVE);
    en_fDensity = m_fDensity;
    m_fSpawnDamage = 1e6f;
    m_fDamageWounded = 1e6f;
    m_fBlowUpAmount = 1E30f;
    m_bSpawnEnabled = FALSE;
    m_bBoss = FALSE;

    // set your appearance
    switch (m_EetType) {
      case ELT_AIR:
        SetComponents(this, *GetModelObject(), MODEL_AIR, TEXTURE_AIR_MAN, 0, 0, TEXTURE_AIR_DETAIL);
        break;

      case ELT_LAVA:
        SetComponents(this, *GetModelObject(), MODEL_GENERIC, TEXTURE_LAVA_MAN, 0, 0, TEXTURE_LAVA_DETAIL);
        break;

      case ELT_STONE:
        SetComponents(this, *GetModelObject(), MODEL_EARTH, TEXTURE_EARTH_MAN, 0, 0, TEXTURE_EARTH_DETAIL);
        break;

      case ELT_WATER:
        SetComponents(this, *GetModelObject(), MODEL_GENERIC, TEXTURE_WATER_MAN, 0, 0, TEXTURE_WATER_DETAIL);
        break;

      case ELT_OMNI:
        SetComponents(this, *GetModelObject(), MODEL_OMNI, TEXTURE_OMNI_MAN, 0, 0, TEXTURE_OMNI_DETAIL);
        break;
    }
    ModelChangeNotify();

    // character settings
    if (m_EecChar==ELC_LARGE)
    {
      // this one is boss!
      m_bBoss = TRUE;

      // [Cecil] Rev: Set health and spray particles per type
      switch (m_EetType) {
        case ELT_AIR:
          m_sptType = SPT_AIRSPOUTS;
          SetHealth(10000.0f);
          break;

        case ELT_LAVA:
          m_sptType = SPT_SMALL_LAVA_STONES;
          SetHealth(10000.0f);
          break;

        case ELT_STONE:
          m_sptType = SPT_STONES;
          SetHealth(10000.0f);
          break;

        case ELT_WATER:
          m_sptType = SPT_SMALLPLASMA;
          SetHealth(7500.0f);
          break;

        case ELT_OMNI:
          m_sptType = SPT_SMALLPLASMA;
          SetHealth(16000.0f);
          break;
      }

      // after loosing this ammount of damage we will spawn new elemental
      m_fSpawnDamage = 2000.0f;
      // setup moving speed
      m_fWalkSpeed = FRnd()/2 + 1.0f;
      m_aWalkRotateSpeed = AngleDeg(FRnd()*10.0f + 25.0f);
      m_fAttackRunSpeed = FRnd() + 2.0f;
      m_aAttackRotateSpeed = AngleDeg(FRnd()*50 + 245.0f);
      m_fCloseRunSpeed = FRnd() + 2.0f;
      m_aCloseRotateSpeed = AngleDeg(FRnd()*50 + 245.0f);
      // setup attack distances
      m_fAttackDistance = 300.0f;
      m_fCloseDistance = 60.0f;
      m_fStopDistance = 30.0f;
      m_fAttackFireTime = 0.5f;
      m_fCloseFireTime = 1.0f;
      m_fIgnoreRange = 600.0f;
      m_iScore = 50000;
    }
    else if (m_EecChar==ELC_BIG)
    {
      // [Cecil] Rev: Set health and spray particles per type
      switch (m_EetType) {
        case ELT_AIR:
          m_sptType = SPT_AIRSPOUTS;
          SetHealth(1600.0f);
          break;

        case ELT_LAVA:
          m_sptType = SPT_LAVA_STONES;
          SetHealth(800.0f);
          break;

        case ELT_STONE:
          m_sptType = SPT_STONES;
          SetHealth(800.0f);
          break;

        case ELT_WATER:
          m_sptType = SPT_SMALLPLASMA;
          SetHealth(400.0f);
          break;

        case ELT_OMNI:
          m_sptType = SPT_SMALLPLASMA;
          SetHealth(2000.0f);
          break;
      }

      // after loosing this ammount of damage we will spawn new elemental
      m_fSpawnDamage = 500.0f;
      // setup moving speed
      m_fWalkSpeed = FRnd() + 1.5f;
      m_aWalkRotateSpeed = AngleDeg(FRnd()*10.0f + 25.0f);
      m_fAttackRunSpeed = FRnd()*1.0f + 6.0f;
      m_aAttackRotateSpeed = AngleDeg(FRnd()*50 + 300.0f);
      m_fCloseRunSpeed = FRnd()*2.0f + 2.0f;
      m_aCloseRotateSpeed = AngleDeg(FRnd()*50 + 300.0f);
      // setup attack distances
      m_fAttackDistance = 150.0f;
      m_fCloseDistance = 20.0f;
      m_fStopDistance = 5.0f;
      m_fAttackFireTime = 0.5f;
      m_fCloseFireTime = 1.0f;
      m_fIgnoreRange = 400.0f;
      // damage/explode properties
      m_iScore = 2500;
    }
    else
    {
      // [Cecil] Rev: Set health and spray particles per type
      switch (m_EetType) {
        case ELT_AIR:
          m_sptType = SPT_AIRSPOUTS;
          SetHealth(200.0f);
          break;

        case ELT_LAVA:
          m_sptType = SPT_LAVA_STONES;
          SetHealth(100.0f);
          break;

        case ELT_STONE:
          m_sptType = SPT_STONES;
          SetHealth(50.0f);
          break;

        case ELT_WATER:
          m_sptType = SPT_SMALLPLASMA;
          SetHealth(50.0f);
          break;

        case ELT_OMNI:
          m_sptType = SPT_SMALLPLASMA;
          SetHealth(300.0f);
          break;
      }

      // setup moving speed
      m_fWalkSpeed = FRnd() + 1.5f;
      m_aWalkRotateSpeed = AngleDeg(FRnd()*10.0f + 25.0f);
      m_fAttackRunSpeed = FRnd()*2.0f + 6.0f;
      m_aAttackRotateSpeed = AngleDeg(FRnd()*50 + 500.0f);
      m_fCloseRunSpeed = FRnd()*3.0f + 4.0f;
      m_aCloseRotateSpeed = AngleDeg(FRnd()*50 + 500.0f);
      // setup attack distances
      m_fAttackDistance = 100.0f;
      m_fCloseDistance = 10.0f;
      m_fStopDistance = 5.0f;
      m_fAttackFireTime = 1.5f;
      m_fCloseFireTime = 1.0f;
      m_fIgnoreRange = 200.0f;
      // damage/explode properties
      m_iScore = 500;
    }

    // [Cecil] For convenience
    m_fMaxHealth = GetHealth();

    // non movable
    if (!m_bMovable)
    {
      m_EesStartState = ELS_NORMAL;
      m_bSpawnWhenHarmed = FALSE;
      m_bSpawnOnBlowUp = FALSE;
      // fire count
      if (m_iFireCount <= 0)
      {
        WarningMessage("Entity: %s - Fire count must be greater than zero", GetName());
        m_iFireCount = 1;
      }
    }

    // state and flare attachments
    m_EesCurrentState = m_EesStartState;
    RemoveAttachments();
    switch (m_EesCurrentState) {
      case ELS_NORMAL:
        SetPhysicsFlags(EPF_MODEL_WALKING);
        AddAttachments();
        break;
      case ELS_BOX:
        SetPhysicsFlags(EPF_BOX_PLANE_ELEMENTAL);
        break;
      case ELS_PLANE:
        SetPhysicsFlags(EPF_MODEL_IMMATERIAL|EPF_MOVABLE);
        SwitchToEditorModel();
        break;
    }
    StandingAnim();

    // stretch
    if (m_EecChar==ELC_SMALL) {
      GetModelObject()->StretchModel(FLOAT3D(LAVAMAN_SMALL_STRETCH, LAVAMAN_SMALL_STRETCH, LAVAMAN_SMALL_STRETCH));
    }
    else if (m_EecChar==ELC_LARGE) {
      GetModelObject()->StretchModel(FLOAT3D(LAVAMAN_LARGE_STRETCH, LAVAMAN_LARGE_STRETCH, LAVAMAN_LARGE_STRETCH));
    } else if (m_EecChar==ELC_BIG) {
      GetModelObject()->StretchModel(FLOAT3D(LAVAMAN_BIG_STRETCH, LAVAMAN_BIG_STRETCH, LAVAMAN_BIG_STRETCH));
    }
    ModelChangeNotify();

    // continue behavior in base class
    jump CEnemyBase::MainLoop();
  };
};
