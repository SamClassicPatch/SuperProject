; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CDlgCreateNormalTexture
LastTemplate=CDialog
NewFileInclude1=#include "StdH.h"
NewFileInclude2=#include "ctgfx.h"
LastPage=0

ClassCount=8

ResourceCount=15
Class1=CDlgSelectMode
Resource1=IDD_CREATE_EFFECT_TEXTURE
Class2=CDlgCreateAnimatedTexture
Resource2=IDD_CHOOSE_TEXTURE_TYPE
Resource3=IDD_RECURSIVE_FILE_BROWSER
Resource4=IDD_SELECT_MODE_DIALOG
Class3=CDlgChooseTextureType
Class4=CDlgCreateNormalTexture
Class5=CDlgCreateEffectTexture
Class6=CWndDetailTexture
Class7=CWndDisplayTexture
Resource5=IDD_CREATE_ANIMATED_TEXTURE
Resource6=IDD_CREATE_ANIMATED_TEXTURE (English (U.S.))
Resource7=IDD_CHOOSE_TEXTURE_TYPE (English (U.S.))
Resource8=IDD_RECURSIVE_FILE_BROWSER (English (U.S.))
Resource9=IDD_CREATE_NORMAL_TEXTURE
Resource10=IDD_GFX_FILE_REQUESTER (English (U.S.))
Resource11=IDD_DIALOG2
Resource12=IDD_GFX_FILE_REQUESTER
Class8=CDlgRecursiveFileBrowser
Resource13=IDD_CREATE_EFFECT_TEXTURE (English (U.S.))
Resource14=IDD_CREATE_NORMAL_TEXTURE (English (U.S.))
Resource15=IDD_SELECT_MODE_DIALOG (English (U.S.))

[DLG:IDD_SELECT_MODE_DIALOG]
Type=1
Class=CDlgSelectMode
ControlCount=17
Control1=IDC_API,combobox,1344339971
Control2=IDC_RESOLUTIONS,combobox,1344339971
Control3=IDC_COLOR_DEFAULT,button,1342373897
Control4=IDC_COLOR_HIGH,button,1342242825
Control5=IDC_COLOR_TRUE,button,1342242825
Control6=ID_TEST_BUTTON,button,1342242817
Control7=IDOK,button,1342242816
Control8=IDCANCEL,button,1342242816
Control9=IDC_CURRENT_MODE,static,1342308352
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308354
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,static,1342308352
Control15=IDC_CURRENT_DRIVER,static,1342308352
Control16=IDC_STATIC,button,1342177287
Control17=IDC_STATIC,button,1342177287

[CLS:CDlgSelectMode]
Type=0
HeaderFile=DlgSelectMode.h
ImplementationFile=DlgSelectMode.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_COLOR_HIGH
VirtualFilter=dWC

[DLG:IDD_CREATE_ANIMATED_TEXTURE]
Type=1
Class=CDlgCreateAnimatedTexture
ControlCount=13
Control1=IDC_EDIT_SCRIPT,edit,1352732868
Control2=ID_REFRESH_TEXTURE,button,1342242816
Control3=IDC_TEXTURE_PREVIEW_WINDOW,static,1350569991
Control4=IDC_STATIC,static,1342308352
Control5=IDC_SIZE_IN_PIXELS,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_TEXTURE_ANIMATIONS,combobox,1344339971
Control8=IDC_CHEQUERED_ALPHA,button,1342242819
Control9=ID_CREATE_TEXTURE,button,1342242817
Control10=IDCANCEL,button,1342242816
Control11=IDC_STATIC,static,1342308353
Control12=IDC_STATIC,static,1342308352
Control13=IDC_TEXTURE_NAME,static,1342308352

[DLG:IDD_CREATE_NORMAL_TEXTURE]
Type=1
Class=CDlgCreateNormalTexture
ControlCount=18
Control1=IDC_CREATED_TEXTURE_NAME,static,1342312448
Control2=IDC_STATIC,static,1342308352
Control3=IDC_SIZE_IN_PIXELS,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_MEX_SIZE,combobox,1344339971
Control6=IDC_CREATE_MIPMAPS,button,1342242819
Control7=IDC_FORCE32,button,1342242819
Control8=IDC_STATICTEX,button,1342242819
Control9=IDC_CONSTANT,button,1342242819
Control10=IDC_COMPRESSED,button,1342242819
Control11=IDC_COMPRESSALPHA,button,1342242819
Control12=ID_CREATE_TEXTURE,button,1342242817
Control13=IDCANCEL,button,1342242816
Control14=IDC_CHEQUERED_ALPHA,button,1342242819
Control15=IDC_TEXTURE_PREVIEW_WINDOW,static,1350569991
Control16=IDC_STATIC,static,1342308353
Control17=IDC_MEMORY_T,static,1342308352
Control18=IDC_STATIC,button,1342178055

