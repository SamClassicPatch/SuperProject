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

805
%{
#include "StdH.h"
#include "Models/Items/ItemHolder/ItemHolder.h"
%}

uses "Items/Item";

// key type 
enum KeyItemType {
  0 KIT_BOOKOFWISDOM      "Book of wisdom",
  1 KIT_CROSSWOODEN       "Wooden cross",
  2 KIT_CROSSMETAL        "Silver cross",
  3 KIT_CROSSGOLD         "Gold cross",
  4 KIT_JAGUARGOLDDUMMY   "Gold jaguar",
  5 KIT_HAWKWINGS01DUMMY  "Hawk wings - part 1",
  6 KIT_HAWKWINGS02DUMMY  "Hawk wings - part 2",
  7 KIT_HOLYGRAIL         "Holy grail",
  8 KIT_TABLESDUMMY       "Tablet of wisdom",
  9 KIT_WINGEDLION        "Winged lion",
 10 KIT_ELEPHANTGOLD      "Gold elephant",
 11 KIT_STATUEHEAD01      "Seriously scary ceremonial mask",
 12 KIT_STATUEHEAD02      "Hilariously happy ceremonial mask",
 13 KIT_STATUEHEAD03      "Ix Chel mask",
 14 KIT_KINGSTATUE        "Statue of King Tilmun",
 15 KIT_CRYSTALSKULL      "Crystal Skull",

 // [Cecil] TFE keys (original indices offsetted by 16)
 16 KIT_ANKHWOOD          "[TFE/SSR] Wooden ankh",
 17 KIT_ANKHROCK          "[TFE/SSR] Stone ankh",
 18 KIT_ANKHGOLD          "[TFE/SSR] Gold ankh",
 19 KIT_AMONGOLD          "[TFE/SSR] Gold amon",
 20 KIT_ANKHGOLDDUMMY     "[TFE/SSR] Gold ankh dummy key",
 21 KIT_ELEMENTEARTH      "[TFE/SSR] Element - Earth",
 22 KIT_ELEMENTWATER      "[TFE/SSR] Element - Water",
 23 KIT_ELEMENTAIR        "[TFE/SSR] Element - Air",
 24 KIT_ELEMENTFIRE       "[TFE/SSR] Element - Fire",
 25 KIT_RAKEY             "[TFE/SSR] Ra Key",
 26 KIT_MOONKEY           "[TFE/SSR] Moon Key",
 28 KIT_EYEOFRA           "[TFE/SSR] Eye of Ra",
 29 KIT_SCARAB            "[TFE/SSR] Scarab",
 30 KIT_COBRA             "[TFE/SSR] Cobra",
 31 KIT_SCARABDUMMY       "[TFE/SSR] Scarab dummy",
 32 KIT_HEART             "[TFE/SSR] Gold Heart",
 33 KIT_FEATHER           "[TFE/SSR] Feather of Truth",
 34 KIT_SPHINX1           "[TFE/SSR] Sphinx 1",
 35 KIT_SPHINX2           "[TFE/SSR] Sphinx 2",
};

// event for sending through receive item
event EKey {
  enum KeyItemType kitType,
};

