## Doom8088: Atari ST Edition & AT&T UNIX PC Edition
![Doom8088: Atari ST Edition](readme_imgs/doomstch.png?raw=true)

Doom was originally designed in 1993 for 32-bit DOS computers with 4 MB of RAM.
It's mostly written in C code with very little assembly code.
It has been ported to all kinds of systems.
Usually these systems are 32-bit or more and have a flat memory model.

Doom8088: Atari ST Edition     is a port for Atari ST     computers with at least 512 kB of RAM (1 MB recommended) and a Motorola 68000 CPU.
Doom8088: AT&T UNIX PC Edition is a port for AT&T UNIX PC computers with at least 512 kB of RAM (1 MB recommended) and a Motorola 68010 CPU.
It's based on [Doom8088](https://github.com/FrenkelS/Doom8088), a port of Doom for 16-bit DOS computers.
Download Doom8088: Atari ST Edition & AT&T UNIX PC Edition[here](https://github.com/FrenkelS/Doom8088ST/releases).

**What's special?:**
 - Supports only Doom 1 Episode 1
 - Rotating overlaid automap
 - Only demo3 is supported
 - 16 color mode (Atari ST only)
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

### Atari ST 640x200  4 color mode, effective resolution  60x128 2 colors
![Doom8088 in 2 colors](readme_imgs/doomstbw.png?raw=true)

### AT&T UNIX PC 720x348  2 color mode, effective resolution  60x128 2 colors
![Doom8088 in 2 colors](readme_imgs/doom3b1.png?raw=true)

## Controls:
|Action                 |Atari ST     |AT&T UNIX PC              |
|-----------------------|-------------|--------------------------|
|Fire                   |Ctrl         |/                         |
|Use                    |Enter & Space|Enter & Space             |
|Sprint                 |Shift        |not available             |
|Walk                   |Arrow keys   |Arrow keys & 8 & 2 & 4 & 6|
|Strafe                 |Alt          |not available             |
|Strafe left and right  |< & >        |< & >                     |
|Automap                |Tab          |Tab                       |
|Automap zoom in and out|+ & -        |+ & -                     |
|Automap follow mode    |F            |F                         |
|Weapon up and down     |[ & ]        |[ & ]                     |
|Menu                   |Esc          |Esc                       |
|Quit to OS             |F10          |Shift + Q                 |

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
|Atari ST     |`i_ast.c`, `i_vast16.c`|[m68k-atari-mintelf binutils, GCC, MiNTLib](https://tho-otto.de/crossmint.php)|`bast16h.sh`, `bast16m.sh`, `bast16l.sh`|320x200 16 color mode, high/medium/low detail mode  |
|Atari ST     |`i_ast.c`, `i_vast2.c` |[m68k-atari-mintelf binutils, GCC, MiNTLib](https://tho-otto.de/crossmint.php)|`bast2.sh`, `bast520.sh`                |640x200  4 color mode, 2 colors are used</br>`DOOM520.TOS` is for computers with 512 kB of RAM. It goes from the first level immediately to the last level.|
|AT&T UNIX PC |`i_3b1.c`, `i_v3b1.c`  |[unixpc-gcc](https://github.com/mikehaertel/unixpc-gcc)                       |`b3b1.sh`                               |The build script generates `speed.o` and `space.o`. These files need to be linked to create `doom8088`.|
|IBM PC 16-bit|`i_ibm.c`, `i_vcgabw.c`|[gcc-ia16](https://github.com/tkchia/gcc-ia16)                                |`bcgabw.sh`                             |See [Doom8088](https://github.com/FrenkelS/Doom8088)|

Doom8088: Atari ST Edition & AT&T UNIX PC Edition need an IWAD file that has been preprocessed by [jWadUtil](https://github.com/FrenkelS/jWadUtil).
