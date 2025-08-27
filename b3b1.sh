mkdir 3b1


export RENDER_OPTIONS="-DFLAT_SPAN -DFLAT_NUKAGE1_COLOR=65 -DFLAT_SKY_COLOR=3 -DWAD_FILE=\"DOOM3B1.WAD\" -DMAPWIDTH=480 -DNR_OF_COLORS=2"


export CC=/home/frenkel/gcc-for-unixpc/bin/unixpc-gcc
export CFLAGS="-m68000 -fomit-frame-pointer -funroll-loops -DBIG_ENDIAN=4321 -DBYTE_ORDER=BIG_ENDIAN -std=c99"


export GLOBOBJS=""
export GLOBOBJS+=" r_draw.c"
export GLOBOBJS+=" i_v3b1.c"
export GLOBOBJS+=" p_enemy2.c"
export GLOBOBJS+=" p_map.c"
export GLOBOBJS+=" p_maputl.c"
export GLOBOBJS+=" p_mobj.c"
export GLOBOBJS+=" p_sight.c"
export GLOBOBJS+=" r_data.c"
export GLOBOBJS+=" r_plane.c"
export GLOBOBJS+=" tables.c"
export GLOBOBJS+=" w_wad.c"
export GLOBOBJS+=" z_zone.c"

cat $GLOBOBJS | $CC -x c -c -O3 -fgcse-sm $CFLAGS $RENDER_OPTIONS -o 3b1/speed.o -


export GLOBOBJS=""
export GLOBOBJS+=" am_map.c"
export GLOBOBJS+=" d_items.c"
export GLOBOBJS+=" d_main.c"
export GLOBOBJS+=" f_finale.c"
export GLOBOBJS+=" f_lib.c"
export GLOBOBJS+=" g_game.c"
export GLOBOBJS+=" hu_stuff.c"
export GLOBOBJS+=" i_audio.c"
export GLOBOBJS+=" i_3b1.c"
export GLOBOBJS+=" info.c"
export GLOBOBJS+=" m_cheat.c"
export GLOBOBJS+=" m_menu.c"
export GLOBOBJS+=" m_random.c"
export GLOBOBJS+=" p_doors.c"
export GLOBOBJS+=" p_enemy.c"
export GLOBOBJS+=" p_floor.c"
export GLOBOBJS+=" p_inter.c"
export GLOBOBJS+=" p_lights.c"
export GLOBOBJS+=" p_plats.c"
export GLOBOBJS+=" p_pspr.c"
export GLOBOBJS+=" p_setup.c"
export GLOBOBJS+=" p_spec.c"
export GLOBOBJS+=" p_switch.c"
export GLOBOBJS+=" p_telept.c"
export GLOBOBJS+=" p_tick.c"
export GLOBOBJS+=" p_user.c"
export GLOBOBJS+=" r_sky.c"
export GLOBOBJS+=" r_things.c"
export GLOBOBJS+=" s_sound.c"
export GLOBOBJS+=" sounds.c"
export GLOBOBJS+=" st_pal.c"
export GLOBOBJS+=" st_stuff.c"
export GLOBOBJS+=" v_video.c"
export GLOBOBJS+=" wi_lib.c"
export GLOBOBJS+=" wi_stuff.c"
export GLOBOBJS+=" z_bmallo.c"

cat $GLOBOBJS | $CC -x c -c -Os $CFLAGS $RENDER_OPTIONS -o 3b1/space.o -


echo TODO gcc speed.o space.o -o doom8088
