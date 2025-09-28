Doom8088: AT&T UNIX PC Edition
Doom for AT&T UNIC PC computers
https://github.com/FrenkelS/Doom8088ST

Doom was originally designed in 1993 for 32-bit DOS computers with 4 MB of RAM.
It's mostly written in C code with very little assembly code.
It has been ported to all kinds of systems.
Usually these systems are 32-bit or more and have a flat memory model.

Doom8088: AT&T UNIX PC Edition is a port for AT&T UNIX PC computers with at
least 512 kB of RAM (1 MB recommended) and a Motorola 68010 CPU.
It's based on Doom8088, a port of Doom for 16-bit DOS computers.

What's special?
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

Known bugs:
 - When there's not enough memory for a texture, one color is drawn
 - When there's not enough memory for the intermission screen,
   the last few frames of gameplay are shown instead

Supported video modes:
|Video mode|Executable|
|----------|----------|
|720x348 2c|doom8088  |

Controls:
|Action                 |Keys                      |
|-----------------------|--------------------------|
|Fire                   |/                         |
|Use                    |Enter & Space             |
|Walk                   |Arrow keys & 8 & 2 & 4 & 6|
|Strafe left and right  |< & >                     |
|Automap                |Tab                       |
|Automap zoom in and out|+ & -                     |
|Automap follow mode    |F                         |
|Weapon up and down     |[ & ]                     |
|Menu                   |Esc                       |
|Quit to OS             |Shift + Q                 |

Cheats:
|Code      |Effects                 |Notes                           |
|----------|------------------------|--------------------------------|
|IDCHOPPERS|Chainsaw                |                                |
|IDDQD     |God mode                |                                |
|IDKFA     |Weapons & Keys          |                                |
|IDFA      |Weapons                 |                                |
|IDSPISPOPD|No Clipping             |                                |
|IDBEHOLDV |Invincibility           |                                |
|IDBEHOLDS |Berserk                 |                                |
|IDBEHOLDI |Invisibility            |                                |
|IDBEHOLDR |Radiation shielding suit|                                |
|IDBEHOLDA |Auto-map                |                                |
|IDBEHOLDL |Lite-Amp Goggles        |                                |
|IDCLEV    |Exit Level              |                                |
|IDEND     |Show end text           |                                |
|IDROCKET  |Enemy Rockets           |(GoldenEye)                     |
|IDRATE    |Toggle FPS counter      |Divide by 10 to get the real FPS|

Command line arguments:
|Command line   |Effect       |
|argument       |             |
|---------------|-------------|
|-timedemo demo3|Run benchmark|
