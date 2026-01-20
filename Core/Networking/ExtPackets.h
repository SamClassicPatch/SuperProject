/* Copyright (c) 2023-2026 Dreamy Cecil
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

#ifndef CECIL_INCL_EXTPACKETS_H
#define CECIL_INCL_EXTPACKETS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include "MessageCompression.h"

#if _PATCHCONFIG_EXT_PACKETS

// Report packet actions to the server
CORE_API extern INDEX ser_bReportExtPacketLogic;

// Define built-in extension packets
class CORE_API CExtPacket : public IClassicsBuiltInExtPacket {
  protected:
    // Properties instead of raw data fields for easier modification via API
    se1::map<CTString, CAnyValue> props;

  public:
    // Convenient value getter
    CAnyValue *GetValue(const CTString &strVariable);

    // Convenient value setter
    bool operator()(const CTString &strVariable, const CAnyValue &val);

    // Create new packet from type
    static CExtPacket *CreatePacket(EPacketType ePacket);

    // Register the module
    static void RegisterExtPackets(void);
};

// Entity packets

class CORE_API CExtEntityCreate : public CExtPacket {
  public:
    static CEntity *penLast; // Last created entity

  public:
    CExtEntityCreate() {
      props["fnmClass"] = ""; // Class file to create an entity from (packed as extra if index isn't found in the predefined list)
      props["plPos"] = CPlacement3D(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0)); // Place to create an entity at
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityCreate);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

// Base for entity manipulation packets
class CORE_API CExtEntityPacket : public CExtPacket {
  public:
    CExtEntityPacket() {
      props["ulEntity"] = 0x7FFFFFFF; // Entity ID in the world (31 bits)
    };

    // Write entity ID
    void WriteEntity(CNetworkMessage &nm) {
      ULONG ulEntity = ClampUp((ULONG)props["ulEntity"].GetIndex(), (ULONG)0x7FFFFFFFUL);
      nm.WriteBits(&ulEntity, 31);
    };

    // Read entity ID
    void ReadEntity(CNetworkMessage &nm) {
      ULONG ulEntity = 0;
      nm.ReadBits(&ulEntity, 31);
      props["ulEntity"].GetIndex() = ulEntity;
    };

    // Check for invalid ID
    inline BOOL IsEntityValid(void) {
      // 0x7FFFFFFF - 0xFFFFFFFF are invalid
      return ULONG(props["ulEntity"].GetIndex()) < 0x7FFFFFFF;
    };

    // Retrieve an entity from an ID
    CEntity *FindExtEntity(ULONG ulID);

    // Make sure to return some entity from the ID
    CEntity *GetEntity(void);

    // Check if some entity exists
    static inline BOOL EntityExists(CEntity *pen) {
      return (pen != NULL && !(pen->GetFlags() & ENF_DELETED));
    };
};

// Holder for event fields
class CORE_API EExtEntityEvent : public CEntityEvent {
  public:
    // Accommodate for multiple fields of varying data
    ULONG aulFields[64];

  public:
    EExtEntityEvent() : CEntityEvent(EVENTCODE_EVoid) {
      Reset();
    };

    CEntityEvent *MakeCopy(void) {
      return new EExtEntityEvent(*this);
    };

  public:
    // Reset event fields
    inline void Reset(void) {
      ee_slEvent = EVENTCODE_EVoid;

      // Fill fields with NULL, FALSE, 0, 0.0f etc.
      memset(aulFields, 0, sizeof(aulFields));
    };

    // Convert entity ID into a pointer
    inline ULONG EntityFromID(INDEX i) {
      return (ULONG)IWorld::FindEntityByID(IWorld::GetWorld(), aulFields[i]);
    };

    // Convert entity ID into a pointer, if possible
    inline ULONG MaybeEntity(INDEX i) {
      // Return entity if found any
      ULONG ulPtr = EntityFromID(i);
      if (ulPtr != NULL) return ulPtr;

      // Return value as is
      return aulFields[i];
    };

    // Write event into a network packet
    void Write(CNetworkMessage &nm, ULONG ctFields);

    // Read event from a network packet
    ULONG Read(CNetworkMessage &nm);

    // Convert fields according to the event type
    void ConvertTypes(void);
};

class CORE_API CExtEntityDelete : public CExtEntityPacket {
  public:
    CExtEntityDelete() : CExtEntityPacket() {
      props["bSameClass"] = false; // Delete all instances of the same class
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityDelete);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityCopy : public CExtEntityPacket {
  public:
    CExtEntityCopy() : CExtEntityPacket() {
      props["iCopies"] = 1;
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityCopy);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityEvent : public CExtEntityPacket {
  protected:
    EExtEntityEvent eEvent; // Data holder
    ULONG ctFields; // Amount of used fields

  public:
    CExtEntityEvent() : CExtEntityPacket(), ctFields(0)
    {
    };

    // Copy event bytes (iEventSize = sizeof(ee))
    // If some event has CEntityPointer fields, they need to contain entity IDs as 4-byte integers instead
    // of pointers to entities directly. When setting entity IDs for CEntityPointer fields, do it like this:
    //   (ULONG &)ee.pen = iEntityID;
    // DO NOT FORGET to do this at the end of the function to avoid crashes upon calling the pointer destructor:
    //   (ULONG &)ee.pen = NULL;
    void SetEvent(CEntityEvent &ee, size_t iEventSize);

    // Copy event data from another event
    void Copy(const EExtEntityEvent &eeOther, ULONG ctSetFields);

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityEvent);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityItem : public CExtEntityEvent {
  public:
    CExtEntityItem() : CExtEntityEvent()
    {
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityItem);

    virtual void Process(void);
};

class CORE_API CExtEntityInit : public CExtEntityEvent {
  public:
    CExtEntityInit() : CExtEntityEvent()
    {
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityInit);

    virtual void Process(void);
};

class CORE_API CExtEntityTeleport : public CExtEntityPacket {
  public:
    CExtEntityTeleport() : CExtEntityPacket() {
      props["plSet"] = CPlacement3D(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0)); // Placement to set
      props["bRelative"] = false; // Relative to the current placement (oriented)
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityTeleport);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityPosition : public CExtEntityPacket {
  public:
    CExtEntityPosition() : CExtEntityPacket() {
      props["vSet"] = FLOAT3D(0, 0, 0); // Position or rotation to set
      props["bRotation"] = false; // Set rotation instead of position
      props["bRelative"] = false; // Relative to the current placement (axis-aligned)
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityPosition);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityParent : public CExtEntityPacket {
  public:
    CExtEntityParent() : CExtEntityPacket() {
      props["ulParent"] = -1;
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityParent);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityProp : public CExtEntityPacket {
  public:
    CExtEntityProp() : CExtEntityPacket() {
      props["bName"] = false; // Using a name to find the property or not
      props["ulProp"] = 0; // Property ID or name hash
      props["value"] = 0.0; // DOUBLE or CTString
    };

    // Set property name
    inline void SetProperty(const CTString &strName) {
      props["bName"].GetIndex() = true;
      props["ulProp"].GetIndex() = strName.GetHash();
    };

    // Set property ID
    inline void SetProperty(ULONG ulID) {
      props["bName"].GetIndex() = false;
      props["ulProp"].GetIndex() = ulID;
    };

    // Set string value
    inline void SetValue(const CTString &str) {
      props["value"] = str;
    };

    // Set number value
    inline void SetValue(DOUBLE f) {
      props["value"] = f;
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityProp);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityHealth : public CExtEntityPacket {
  public:
    CExtEntityHealth() : CExtEntityPacket() {
      props["fHealth"] = 0.0f; // Health to set
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityHealth);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityFlags : public CExtEntityPacket {
  public:
    CExtEntityFlags() : CExtEntityPacket() {
      props["ulFlags"] = 0; // Flags to apply
      props["iType"] = 0; // Type of flags
      props["bRemove"] = false; // Disable flags instead of enabling
    };

    // Set normal flags
    inline void EntityFlags(ULONG ul, BOOL bRemoveFlags) {
      props["ulFlags"].GetIndex() = ul;
      props["iType"].GetIndex() = 0;
      props["bRemove"].GetIndex() = bRemoveFlags;
    };

    // Set physical flags
    inline void PhysicalFlags(ULONG ul, BOOL bRemoveFlags) {
      props["ulFlags"].GetIndex() = ul;
      props["iType"].GetIndex() = 1;
      props["bRemove"].GetIndex() = bRemoveFlags;
    };

    // Set collision flags
    inline void CollisionFlags(ULONG ul, BOOL bRemoveFlags) {
      props["ulFlags"].GetIndex() = ul;
      props["iType"].GetIndex() = 2;
      props["bRemove"].GetIndex() = bRemoveFlags;
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityFlags);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityMove : public CExtEntityPacket {
  public:
    CExtEntityMove() : CExtEntityPacket() {
      props["vSpeed"] = FLOAT3D(0, 0, 0); // Desired speed
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityMove);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityRotate : public CExtEntityMove {
  public:
    CExtEntityRotate() : CExtEntityMove()
    {
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityRotate);

    virtual void Process(void);
};

class CORE_API CExtEntityImpulse : public CExtEntityMove {
  public:
    CExtEntityImpulse() : CExtEntityMove()
    {
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityImpulse);

    virtual void Process(void);
};

// Abstract damage packet
class CORE_API CExtEntityDamage : public CExtEntityPacket {
  public:
    CExtEntityDamage() : CExtEntityPacket() {
      props["eDamageType"] = (int)DMT_NONE; // Damage type to use
      props["fDamage"] = 0.0f; // Damage to inflict
    };

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
};

class CORE_API CExtEntityDirectDamage : public CExtEntityDamage {
  public:
    CExtEntityDirectDamage() : CExtEntityDamage() {
      props["ulTarget"] = -1; // Target entity for damaging
      props["vHitPoint"] = FLOAT3D(0, 0, 0); // Where exactly the damage occurred
      props["vDirection"] = FLOAT3D(0, 0, 0); // From which direction the damage came from
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityDirDmg);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityRangeDamage : public CExtEntityDamage {
  public:
    CExtEntityRangeDamage() : CExtEntityDamage() {
      props["vCenter"] = FLOAT3D(0, 0, 0); // Place to inflict damage from
      props["fFallOff"] = 0.0f; // Total damage radius
      props["fHotSpot"] = 0.0f; // Full damage radius
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityRadDmg);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtEntityBoxDamage : public CExtEntityDamage {
  public:
    CExtEntityBoxDamage() : CExtEntityDamage() {
      props["boxArea"] = FLOATaabbox3D(FLOAT3D(0, 0, 0), 0.0f); // Area to inflict the damage in
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_EntityBoxDmg);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtChangeLevel : public CExtPacket {
  public:
    CExtChangeLevel() {
      props["strWorld"] = ""; // World file to change to
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_ChangeLevel);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtChangeWorld : public CExtChangeLevel {
  public:
    CExtChangeWorld() : CExtChangeLevel()
    {
    };

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_ChangeWorld);

    virtual void Process(void);
};

class CORE_API CExtSessionProps : public CExtPacket {
  public:
    CSesPropsContainer sp; // Session properties to set (data that's not processed isn't being zeroed!)

  public:
    CExtSessionProps() {
      props["iSize"] = 0; // Amount of bytes to set
      props["iOffset"] = 0; // Starting byte (up to NET_MAXSESSIONPROPERTIES - 1)
    };

    inline INDEX &GetSize(void) { return props["iSize"].GetIndex(); };
    inline INDEX &GetOffset(void) { return props["iOffset"].GetIndex(); };

    // Set new data at the current end and expand session properties size
    BOOL AddData(const void *pData, size_t ctBytes);

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_SessionProps);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtGameplayExt : public CExtPacket {
  public:
    CExtGameplayExt() {
      props["iVar"] = 0; // Variable in the structure (0 is invalid, starts from 1)
      props["value"] = 0.0; // DOUBLE or CTString
    };

    // Find variable index by its name
    int FindVar(const CTString &strVar);

    // Set string value
    void SetValue(const CTString &strVar, const CTString &str);

    // Set number value
    void SetValue(const CTString &strVar, DOUBLE f);

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_GameplayExt);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

class CORE_API CExtPlaySound : public CExtPacket {
  public:
    CExtPlaySound() {
      // Sound file to play
      // - Setting it to "/stop/" stops any playing sound on a specified channel
      // - Leaving it blank simply changes sound parameters of a channel without playing/resetting any sounds
      props["strFile"] = "";

      props["iChannel"] = 0; // Playback channel (0-31)
      props["ulFlags"] = SOF_NONE; // Playback flags

      // Sound parameters
      props["fDelay"] = 0.0f; // Playback delay (0.0+)
      props["fOffset"] = 0.0f; // Playback offset in seconds
      props["fVolumeL"] = 1.0f; // Left ear volume (0.0 .. 4.0)
      props["fVolumeR"] = 1.0f; // Right ear volume (0.0 .. 4.0)
      props["fFilterL"] = 1.0f; // Left ear filter (1.0 .. 500.0)
      props["fFilterR"] = 1.0f; // Right ear filter (1.0 .. 500.0)
      props["fPitch"] = 1.0f; // Playback pitch (0.0 .. 10.0)
    };

    // Get channel from index
    static CSoundObject *GetChannel(INDEX iChannel);

    // Stop sounds on all channels
    static void StopAllSounds(void);

  public:
    EXTPACKET_DEFINEFORTYPE(k_EPacketType_PlaySound);

    virtual bool Write(CNetworkMessage &nm);
    virtual void Read(CNetworkMessage &nm);
    virtual void Process(void);
};

#endif // _PATCHCONFIG_EXT_PACKETS

#endif
