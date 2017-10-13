# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

bin_PROGRAMS=cognition
cognition_SOURCES= \
		base_bind.c \
		base_bitstream.c \
		base_command.c \
		base_filelog.c \
		base_variable.c \
		cog_global.h \
		cog_opengl.h \
		cog_structures.h \
		cog_sysdeps.h \
		dr_camera.c \
		dr_console.c \
		dr_draw.c \
		dr_entity.c \
		dr_light.c \
		dr_loading.c \
		dr_menu.c \
		dr_misc.c \
		dr_skybox.c \
		dr_text.c \
		dr_transparent.c \
		dr_vbo.c \
		dr_world.c \
		game_background.c \
		game_engine.c \
		game_entity.c \
		game_game.c \
		game_particlesys.c \
		game_world.c \
		lib_filesys.c \
		lib_parse.c \
		lib_rng.c \
		media_bitmap.c \
		media_colormap.c \
		media_image.c \
		media_jpeg.c \
		media_model.c \
		media_moo.c \
		media_mp3.c \
		media_shader.c \
		media_targa.c \
		media_wave.c \
		sal_directory_linux.c \
		sal_display_sdl.c \
		sal_input_sdl.c \
		sal_module_linux.c \
		sal_opengl_linux.c \
		sal_sound_openal.c \
		sal_system_linux.c \
		sal_timesys_sdl.c \
		sv_collision.c \
		sv_console.c \
		sv_console.h \
		sv_input.c \
		sv_mouse.c \
		sv_music.c \
		sv_network.c \
		sv_sound.c \
		sv_timer.c
		
#sal_input_sdl.c 
#sal_module_linux.c 
#sal_network_linux.c 
#sal_opengl_linux.c 

if DEBUG
cognition_SOURCES += base_memory_debug.c
else
cognition_SOURCES += base_memory_release.c
endif

cognition_CFLAGS=-g -Wall -DHAVE_SDL ${GL_CFLAGS} @GLEW_CFLAGS@ @SDL_CFLAGS@ -DHAVE_OPENAL
cognition_LDFLAGS=-g -Wall
cognition_LDADD=../shared/libcognitionshared.la -lopenal ${GL_LIBS} @GLEW_LIBS@ @SDL_LIBS@