%{

const char *GetKeyName(enum KeyItemType kit)
{
  switch(kit) {
    case KIT_BOOKOFWISDOM     :  return LOCALIZE("Book of wisdom"); break;
    case KIT_CROSSWOODEN      :  return LOCALIZE("Wooden cross"); break;
    case KIT_CROSSGOLD        :  return LOCALIZE("Gold cross"); break;
    case KIT_CROSSMETAL       :  return LOCALIZE("Silver cross"); break;
    case KIT_JAGUARGOLDDUMMY  :  return LOCALIZE("Gold jaguar"); break;
    case KIT_HAWKWINGS01DUMMY :  return LOCALIZE("Hawk wings - part 1"); break;
    case KIT_HAWKWINGS02DUMMY :  return LOCALIZE("Hawk wings - part 2"); break;
    case KIT_HOLYGRAIL        :  return LOCALIZE("Holy grail"); break;
    case KIT_TABLESDUMMY      :  return LOCALIZE("Tablet of wisdom"); break;
    case KIT_WINGEDLION       :  return LOCALIZE("Winged lion"); break;
    case KIT_ELEPHANTGOLD     :  return LOCALIZE("Gold elephant"); break;    
    case KIT_STATUEHEAD01     :  return LOCALIZE("Seriously scary ceremonial mask"); break;
    case KIT_STATUEHEAD02     :  return LOCALIZE("Hilariously happy ceremonial mask"); break;
    case KIT_STATUEHEAD03     :  return LOCALIZE("Ix Chel mask"); break;   
    case KIT_KINGSTATUE       :  return LOCALIZE("Statue of King Tilmun"); break;   
    case KIT_CRYSTALSKULL     :  return LOCALIZE("Crystal Skull"); break;   

    // [Cecil] TFE keys
    case KIT_ANKHWOOD:      return LOCALIZE("Wooden ankh"); break;
    case KIT_ANKHROCK:      return LOCALIZE("Stone ankh"); break;
    case KIT_ANKHGOLD:
    case KIT_ANKHGOLDDUMMY: return LOCALIZE("Gold ankh"); break;
    case KIT_AMONGOLD:      return LOCALIZE("Gold Amon statue"); break;
    case KIT_ELEMENTEARTH:  return LOCALIZE("Earth element"); break;
    case KIT_ELEMENTWATER:  return LOCALIZE("Water element"); break;
    case KIT_ELEMENTAIR:    return LOCALIZE("Air element"); break;
    case KIT_ELEMENTFIRE:   return LOCALIZE("Fire element"); break;
    case KIT_RAKEY:         return LOCALIZE("Ra key"); break;
    case KIT_MOONKEY:       return LOCALIZE("Moon key"); break;
    case KIT_EYEOFRA:       return LOCALIZE("Eye of Ra"); break;
    case KIT_SCARAB:
    case KIT_SCARABDUMMY:   return LOCALIZE("Scarab"); break;
    case KIT_COBRA:         return LOCALIZE("Cobra"); break;
    case KIT_HEART:         return LOCALIZE("Gold Heart"); break;
    case KIT_FEATHER:       return LOCALIZE("Feather of Truth"); break;
    case KIT_SPHINX1:
    case KIT_SPHINX2:       return LOCALIZE("Gold Sphinx"); break;

    default: return LOCALIZE("unknown item"); break;
  };
}

%}

class CKeyItem : CItem {
name      "KeyItem";
thumbnail "Thumbnails\\KeyItem.tbn";
features  "IsImportant";

properties:
  1 enum KeyItemType m_kitType    "Type" 'Y' = KIT_BOOKOFWISDOM, // key type
  3 INDEX m_iSoundComponent = 0,
  5 FLOAT m_fSize "Size" = 1.0f,

components:
  0 class   CLASS_BASE        "Classes\\Item.ecl",

// ********* ANKH KEY *********
  1 model   MODEL_BOOKOFWISDOM      "ModelsMP\\Items\\Keys\\BookOfWisdom\\Book.mdl",
  2 texture TEXTURE_BOOKOFWISDOM    "ModelsMP\\Items\\Keys\\BookOfWisdom\\Book.tex",

  5 model   MODEL_CROSSWOODEN       "ModelsMP\\Items\\Keys\\Cross\\Cross.mdl",
  6 texture TEXTURE_CROSSWOODEN     "ModelsMP\\Items\\Keys\\Cross\\CrossWooden.tex",
  
  7 model   MODEL_CROSSMETAL        "ModelsMP\\Items\\Keys\\Cross\\Cross.mdl",
  8 texture TEXTURE_CROSSMETAL      "ModelsMP\\Items\\Keys\\Cross\\CrossMetal.tex",

 10 model   MODEL_CROSSGOLD         "ModelsMP\\Items\\Keys\\GoldCross\\Cross.mdl",
 11 texture TEXTURE_CROSSGOLD       "ModelsMP\\Items\\Keys\\GoldCross\\Cross.tex",

