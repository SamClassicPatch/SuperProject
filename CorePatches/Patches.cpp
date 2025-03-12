/* Copyright (c) 2022-2025 Dreamy Cecil
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

#include <Core/Base/Unzip.h>

#if _PATCHCONFIG_ENGINEPATCHES

// Singleton for patching
CPatches _EnginePatches;

// Constructor
CPatches::CPatches() {
  _bAdjustForAspectRatio = TRUE;
  _bUseVerticalFOV = 2; // Only player FOV
  _fCustomFOV = -1.0f;
  _fThirdPersonFOV = -1.0f;
  _bCheckFOV = FALSE;

  _bNoListening = FALSE;

  _ulMaxWriteMemory = (1 << 20) * 128; // 128 MB

  _eWorldFormat = E_LF_CURRENT;
  _iWorldConverter = -1;
};

// Apply core patches (called after Core initialization!)
void CPatches::CorePatches(void) {
  // Hook this interface
  _pCorePatches = this;

  const bool bGame = ClassicsCore_IsGameApp();
  const bool bServer = ClassicsCore_IsServerApp();
  const bool bEditor = ClassicsCore_IsEditorApp();

  // Patch for everything
  Strings();
  Textures();

  // Patch for the game and the editor
  if (bGame || bEditor) {
    Entities();
    Input();
    LogicTimers();
    Network();
    Rendering();
    Worlds();

    // [Cecil] TODO: Make SKA patches work in Debug
    #if SE1_VER >= SE1_107 && defined(NDEBUG)
      Ska();
    #endif

    if (bGame) {
      SoundLibrary();
    }
  }

  // Patch for the server
  if (bServer) {
    Entities();
    LogicTimers();
    Network();
    Worlds();
  }

  // Custom symbols for pre-engine initialization patches
#if _PATCHCONFIG_FIX_STREAMPAGING
  _pShell->DeclareSymbol("user INDEX sam_bUsePlaceholderResources;", &_EnginePatches._bUsePlaceholderResources);
#endif
};

#include "Patches/Entities.h"

void CPatches::Entities(void) {
#if _PATCHCONFIG_EXTEND_ENTITIES

  // CEntity
  void (CEntity::*pReadProps)(CTStream &) = &CEntity::ReadProperties_t;
  CreatePatch(pReadProps, &CEntityPatch::P_ReadProperties, "CEntity::ReadProperties_t(...)");

  extern void (CEntity::*pSendEvent)(const CEntityEvent &);
  pSendEvent = &CEntity::SendEvent;
  CreatePatch(pSendEvent, &CEntityPatch::P_SendEvent, "CEntity::SendEvent(...)");

  // CRationalEntity
  void (CRationalEntity::*pCall)(SLONG, SLONG, BOOL, const CEntityEvent &) = &CRationalEntity::Call;
  CreatePatch(pCall, &CRationalEntityPatch::P_Call, "CRationalEntity::Call(...)");

  // CPlayer
  extern CEntityPatch::CReceiveItem pReceiveItem;
  StructPtr pReceiveItemPtr(ClassicsCore_GetEntitiesSymbol("?ReceiveItem@CPlayer@@UAEHABVCEntityEvent@@@Z"));

  if (pReceiveItemPtr.iAddress != NULL) {
    pReceiveItem = pReceiveItemPtr(CEntityPatch::CReceiveItem());
    CreatePatch(pReceiveItem, &CEntityPatch::P_ReceiveItem, "CPlayer::ReceiveItem(...)");
  }

  extern CEntityPatch::CRenderGameView pRenderGameView;
  StructPtr pRenderGameViewPtr(ClassicsCore_GetEntitiesSymbol("?RenderGameView@CPlayer@@UAEXPAVCDrawPort@@PAX@Z"));

  if (pRenderGameViewPtr.iAddress != NULL) {
    pRenderGameView = pRenderGameViewPtr(CEntityPatch::CRenderGameView());
    CreatePatch(pRenderGameView, &CEntityPatch::P_RenderGameView, "CPlayer::RenderGameView(...)");
  }

#if _PATCHCONFIG_ENTITY_FORCE
  // Mod's brush entities
  CWorld woTemp;

  try {
    CSetFPUPrecision FPUPrecision(FPT_24BIT);

    static const CPlacement3D plDummy(FLOAT3D(0, 0, 0), ANGLE3D(0, 0, 0));
    CEntity *penWB = woTemp.CreateEntity_t(plDummy, CTFILENAME("Classes\\WorldBase.ecl"));
    CEntity *penMB = woTemp.CreateEntity_t(plDummy, CTFILENAME("Classes\\MovingBrush.ecl"));

    // Not sure if required but I don't wanna risk it
    penWB->Initialize();
    penMB->Initialize();

    StructPtr pEntityGetForcePtr = ClassicsCore_GetEngineSymbol("?GetForce@CEntity@@UAEXJABV?$Vector@M$02@@AAVCForceStrength@@1@Z");
    CEntityPatch::CGetForce pEntityGetForce = pEntityGetForcePtr(CEntityPatch::CGetForce());

    // Pointer to the virtual table of CWorldBase
    size_t *pVFTable = *(size_t **)penWB;

    extern CEntityPatch::CGetForce pWorldBase_GetForce;
    pWorldBase_GetForce = *(CEntityPatch::CGetForce *)(pVFTable + VFOFFSET_ENTITY_GETFORCE);

    // Don't patch engine function by mistake
    if (pWorldBase_GetForce != pEntityGetForce) {
      CreatePatch(pWorldBase_GetForce, &CEntityPatch::P_WorldBase_GetForce, "CWorldBase::GetForce(...)");
    }

    // Pointer to the virtual table of CMovingBrush
    pVFTable = *(size_t **)penMB;

    extern CEntityPatch::CGetForce pMovingBrush_GetForce;
    pMovingBrush_GetForce = *(CEntityPatch::CGetForce *)(pVFTable + VFOFFSET_ENTITY_GETFORCE);

    // Don't patch engine function by mistake
    if (pMovingBrush_GetForce != pEntityGetForce) {
      CreatePatch(pMovingBrush_GetForce, &CEntityPatch::P_MovingBrush_GetForce, "CMovingBrush::GetForce(...)");
    }

    // Same as for initialization
    penWB->Destroy();
    penMB->Destroy();

  // Ignore errors
  } catch (char *strError) {
    (void)strError;
  }
#endif // _PATCHCONFIG_ENTITY_FORCE

#endif // _PATCHCONFIG_EXTEND_ENTITIES
};

#include "Input/Input.h"

void CPatches::Input(void) {
#if _PATCHCONFIG_EXTEND_INPUT

  // Don't patch input
  if (!IConfig::global[k_EConfigProps_ExtendedInput]) return;

  // Initialization
  void (CInput::*pInitialize)(void) = &CInput::Initialize;
  CreatePatch(pInitialize, &CInputPatch::P_Initialize, "CInput::Initialize()");

  // Input
  void (CInput::*pEnableInput)(HWND) = &CInput::EnableInput;
  CreatePatch(pEnableInput, &CInputPatch::P_EnableInput, "CInput::EnableInput(HWND)");

  void (CInput::*pDisableInput)(void) = &CInput::DisableInput;
  CreatePatch(pDisableInput, &CInputPatch::P_DisableInput, "CInput::DisableInput()");

  void (CInput::*pGetInput)(BOOL) = &CInput::GetInput;
  CreatePatch(pGetInput, &CInputPatch::P_GetInput, "CInput::GetInput(...)");

  void (CInput::*pClearInput)(void) = &CInput::ClearInput;
  CreatePatch(pClearInput, &CInputPatch::P_ClearInput, "CInput::ClearInput()");

  // Joysticks
  void (CInput::*pSetKeyNames)(void) = &CInput::SetKeyNames;
  CreatePatch(pSetKeyNames, &CInputPatch::P_SetKeyNames, "CInput::SetKeyNames()");

  void (CInput::*pAddJoystickAbbilities)(INDEX) = &CInput::AddJoystickAbbilities;
  CreatePatch(pAddJoystickAbbilities, &CInputPatch::P_AddJoystickAbbilities, "CInput::AddJoystickAbbilities()");

  BOOL (CInput::*pScanJoystick)(INDEX, BOOL) = &CInput::ScanJoystick;
  CreatePatch(pScanJoystick, &CInputPatch::P_ScanJoystick, "CInput::ScanJoystick()");

  // Getters
  const CTString &(CInput::*pGetAxisTransName)(INDEX) const = &CInput::GetAxisTransName;
  CreatePatch(pGetAxisTransName, &CInputPatch::P_GetAxisTransName, "CInput::GetAxisTransName()");

  FLOAT (CInput::*pGetAxisValue)(INDEX) const = &CInput::GetAxisValue;
  CreatePatch(pGetAxisValue, &CInputPatch::P_GetAxisValue, "CInput::GetAxisValue()");

  // Initialize the new input system after patching the old one
  CInputPatch::Construct();
  _pInput->Initialize();

  // Custom symbols
  _pShell->DeclareSymbol("persistent user FLOAT inp_fAxisPressThreshold;", &inp_fAxisPressThreshold);
  _pShell->DeclareSymbol("user void inp_JoysticksInfo(void);", &CInputPatch::PrintJoysticksInfo);

#endif // _PATCHCONFIG_EXTEND_INPUT
};

#include "Patches/LogicTimers.h"

void CPatches::LogicTimers(void) {
#if _PATCHCONFIG_FIX_LOGICTIMERS

  void (CRationalEntity::*pSetTimerAfter)(TIME) = &CRationalEntity::SetTimerAfter;
  CreatePatch(pSetTimerAfter, &CRationalEntityTimerPatch::P_SetTimerAfter, "CRationalEntity::SetTimerAfter(...)");

#endif // _PATCHCONFIG_FIX_LOGICTIMERS
};

#include "Patches/Network.h"

void CPatches::Network(void) {
#if _PATCHCONFIG_EXTEND_NETWORK

  // CCommunicationInterface
#if _PATCHCONFIG_NEW_QUERY
  void (CCommunicationInterface::*pEndWindock)(void) = &CCommunicationInterface::EndWinsock;
  CreatePatch(pEndWindock, &CComIntPatch::P_EndWinsock, "CCommunicationInterface::EndWinsock()");
#endif

  extern void (CCommunicationInterface::*pServerInit)(void);
  pServerInit = &CCommunicationInterface::Server_Init_t;
  CreatePatch(pServerInit, &CComIntPatch::P_ServerInit, "CCommunicationInterface::Server_Init_t()");

  extern void (CCommunicationInterface::*pServerClose)(void);
  pServerClose = &CCommunicationInterface::Server_Close;
  CreatePatch(pServerClose, &CComIntPatch::P_ServerClose, "CCommunicationInterface::Server_Close()");

  // CMessageDispatcher
  void (CMessageDispatcher::*pSendToServer)(const CNetworkMessage &) = &CMessageDispatcher::SendToServerReliable;
  CreatePatch(pSendToServer, &CMessageDisPatch::P_SendToServerReliable, "CMessageDispatcher::SendToServerReliable(...)");

  BOOL (CMessageDispatcher::*pRecFromClient)(INDEX, CNetworkMessage &) = &CMessageDispatcher::ReceiveFromClient;
  CreatePatch(pRecFromClient, &CMessageDisPatch::P_ReceiveFromClient, "CMessageDispatcher::ReceiveFromClient(...)");

  pRecFromClient = &CMessageDispatcher::ReceiveFromClientReliable;
  CreatePatch(pRecFromClient, &CMessageDisPatch::P_ReceiveFromClientReliable, "CMessageDispatcher::ReceiveFromClientReliable(...)");

  // CNetworkLibrary
  extern void (CNetworkLibrary::*pChangeLevel)(void);
  pChangeLevel = &CNetworkLibrary::ChangeLevel_internal;
  CreatePatch(pChangeLevel, &CNetworkPatch::P_ChangeLevelInternal, "CNetworkLibrary::ChangeLevel_internal()");

  void (CNetworkLibrary::*pSaveGame)(const CTFileName &) = &CNetworkLibrary::Save_t;
  CreatePatch(pSaveGame, &CNetworkPatch::P_Save, "CNetworkLibrary::Save_t(...)");

  extern void (CNetworkLibrary::*pLoadGame)(const CTFileName &);
  pLoadGame = &CNetworkLibrary::Load_t;
  CreatePatch(pLoadGame, &CNetworkPatch::P_Load, "CNetworkLibrary::Load_t(...)");

  extern void (CNetworkLibrary::*pStopGame)(void);
  pStopGame = &CNetworkLibrary::StopGame;
  CreatePatch(pStopGame, &CNetworkPatch::P_StopGame, "CNetworkLibrary::StopGame()");

  extern CNetworkPatch::CStartP2PFunc pStartPeerToPeer;
  pStartPeerToPeer = &CNetworkLibrary::StartPeerToPeer_t;
  CreatePatch(pStartPeerToPeer, &CNetworkPatch::P_StartPeerToPeer, "CNetworkLibrary::StartPeerToPeer_t(...)");

  extern void (CNetworkLibrary::*pStartDemoPlay)(const CTFileName &);
  pStartDemoPlay = &CNetworkLibrary::StartDemoPlay_t;
  CreatePatch(pStartDemoPlay, &CNetworkPatch::P_StartDemoPlay, "CNetworkLibrary::StartDemoPlay_t(...)");

  void (CNetworkLibrary::*pStartDemoRec)(const CTFileName &) = &CNetworkLibrary::StartDemoRec_t;
  CreatePatch(pStartDemoRec, &CNetworkPatch::P_StartDemoRec, "CNetworkLibrary::StartDemoRec_t(...)");

  void (CNetworkLibrary::*pStopDemoRec)(void) = &CNetworkLibrary::StopDemoRec;
  CreatePatch(pStopDemoRec, &CNetworkPatch::P_StopDemoRec, "CNetworkLibrary::StopDemoRec()");

  // CSessionState
  extern void (CSessionState::*pFlushPredictions)(void);
  pFlushPredictions = &CSessionState::FlushProcessedPredictions;
  CreatePatch(pFlushPredictions, &CSessionStatePatch::P_FlushProcessedPredictions, "CSessionState::FlushProcessedPredictions()");

  extern void (CSessionState::*pStartAtClient)(INDEX);
  pStartAtClient = &CSessionState::Start_AtClient_t;
  CreatePatch(pStartAtClient, &CSessionStatePatch::P_Start_AtClient, "CSessionState::Start_AtClient_t(...)");

  void (CSessionState::*pWaitStream)(CTMemoryStream &, const CTString &, INDEX) = &CSessionState::WaitStream_t;
  CreatePatch(pWaitStream, &CSessionStatePatch::P_WaitStream, "CSessionState::WaitStream_t(...)");

  void (CSessionState::*pProcGameStreamBlock)(CNetworkMessage &) = &CSessionState::ProcessGameStreamBlock;
  CreatePatch(pProcGameStreamBlock, &CSessionStatePatch::P_ProcessGameStreamBlock, "CSessionState::ProcessGameStreamBlock(...)");

  void (CSessionState::*pStopSession)(void) = &CSessionState::Stop;
  CreatePatch(pStopSession, &CSessionStatePatch::P_Stop, "CSessionState::Stop()");

  extern void (CSessionState::*pReadSessionState)(CTStream *);
  pReadSessionState = &CSessionState::Read_t;
  CreatePatch(pReadSessionState, &CSessionStatePatch::P_Read, "CSessionState::Read_t(...)");

  extern void (CSessionState::*pWriteSessionState)(CTStream *);
  pWriteSessionState = &CSessionState::Write_t;
  CreatePatch(pWriteSessionState, &CSessionStatePatch::P_Write, "CSessionState::Write_t(...)");

#if _PATCHCONFIG_GUID_MASKING

  void (CSessionState::*pMakeSyncCheck)(void) = &CSessionState::MakeSynchronisationCheck;
  CreatePatch(pMakeSyncCheck, &CSessionStatePatch::P_MakeSynchronisationCheck, "CSessionState::MakeSynchronisationCheck()");

  // CPlayerEntity

  // Pointer to CPlayerEntity::Write_t()
  void *pPlayerWrite = ClassicsCore_GetEngineSymbol("?Write_t@CPlayerEntity@@UAEXPAVCTStream@@@Z");
  CreatePatch(pPlayerWrite, &CPlayerEntityPatch::P_Write, "CPlayerEntity::Write_t(...)");

  // Pointer to CPlayerEntity::ChecksumForSync()
  void *pChecksumForSync = ClassicsCore_GetEngineSymbol("?ChecksumForSync@CPlayerEntity@@UAEXAAKJ@Z");
  CreatePatch(pChecksumForSync, &CPlayerEntityPatch::P_ChecksumForSync, "CPlayerEntity::ChecksumForSync(...)");

  // Custom symbols
  _pShell->DeclareSymbol("persistent user INDEX ser_bMaskGUIDs pre:UpdateServerSymbolValue;", &IProcessPacket::_bMaskGUIDs);

#endif // _PATCHCONFIG_GUID_MASKING

#endif // _PATCHCONFIG_EXTEND_NETWORK
};

#include "Patches/Rendering.h"

void CPatches::Rendering(void) {
#if _PATCHCONFIG_FIX_RENDERING

  extern void (*pRenderView)(CWorld &, CEntity &, CAnyProjection3D &, CDrawPort &);
  pRenderView = &RenderView;
  CreatePatch(pRenderView, &P_RenderView, "::RenderView(...)");

  // Pointer to CPerspectiveProjection3D::Prepare()
  void *pPrepare = ClassicsCore_GetEngineSymbol("?Prepare@CPerspectiveProjection3D@@UAEXXZ");
  CreatePatch(pPrepare, &CProjectionPatch::P_Prepare, "CPerspectiveProjection3D::Prepare()");

  // Custom symbols
  _pShell->DeclareSymbol("persistent user INDEX sam_bAdjustForAspectRatio;", &_EnginePatches._bAdjustForAspectRatio);
  _pShell->DeclareSymbol("persistent user INDEX sam_bUseVerticalFOV;", &_EnginePatches._bUseVerticalFOV);
  _pShell->DeclareSymbol("persistent user FLOAT sam_fCustomFOV;",      &_EnginePatches._fCustomFOV);
  _pShell->DeclareSymbol("persistent user FLOAT sam_fThirdPersonFOV;", &_EnginePatches._fThirdPersonFOV);
  _pShell->DeclareSymbol("           user INDEX sam_bCheckFOV;",       &_EnginePatches._bCheckFOV);

#endif // _PATCHCONFIG_FIX_RENDERING
};

// [Cecil] TODO: Make SKA patches work in Debug
#if SE1_VER >= SE1_107 && defined(NDEBUG)

#include "Patches/Ska.h"

void CPatches::Ska(void) {
#if _PATCHCONFIG_FIX_SKA

  // SKA models have been patched
  static BOOL _bSkaPatched = FALSE;

  if (_bSkaPatched) return;
  _bSkaPatched = TRUE;

  void (*pFogHazeFunc)(BOOL) = &RM_DoFogAndHaze;
  CreatePatch(pFogHazeFunc, &P_DoFogAndHaze, "RM_DoFogAndHaze(...)");

  void (*pFogPassFunc)(void) = &shaDoFogPass;
  FuncPatch_ForceRewrite(7); // Rewrite complex instruction
  CreatePatch(pFogPassFunc, &P_shaDoFogPass, "shaDoFogPass(...)");

  void (*pSetWrappingFunc)(GfxWrap, GfxWrap) = &shaSetTextureWrapping;
  CreatePatch(pSetWrappingFunc, &P_shaSetTextureWrapping, "shaSetTextureWrapping(...)");

  extern void (CModelInstance::*pModelInstanceCopyFunc)(CModelInstance &);
  pModelInstanceCopyFunc = &CModelInstance::Copy;
  CreatePatch(pModelInstanceCopyFunc, &CModelInstancePatch::P_Copy, "CModelInstance::Copy(...)");

#endif // _PATCHCONFIG_FIX_SKA
};

#endif

#include "Patches/SoundLibrary.h"

void CPatches::SoundLibrary(void) {
  void (CSoundLibrary::*pListen)(CSoundListener &) = &CSoundLibrary::Listen;
  CreatePatch(pListen, &CSoundLibPatch::P_Listen, "CSoundLibrary::Listen(...)");

  void (CSoundObject::*pUpdate3DEffects)(void) = &CSoundObject::Update3DEffects;
  CreatePatch(pUpdate3DEffects, &CSoundObjPatch::P_Update3DEffects, "CSoundObject::Update3DEffects()");
};

#include "Patches/Strings.h"

void CPatches::Strings(void) {
#if _PATCHCONFIG_FIX_STRINGS

  INDEX (CTString::*pVPrintF)(const char *, va_list) = &CTString::VPrintF;
  CreatePatch(pVPrintF, &CStringPatch::P_VPrintF, "CTString::VPrintF(...)");

  CTString (CTString::*pUndecorated)(void) const = &CTString::Undecorated;
  CreatePatch(pUndecorated, &CStringPatch::P_Undecorated, "CTString::Undecorated()");

#endif // _PATCHCONFIG_FIX_STRINGS
};

#include "Patches/Textures.h"

void CPatches::Textures(void) {
#if _PATCHCONFIG_EXTEND_TEXTURES

  void (CTextureData::*pCreateTex)(const CImageInfo *, MEX, INDEX, int) = &CTextureData::Create_t;
  CreatePatch(pCreateTex, &CTexDataPatch::P_Create, "CTextureData::Create_t(...)");

  // Pointer to CTextureData::Write_t()
  void *pWriteTex = ClassicsCore_GetEngineSymbol("?Write_t@CTextureData@@UAEXPAVCTStream@@@Z");
  CreatePatch(pWriteTex, &CTexDataPatch::P_Write, "CTextureData::Write_t(...)");

  void (*pProcessScript)(const CTFileName &) = &ProcessScript_t;
  CreatePatch(pProcessScript, &P_ProcessTextureScript, "ProcessScript_t(...)");

  void (*pCreateTextureOut)(const CTFileName &, const CTFileName &, MEX, INDEX, int) = &CreateTexture_t;
  CreatePatch(pCreateTextureOut, &P_CreateTextureOut, "CreateTexture_t(out)");

  void (*pCreateTexture)(const CTFileName &, MEX, INDEX, int) = &CreateTexture_t;
  CreatePatch(pCreateTexture, &P_CreateTexture, "CreateTexture_t(...)");

#endif // _PATCHCONFIG_EXTEND_TEXTURES
};

#include "Patches/Worlds.h"

void CPatches::Worlds(void) {
  void (CWorld::*pWorldLoad)(const CTFileName &) = &CWorld::Load_t;
  CreatePatch(pWorldLoad, &CWorldPatch::P_Load, "CWorld::Load_t(...)");

  pWorldLoad = &CWorld::LoadBrushes_t;
  CreatePatch(pWorldLoad, &CWorldPatch::P_LoadBrushes, "CWorld::LoadBrushes_t(...)");

  void (CWorld::*pReadInfo)(CTStream *, BOOL) = &CWorld::ReadInfo_t;
  CreatePatch(pReadInfo, &CWorldPatch::P_ReadInfo, "CWorld::ReadInfo_t(...)");

  CEntity *(CWorld::*pCreateEntity)(const CPlacement3D &, const CTFileName &) = &CWorld::CreateEntity_t;
  CreatePatch(pCreateEntity, &CWorldPatch::P_CreateEntity, "CWorld::CreateEntity_t(...)");

  // Custom symbols
  _pShell->DeclareSymbol("user INDEX sam_iWorldConverter;", &_EnginePatches._iWorldConverter);
};

#include "Patches/UnpageStreams.h"

// Specific stream patching
static void PatchStreams(void) {
#if _PATCHCONFIG_FIX_STREAMPAGING

  // Streams have been patched
  static BOOL _bStreamsPatched = FALSE;

  if (_bStreamsPatched) return;
  _bStreamsPatched = TRUE;

  // CTStream
  void (CTStream::*pAllocMemory)(ULONG) = &CTStream::AllocateVirtualMemory;
  CreatePatch(pAllocMemory, &CUnpageStreamPatch::P_AllocVirtualMemory, "CTStream::AllocateVirtualMemory(...)");

  void (CTStream::*pFreeBufferFunc)(void) = &CTStream::FreeBuffer;
  CreatePatch(pFreeBufferFunc, &CUnpageStreamPatch::P_FreeBuffer, "CTStream::FreeBuffer()");

  // CTFileStream
  void (CTFileStream::*pCreateFunc)(const CTFileName &, CTStream::CreateMode) = &CTFileStream::Create_t;
  CreatePatch(pCreateFunc, &CFileStreamPatch::P_Create, "CTFileStream::Create_t(...)");

  void (CTFileStream::*pOpenFunc)(const CTFileName &, CTStream::OpenMode) = &CTFileStream::Open_t;
  CreatePatch(pOpenFunc, &CFileStreamPatch::P_Open, "CTFileStream::Open_t(...)");

  void (CTFileStream::*pCloseFunc)(void) = &CTFileStream::Close;
  CreatePatch(pCloseFunc, &CFileStreamPatch::P_Close, "CTFileStream::Close()");

#endif // _PATCHCONFIG_FIX_STREAMPAGING
};

void CPatches::UnpageStreams(void) {
#if _PATCHCONFIG_FIX_STREAMPAGING

  // Streams have been unpaged
  static BOOL _bStreamsUnpaged = FALSE;

  if (_bStreamsUnpaged) return;
  _bStreamsUnpaged = TRUE;

  PatchStreams();

  // Dummy methods
  void (CTFileStream::*pPageFunc)(INDEX) = &CTStream::CommitPage;
  CreatePatch(pPageFunc, &IDummy::PageFunc, "CTStream::CommitPage(...)");

  pPageFunc = &CTStream::DecommitPage;
  CreatePatch(pPageFunc, &IDummy::PageFunc, "CTStream::DecommitPage(...)");

  pPageFunc = &CTStream::ProtectPageFromWritting;
  CreatePatch(pPageFunc, &IDummy::PageFunc, "CTStream::ProtectPageFromWritting(...)");

  pPageFunc = &CTFileStream::WritePageToFile;
  CreatePatch(pPageFunc, &IDummy::PageFunc, "CTFileStream::WritePageToFile(...)");

  pPageFunc = &CTFileStream::FileCommitPage;
  CreatePatch(pPageFunc, &IDummy::PageFunc, "CTFileStream::FileCommitPage(...)");

  pPageFunc = &CTFileStream::FileDecommitPage;
  CreatePatch(pPageFunc, &IDummy::PageFunc, "CTFileStream::FileDecommitPage(...)");

  void (CNetworkLibrary::*pFinishCRCFunc)(void) = &CNetworkLibrary::FinishCRCGather;
  CreatePatch(pFinishCRCFunc, &IDummy::Void, "CNetworkLibrary::FinishCRCGather()");

  // Level remembering methods
  void (CSessionState::*pRemCurLevel)(const CTString &) = &CSessionState::RememberCurrentLevel;
  CreatePatch(pRemCurLevel, &CRemLevelPatch::P_RememberCurrentLevel, "CSessionState::RememberCurrentLevel(...)");

  CRememberedLevel *(CSessionState::*pFindRemLevel)(const CTString &) = &CSessionState::FindRememberedLevel;
  CreatePatch(pFindRemLevel, &CRemLevelPatch::P_FindRememberedLevel, "CSessionState::FindRememberedLevel(...)");

  void (CSessionState::*pRestoreOldLevel)(const CTString &) = &CSessionState::RestoreOldLevel;
  CreatePatch(pRestoreOldLevel, &CRemLevelPatch::P_RestoreOldLevel, "CSessionState::RestoreOldLevel(...)");

  void (CSessionState::*pForgetOldLevels)(void) = &CSessionState::ForgetOldLevels;
  CreatePatch(pForgetOldLevels, &CRemLevelPatch::P_ForgetOldLevels, "CSessionState::ForgetOldLevels()");

#endif // _PATCHCONFIG_FIX_STREAMPAGING
};

#include "Patches/FileSystem.h"

void CPatches::FileSystem(void) {
#if _PATCHCONFIG_EXTEND_FILESYSTEM

  // File system has been extended
  static BOOL _bFileSystemExtended = FALSE;

  if (_bFileSystemExtended) return;
  _bFileSystemExtended = TRUE;

  // Don't patch file system
  if (!IConfig::global[k_EConfigProps_ExtendedFileSystem]) return;

  PatchStreams();

  // CEntityClass
  void (CEntityClass::*pObtainComponents)(void) = &CEntityClass::ObtainComponents_t;
  CreatePatch(pObtainComponents, &CEntityClassPatch::P_ObtainComponents, "CEntityClass::ObtainComponents_t()");

  void *pReadClass = ClassicsCore_GetEngineSymbol("?Read_t@CEntityClass@@UAEXPAVCTStream@@@Z");
  CreatePatch(pReadClass, &CEntityClassPatch::P_Read, "CEntityClass::Read_t(...)");

#if SE1_VER >= SE1_107
  // CShader
  void *pReadShader = ClassicsCore_GetEngineSymbol("?Read_t@CShader@@UAEXPAVCTStream@@@Z");
  CreatePatch(pReadShader, &CShaderPatch::P_Read, "CShader::Read_t(...)");
#endif

  // CTStream
  #if SE1_GAME == SS_REV || SE1_VER == SE1_110
    void *pGetLine = ClassicsCore_GetEngineSymbol("?GetLine_t@CTStream@@UAEXPADJD@Z");
  #else
    void *pGetLine = ClassicsCore_GetEngineSymbol("?GetLine_t@CTStream@@QAEXPADJD@Z");
  #endif
  CreatePatch(pGetLine, &CStreamPatch::P_GetLine, "CTStream::GetLine_t(...)");

  void (CTStream::*pReadDictionary)(SLONG) = &CTStream::ReadDictionary_intenal_t;
  CreatePatch(pReadDictionary, &CStreamPatch::P_ReadDictionary_intenal, "CTStream::ReadDictionary_intenal_t(...)");

  // Global methods
  extern void (*pInitStreams)(void);
  pInitStreams = StructPtr(ADDR_INITSTREAMS)(&P_InitStreams);
  CreatePatch(pInitStreams, &P_InitStreams, "::InitStreams()");

  void (*pMakeDirList)(CFileList &, const CTFileName &, const CTString &, ULONG) = &MakeDirList;
  CreatePatch(pMakeDirList, &P_MakeDirList, "::MakeDirList(...)");

#if SE1_GAME != SS_REV
  INDEX (*pExpandFilePath)(EXPAND_PATH_ARGS(ULONG, const CTFileName &, CTFileName &, BOOL)) = &ExpandFilePath;
  CreatePatch(pExpandFilePath, &P_ExpandFilePath, "::ExpandFilePath(...)");
#endif

#endif // _PATCHCONFIG_EXTEND_FILESYSTEM
};

// Clean up on Core shutdown (only for patches set by CorePatches() method)
void CPatches::Cleanup(void) {
#if _PATCHCONFIG_EXTEND_INPUT
  CInputPatch::Destruct();
#endif
};

#endif // _PATCHCONFIG_ENGINEPATCHES
