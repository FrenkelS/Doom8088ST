md ql

@set PATH=C:\qdos-gcc\bin;%PATH%

set RENDER_OPTIONS=-DFLAT_SPAN -DFLAT_NUKAGE1_COLOR=65 -DFLAT_SKY_COLOR=3 -DWAD_FILE=\"WIN2_DOOMQL8.WAD\" -DMAPWIDTH=480 -DNR_OF_COLORS=2
@rem set CFLAGS=-m68000 -fomit-frame-pointer -funroll-loops -DBIG_ENDIAN=4321 -DBYTE_ORDER=BIG_ENDIAN
set CFLAGS=-m68000 -fomit-frame-pointer -DBIG_ENDIAN=4321 -DBYTE_ORDER=BIG_ENDIAN


@set GLOBOBJS=
@set GLOBOBJS=%GLOBOBJS% r_draw.c
@set GLOBOBJS=%GLOBOBJS% i_qlv8.c
@set GLOBOBJS=%GLOBOBJS% p_enemy2.c
@set GLOBOBJS=%GLOBOBJS% p_map.c
@set GLOBOBJS=%GLOBOBJS% p_maputl.c
@set GLOBOBJS=%GLOBOBJS% p_mobj.c
@set GLOBOBJS=%GLOBOBJS% p_sight.c
@set GLOBOBJS=%GLOBOBJS% r_data.c
@set GLOBOBJS=%GLOBOBJS% r_plane.c
@set GLOBOBJS=%GLOBOBJS% tables.c
@set GLOBOBJS=%GLOBOBJS% w_wad.c
@set GLOBOBJS=%GLOBOBJS% z_zone.c

@rem @type %GLOBOBJS% > speed.c
@rem qdos-gcc -c -O3 %CFLAGS% %RENDER_OPTIONS% speed.c -o ql/speed.o
@rem del speed.c
@rem @set GLOBOBJS=


@set GLOBOBJS=%GLOBOBJS% am_map.c
@set GLOBOBJS=%GLOBOBJS% d_items.c
@set GLOBOBJS=%GLOBOBJS% d_main.c
@set GLOBOBJS=%GLOBOBJS% f_finale.c
@set GLOBOBJS=%GLOBOBJS% f_lib.c
@set GLOBOBJS=%GLOBOBJS% g_game.c
@set GLOBOBJS=%GLOBOBJS% hu_stuff.c
@set GLOBOBJS=%GLOBOBJS% i_audio.c
@set GLOBOBJS=%GLOBOBJS% i_ql.c
@set GLOBOBJS=%GLOBOBJS% info.c
@set GLOBOBJS=%GLOBOBJS% m_cheat.c
@set GLOBOBJS=%GLOBOBJS% m_menu.c
@set GLOBOBJS=%GLOBOBJS% m_random.c
@set GLOBOBJS=%GLOBOBJS% p_doors.c
@set GLOBOBJS=%GLOBOBJS% p_enemy.c
@set GLOBOBJS=%GLOBOBJS% p_floor.c
@set GLOBOBJS=%GLOBOBJS% p_inter.c
@set GLOBOBJS=%GLOBOBJS% p_lights.c
@set GLOBOBJS=%GLOBOBJS% p_plats.c
@set GLOBOBJS=%GLOBOBJS% p_pspr.c
@set GLOBOBJS=%GLOBOBJS% p_setup.c
@set GLOBOBJS=%GLOBOBJS% p_spec.c
@set GLOBOBJS=%GLOBOBJS% p_switch.c
@set GLOBOBJS=%GLOBOBJS% p_telept.c
@set GLOBOBJS=%GLOBOBJS% p_tick.c
@set GLOBOBJS=%GLOBOBJS% p_user.c
@set GLOBOBJS=%GLOBOBJS% r_sky.c
@set GLOBOBJS=%GLOBOBJS% r_things.c
@set GLOBOBJS=%GLOBOBJS% s_sound.c
@set GLOBOBJS=%GLOBOBJS% sounds.c
@set GLOBOBJS=%GLOBOBJS% st_pal.c
@set GLOBOBJS=%GLOBOBJS% st_stuff.c
@set GLOBOBJS=%GLOBOBJS% v_video.c
@set GLOBOBJS=%GLOBOBJS% wi_lib.c
@set GLOBOBJS=%GLOBOBJS% wi_stuff.c
@set GLOBOBJS=%GLOBOBJS% z_bmallo.c

@rem @type %GLOBOBJS% > space.c
@rem qdos-gcc -c -Os %CFLAGS% %RENDER_OPTIONS% space.c -o ql/space.o
@rem del space.c
@rem qdos-gcc ql/speed.o ql/space.o -o ql/doom8088.out

qdos-gcc -Os %CFLAGS% %RENDER_OPTIONS% %GLOBOBJS% -o ql/doom8088.out


del ql\doomql8
rename ql\doom8088.out doomql8
