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

#include "StdH.h"

#include "Networking/ExtPackets.h"

#include <Extras/XGizmo/Interfaces/Sounds.h>

#if _PATCHCONFIG_EXT_PACKETS

// Available sound channels
static CSoundObject _asoChannels[32];

// Played at least one sound
static BOOL _bPlayedSound = FALSE;

// Get channel from index
CSoundObject *CExtPlaySound::GetChannel(INDEX iChannel) {
  if (iChannel < 0 || iChannel > 31) return NULL;

  return &_asoChannels[iChannel];
};

// Stop sounds on all channels
void CExtPlaySound::StopAllSounds(void)
{
  if (!_bPlayedSound) return;
  _bPlayedSound = FALSE;

  for (INDEX i = 0; i < 32; i++) {
    GetChannel(i)->Stop();
  }
};

#define COMPRESSED_VOLUME_MAX SL_VOLUME_MAX
#define COMPRESSED_FILTER_MAX 500.0f
#define COMPRESSED_PITCH_MAX 10.0f

// Compress volume into an 8-bit integer
static inline UBYTE CompressVolume(FLOAT fVolume) {
  return NormFloatToByte(Clamp(fVolume / COMPRESSED_VOLUME_MAX, 0.0f, 1.0f));
};

// Compress filter into a 16-bit integer
static inline UWORD CompressFilter(FLOAT fFilter) {
  return Clamp(fFilter / COMPRESSED_FILTER_MAX, 0.0f, 1.0f) * 65535.0f;
};

// Compress pitch into a 16-bit integer
static inline UWORD CompressPitch(FLOAT fPitch) {
  // Compress into precise increments (using up 64001 of 65536 possible values)
  return Clamp(fPitch / COMPRESSED_PITCH_MAX, 0.0f, 1.0f) * 64000.0f;
};

// Decompress volume from an 8-bit integer
static inline FLOAT DecompressVolume(UBYTE ubVolume) {
  return NormByteToFloat(ubVolume) * COMPRESSED_VOLUME_MAX;
};

// Decompress filter from a 10-bit integer
static inline FLOAT DecompressFilter(UWORD uwFilter) {
  return ClampDn(FLOAT(uwFilter) / 65535.0f * COMPRESSED_FILTER_MAX, 1.0f);
};

// Decompress pitch from a 10-bit integer
static inline FLOAT DecompressPitch(UWORD uwPitch) {
  return FLOAT(uwPitch) / 64000.0f * COMPRESSED_PITCH_MAX;
};

bool CExtPlaySound::Write(CNetworkMessage &nm)
{
  INDEX iChannel = props["iChannel"].GetIndex();

  // Invalid channel
  if (iChannel < 0 || iChannel > 31) {
    return false;
  }

  nm << props["strFile"].GetString();

  // Channel index occupies 5/16 bits
  nm.WriteBits(&iChannel, 5);

  // Flags occupy 11/16 bits
  INDEX iFlags = props["ulFlags"].GetIndex();
  nm.WriteBits(&iFlags, 11);

  nm << props["fDelay"].GetFloat();
  nm << props["fOffset"].GetFloat();
  nm << CompressVolume(props["fVolumeL"].GetFloat());
  nm << CompressVolume(props["fVolumeR"].GetFloat());
  nm << CompressFilter(props["fFilterL"].GetFloat());
  nm << CompressFilter(props["fFilterR"].GetFloat());
  nm << CompressPitch(props["fPitch"].GetFloat());
  return true;
};

void CExtPlaySound::Read(CNetworkMessage &nm) {
  nm >> props["strFile"].GetString();

  // Channel index occupies 5/16 bits
  INDEX iChannel = 0;
  nm.ReadBits(&iChannel, 5);
  props["iChannel"].GetIndex() = iChannel;

  // Flags occupy 11/16 bits
  ULONG ulFlags = 0;
  nm.ReadBits(&ulFlags, 11);
  props["ulFlags"].GetIndex() = ulFlags;

  nm >> props["fDelay"].GetFloat();
  nm >> props["fOffset"].GetFloat();

  UBYTE ubVolume;
  UWORD uwFilter, uwPitch;

  nm >> ubVolume;
  props["fVolumeL"].GetFloat() = DecompressVolume(ubVolume);
  nm >> ubVolume;
  props["fVolumeR"].GetFloat() = DecompressVolume(ubVolume);

  nm >> uwFilter;
  props["fFilterL"].GetFloat() = DecompressFilter(uwFilter);
  nm >> uwFilter;
  props["fFilterR"].GetFloat() = DecompressFilter(uwFilter);

  nm >> uwPitch;
  props["fPitch"].GetFloat() = DecompressPitch(uwPitch);
};

void CExtPlaySound::Process(void) {
  INDEX iChannel = props["iChannel"].GetIndex();

  // Invalid channel
  if (iChannel < 0 || iChannel > 31) {
    ClassicsPackets_ServerReport(this, TRANS("Invalid channel index: %d\n"), iChannel);
    return;
  }

  CSoundObject &so = _asoChannels[iChannel];
  CTString strFile = props["strFile"].GetString();

  // Stop playing on a specific channel
  if (strFile == "/stop/") {
    so.Stop();
    return;
  }

  // Set sound parameters for a specific channel
  so.SetDelay(props["fDelay"].GetFloat());
  so.SetVolume(props["fVolumeL"].GetFloat(), props["fVolumeR"].GetFloat());
  so.SetFilter(props["fFilterL"].GetFloat(), props["fFilterR"].GetFloat());
  so.SetPitch(props["fPitch"].GetFloat());

  // Play a new sound
  if (strFile != "") {
    try {
      so.Play_t(strFile, props["ulFlags"].GetIndex());
    } catch (char *strError) {
      ClassicsPackets_ServerReport(this, TRANS("Cannot play '%s' sound: %s\n"), strFile.str_String, strError);
    }

    _bPlayedSound = TRUE;
  }

  // Set offset after playing the sound
  const FLOAT fOffset = props["fOffset"].GetFloat();
  ISounds::SetOffset(so, fOffset, fOffset);
};

#endif // _PATCHCONFIG_EXT_PACKETS
