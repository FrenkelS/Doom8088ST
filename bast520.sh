mkdir atarist

unset CFLAGS


export RENDER_OPTIONS="-DFLAT_SPAN -DFLAT_NUKAGE1_COLOR=65 -DFLAT_SKY_COLOR=3 -DWAD_FILE=\"DOOMST2.WAD\" -DMAPWIDTH=480 -DNR_OF_COLORS=2 -DLOW_MEMORY"

export CPU=$1
export OUTPUT=$2

if [ -z "$CPU" ]
then
  export CPU=68000
fi

if [ -z "$OUTPUT" ]
then
  export OUTPUT=DOOM520.TOS
fi

export CFLAGS="-march=$CPU -Os -fomit-frame-pointer -mfastcall -flto -fwhole-program -fira-loop-pressure -funsafe-loop-optimizations -freorder-blocks-algorithm=stc -fno-tree-pre"
#export CFLAGS="$CFLAGS -Ofast -flto -fwhole-program -fomit-frame-pointer -funroll-loops -fgcse-sm -fgcse-las -fipa-pta -Wno-attributes -Wpedantic"
#export CFLAGS="$CFLAGS -Wall -Wextra"

export GLOBOBJS="  am_map.c"
export GLOBOBJS+=" d_items.c"
export GLOBOBJS+=" d_main.c"
export GLOBOBJS+=" f_finale.c"
export GLOBOBJS+=" f_lib.c"
export GLOBOBJS+=" g_game.c"
export GLOBOBJS+=" hu_stuff.c"
export GLOBOBJS+=" i_audio.c"
export GLOBOBJS+=" i_ast.c"
export GLOBOBJS+=" i_vast2.c"
#export GLOBOBJS+=" i_vast2.o"
export GLOBOBJS+=" info.c"
export GLOBOBJS+=" m_cheat.c"
export GLOBOBJS+=" m_menu.c"
export GLOBOBJS+=" m_random.c"
export GLOBOBJS+=" p_doors.c"
export GLOBOBJS+=" p_enemy.c"
export GLOBOBJS+=" p_enemy2.c"
#export GLOBOBJS+=" p_enemy2.o"
export GLOBOBJS+=" p_floor.c"
export GLOBOBJS+=" p_inter.c"
export GLOBOBJS+=" p_lights.c"
export GLOBOBJS+=" p_map.c"
#export GLOBOBJS+=" p_map.o"
export GLOBOBJS+=" p_maputl.c"
#export GLOBOBJS+=" p_maputl.o"
export GLOBOBJS+=" p_mobj.c"
#export GLOBOBJS+=" p_mobj.o"
export GLOBOBJS+=" p_plats.c"
export GLOBOBJS+=" p_pspr.c"
export GLOBOBJS+=" p_setup.c"
export GLOBOBJS+=" p_sight.c"
#export GLOBOBJS+=" p_sight.o"
export GLOBOBJS+=" p_spec.c"
export GLOBOBJS+=" p_switch.c"
export GLOBOBJS+=" p_telept.c"
export GLOBOBJS+=" p_tick.c"
export GLOBOBJS+=" p_user.c"
export GLOBOBJS+=" r_data.c"
#export GLOBOBJS+=" r_data.o"
export GLOBOBJS+=" r_draw.c"
#export GLOBOBJS+=" r_draw.o"
export GLOBOBJS+=" r_plane.c"
#export GLOBOBJS+=" r_plane.o"
export GLOBOBJS+=" r_sky.c"
export GLOBOBJS+=" r_things.c"
export GLOBOBJS+=" s_sound.c"
export GLOBOBJS+=" sounds.c"
export GLOBOBJS+=" st_pal.c"
export GLOBOBJS+=" st_stuff.c"
export GLOBOBJS+=" tables.c"
#export GLOBOBJS+=" tables.o"
export GLOBOBJS+=" v_video.c"
export GLOBOBJS+=" w_wad.c"
#export GLOBOBJS+=" w_wad.o"
export GLOBOBJS+=" wi_lib.c"
export GLOBOBJS+=" wi_stuff.c"
export GLOBOBJS+=" z_bmallo.c"
export GLOBOBJS+=" z_zone.c"
#export GLOBOBJS+=" z_zone.o"

#m68k-atari-mintelf-gcc $GLOBOBJS $CFLAGS $RENDER_OPTIONS -o atarist/$OUTPUT
m68k-atari-mintelf-gcc -nostdlib libcmini/crt0.o $GLOBOBJS $CFLAGS $RENDER_OPTIONS -o atarist/$OUTPUT -s -Llibcmini -lcmini -lgcc