[DLG:IDD_CREATE_EFFECT_TEXTURE]
Type=1
Class=CDlgCreateEffectTexture
ControlCount=22
Control1=IDC_MEX_SIZE,combobox,1344339971
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308353
Control4=IDC_EFFECT_CLASS,combobox,1344339971
Control5=IDC_STATIC,static,1342308354
Control6=IDC_PIX_WIDTH,combobox,1344339971
Control7=IDC_STATIC,static,1342308354
Control8=IDC_PIX_HEIGHT,combobox,1344339971
Control9=IDC_STATIC,static,1342308352
Control10=IDC_TEXTURE_PREVIEW_WINDOW,static,1350569991
Control11=IDC_STATIC,static,1342308352
Control12=ID_BROWSE_BASE,button,1342242816
Control13=IDC_CHEQUERED_ALPHA,button,1342242819
Control14=IDC_STATIC,static,1342308353
Control15=IDC_EFFECT_TYPE,combobox,1344339971
Control16=ID_REMOVE_ALL_EFFECTS,button,1342242816
Control17=ID_CREATE,button,1342242817
Control18=IDCANCEL,button,1342242816
Control19=IDC_BASE_TEXTURE_NAME,static,1342312448
Control20=IDC_CREATED_TEXTURE_NAME,static,1342312448
Control21=ID_CREATE_AS,button,1342242816
Control22=IDC_REND_SPEED,static,1342308352

[DLG:IDD_CHOOSE_TEXTURE_TYPE]
Type=1
Class=CDlgChooseTextureType
ControlCount=4
Control1=ID_NORMAL_TEXTURE,button,1342242817
Control2=ID_ANIMATED_TEXTURE,button,1342242816
Control3=ID_EFFECT_TEXTURE,button,1342242816
Control4=IDCANCEL,button,1342242816

[CLS:CDlgChooseTextureType]
Type=0
HeaderFile=DlgChooseTextureType.h
ImplementationFile=DlgChooseTextureType.cpp
BaseClass=CDialog
Filter=D
LastObject=CDlgChooseTextureType
VirtualFilter=dWC

[CLS:CDlgCreateNormalTexture]
Type=0
HeaderFile=DlgCreateNormalTexture.h
ImplementationFile=DlgCreateNormalTexture.cpp
BaseClass=CDialog
Filter=D
LastObject=CDlgCreateNormalTexture
VirtualFilter=dWC

[CLS:CDlgCreateAnimatedTexture]
Type=0
HeaderFile=DlgCreateAnimatedTexture.h
ImplementationFile=DlgCreateAnimatedTexture.cpp
BaseClass=CDialog
Filter=D
LastObject=CDlgCreateAnimatedTexture
VirtualFilter=dWC

[CLS:CDlgCreateEffectTexture]
Type=0
HeaderFile=DlgCreateEffectTexture.h
ImplementationFile=DlgCreateEffectTexture.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_REND_SPEED
VirtualFilter=dWC

[CLS:CWndDetailTexture]
Type=0
HeaderFile=WndDetailTexture.h
ImplementationFile=WndDetailTexture.cpp
BaseClass=CWnd
Filter=W
LastObject=CWndDetailTexture
VirtualFilter=WC

[CLS:CWndDisplayTexture]
Type=0
HeaderFile=WndDisplayTexture.h
ImplementationFile=WndDisplayTexture.cpp
BaseClass=CWnd
Filter=W
LastObject=IDC_THUMBNAIL_DESCRIPTION
VirtualFilter=WC

[DLG:IDD_CREATE_EFFECT_TEXTURE (English (U.S.))]
Type=1
Class=CDlgCreateEffectTexture
ControlCount=22
Control1=IDC_MEX_SIZE,combobox,1344339971
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308353
Control4=IDC_EFFECT_CLASS,combobox,1344339971
Control5=IDC_STATIC,static,1342308354
Control6=IDC_PIX_WIDTH,combobox,1344339971
Control7=IDC_STATIC,static,1342308354
Control8=IDC_PIX_HEIGHT,combobox,1344339971
Control9=IDC_STATIC,static,1342308352
Control10=IDC_TEXTURE_PREVIEW_WINDOW,static,1350569991
Control11=IDC_STATIC,static,1342308352
Control12=ID_BROWSE_BASE,button,1342242816
Control13=IDC_CHEQUERED_ALPHA,button,1342242819
Control14=IDC_STATIC,static,1342308353
Control15=IDC_EFFECT_TYPE,combobox,1344339971
Control16=ID_REMOVE_ALL_EFFECTS,button,1342242816
Control17=ID_CREATE,button,1342242817
Control18=IDCANCEL,button,1342242816
Control19=IDC_BASE_TEXTURE_NAME,static,1342312448
Control20=IDC_CREATED_TEXTURE_NAME,static,1342312448
Control21=ID_CREATE_AS,button,1342242816
Control22=IDC_REND_SPEED,static,1342308352

