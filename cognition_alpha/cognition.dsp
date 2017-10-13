# Microsoft Developer Studio Project File - Name="cognition" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=cognition - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cognition.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cognition.mak" CFG="cognition - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cognition - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "cognition - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cognition - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "VC98" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib opengl32.lib glu32.lib Ws2_32.lib dxguid.lib dinput8.lib dsound.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "cognition - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "VC98" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib glu32.lib Ws2_32.lib kernel32.lib user32.lib gdi32.lib dxguid.lib dinput8.lib dsound.lib /nologo /subsystem:windows /pdb:"cognition/cognition.pdb" /debug /machine:I386 /out:"cognition.exe" /pdbtype:sept
# SUBTRACT LINK32 /profile /pdb:none /map

!ENDIF 

# Begin Target

# Name "cognition - Win32 Release"
# Name "cognition - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cognition\base_bind.c
# End Source File
# Begin Source File

SOURCE=.\cognition\base_bitstream.c
# End Source File
# Begin Source File

SOURCE=.\cognition\base_command.c
# End Source File
# Begin Source File

SOURCE=.\cognition\base_filelog.c
# End Source File
# Begin Source File

SOURCE=.\cognition\base_memory_debug.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\cognition\base_memory_release.c

!IF  "$(CFG)" == "cognition - Win32 Release"

!ELSEIF  "$(CFG)" == "cognition - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cognition\base_variable.c
# End Source File
# Begin Source File

SOURCE=.\cognition\dr_camera.c
# End Source File
# Begin Source File

SOURCE=.\cognition\dr_console.c
# End Source File
# Begin Source File

SOURCE=.\cognition\dr_draw.c
# End Source File
# Begin Source File

SOURCE=.\cognition\dr_entity.c
# End Source File
# Begin Source File

SOURCE=.\cognition\dr_light.c
# End Source File
# Begin Source File

SOURCE=.\cognition\dr_loading.c
# End Source File
# Begin Source File

SOURCE=.\cognition\dr_menu.c
# End Source File
# Begin Source File

SOURCE=.\cognition\dr_misc.c
# End Source File
# Begin Source File

SOURCE=.\cognition\dr_skybox.c
# End Source File
# Begin Source File

SOURCE=.\cognition\dr_text.c
# End Source File
# Begin Source File

SOURCE=.\cognition\dr_transparent.c
# End Source File
# Begin Source File

SOURCE=.\cognition\dr_world.c
# End Source File
# Begin Source File

SOURCE=.\cognition\game_background.c
# End Source File
# Begin Source File

SOURCE=.\cognition\game_entity.c
# End Source File
# Begin Source File

SOURCE=.\cognition\game_game.c
# End Source File
# Begin Source File

SOURCE=.\cognition\game_particlesys.c
# End Source File
# Begin Source File

SOURCE=.\cognition\game_world.c
# End Source File
# Begin Source File

SOURCE=.\cognition\lib_filesys.c
# End Source File
# Begin Source File

SOURCE=.\cognition\lib_parse.c
# End Source File
# Begin Source File

SOURCE=.\cognition\lib_rng.c
# End Source File
# Begin Source File

SOURCE=.\cognition\linked_list.c
# End Source File
# Begin Source File

SOURCE=.\cognition\media_bitmap.c
# End Source File
# Begin Source File

SOURCE=.\cognition\media_colormap.c
# End Source File
# Begin Source File

SOURCE=.\cognition\media_image.c
# End Source File
# Begin Source File

SOURCE=.\cognition\media_jpeg.c
# End Source File
# Begin Source File

SOURCE=.\cognition\media_model.c
# End Source File
# Begin Source File

SOURCE=.\cognition\media_moo.c
# End Source File
# Begin Source File

SOURCE=.\cognition\media_mp3.c
# End Source File
# Begin Source File

SOURCE=.\cognition\media_shader.c
# End Source File
# Begin Source File

SOURCE=.\cognition\media_targa.c
# End Source File
# Begin Source File

SOURCE=.\cognition\media_wave.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_display_win.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_input_win.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_input_win_di.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_input_win_win32.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_module_win.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_network_win.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_opengl_win.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_sound_win.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_sound_win_ds.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_system_win.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_timesys_bsd.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_timesys_linux.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_timesys_win.c
# End Source File
# Begin Source File

SOURCE=.\shared\shared_library.c
# End Source File
# Begin Source File

SOURCE=.\shared\shared_mathlib.c
# End Source File
# Begin Source File

SOURCE=.\shared\shared_queue.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sv_collision.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sv_console.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sv_engine.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sv_mouse.c
# End Source File
# Begin Source File

SOURCE=.\cognition\sv_music.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cognition\cog_global.h
# End Source File
# Begin Source File

SOURCE=.\cognition\cog_opengl.h
# End Source File
# Begin Source File

SOURCE=.\cognition\cog_structures.h
# End Source File
# Begin Source File

SOURCE=.\cognition\cog_sysdeps.h
# End Source File
# Begin Source File

SOURCE=.\cognition\linked_list.h
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_input_win_di.h
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_input_win_win32.h
# End Source File
# Begin Source File

SOURCE=.\cognition\sal_sound_win_ds.h
# End Source File
# Begin Source File

SOURCE=.\shared\shared_keycodes.h
# End Source File
# Begin Source File

SOURCE=.\shared\shared_library.h
# End Source File
# Begin Source File

SOURCE=.\shared\shared_mathlib.h
# End Source File
# Begin Source File

SOURCE=.\shared\shared_misc.h
# End Source File
# Begin Source File

SOURCE=.\shared\shared_structures.h
# End Source File
# Begin Source File

SOURCE=.\cognition\sv_console.h
# End Source File
# Begin Source File

SOURCE=.\cognition\sv_window.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\runlog.txt
# End Source File
# End Group
# End Target
# End Project
