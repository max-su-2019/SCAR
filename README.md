# SCAR

SKSE/VR plugin that brings up the modern ACT game‘s combos attack AI into Skyrim.

- [SSE](https://www.nexusmods.com/skyrimspecialedition/mods/72014)
- [AE](https://www.nexusmods.com/skyrimspecialedition/mods/77285)
- [VR](https://www.nexusmods.com/skyrimspecialedition/mods/89492)

## Requirements

- [CMake](https://cmake.org/)
  - Add this to your `PATH`
- [PowerShell](https://github.com/PowerShell/PowerShell/releases/latest)
- [Vcpkg](https://github.com/microsoft/vcpkg)
  - Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
- [Visual Studio Community 2022](https://visualstudio.microsoft.com/)
  - Desktop development with C++
- [CommonLibNG](https://github.com/max-su-2019/CommonLibSSE/tree/NG)
  - You need to build from the NG branch
  - Add this as as an environment variable `CommonLibSSEPath` or use extern
- [DKUtil](https://github.com/gottyduke/DKUtil)
  - Add this as as an environment variable `DKUtilPath` or use extern

## User Requirements

- [Address Library for SKSE](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
  - Needed for SSE/AE
- [VR Address Library for SKSEVR](https://www.nexusmods.com/skyrimspecialedition/mods/58101)
  - Needed for VR

## Register Visual Studio as a Generator

- Open `x64 Native Tools Command Prompt`
- Run `cmake`
- Close the cmd window

## Building

```
git clone https://github.com/max-su-2019/SCAR.git
cd SCAR
# pull dependencies into /extern to override the path settings
git submodule init
# to update submodules to checked in build
git submodule update
```

### SSE/AE/VR

```
cmake --preset ALL
cmake --build build --config Release
```

## License

[MIT](LICENSE)

## Credits

- [Maxsu](https://www.nexusmods.com/skyrimspecialedition/users/47103898)([GitHub](https://github.com/max-su-2019)) for original mod
- Monitor144hz for made the original mod and allow me to remake it through SKSE plugins. Help me on behviour edit and made the moveset patcher tool.
- Dropkicker for wrote the assembly hook cdoes for AI attack distance check function. Also provided helping on porting the plugin to AE.
- Fenix for pointedd me the address of AI attack distance check function, and the AI navigating check function.
- Atom for provided me the source code related to havok behaviour engine.
- alexsylex for taught me how to use havok class reflection.
- KernalsEgg for provided me the codes of class TESActionData and helped me deal with LOS check feature. Helped me on porting the plugin to AE.
- Shrimperator for his debug overlay menu and API.
- Ersh for gave me tips on getting actor radius.
- Ryan for his commonLibSSE.
- Distar for made the modern combat framework, and gave me tips on behaviour edits.
- MikeNike for allowed me including his elder soul moveset as default animaitons pack.
- Black for help me testing SCAR's features and made movesets for SCAR.
- WillamWang﻿ for made the detection meter texture files, and provide me ideas on making the animated effect.
- Powerofthree & meh321: Don't remember what these two guys did exactly , but they must had provided some useful tips for the project.
- People from SkyrimRE discord channel.
- [Doodlum](https://www.nexusmods.com/skyrimspecialedition/users/28038035)([GitHub](https://github.com/doodlum)) for AE port.
- [alandtse](https://github.com/alandtse) VR port
