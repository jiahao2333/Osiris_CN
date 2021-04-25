# Osiris_CHS
[![C++](https://img.shields.io/badge/language-C%2B%2B-%23f34b7d.svg?style=plastic)](https://en.wikipedia.org/wiki/C%2B%2B) 
[![CS:GO](https://img.shields.io/badge/game-CS%3AGO-yellow.svg?style=plastic)](https://store.steampowered.com/app/730/CounterStrike_Global_Offensive/) 
[![Windows](https://img.shields.io/badge/platform-Windows-0078d7.svg?style=plastic)](https://en.wikipedia.org/wiki/Microsoft_Windows) 
[![x86](https://img.shields.io/badge/arch-x86-red.svg?style=plastic)](https://en.wikipedia.org/wiki/X86) 
[![License](https://img.shields.io/github/license/danielkrupinski/Osiris.svg?style=plastic)](LICENSE)
[![Issues](https://img.shields.io/github/issues/danielkrupinski/Osiris.svg?style=plastic)](https://github.com/danielkrupinski/Osiris/issues)
[![PayPal](https://img.shields.io/badge/donate-PayPal-104098.svg?style=plastic&logo=PayPal)](https://paypal.me/DanielK19)
<br>![Windows](https://github.com/danielkrupinski/Osiris/workflows/Windows/badge.svg?branch=master&event=push)
![Linux](https://github.com/danielkrupinski/Osiris/workflows/Linux/badge.svg?branch=master&event=push)

免费开源的跨平台作弊软件，适用于 **反恐精英: 全球攻势** 游戏。设计为内部作弊 - [动态链接库](https://zh.wikipedia.org/wiki/%E5%8A%A8%E6%80%81%E9%93%BE%E6%8E%A5%E5%BA%93) (DLL) 可加载到游戏进程中。与游戏的Steam版本兼容。适用于 Windows 和 Linux 系统。 

更多使用技巧与翻译说明可点击 [此处](https://www.hxiaoh.cn/osiris_chs/) 、。

## 特征
*   **自瞄** - 辅助瞄准
*   **自动开火** - 准星对准敌人时自动开火
*   **回溯** - 滥用滞后补偿以使玩家回到过去
*   **发光** - 对实体渲染发光效果
*   **实体** - 彩色玩家模型以提高可见度
*   **ESP** - 显示有关玩家，掉落的武器和投掷物的信息
*   **视觉** - 其他视觉选择
*   **皮肤修改** - 更换武器的皮肤，匕首和贴纸
*   **声音** - 修改某些声音效果的音量
*   **样式** - 选择菜单窗口的布局和颜色
*   **杂项** - 杂项功能
*   **配置** - 基于 JSON 的配置系统

<details>

*   **Aimbot** - aim assistance
    *   **Enabled** - on / off master switch
    *   **On key \[ key \]** - aimbot works only when chosen key is being held
    *   **Aimlock** - brings your aim to the target (affected by Smooth).
    *   **Silent** - aimbot is not visible on your screen (client-sided only)
    *   **Friendly fire** - treat allies as enemies
    *   **Visible only** - aim only on visible players
    *   **Scoped only** - aimbot works only when using scope (applies only to sniper rifles)
    *   **Ignore flash** - ignore flashbang i.e. aim when local player is flashed
    *   **Ignore smoke** - ignore smoke i.e. aim when target is in smoke
    *   **Auto shot** - shoot automatically when target found
    *   **Auto scope** - automatically scopes sniper rifle before shooting
    *   **Bone** - bone which aimbot aims at
    *   **Fov** - field-of-view which aimbot operates \[*0*-*255*\]
    *   **Smooth** - smooth aimbot movement in order to seem more human-like
    *   **Max aim inaccuracy** - maximum weapon inaccuracy allowing aimbot to run, lowering this value will e.g. disable aimbot while jumping or running

*   **Triggerbot** - automatically fires when crosshair is on enemy
    *   **Enabled** - on / off master switch
    *   **On key \[ key \]** - triggerbot works only when chosen key is being held
    *   **Friendly fire** - treat allies as enemies
    *   **Scoped only** - triggerbot works only when using scope (applies only to sniper rifles)
    *   **Ignore flash** - ignore flashbang i.e. shoot when local player is flashed
    *   **Ignore smoke** - ignore smoke i.e. shoot when target is in smoke
    *   **Hitgroup** - body parts on which triggerbot works
    *   **Shot delay** - delay time in ms (milliseconds)
    *   **Min damage** - minimal damage to fire.

*   **Backtrack** - abuse lag compensation in order to move players back in time
    *   **Enabled** - on / off master switch
    *   **Ignore smoke** - ignore smoke i.e. backtrack when target is in smoke
    *   **Time limit** - limit the backtracking window \[*1*-*200*ms\]

*   **Glow** - render glow effect on entities

    *Allies, Enemies, Planting (player planting bomb), Defusing (player defusing bomb), Local player, Weapons (dropped weapons), C4, Planted C4, Chickens, Defuse kits, Projectiles, Hostages, Ragdolls* **/** *All, Visible, Occluded*

    *   **Enabled** - on / off master switch
    *   **Health based** - color is based on player's hp
    *   **Color** - glow color in rgba format
    *   **Style** - glow style { `Default`, `Rim3d`, `Edge`, `Edge Pulse` }

*   **Chams** - color player models to improve visibility

    *Allies, Enemies, Planting (player planting bomb), Defusing (player defusing bomb), Local player, Weapons (dropped weapons), Hands (view model hands), Backtrack (requires backtrack to be enabled), Sleeves (view model)* **/** *All, Visible, Occluded*
    *   **Enabled** - on / off master switch
    *   **Health based** - color is based on player's hp
    *   **Blinking** - change transparency frequently
    *   **Material** - material applied to model { `Normal`, `Flat`, `Animated`, `Platinum`, `Glass`, `Chrome`, `Crystal`, `Silver`, `Gold`, `Plastic`, `Glow` }
    *   **Wireframe** - render triangle mesh instead of solid material
    *   **Cover** - draw chams material on top of the original material instead of overriding it
    *   **Ignore-Z** - draw material through walls

*   **ESP** - show additional information about players and game world
    1.  *Allies, Enemies*
        *   *All, Visible, Occluded*

    2.  *Weapons*

    3.  *Projectiles*
        *   *Flashbang, HE Grenade, Breach Charge, Bump Mine, Decoy Grenade, Molotov, TA Grenade, Smoke Grenade, Snowball*

    4.  *Danger Zone*
        *   *Sentries, Drones, Cash, Cash Dufflebag, Pistol Case, Light Case, Heavy Case, Explosive Case, Tools Case, Full Armor, Armor, Helmet, Parachute, Briefcase, Tablet Upgrade, ExoJump, Ammobox, Radar Jammer*

    *   **Enabled** - on / off master switch
    *   **Font** - esp text font
    *   **Snaplines** - draw snapline to player
    *   **Eye traces** - draw player eye traces (shows where player looks)
    *   **Box** - draw 2D box over player model
    *   **Name** - draw player name
    *   **Health** - draw player health
    *   **Health bar** - draw rectangle indicating player health
    *   **Armor** - draw player armor
    *   **Armor bar** - draw rectangle indicating player armor
    *   **Money** - draw player money
    *   **Head dot** - draw dot on player's head
    *   **Active Weapon** - draw player equipped weapon

*   **Visuals** - miscellaneous visual options
    *   **Disable post-processing** - disable post-processing effects in order to increase FPS
    *   **Inverse ragdoll gravity** - inverse gravitational acceleration on falling player ragdoll corpse (during death sequence)
    *   **No fog** - remove fog from map for better visibility
    *   **No 3d sky** - remove 3d skybox from map - increases FPS
    *   **No visual recoil** - remove visual recoil punch effect
    *   **No hands** - remove arms / hands model from first-person view
    *   **No sleeves** - remove sleeves model from first-person view
    *   **No weapons** - remove weapons model from first-person view
    *   **No smoke** - remove smoke grenade effect
    *   **No blur** - remove blur
    *   **No scope overlay** - remove black overlay while scoping
    *   **No grass** - remove grass from map in Danger Zone mode (`dz_blacksite` and `dz_sirocco` maps)
    *   **No shadows** - disable dynamic shadows
    *   **Wireframe smoke** - render smoke skeleton instead of particle effect
    *   **Zoom \[ key \]** - enable zoom on unzoomable weapons
    *   **Thirdperson** - thirdperson view
    *   **Thirdperson distance** - camera distance in thirdperson view
    *   **View model FOV** - change view model FOV \[*-60*-*0*-*60*\] (0 - actual view model, negative values - decreased view model, positive values - increased view model)
    *   **FOV** - change view FOV \[*-60*-*0*-*60*\] (0 - actual view fov, negative values - decreased, positive values - increased)
    *   **Far Z** - far clipping range, useful after disabling fog on large maps (e.g `dz_sirocco`) to render distant buildings
    *   **Flash reduction** - reduces flashbang grenade effect \[*0*-*100*%\] (0 - full flash, 100 - no flash)
    *   **Brightness** - control game brightness \[*0.0*-*1.0*\]
    *   **Skybox** - change sky(box)
    *   **World color** - set world material ambient light color
    *   **Deagle spinner** - play "spinning" inspect animation when holding Deagle
    *   **Screen effect** - screenspace effect - *Drone cam, Drone cam with noise, Underwater, Healthboost, Dangerzone*
    *   **Hit effect** - show screen effect on enemy hit
    *   **Hit marker** - show a cross detail on enemy hit

*   **Skin changer** - change knives, gloves, weapon skins and stickers

*   **Sound** - modify volume of certain sound effects
    *   **Chicken volume** - volume of chicken sounds

    *Local player, Allies, Enemies*
    *   **Master volume** - overall volume of sounds emitted by player
    *   **Headshot volume** - volume of headshot sound (when player gets headshoted)
    *   **Weapon volume** - volume of player weapon shots
    *   **Footstep volume** - volume of player footsteps

*   **Misc** - miscellaneous features
    *   **Menu key \[ key \]** - menu toggle key

    *   **Menu style** - menu style toggle (*Classic* **/** *One window*)

    *   **Menu colors** - menu color theme (*Dark **/** Light **/** Classic*)

    *   **Anti AFK kick** - avoid auto-kick by server for inactivity

    *   **Auto strafe** - automatically strafe in air following mouse movement

    *   **Bunny hop** - automatically simulate space bar press / release while jump button is being held; increases movement speed

    *   **Clan tag** - set custom clan tag

    *   **Animated clan tag** - animate clan tag

    *   **Fast duck** - remove crouch delay

    *   **Sniper crosshair** - draw crosshair while holding sniper rifle

    *   **Recoil crosshair** - crosshair follows recoil pattern

    *   **Auto pistol** - fire pistols like automatic rifles

    *   **Auto reload** - automatically reload if weapon has empty clip

    *   **Auto accept** - automatically accept competitive match

    *   **Radar hack** - show enemies positions on radar

    *   **Reveal ranks** - show player ranks in scoreboard in competitive modes

    *   **Reveal money** - show enemies' money in scoreboard

    *   **Spectator list** - show nicknames of players spectating you

    *   **Watermark** - show cheat name in the upper-left screen corner and fps & ping in the upper-right corner

    *   **Offscreen Enemies** - draw circles on the screen indicating that there are enemies behind us

    *   **Fix animation LOD** - fix aimbot inaccuracy for players behind local player

    *   **Fix bone matrix** - correct client bone matrix to be closer to server one

    *   **Disable model occlusion** - draw player models even if they are behind thick walls

    *   **Kill message** - print message to chat after killing an enemy

    *   **Name stealer** - mimic other players names

    *   **Custom clantag** - set a custom clantag

    *   **Fast plant** - plants bomb on bombsite border, when holding <kbd>LMB</kbd> or <kbd>E</kbd> key

    *   **Fast Stop** - stops the player faster than normal

    *   **Quick reload** - perform quick weapon switch during reload for faster reload

    *   **Prepare revolver \[ key \]** - keep revolver cocked, optionally on key

    *   **Fix tablet signal** - allow use tablet on underground (dangerzone)

    *   **Hit Sound** - sound emitted when hurting enemy

    *   **Chocked packets** - length of sequence of chocked ticks

    *   **Max angle delta** - maximum viewangles change per tick

    *   **Fake Prime** - set a fake prime (visible in lobby)

    *   **Purchase List** - show the purchased equipment by enemies.

    *   **Reportbot** - automatically report players on server for cheating or other abusive actions
        *   **Enabled** - on / off master switch
        *   **Target** - report target *Enemies/Allies/All*
        *   **Delay** - delay between reports, in seconds
        *   **Aimbot** - report for aim assistance
        *   **Wallhack** - report for visual assistance
        *   **Other** - report for other assistance
        *   **Griefing** - report for griefing
        *   **Abusive Communications** - report for abusive communications

    *   **Unhook** - unload cheat

*   **Config** - JSON-based configuration system
    *   **Create config** - create new configuration file
    *   **Reset config** - restore default configuration settings (does not touch saved configuration)
    *   **Load selected** - load selected configuration file
    *   **Save selected** - save selected configuration file
    *   **Delete selected** - delete selected configuration file
    *   **Reload configs** - reload configs list
</details>

## 入门

### 先决条件

为了编译 Osiris_CHS，需要 Microsoft Visual Studio 2019 (最好是最新版本)，平台工具集v142 和 Windows SDK10.0.x.x。 如果您没有VS，则可以在 [此处](https://visualstudio.microsoft.com/) 下载 VS (在 Visual Studio 安装过程中会安装 Windows SDK)。

### 下载

有两种下载源代码的选项：

#### 没有 [git](https://git-scm.com)

如果您想使用纯资源并且不打算回购，请选择此选项。 下载大小 ~600 kB。

以这种方式下载源代码 [点击此处](https://github.com/H-xiaoH/Osiris_CHS/archive/master.zip)。

#### 使用 [git](https://git-scm.com)

如果您要为回购做贡献或要使用版本控制系统，请选择此选项。下载大小约 ~4MB。如果未安装Git，则需要进一步进行操作，请在 [此处](https://git-scm.com) 下载。

打开git命令提示符并输入以下命令：

    git clone --depth=1 https://github.com/H-xiaoH/Osiris_CHS.git

`Osiris_CHS` 文件夹应该已经成功创建，其中包含所有源文件。 

### 从源代码编译

配备源代码副本后，下一步是在 Microsoft Visual Studio 2019 中打开 **Osiris.sln**。

然后将构建配置更改为 `Release | x86` ，然后只需按 **生成解决方案** 即可。

如果一切顺利，您应该得到 `Osiris.dll` 二进制文件。 

### 加载/注入游戏

打开您喜欢的 [DLL注入](https://zh.wikipedia.org/wiki/DLL%E6%B3%A8%E5%85%A5) 然后将 `Osiris.dll` 注入到 `csgo.exe` 进程中。

注入后，可以按下 `INSERT` 键打开菜单。 

### 进一步优化
如果您的CPU支持 AVX / AVX2 / AVX-512 指令集，则可以在项目设置中启用它。 这将产生更多性能更好的代码，并为您的 CPU 优化。 当前在项目设置中选择了 AVX2 指令。

## 常问问题

### 如何打开菜单？
在 CS:GO 窗口中，按 <kbd>INSERT</kbd> 键。

### 我的配置文件保存在哪里？
配置文件保存在 Documents 文件夹的 Osiris 文件夹中 (％USERPROFILE％\Documents\Osiris) 。该配置采用人类可读的格式，并且可以进行编辑（例如，使用记事本。)有时，更新后需要删除并重新创建配置文件。

### Osiris_CHS 使用什么挂钩方法？
当前实现的挂钩方法是：
*   MinHook - trampoline hook
*   VmtHook - hook a function directly in a vtable
*   VmtSwap - create a copy of a vtable and swap the pointer on the class instance

挂钩实现文件位于 [Hooks](https://github.com/H-xiaoH/Osiris_CHS/tree/master/Osiris/Hooks) 目录.

## 致谢

*   [ocornut](https://github.com/ocornut) 和 [contributors](https://github.com/ocornut/imgui/graphs/contributors) 创建和维护出色的 GUI 库 - [Dear imgui](https://github.com/ocornut/imgui).
*   [Zer0Mem0ry](https://github.com/Zer0Mem0ry) - 有关逆向工程和游戏黑客的出色教程

## 许可

> 版权所有 (c) 2018-2021 Daniel Krupiński

该项目已获得 [MIT 许可](https://opensource.org/licenses/mit-license.php) - 请参阅 [LICENSE](https://github.com/danielkrupinski/Osiris/blob/master/LICENSE) 文件以了解详细信息。 

## 也可以看看
*   [Anubis](https://github.com/danielkrupinski/Anubis) - 具有与 Osiris 兼容的配置的 CS:GO 的免费和开源作弊
*   [GOESP](https://github.com/danielkrupinski/GOESP) - 免费和开源的跨平台ESP作弊，适用于 《反恐精英:全球攻势》，以现代C++编写
