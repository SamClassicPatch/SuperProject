# Microsoft Developer Studio Project File - Name="PluginModule" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PluginModule - Win32 Debug_TSE107
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PluginModule.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PluginModule.mak" CFG="PluginModule - Win32 Debug_TSE107"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PluginModule - Win32 Debug_TSE107" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PluginModule - Win32 Release_TFE105" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PluginModule - Win32 Release_TSE105" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PluginModule - Win32 Release_TSE107" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PluginModule - Win32 Debug_TSE107"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "../../Bin/Debug_TSE107"
# PROP BASE Intermediate_Dir "../../Obj/Debug_TSE107/PluginModule"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../Bin/Debug_TSE107"
# PROP Intermediate_Dir "../../Obj/Debug_TSE107/PluginModule"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../.." /I "../../Includes/Engine107" /I "../../Includes/Common" /D "_DEBUG" /Fp"../../Bin/Debug_TSE107/ExtensionSample.pch" /Yu"StdH.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../.." /I "../../Includes/Engine107" /I "../../Includes/Common" /D "_DEBUG" /Fp"../../Bin/Debug_TSE107/ExtensionSample.pch" /Yu"StdH.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"../../Bin/Debug_TSE107/ExtensionSample.bsc"
# ADD BSC32 /nologo /o"../../Bin/Debug_TSE107/ExtensionSample.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../Bin/Debug_TSE107/ExtensionSample.dll" /pdbtype:sept /libpath:"../../Includes/Engine107" /libpath:"../../Bin/Debug_TSE107"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../Bin/Debug_TSE107/ExtensionSample.dll" /pdbtype:sept /libpath:"../../Includes/Engine107" /libpath:"../../Bin/Debug_TSE107"

!ELSEIF  "$(CFG)" == "PluginModule - Win32 Release_TFE105"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "../../Bin/Release_TFE105"
# PROP BASE Intermediate_Dir "../../Obj/Release_TFE105/PluginModule"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../Bin/Release_TFE105"
# PROP Intermediate_Dir "../../Obj/Release_TFE105/PluginModule"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "../.." /I "../../Includes/Engine105FE" /I "../../Includes/Common" /D "NDEBUG" /Fp"../../Bin/Release_TFE105/ExtensionSample.pch" /Yu"StdH.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../.." /I "../../Includes/Engine105FE" /I "../../Includes/Common" /D "NDEBUG" /Fp"../../Bin/Release_TFE105/ExtensionSample.pch" /Yu"StdH.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# SUBTRACT BASE MTL /mktyplib203 /Oicf
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203 /Oicf
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"../../Bin/Release_TFE105/ExtensionSample.bsc"
# ADD BSC32 /nologo /o"../../Bin/Release_TFE105/ExtensionSample.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../Bin/Release_TFE105/ExtensionSample.dll" /libpath:"../../Includes/Engine105FE" /libpath:"../../Bin/Release_TFE105"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib /force
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../Bin/Release_TFE105/ExtensionSample.dll" /libpath:"../../Includes/Engine105FE" /libpath:"../../Bin/Release_TFE105"
# SUBTRACT LINK32 /pdb:none /nodefaultlib /force

!ELSEIF  "$(CFG)" == "PluginModule - Win32 Release_TSE105"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "../../Bin/Release_TSE105"
# PROP BASE Intermediate_Dir "../../Obj/Release_TSE105/PluginModule"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../Bin/Release_TSE105"
# PROP Intermediate_Dir "../../Obj/Release_TSE105/PluginModule"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "../.." /I "../../Includes/Engine105" /I "../../Includes/Common" /D "NDEBUG" /Fp"../../Bin/Release_TSE105/ExtensionSample.pch" /Yu"StdH.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../.." /I "../../Includes/Engine105" /I "../../Includes/Common" /D "NDEBUG" /Fp"../../Bin/Release_TSE105/ExtensionSample.pch" /Yu"StdH.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# SUBTRACT BASE MTL /mktyplib203 /Oicf
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203 /Oicf
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"../../Bin/Release_TSE105/ExtensionSample.bsc"
# ADD BSC32 /nologo /o"../../Bin/Release_TSE105/ExtensionSample.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../Bin/Release_TSE105/ExtensionSample.dll" /libpath:"../../Includes/Engine105" /libpath:"../../Bin/Release_TSE105"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib /force
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../Bin/Release_TSE105/ExtensionSample.dll" /libpath:"../../Includes/Engine105" /libpath:"../../Bin/Release_TSE105"
# SUBTRACT LINK32 /pdb:none /nodefaultlib /force

!ELSEIF  "$(CFG)" == "PluginModule - Win32 Release_TSE107"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "../../Bin/Release_TSE107"
# PROP BASE Intermediate_Dir "../../Obj/Release_TSE107/PluginModule"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../Bin/Release_TSE107"
# PROP Intermediate_Dir "../../Obj/Release_TSE107/PluginModule"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "../.." /I "../../Includes/Engine107" /I "../../Includes/Common" /D "NDEBUG" /Fp"../../Bin/Release_TSE107/ExtensionSample.pch" /Yu"StdH.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../.." /I "../../Includes/Engine107" /I "../../Includes/Common" /D "NDEBUG" /Fp"../../Bin/Release_TSE107/ExtensionSample.pch" /Yu"StdH.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# SUBTRACT BASE MTL /mktyplib203 /Oicf
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203 /Oicf
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"../../Bin/Release_TSE107/ExtensionSample.bsc"
# ADD BSC32 /nologo /o"../../Bin/Release_TSE107/ExtensionSample.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../Bin/Release_TSE107/ExtensionSample.dll" /libpath:"../../Includes/Engine107" /libpath:"../../Bin/Release_TSE107"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib /force
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../Bin/Release_TSE107/ExtensionSample.dll" /libpath:"../../Includes/Engine107" /libpath:"../../Bin/Release_TSE107"
# SUBTRACT LINK32 /pdb:none /nodefaultlib /force

!ENDIF 

# Begin Target

# Name "PluginModule - Win32 Debug_TSE107"
# Name "PluginModule - Win32 Release_TFE105"
# Name "PluginModule - Win32 Release_TSE105"
# Name "PluginModule - Win32 Release_TSE107"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Events"

# PROP Default_Filter ""
# Begin Source File

SOURCE=Source\Events\DemoEvents.cpp
# End Source File
# Begin Source File

SOURCE=Source\Events\GameEvents.cpp
# End Source File
# Begin Source File

SOURCE=Source\Events\Listeners.cpp
# End Source File
# Begin Source File

SOURCE=Source\Events\Networking.cpp
# End Source File
# Begin Source File

SOURCE=Source\Events\Packets.cpp
# End Source File
# Begin Source File

SOURCE=Source\Events\Processing.cpp
# End Source File
# Begin Source File

SOURCE=Source\Events\Rendering.cpp
# End Source File
# Begin Source File

SOURCE=Source\Events\TimerEvents.cpp
# End Source File
# Begin Source File

SOURCE=Source\Events\WorldEvents.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=Source\Main.cpp
# End Source File
# Begin Source File

SOURCE=Source\StdH.cpp
# ADD BASE CPP /Yc"StdH.h"
# ADD CPP /Yc"StdH.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=Source\StdH.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
