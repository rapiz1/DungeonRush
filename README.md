# DungeonRush
![](screenshot.gif)
>A game inspired by Snake, in pure C with SDL2.
>My piece of work from C assignment. Hope you like it :)

*I work on the small project on my own so the code lacks of comments.*

## Download

[Windows](https://github.com/Rapiz1/DungeonRush/releases/download/v1.0-alpha/DungeonRush-v1.0-alpha-win32.zip)

[Linux](https://github.com/Rapiz1/DungeonRush/releases/download/v1.0-alpha/DungeonRush-v1.0-alpha-linux.zip)

The executable is called `snake`

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
### For MacOs

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
