mkdir archie

export PATH=~/archiesdk/tools/bin:$PATH


unset CFLAGS


export RENDER_OPTIONS="-DFLAT_SPAN -DFLAT_NUKAGE1_COLOR=64 -DFLAT_SKY_COLOR=47 -DWAD_FILE=\"HostFS::HostFS.$.DOOMAR56\" -DVIEWWINDOWWIDTH=60 -DC_ONLY"


arm-archie-gcc -c i_archiv.c $RENDER_OPTIONS -mno-thumb-interwork -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN
arm-archie-gcc -c p_enemy2.c $RENDER_OPTIONS -mno-thumb-interwork -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN
arm-archie-gcc -c p_map.c    $RENDER_OPTIONS -mno-thumb-interwork -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN
arm-archie-gcc -c p_maputl.c $RENDER_OPTIONS -mno-thumb-interwork -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN
arm-archie-gcc -c p_mobj.c   $RENDER_OPTIONS -mno-thumb-interwork -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN
arm-archie-gcc -c p_sight.c  $RENDER_OPTIONS -mno-thumb-interwork -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN
arm-archie-gcc -c r_data.c   $RENDER_OPTIONS -mno-thumb-interwork -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN
arm-archie-gcc -c r_draw.c   $RENDER_OPTIONS -mno-thumb-interwork -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN
arm-archie-gcc -c r_plane.c  $RENDER_OPTIONS -mno-thumb-interwork -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN
arm-archie-gcc -c tables.c   $RENDER_OPTIONS -mno-thumb-interwork -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN
arm-archie-gcc -c w_wad.c    $RENDER_OPTIONS -mno-thumb-interwork -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN
arm-archie-gcc -c z_zone.c   $RENDER_OPTIONS -mno-thumb-interwork -Ofast -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-all-loops -fira-loop-pressure -freorder-blocks-algorithm=simple -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN

export CFLAGS="-mno-thumb-interwork -Os -fomit-frame-pointer -fgcse-sm -fgcse-las -fipa-pta -flto -fwhole-program -funroll-all-loops -freorder-blocks-algorithm=stc -fno-tree-pre -fira-region=mixed -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN"
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
export GLOBOBJS+=" i_archie.c"
#export GLOBOBJS+=" i_archiv.c"
export GLOBOBJS+=" i_archiv.o"
export GLOBOBJS+=" info.c"
export GLOBOBJS+=" m_cheat.c"
export GLOBOBJS+=" m_menu.c"
export GLOBOBJS+=" m_random.c"
export GLOBOBJS+=" p_doors.c"
export GLOBOBJS+=" p_enemy.c"
#export GLOBOBJS+=" p_enemy2.c"
export GLOBOBJS+=" p_enemy2.o"
export GLOBOBJS+=" p_floor.c"
export GLOBOBJS+=" p_inter.c"
export GLOBOBJS+=" p_lights.c"
#export GLOBOBJS+=" p_map.c"
export GLOBOBJS+=" p_map.o"
#export GLOBOBJS+=" p_maputl.c"
export GLOBOBJS+=" p_maputl.o"
#export GLOBOBJS+=" p_mobj.c"
export GLOBOBJS+=" p_mobj.o"
export GLOBOBJS+=" p_plats.c"
export GLOBOBJS+=" p_pspr.c"
export GLOBOBJS+=" p_setup.c"
#export GLOBOBJS+=" p_sight.c"
export GLOBOBJS+=" p_sight.o"
export GLOBOBJS+=" p_spec.c"
export GLOBOBJS+=" p_switch.c"
export GLOBOBJS+=" p_telept.c"
export GLOBOBJS+=" p_tick.c"
export GLOBOBJS+=" p_user.c"
#export GLOBOBJS+=" r_data.c"
export GLOBOBJS+=" r_data.o"
#export GLOBOBJS+=" r_draw.c"
export GLOBOBJS+=" r_draw.o"
#export GLOBOBJS+=" r_plane.c"
export GLOBOBJS+=" r_plane.o"
export GLOBOBJS+=" r_sky.c"
export GLOBOBJS+=" r_things.c"
export GLOBOBJS+=" s_sound.c"
export GLOBOBJS+=" sounds.c"
export GLOBOBJS+=" st_pal.c"
export GLOBOBJS+=" st_stuff.c"
#export GLOBOBJS+=" tables.c"
export GLOBOBJS+=" tables.o"
export GLOBOBJS+=" v_video.c"
#export GLOBOBJS+=" w_wad.c"
export GLOBOBJS+=" w_wad.o"
export GLOBOBJS+=" wi_lib.c"
export GLOBOBJS+=" wi_stuff.c"
export GLOBOBJS+=" z_bmallo.c"
#export GLOBOBJS+=" z_zone.c"
export GLOBOBJS+=" z_zone.o"

arm-archie-gcc $GLOBOBJS $CFLAGS $RENDER_OPTIONS -o archie/tmpProg
arm-archie-objcopy -O binary archie/tmpProg archie/DOOM8088,ff8

rm i_archiv.o
rm p_enemy2.o
rm p_map.o
rm p_maputl.o
rm p_mobj.o
rm p_sight.o
rm r_data.o
rm r_draw.o
rm r_plane.o
rm tables.o
rm w_wad.o
rm z_zone.o
rm archie/tmpProg
