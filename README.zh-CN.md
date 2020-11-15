# DungeonRush
![](screenshot.gif)
[English](README.md) | 简体中文
> 受贪吃蛇启发的游戏，纯C + SDL图形库。
> 它是我的C作业。希望你喜欢 :)

*这个项目由我自己编写，所以注释比较少*
## 发行说明

### v1.0-beta
- 修复Bug
- 添加 多人 / 局域网（Multiplayer / LAN）模式

**通过互联网和你的好朋友玩耍吧！**
*你们要在同一个网络里，用IP直接连接*

### v1.0-alpha

初次发行

## 玩法

### 单人模式

使用 WASD 来移动。

收集英雄来扩大你的军队，并避免遭受怪物的攻击。每个关卡都有一个军队的目标长度。如果到达目标长度，你则会在下一级别重新开始。有很多东西会根据你的等级进行调整，比如生命值、伤害系数、怪物的数量和力量等。

### 多人模式
使用 WASD 和箭头（↑↓←→）来移动。

这种模式很有竞争力。防止遭受怪物和朋友的攻击！

### 武器

怪物们会随机扔下威力强大的武器。不同种类的英雄可以装备不同种类的武器。

*我最爱雷霆塔夫。一根很酷的~~搅屎棍~~棍子，可以让你的巫师召唤雷霆攻击周围的恩美人。*

### Buff/Debuff

Buff/Debuff会让一个拥有武器的人攻击时触发他自己的某些增益或者对敌人的减益。

- 冰剑可以冰冻敌人，敌人将无法移动。
- 圣剑可以给你一个盾牌，吸收伤害并使你对Debuff免疫。
- 大弓可以增加你所有的英雄的攻击伤害。
- 还有很多。

当然，有些怪物有武器，可以给你施加Debuff！ *（就比如烦人的泥泞怪可以减缓你的移动速度）*


## 下载

[Windows(64位)](https://github.com/Rapiz1/DungeonRush/releases/download/v1.0-beta/DungeonRush-v1.0-beta-Windows-x86_64.zip)

[Windows(32位)](https://github.com/Rapiz1/DungeonRush/releases/download/v1.0-beta/DungeonRush-v1.0-beta-Windows-i686.zip)

[Linux](https://github.com/Rapiz1/DungeonRush/releases/download/v1.0-beta/DungeonRush-v1.0-beta-linux.zip)

AppImage for Linux 还可以在 Release 页面中找到。

*这个游戏支持macOS，但我没有Mac来兼容它。*

可执行文件叫`dungeon_rush`

## 依赖关系
该项目仅依赖 SDL 库。
`SDL2, SDL2-image, SDL2-mixer, SDL2-net, SDL2-ttf`
### Arch
```
# pacman -S sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf --needed
```
### Debian

```
# apt install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-net-dev libsdl2-ttf-dev
```

### openSUSE

```
# zypper in libSDL2-devel libSDL2_image-devel libSDL2_mixer-devel libSDL2_net-devel libSDL2_ttf-devel
```

### MacOS

```
#  brew install sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf
```

### 其他系统

包名大概差不多。您可以查找 SDL 教程来设置环境。
## Compilation
**您应该确保在编译之前安装了所有依赖项**
```
$ cmake -B build && cmake --build build
```
## 已知问题
[在一些装载NVIDIA卡的Linux系统上，该游戏速度太快](https://github.com/Rapiz1/DungeonRush/issues/4)
## 版权和许可
DungeonRush 已经把各种许可证的媒体混杂在一起。不幸的是我没找到他们。换句话说，有许多不包括代码在内的未知许可证。你不该在这个项目中重复使用任何音频、位图、字体。如果你坚持使用，风险自负。
### 代码
GPL
### 位图
|名称|版权协议|
|----|--------|
|来自 0x72 的 DungeonTilesetII_v1.3|CC 0|
|其他来自 rapiz 的位图|CC BY-NC-SA 4.0|
### 音频
|名称|版权协议|
|----|--------|
|来自 Starbox 的 Digital_Dream_Azureflux_Remix|CC BY-NC-SA 4.0|
|来自 Azureflux 的 BOMB|CC BY-NC-SA 4.0|
|未知音频|未知|
|来自 Juhani Junkala 的 The Essential Retro Video Game Sound Effects Collection|CC BY 3.0|
### 字体
|名称|版权协议|
|----|--------|
|Unifont|GPLv2|