# sc4-no-kick-out

A DLL Plugin for SimCity 4 that stops the game from kicking out lower wealth occupants.   

This mod is a DLL version of [Cori's No Kickout Lower Wealth Mod (NKO)](https://community.simtropolis.com/files/file/34027-coris-no-kickout-lower-wealth-mod/) mod. 
Unlike Cori's mod the DLL does not override the Building Development Simulator exemplar, so it should be compatible with CAM or any other mod that alters that file.

The DLL sets the Building Development Simulator *Tract Developer Kick Out Lower Wealth* property to *false* when a city is loaded.

## Technical Details

The DLL loads the Building Development Simulator tuning exemplar before the game does, and modifies the
copy of the exemplar that the game temporarily caches in-memory.
The game will then use the modified in-memory copy when it initializes the game systems.   
Finally, the DLL verifies that the modification is still present after SC4 has run its initialization steps. 

The plugin can be downloaded from the Releases tab: https://github.com/0xC0000054/sc4-no-kick-out/releases

## System Requirements

* Windows 10 or later

The plugin may work on Windows 7 or later with the [Microsoft Visual C++ 2022 x86 Redistribute](https://aka.ms/vs/17/release/vc_redist.x86.exe) installed, but I do not have the ability to test that.

## Installation

1. Close SimCity 4.
2. Copy `SC4NoKickOut.dll` into the Plugins folder in the SimCity 4 installation directory.
3. Start SimCity 4.

## Troubleshooting

The plugin should write a `SC4NoKickOut.log` file in the same folder as the plugin.    
The log contains status information for the most recent run of the plugin.

# License

This project is licensed under the terms of the MIT License.    
See [LICENSE.txt](LICENSE.txt) for more information.

## 3rd party code

[gzcom-dll](https://github.com/nsgomez/gzcom-dll/tree/master) Located in the vendor folder, MIT License.    
[EABase](https://github.com/electronicarts/EABase) Located in the vendor folder, BSD 3-Clause License.    
[EASTL](https://github.com/electronicarts/EASTL) Located in the vendor folder, BSD 3-Clause License.    
[Windows Implementation Library](https://github.com/microsoft/wil) - MIT License    

# Source Code

## Prerequisites

* Visual Studio 2022

## Building the plugin

* Open the solution in the `src` folder
* Update the post build events to copy the build output to you SimCity 4 application plugins folder.
* Build the solution

## Debugging the plugin

Visual Studio can be configured to launch SimCity 4 on the Debugging page of the project properties.
I configured the debugger to launch the game in a window with the following command line:    
`-intro:off -CPUcount:1 -w -CustomResolution:enabled -r1920x1080x32`

You may need to adjust the resolution for your screen.
