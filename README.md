## Doom8088: Atari ST Edition
![Doom8088: Atari ST Edition](readme_imgs/doomcgabw.png?raw=true)

Doom was originally designed in 1993 for 32-bit DOS computers with 4 MB of RAM.
It's mostly written in C code with very little assembly code.
It has been ported to all kinds of systems.
Usually these systems are 32-bit or more and have a flat memory model.

Doom8088: Atari ST Edition is a port for Atari ST computers.
It's based on [Doom8088](https://github.com/FrenkelS/Doom8088), a port of Doom for 16-bit DOS computers.
Download Doom8088: Atari ST Edition [here](https://github.com/FrenkelS/Doom8088ST/releases).

**What's special?:**
 - Supports only Doom 1 Episode 1
 - Rotating overlaid automap
 - Only demo3 is supported
 - 2 color mode
 - No sound effects
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

## Controls:
|Action                           |Keys         |
|---------------------------------|-------------|
|Fire                             |Ctrl         |
|Use                              |Enter & Space|
|Sprint                           |Shift        |
|Walk                             |Arrow keys   |
|Strafe                           |Alt          |
|Strafe left and right            |< & >        |
|Automap                          |Tab          |
|Automap zoom in and out          |+ & -        |
|Automap follow mode              |F            |
|Weapon up and down               |[ & ]        |
|Menu                             |Esc          |
|Quit to OS                       |F10          |

## Cheats:
|Code      |Effects                  |
|----------|-------------------------|
|IDCHOPPERS|Chainsaw                 |
|IDDQD     |God mode                 |
|IDKFA     |Weapons & Keys           |
|IDFA      |Weapons                  |
|IDSPISPOPD|No Clipping              |
|IDBEHOLDV |Invincibility            |
|IDBEHOLDS |Berserk                  |
|IDBEHOLDI |Invisibility             |
|IDBEHOLDR |Radiation shielding suit |
|IDBEHOLDA |Auto-map                 |
|IDBEHOLDL |Lite-Amp Goggles         |
|IDCLEV    |Exit Level               |
|IDEND     |Show end text            |
|IDROCKET  |Enemy Rockets (GoldenEye)|
|IDRATE    |Toggle FPS counter       |

## Command line arguments:
|Command line argument|Effect       |
|---------------------|-------------|
|`/timedemo demo3`    |Run benchmark|

## Building:
|Platform     |Platform specific code |Compiler                                                                    |Build script|Additional information                              |
|-------------|-----------------------|----------------------------------------------------------------------------|------------|----------------------------------------------------|
|Atari ST     |`i_ast.c`, `i_vast.c`  |[m68k-atari-mintelf](http://vincent.riviere.free.fr/soft/m68k-atari-mintelf)|`batari.sh` |640x200 4 colors resolution, only 2 colors are used |
|IBM PC 16-bit|`i_ibm.c`, `i_vcgabw.c`|[gcc-ia16](https://github.com/tkchia/gcc-ia16)                              |`bcgabw.sh` |See [Doom8088](https://github.com/FrenkelS/Doom8088)|

Doom8088: Atari ST Edition needs an IWAD file that has been preprocessed by [jWadUtil](https://github.com/FrenkelS/jWadUtil).
