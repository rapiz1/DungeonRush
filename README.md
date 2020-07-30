# DungeonRush
![](screenshot.gif)
>A game inspired by Snake, in pure C with SDL2.
>My piece of work from C assignment. Hope you like it :)

*I work on the small project on my own so the code lacks of comments.*
## How to Play

### Singleplayer

Use WASD to move.

Collect heros to enlarge your army while defending yourself from the monsters. Each level has a target length of the hero queue. Once it's reached, you will be sent to the next level and start over. There are lots of stuff that will be adjusted according to the level you're on, including factors of HP and damage, duration of buffs and debuffs, the number and strength of monsters and so on.

### Multiplayers
Use WASD and the arrow keys to move.

This mode is competitive. Defend yourself from the monsters and your friend!

### Weapons

There are powerful weapons randomly dropped by the monsters. Different kinds of heros can be equipped with different kind of weapons.

*My favorite is the ThunderStaff. A cool staff that makes your wizard summon thunder striking all enmeies around.*

### Buff/Debuff

There's a possibility that the attack from one with weapon triggers certain buff on himself or debuff on the enemey.

- IceSword can frozen enemeys.
- HolySword can give you a shield that absorbs damage and makes you immune to debuff.
- GreatBow can increase the damage of all your heros' attack.
- And so on.

For sure, some kinds of monsters have weapons that can put a debuff on you! *(Like the troublesome muddy monsters can slow down your movement.)*


## Download

[Windows](https://github.com/Rapiz1/DungeonRush/releases/download/v1.0-alpha/DungeonRush-v1.0-alpha-win32.zip)

[Linux](https://github.com/Rapiz1/DungeonRush/releases/download/v1.0-alpha/DungeonRush-v1.0-alpha-linux.zip)

AppImage for Linux is also available in release page.

*The game supports macOS but I don't have an Mac to complie for it.*

The executable is called `dungeon_rush`

## Dependencies
The project requires no more than common SDL2 libraries.
`SDL2, SDL2-ttf, SDL2-image, SDL2-mixer`
### For Arch
```
# pacman -S sdl2 sdl2_image sdl2_mixer sdl2_ttf
```
### For Debian

```
# apt install libsdl2-*
```

### For openSUSE

```
# zypper in libSDL2-devel libSDL2_mixer-devel libSDL2_ttf-devel libSDL2_image-devel
```

### For MacOS

```
#  brew install sdl2 sdl2_image sdl2_ttf sdl2_mixer
```

### For other OS

The names of packages should be similiar. You can look for SDL tutorial to set up the environment.
## Compilation
**You should make sure all dependencies are installed before compiling**
```
$ make
```
## Known issues
[Game speed too fast with some NVIDIA graphic cards on Linux](https://github.com/Rapiz1/DungeonRush/issues/4)
## License and Credits
DungeonRush has mixed meida with 
various licenses. Unfortunately I failed to track them all. In other word, there are many stuff excluding code that comes with unknown license. You should not reuse any of audio, bitmaps, font in this project. If you insist, use at your own risk.
### Code
GPL
### Bitmap
|Name|License|
|----|-------|
|DungeonTilesetII_v1.3 By 0x72|CC 0|
|Other stuff By rapiz|CC BY-NC-SA 4.0|
### Music
|Name|License|
|----|-------|
|Digital_Dream_Azureflux_Remix By Starbox|CC BY-NC-SA 4.0|
|BOMB By Azureflux|CC BY-NC-SA 4.0|
|Unkown BGM|Unkown|
|The Essential Retro Video Game Sound Effects Collection By Juhani Junkala |CC BY 3.0|