[DLG:IDD_SELECT_MODE_DIALOG (English (U.S.))]
Type=1
Class=CDlgSelectMode
ControlCount=17
Control1=IDC_API,combobox,1344339971
Control2=IDC_RESOLUTIONS,combobox,1344339971
Control3=IDC_COLOR_DEFAULT,button,1342373897
Control4=IDC_COLOR_HIGH,button,1342242825
Control5=IDC_COLOR_TRUE,button,1342242825
Control6=ID_TEST_BUTTON,button,1342242817
Control7=IDOK,button,1342242816
Control8=IDCANCEL,button,1342242816
Control9=IDC_CURRENT_MODE,static,1342308352
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308354
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,static,1342308352
Control15=IDC_CURRENT_DRIVER,static,1342308352
Control16=IDC_STATIC,button,1342177287
Control17=IDC_STATIC,button,1342177287

[DLG:IDD_CREATE_ANIMATED_TEXTURE (English (U.S.))]
Type=1
Class=CDlgCreateAnimatedTexture
ControlCount=13
Control1=IDC_EDIT_SCRIPT,edit,1352732868
Control2=ID_REFRESH_TEXTURE,button,1342242816
Control3=IDC_TEXTURE_PREVIEW_WINDOW,static,1350569991
Control4=IDC_STATIC,static,1342308352
Control5=IDC_SIZE_IN_PIXELS,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_TEXTURE_ANIMATIONS,combobox,1344339971
Control8=IDC_CHEQUERED_ALPHA,button,1342242819
Control9=ID_CREATE_TEXTURE,button,1342242817
Control10=IDCANCEL,button,1342242816
Control11=IDC_STATIC,static,1342308353
Control12=IDC_STATIC,static,1342308352
Control13=IDC_TEXTURE_NAME,static,1342308352

[DLG:IDD_CREATE_NORMAL_TEXTURE (English (U.S.))]
Type=1
Class=CDlgCreateNormalTexture
ControlCount=18
Control1=IDC_CREATED_TEXTURE_NAME,static,1342312448
Control2=IDC_STATIC,static,1342308352
Control3=IDC_SIZE_IN_PIXELS,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_MEX_SIZE,combobox,1344339971
Control6=IDC_CREATE_MIPMAPS,button,1342242819
Control7=IDC_FORCE32,button,1342242819
Control8=IDC_STATICTEX,button,1342242819
Control9=IDC_CONSTANT,button,1342242819
Control10=IDC_COMPRESSED,button,1342242819
Control11=IDC_COMPRESSALPHA,button,1342242819
Control12=ID_CREATE_TEXTURE,button,1342242817
Control13=IDCANCEL,button,1342242816
Control14=IDC_CHEQUERED_ALPHA,button,1342242819
Control15=IDC_TEXTURE_PREVIEW_WINDOW,static,1350569991
Control16=IDC_STATIC,static,1342308353
Control17=IDC_MEMORY_T,static,1342308352
Control18=IDC_STATIC,button,1342178055

[DLG:IDD_CHOOSE_TEXTURE_TYPE (English (U.S.))]
Type=1
Class=CDlgChooseTextureType
ControlCount=4
Control1=ID_NORMAL_TEXTURE,button,1342242817
Control2=ID_ANIMATED_TEXTURE,button,1342242816
Control3=ID_EFFECT_TEXTURE,button,1342242816
Control4=IDCANCEL,button,1342242816

[DLG:IDD_DIALOG2]
Type=1
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816

[DLG:IDD_GFX_FILE_REQUESTER]
Type=1
Class=?
ControlCount=2
Control1=IDC_THUMBNAIL_DESCRIPTION,static,1342308353
Control2=IDC_THUMBNAIL_RECT,static,1073741831

[DLG:IDD_RECURSIVE_FILE_BROWSER]
Type=1
Class=CDlgRecursiveFileBrowser
ControlCount=6
Control1=IDC_FILE_BEING_PROCESSED,static,1342308352
Control2=IDC_ELAPSED_TIME,static,1342308352
Control3=IDC_ESTIMATED_TIME,static,1342308352
Control4=IDC_STATIC,static,1342308864
Control5=IDC_PROGRESS_BAR,msctls_progress32,1350565889
Control6=IDC_PROGRESS_COUNT,static,1342308354

[CLS:CDlgRecursiveFileBrowser]
Type=0
HeaderFile=DlgRecursiveFileBrowser.h
ImplementationFile=DlgRecursiveFileBrowser.cpp
BaseClass=CDialog
Filter=D
LastObject=CDlgRecursiveFileBrowser
VirtualFilter=dWC

[DLG:IDD_GFX_FILE_REQUESTER (English (U.S.))]
Type=1
Class=?
ControlCount=2
Control1=IDC_THUMBNAIL_DESCRIPTION,static,1342308353
Control2=IDC_THUMBNAIL_RECT,static,1073741831

[DLG:IDD_RECURSIVE_FILE_BROWSER (English (U.S.))]
Type=1
Class=?
ControlCount=6
Control1=IDC_FILE_BEING_PROCESSED,static,1342308352
Control2=IDC_ELAPSED_TIME,static,1342308352
Control3=IDC_ESTIMATED_TIME,static,1342308352
Control4=IDC_STATIC,static,1342308864
Control5=IDC_PROGRESS_BAR,msctls_progress32,1350565889
Control6=IDC_PROGRESS_COUNT,static,1342308354

