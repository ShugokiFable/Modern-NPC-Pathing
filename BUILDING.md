# Building NPC Pathing NG

## Requirements

- Windows x64
- Visual Studio 2022 or newer with Desktop C++ tools
- CMake 3.21+
- Python 3.10+
- vcpkg (`VCPKG_ROOT` or `VCPKG_INSTALLATION_ROOT` set)
- CommonLibSSE-NG commit `b93280e832f263dbef44e44cbe2936622a02f91a`

## One-command release build

From PowerShell in the repository root:

```powershell
./build_release.ps1 -Generator "Visual Studio 17 2022"
```

The script pins CommonLibSSE-NG, builds it and the plugin with the static MSVC runtime, runs validation, and creates the Nexus archive.

The clean GitHub/source tree intentionally does not contain a DLL. The script creates it only for the release package.

The included presets can also configure and build the plugin after CommonLibSSE-NG has been installed:

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-msvc-release
```

## Manual clean build

```powershell
git clone https://github.com/CharmedBaryon/CommonLibSSE-NG.git extern/CommonLibSSE
git -C extern/CommonLibSSE checkout b93280e832f263dbef44e44cbe2936622a02f91a

cmake -S extern/CommonLibSSE -B extern/CommonLibSSE/build `
  -G "Visual Studio 17 2022" -A x64 `
  -DBUILD_TESTS=OFF -DENABLE_SKYRIM_VR=OFF `
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded `
  -DCMAKE_INSTALL_PREFIX="$PWD/extern/CommonLibSSE/install" `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows-static `
  -DVCPKG_HOST_TRIPLET=x64-windows-static
cmake --build extern/CommonLibSSE/build --config Release --target install

cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
python generate_esp.py
python -m unittest discover -s tests -v
python tools/package_release.py
```

The compiled DLL is written to `package/Data/SKSE/Plugins/NPCPathingNG.dll`. The final Nexus archive is written to `dist/`.

Do not distribute `build/`, `extern/`, vcpkg packages, object files, PDBs, nested `.git` directories, or old ZIP files. They are development material and can inflate a sub-megabyte mod into hundreds of megabytes.
