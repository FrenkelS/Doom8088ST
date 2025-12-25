## Doom8088: Motorola 68000 Edition
![Doom8088: Atari ST Edition](readme_imgs/doomstch.png?raw=true)

Doom was originally designed in 1993 for 32-bit DOS computers with 4 MB of RAM.
It's mostly written in C code with very little assembly code.
It has been ported to all kinds of systems.
Usually these systems are 32-bit or more and have a flat memory model.

Doom8088: Motorola 68000 Edition is a port of Doom for Atari ST, AT&T UNIX PC and Sinclair QL computers with at least 512 kB of RAM (1 MB recommended) and a Motorola 68000, 68008 or 68010 CPU.
It's based on [Doom8088](https://github.com/FrenkelS/Doom8088), a port of Doom for 16-bit DOS computers.
Download Doom8088: Motorola 68000 Edition [here](https://github.com/FrenkelS/Doom8088ST/releases).

**What's special?:**
 - Supports only Doom 1 Episode 1
 - Rotating overlaid automap
 - Only demo3 is supported
 - 16 color mode (Atari ST only)
 -  8 color mode (Sinclair QL only)
 -  2 color mode
 - PC speaker like sound effects (Atari ST only)
 - No music
 - No texture mapped floors and ceilings
 - No light diminishing
 - No saving and loading
 - No multiplayer
 - No PWADs
 - No screen resizing
 - No mouse and joystick support

**Known bugs:**
 - When there's not enough memory for a texture, one color is drawn
 - When there's not enough memory for the intermission screen, the last few frames of gameplay are shown instead

## Supported video modes

### Atari ST 320x200 16 color mode, effective resolution 120x128
![Doom8088 in 16 colors, high detail](readme_imgs/doomstch.png?raw=true)

### Atari ST 320x200 16 color mode, effective resolution  60x128
![Doom8088 in 16 colors, medium detail](readme_imgs/doomstcm.png?raw=true)

### Atari ST 320x200 16 color mode, effective resolution  30x128
![Doom8088 in 16 colors, low detail](readme_imgs/doomstcl.png?raw=true)

### Sinclair QL 256x256 8 color mode, effective resolution 60x128
![Doom8088 in 8 colors](readme_imgs/doomql8.png?raw=true)

### Atari ST 640x200 and Sinclair QL 512x256 4 color mode, effective resolution 60x128 2 colors
![Doom8088 in 2 colors](readme_imgs/doomstbw.png?raw=true)

### AT&T UNIX PC 720x348  2 color mode, effective resolution  60x128 2 colors
![Doom8088 in 2 colors](readme_imgs/doom3b1.png?raw=true)

## Controls:
|Action                 |Atari ST     |Sinclair QL  |AT&T UNIX PC              |
|-----------------------|-------------|-------------|--------------------------|
|Fire                   |Ctrl         |Ctrl         |/                         |
|Use                    |Enter & Space|Enter & Space|Enter & Space             |
|Sprint                 |Shift        |Shift        |not available             |
|Walk                   |Arrow keys   |Arrow keys   |Arrow keys & 8 & 2 & 4 & 6|
|Strafe                 |Alt          |Alt          |not available             |
|Strafe left and right  |< & >        |< & >        |< & >                     |
|Automap                |Tab          |Tabulate     |Tab                       |
|Automap zoom in and out|+ & -        |+ & -        |+ & -                     |
|Automap follow mode    |F            |F            |F                         |
|Weapon up and down     |[ & ]        |[ & ]        |[ & ]                     |
|Menu                   |Esc          |Esc          |Esc                       |
|Quit to OS             |F10          |not available|Shift + Q                 |

## Cheats:
|Code      |Effects                  |Notes                           |
|----------|-------------------------|--------------------------------|
|IDCHOPPERS|Chainsaw                 |                                |
|IDDQD     |God mode                 |                                |
|IDKFA     |Weapons & Keys           |                                |
|IDFA      |Weapons                  |                                |
|IDSPISPOPD|No Clipping              |                                |
|IDBEHOLDV |Invincibility            |                                |
|IDBEHOLDS |Berserk                  |                                |
|IDBEHOLDI |Invisibility             |                                |
|IDBEHOLDR |Radiation shielding suit |                                |
|IDBEHOLDA |Auto-map                 |                                |
|IDBEHOLDL |Lite-Amp Goggles         |                                |
|IDCLEV    |Exit Level               |                                |
|IDEND     |Show end text            |                                |
|IDROCKET  |Enemy Rockets            |(GoldenEye)                     |
|IDRATE    |Toggle FPS counter       |Divide by 10 to get the real FPS|

## Command line arguments:
|Command line argument|Effect               |
|---------------------|---------------------|
|`-nosfx`             |Disable sound effects|
|`-nosound`           |Disable sound effects|
|`-timedemo demo3`    |Run benchmark        |

## Building:
|Platform     |Platform specific code |Compiler                                                                      |Build script                            |Additional information                              |
|-------------|-----------------------|------------------------------------------------------------------------------|----------------------------------------|----------------------------------------------------|
|Atari ST     |`i_ast.c`, `i_astv16.c`|[m68k-atari-mintelf binutils, GCC 14, MiNTLib](https://tho-otto.de/crossmint.php)|`bast16h.sh`, `bast16m.sh`, `bast16l.sh`|320x200 16 color mode, high/medium/low detail mode  |
|Atari ST     |`i_ast.c`, `i_astv2.c` |[m68k-atari-mintelf binutils, GCC 14, MiNTLib](https://tho-otto.de/crossmint.php)|`bast2.sh`, `bast520.sh`                |640x200  4 color mode, 2 colors are used</br>`DOOM520.TOS` is for computers with 512 kB of RAM. It goes from the first level immediately to the last level.|
|AT&T UNIX PC |`i_3b1.c`, `i_3b1v.c`  |[unixpc-gcc](https://github.com/mikehaertel/unixpc-gcc)                       |`b3b1.sh`                               |The build script generates `speed.o` and `space.o`. These files need to be linked to create `doom8088`.|
|Sinclair QL  |`i_ql.c`,  `i_qlv8.c`  |[qdos-gcc](https://github.com/xXorAa/qdos-gcc-2.95.3)                         |`bql8.bat`                              |256x256 8 color mode                                |
|Sinclair QL  |`i_ql.c`,  `i_qlv2.c`  |[qdos-gcc](https://github.com/xXorAa/qdos-gcc-2.95.3)                         |`bql2.bat`                              |512x256 4 color mode                                |
|IBM PC 16-bit|`i_ibm.c`, `i_vcgabw.c`|[gcc-ia16](https://github.com/tkchia/gcc-ia16)                                |`bcgabw.sh`                             |See [Doom8088](https://github.com/FrenkelS/Doom8088)|

Doom8088: Motorola 68000 Edition needs an IWAD file that has been preprocessed by [jWadUtil](https://github.com/FrenkelS/jWadUtil).

## Porting to other platforms:
To port Doom8088 to another platform, start by copying `i_dummy.c` and `i_dummyv.c` and see if it compiles.
Then implement the functions `I_ZoneBase()`, `R_DrawColumnSprite()`, `R_DrawColumnFlat()` and `I_FinishUpdate()`.
Look at the other platform implementations for inspiration on how to implement these functions.
Take one of the WAD files from another platform port.
That should be enough to run the timedemo benchmark.

Implement `V_DrawPatchScaled()` so the game can show the main menu.
Implement `I_StartTic()` and other keyboard code so the main menu can be navigated and the game can be played.

Implement the other functions in the copy of `i_dummyv.c`.

Optionally implement the sound code and improve the timer code.
