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

#include "StdH.h"

// Fix player info sound
static CPluginSymbol _psFixSound(SSF_PERSISTENT | SSF_USER, INDEX(1));

class CEntityPatch : public CEntity {
  public:
    void P_PlaySound(CSoundObject &so, SLONG idSoundComponent, SLONG slPlayType);
};

// Patched function
void CEntityPatch::P_PlaySound(CSoundObject &so, SLONG idSoundComponent, SLONG slPlayType)
{
  #define SOUND_INFO_COMPONENT ((0x191 << 8) + CHOOSE_FOR_GAME(174, 202, 202))

  // Sound component is SOUND_INFO from CPlayer and it's a prediction
  if (_psFixSound.GetIndex() && idSoundComponent == SOUND_INFO_COMPONENT && IsPredictor())
  {
    // Retrieve player properties for checking
    static CPropertyPtr pptrAnalyse(this); // CPlayer::m_tmAnalyseEnd
    static CPropertyPtr pptrMessage(this); // CPlayer::m_soMessage

    if (pptrAnalyse.ByVariable("CPlayer", "m_tmAnalyseEnd")
     && pptrMessage.ByVariable("CPlayer", "m_soMessage"))
    {
      // Check if it's the right sound object that wants to play the sound
      if (&so == &ENTITYPROPERTY(this, pptrMessage.Offset(), CSoundObject)) {
        // Check if the time has just been set in CPlayer::ReceiveComputerMessage()
        TIME tmAnalyseEnd = ENTITYPROPERTY(this, pptrAnalyse.Offset(), TIME);
        TIME tmExpected = _pTimer->CurrentTick() + 2.0f;

        // Skip the sound if the time has been set less than 2 ticks ago
        if (Abs(tmAnalyseEnd - tmExpected) < _pTimer->TickQuantum * 2) {
          return;
        }
      }
    }
  }

  // Original function code
  CEntityComponent *pecSound = en_pecClass->ComponentForTypeAndID(ECT_SOUND, idSoundComponent);
  so.Play(pecSound->ec_psdSound, slPlayType);
};

// Initialize module
void InitPlayerInfoSound(void) {
  // Patch sound playing function
  void (CEntity::*pPlaySound)(CSoundObject &, SLONG, SLONG) = &CEntity::PlaySound;
  CreatePatch(pPlaySound, &CEntityPatch::P_PlaySound, "CEntity::PlaySound(component)");

  // Custom symbols
  _psFixSound.Register("axs_bFixPlayerInfoSound");
};