 15 model   MODEL_JAGUARGOLD        "ModelsMP\\Items\\Keys\\GoldJaguar\\Jaguar.mdl",

 20 model   MODEL_HAWKWINGS01       "ModelsMP\\Items\\Keys\\HawkWings\\WingRight.mdl",
 21 model   MODEL_HAWKWINGS02       "ModelsMP\\Items\\Keys\\HawkWings\\WingLeft.mdl",
 22 texture TEXTURE_HAWKWINGS       "ModelsMP\\Items\\Keys\\HawkWings\\Wings.tex",

 30 model   MODEL_HOLYGRAIL         "ModelsMP\\Items\\Keys\\HolyGrail\\Grail.mdl",
 31 texture TEXTURE_HOLYGRAIL       "ModelsMP\\Items\\Keys\\HolyGrail\\Grail.tex",

 35 model   MODEL_TABLESOFWISDOM    "ModelsMP\\Items\\Keys\\TablesOfWisdom\\Tables.mdl",
 36 texture TEXTURE_TABLESOFWISDOM  "ModelsMP\\Items\\Keys\\TablesOfWisdom\\Tables.tex",

 40 model   MODEL_WINGEDLION        "ModelsMP\\Items\\Keys\\WingLion\\WingLion.mdl",
 
 45 model   MODEL_ELEPHANTGOLD      "ModelsMP\\Items\\Keys\\GoldElephant\\Elephant.mdl",

 50 model   MODEL_STATUEHEAD01      "ModelsMP\\Items\\Keys\\Statue01\\Statue.mdl",
 51 texture TEXTURE_STATUEHEAD01    "ModelsMP\\Items\\Keys\\Statue01\\Statue.tex",
 52 model   MODEL_STATUEHEAD02      "ModelsMP\\Items\\Keys\\Statue02\\Statue.mdl",
 53 texture TEXTURE_STATUEHEAD02    "ModelsMP\\Items\\Keys\\Statue02\\Statue.tex",
 54 model   MODEL_STATUEHEAD03      "ModelsMP\\Items\\Keys\\Statue03\\Statue.mdl",
 55 texture TEXTURE_STATUEHEAD03    "ModelsMP\\Items\\Keys\\Statue03\\Statue.tex",

 58 model   MODEL_KINGSTATUE        "ModelsMP\\Items\\Keys\\ManStatue\\Statue.mdl",
 
 60 model   MODEL_CRYSTALSKULL      "ModelsMP\\Items\\Keys\\CrystalSkull\\Skull.mdl",
 61 texture TEXTURE_CRYSTALSKULL    "ModelsMP\\Items\\Keys\\CrystalSkull\\Skull.tex",

// [Cecil] TFE keys (original indices offsetted by 100)
101 model   MODEL_ANKHWOOD       "Models\\Items\\Keys\\AnkhWood\\Ankh.mdl",
102 texture TEXTURE_ANKHWOOD     "Models\\Ages\\Egypt\\Vehicles\\BigBoat\\OldWood.tex",
103 model   MODEL_ANKHROCK       "Models\\Items\\Keys\\AnkhStone\\Ankh.mdl",
104 texture TEXTURE_ANKHROCK     "Models\\Items\\Keys\\AnkhStone\\Stone.tex",
105 model   MODEL_ANKHGOLD       "Models\\Items\\Keys\\AnkhGold\\Ankh.mdl",
106 texture TEXTURE_ANKHGOLD     "Models\\Items\\Keys\\AnkhGold\\Ankh.tex",
107 model   MODEL_AMONGOLD       "Models\\Ages\\Egypt\\Gods\\Amon\\AmonGold.mdl",
108 texture TEXTURE_AMONGOLD     "Models\\Ages\\Egypt\\Gods\\Amon\\AmonGold.tex",
110 model   MODEL_ELEMENTAIR     "Models\\Items\\Keys\\Elements\\Air.mdl",
111 texture TEXTURE_ELEMENTAIR   "Models\\Items\\Keys\\Elements\\Air.tex",
120 model   MODEL_ELEMENTWATER   "Models\\Items\\Keys\\Elements\\Water.mdl",
121 texture TEXTURE_ELEMENTWATER "Models\\Items\\Keys\\Elements\\Water.tex",
130 model   MODEL_ELEMENTFIRE    "Models\\Items\\Keys\\Elements\\Fire.mdl",
131 texture TEXTURE_ELEMENTFIRE  "Models\\Items\\Keys\\Elements\\Fire.tex",
140 model   MODEL_ELEMENTEARTH   "Models\\Items\\Keys\\Elements\\Earth.mdl",
141 texture TEXTURE_ELEMENTEARTH "Models\\Items\\Keys\\Elements\\Texture.tex",
150 model   MODEL_RAKEY          "Models\\Items\\Keys\\RaKey\\Key.mdl",
151 texture TEXTURE_RAKEY        "Models\\Items\\Keys\\RaKey\\Key.tex",
160 model   MODEL_MOONKEY        "Models\\Items\\Keys\\RaSign\\Sign.mdl",
161 texture TEXTURE_MOONKEY      "Models\\Items\\Keys\\RaSign\\Sign.tex",
170 model   MODEL_EYEOFRA        "Models\\Items\\Keys\\EyeOfRa\\EyeOfRa.mdl",
171 texture TEXTURE_EYEOFRA      "Models\\Items\\Keys\\EyeOfRa\\EyeOfRa.tex",
180 model   MODEL_SCARAB         "Models\\Items\\Keys\\Scarab\\Scarab.mdl",
181 texture TEXTURE_SCARAB       "Models\\Items\\Keys\\Scarab\\Scarab.tex",
190 model   MODEL_COBRA          "Models\\Items\\Keys\\Uaset\\Uaset.mdl",
191 texture TEXTURE_COBRA        "Models\\Items\\Keys\\Uaset\\Uaset.tex",
192 model   MODEL_FEATHER        "Models\\Items\\Keys\\Luxor\\FeatherOfTruth.mdl",
193 texture TEXTURE_FEATHER      "Models\\Items\\Keys\\Luxor\\FeatherOfTruth.tex",
194 model   MODEL_HEART          "Models\\Items\\Keys\\Luxor\\GoldHeart.mdl",
195 texture TEXTURE_HEART        "Models\\Items\\Keys\\Luxor\\GoldHeart.tex",
196 model   MODEL_SPHINXGOLD     "Models\\Items\\Keys\\GoldSphinx\\GoldSphinx.mdl",
197 texture TEXTURE_SPHINXGOLD   "Models\\Items\\Keys\\GoldSphinx\\Sphinx.tex",

 // ********* MISC *********
250 texture TEXTURE_FLARE       "ModelsMP\\Items\\Flares\\Flare.tex",
251 model   MODEL_FLARE         "ModelsMP\\Items\\Flares\\Flare.mdl",
252 texture TEX_REFL_GOLD01     "ModelsMP\\ReflectionTextures\\Gold01.tex",
253 texture TEX_REFL_METAL01    "ModelsMP\\ReflectionTextures\\LightMetal01.tex",
254 texture TEX_SPEC_MEDIUM     "ModelsMP\\SpecularTextures\\Medium.tex",
255 texture TEX_SPEC_STRONG     "ModelsMP\\SpecularTextures\\Strong.tex",

// ************** SOUNDS **************
300 sound   SOUND_KEY         "Sounds\\Items\\Key.wav",

functions:
  void Precache(void) {
    PrecacheSound(SOUND_KEY);
  }
  /* Fill in entity statistics - for AI purposes only */
  BOOL FillEntityStatistics(EntityStats *pes)
  {
    pes->es_strName = GetKeyName(m_kitType);
    pes->es_ctCount = 1;
    pes->es_ctAmmount = 1;
    pes->es_fValue = 1;
    pes->es_iScore = 0;//m_iScore;
    return TRUE;
  }
  
  // render particles
  void RenderParticles(void) {
    // no particles when not existing
    if (GetRenderType()!=CEntity::RT_MODEL || !ShowItemParticles()) {
      return;
    }

    // [Cecil] TFE keys
    if (_EnginePatches._eWorldFormat != E_LF_TSE && m_kitType >= KIT_ANKHWOOD) {
      switch (m_kitType) {
        case KIT_ANKHWOOD:
        case KIT_ANKHROCK:
        case KIT_ANKHGOLD:
        case KIT_ANKHGOLDDUMMY:
        default:
          Particles_Stardust(this, 0.9f, 0.70f, PT_STAR08, 32);
          break;

        case KIT_AMONGOLD:
          Particles_Stardust(this, 1.6f, 1.00f, PT_STAR08, 32);
          break;
      }

    // TSE keys
    } else {
      switch (m_kitType) {
        case KIT_BOOKOFWISDOM:
        case KIT_CRYSTALSKULL:   
        case KIT_HOLYGRAIL:
          Particles_Stardust(this, 1.0f, 0.5f, PT_STAR08, 64);
          break;

        case KIT_JAGUARGOLDDUMMY:
          Particles_Stardust(this, 2.0f, 2.0f, PT_STAR08, 64);
          break;

        case KIT_CROSSWOODEN:
        case KIT_CROSSMETAL:   
        case KIT_CROSSGOLD:      
        case KIT_HAWKWINGS01DUMMY:
        case KIT_HAWKWINGS02DUMMY:
        case KIT_TABLESDUMMY:
        case KIT_WINGEDLION:
        case KIT_ELEPHANTGOLD:
        case KIT_STATUEHEAD01:
        case KIT_STATUEHEAD02:
        case KIT_STATUEHEAD03:
        case KIT_KINGSTATUE:
        default:
          Particles_Stardust(this, 1.5f, 1.1f, PT_STAR08, 64);
          break;
      }
    }
  }
  


  // set health properties depending on type
  void SetProperties(void)
  {
    m_fRespawnTime = (m_fCustomRespawnTime > 0) ? m_fCustomRespawnTime : 10.0f;
    m_strDescription = GetKeyName(m_kitType);

    switch (m_kitType) {
      case KIT_BOOKOFWISDOM:     AddItem(MODEL_BOOKOFWISDOM,   TEXTURE_BOOKOFWISDOM,   0, 0, 0); break;
      case KIT_CROSSWOODEN:      AddItem(MODEL_CROSSWOODEN,    TEXTURE_CROSSWOODEN,    0, 0, 0); break;
      case KIT_CROSSMETAL:       AddItem(MODEL_CROSSMETAL,     TEXTURE_CROSSMETAL,     TEX_REFL_METAL01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_CROSSGOLD:        AddItem(MODEL_CROSSGOLD,      TEXTURE_CROSSGOLD,      TEX_REFL_GOLD01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_JAGUARGOLDDUMMY:  AddItem(MODEL_JAGUARGOLD,     TEX_REFL_GOLD01,        TEX_REFL_GOLD01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_HAWKWINGS01DUMMY: AddItem(MODEL_HAWKWINGS01,    TEXTURE_HAWKWINGS,      0, 0, 0); break;
      case KIT_HAWKWINGS02DUMMY: AddItem(MODEL_HAWKWINGS02,    TEXTURE_HAWKWINGS,      0, 0, 0); break;
      case KIT_HOLYGRAIL:        AddItem(MODEL_HOLYGRAIL,      TEXTURE_HOLYGRAIL,      TEX_REFL_METAL01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_TABLESDUMMY:      AddItem(MODEL_TABLESOFWISDOM, TEXTURE_TABLESOFWISDOM, TEX_REFL_METAL01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_WINGEDLION:       AddItem(MODEL_WINGEDLION,     TEX_REFL_GOLD01,        TEX_REFL_GOLD01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_ELEPHANTGOLD:     AddItem(MODEL_ELEPHANTGOLD,   TEX_REFL_GOLD01,        TEX_REFL_GOLD01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_STATUEHEAD01:     AddItem(MODEL_STATUEHEAD01,   TEXTURE_STATUEHEAD01,   0, 0, 0); break;
      case KIT_STATUEHEAD02:     AddItem(MODEL_STATUEHEAD02,   TEXTURE_STATUEHEAD02,   0, 0, 0); break;
      case KIT_STATUEHEAD03:     AddItem(MODEL_STATUEHEAD03,   TEXTURE_STATUEHEAD03,   0, 0, 0); break;
      case KIT_KINGSTATUE:       AddItem(MODEL_KINGSTATUE,     TEX_REFL_GOLD01,        TEX_REFL_GOLD01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_CRYSTALSKULL:     AddItem(MODEL_CRYSTALSKULL,   TEXTURE_CRYSTALSKULL,   TEX_REFL_METAL01, TEX_SPEC_MEDIUM, 0); break;

      // [Cecil] TFE keys
      case KIT_ANKHWOOD:      AddItem(MODEL_ANKHWOOD,     TEXTURE_ANKHWOOD,     0, 0, 0); break;
      case KIT_ANKHROCK:      AddItem(MODEL_ANKHROCK,     TEXTURE_ANKHROCK,     0, 0, 0); break;
      case KIT_ANKHGOLD:
      case KIT_ANKHGOLDDUMMY: AddItem(MODEL_ANKHGOLD,     TEXTURE_ANKHGOLD,     TEX_REFL_GOLD01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_SPHINX1:
      case KIT_SPHINX2:       AddItem(MODEL_SPHINXGOLD,   TEXTURE_SPHINXGOLD,   TEX_REFL_GOLD01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_AMONGOLD:      AddItem(MODEL_AMONGOLD,     TEXTURE_AMONGOLD,     TEX_REFL_GOLD01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_ELEMENTEARTH:  AddItem(MODEL_ELEMENTEARTH, TEXTURE_ELEMENTEARTH, TEX_REFL_METAL01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_ELEMENTAIR:    AddItem(MODEL_ELEMENTAIR,   TEXTURE_ELEMENTAIR,   TEX_REFL_METAL01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_ELEMENTWATER:  AddItem(MODEL_ELEMENTWATER, TEXTURE_ELEMENTWATER, TEX_REFL_METAL01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_ELEMENTFIRE:   AddItem(MODEL_ELEMENTFIRE,  TEXTURE_ELEMENTFIRE,  TEX_REFL_METAL01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_RAKEY:         AddItem(MODEL_RAKEY,        TEXTURE_RAKEY,        TEX_REFL_GOLD01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_MOONKEY:       AddItem(MODEL_MOONKEY,      TEXTURE_MOONKEY,      TEX_REFL_GOLD01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_EYEOFRA:       AddItem(MODEL_EYEOFRA,      TEXTURE_EYEOFRA,      TEX_REFL_GOLD01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_SCARAB:
      case KIT_SCARABDUMMY:   AddItem(MODEL_SCARAB,       TEXTURE_SCARAB,       TEX_REFL_METAL01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_COBRA:         AddItem(MODEL_COBRA,        TEXTURE_COBRA,        TEX_REFL_GOLD01, TEX_SPEC_MEDIUM, 0); break;
      case KIT_FEATHER:       AddItem(MODEL_FEATHER,      TEXTURE_FEATHER,      0, 0, 0); break;
      case KIT_HEART:         AddItem(MODEL_HEART,        TEXTURE_HEART,        TEX_REFL_GOLD01, TEX_SPEC_MEDIUM, 0); break;
    }

    // Common flare
    switch (m_kitType) {
      case KIT_JAGUARGOLDDUMMY:
      case KIT_ELEPHANTGOLD:
      case KIT_AMONGOLD: // [Cecil] TFE key
        AddFlare(MODEL_FLARE, TEXTURE_FLARE, FLOAT3D(0.0f, 0.5f, 0.0f), FLOAT3D(2.0f, 2.0f, 0.3f));
        break;

      default:
        AddFlare(MODEL_FLARE, TEXTURE_FLARE, FLOAT3D(0.0f, 0.2f, 0.0f), FLOAT3D(1.0f, 1.0f, 0.3f));
        break;
    }

    // Common settings
    m_iSoundComponent = SOUND_KEY;
    StretchItem(FLOAT3D(1, 1, 1));

    GetModelObject()->StretchModel(FLOAT3D(m_fSize, m_fSize, m_fSize));

    // [Cecil] Bigger TFE keys
    if (_EnginePatches._eWorldFormat != E_LF_TSE
     && (m_kitType == KIT_SPHINX1 || m_kitType == KIT_SPHINX2 || m_kitType == KIT_AMONGOLD)) {
      StretchItem(FLOAT3D(2, 2, 2) * m_fSize);
    }
  };

procedures:
  ItemCollected(EPass epass) : CItem::ItemCollected {
    ASSERT(epass.penOther!=NULL);

    // send key to entity
    EKey eKey;
    eKey.kitType = m_kitType;
    // if health is received
    if (epass.penOther->ReceiveItem(eKey)) {
      if(_pNetwork->IsPlayerLocal(epass.penOther)) {IFeel_PlayEffect("PU_Key");}
      // play the pickup sound
      m_soPick.Set3DParameters(50.0f, 1.0f, 1.0f, 1.0f);
      PlaySound(m_soPick, m_iSoundComponent, SOF_3D);
      m_fPickSoundLen = GetSoundLength(m_iSoundComponent);
      jump CItem::ItemReceived();
    }
    return;
  };

  Main() {
    // [Cecil] Replace TFE keys with TSE
    if (ClassicsCore_IsEditorApp()) {
      INDEX iKey = -1;

      switch (m_kitType) {
        case KIT_ANKHWOOD:      iKey = KIT_CROSSWOODEN; break;
        case KIT_ANKHROCK:      iKey = KIT_CROSSMETAL; break;
        case KIT_ANKHGOLD:      iKey = KIT_CROSSGOLD; break;
        case KIT_AMONGOLD:      iKey = KIT_KINGSTATUE; break;
        case KIT_ANKHGOLDDUMMY: iKey = KIT_TABLESDUMMY; break;
        case KIT_ELEMENTEARTH:  iKey = KIT_CROSSWOODEN; break;
        case KIT_ELEMENTWATER:  iKey = KIT_CROSSMETAL; break;
        case KIT_ELEMENTAIR:    iKey = KIT_CRYSTALSKULL; break;
        case KIT_ELEMENTFIRE:   iKey = KIT_CROSSGOLD; break;
        case KIT_RAKEY:         iKey = KIT_HOLYGRAIL; break;
        case KIT_MOONKEY:       iKey = KIT_BOOKOFWISDOM; break;
        case KIT_EYEOFRA:       iKey = KIT_BOOKOFWISDOM; break;
        case KIT_SCARAB:        iKey = KIT_STATUEHEAD03; break;
        case KIT_COBRA:         iKey = KIT_HOLYGRAIL; break;
        case KIT_SCARABDUMMY:   iKey = KIT_TABLESDUMMY; break;
        case KIT_HEART:         iKey = KIT_STATUEHEAD01; break;
        case KIT_FEATHER:       iKey = KIT_STATUEHEAD02; break;
        case KIT_SPHINX1:       iKey = KIT_WINGEDLION; break;
        case KIT_SPHINX2:       iKey = KIT_ELEPHANTGOLD; break;
      }

      if (iKey != -1) {
        ResetCompatibilityValue(m_kitType, m_kitType, (KeyItemType)iKey);
      }
    }

    Initialize();     // initialize base class
    StartModelAnim(ITEMHOLDER_ANIM_SMALLOSCILATION, AOF_LOOPING|AOF_NORESTART);
    ForceCollisionBoxIndexChange(ITEMHOLDER_COLLISION_BOX_BIG);
    SetProperties();  // set properties

    jump CItem::ItemLoop();
  };
};
