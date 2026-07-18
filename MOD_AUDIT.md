# NPC Pathing NG 2.4.0 audit

Audit date: 2026-07-18

## Scope

Reviewed the 2.4.0 Nexus archive, the supplied development archive, the C++ runtime plugin, generated ESP/SEQ data, MCM Helper configuration, fallback INI, CMake/vcpkg metadata, and the public GitHub tree.

## Confirmed defects fixed in 2.4.1 source

1. **Unsafe disable during parkour**: disabling the master toggle while an NPC job was active could leave the character controller in `kNoSim` and leave SkyParkour variables dirty. Active jobs are now cancelled and cleaned up on the enabled-to-disabled transition.
2. **Follower integration toggle coupling**: EVG follower replay incorrectly depended on the SkyParkour toggle, and stored EVG events could still replay after EVG was disabled. Each integration now gates only its own events, and disabled event types are removed so they cannot revive after re-enabling a toggle.
3. **Leaked lower-body graph state**: a weapon-drawn step could leave `SkyParkourLowerBody` enabled for a later full-body climb. The value is now written for every trigger and cleared on rejection and completion.
4. **False forced interruptions**: normal graph completion was sent through the forced-interrupt path. Interrupts are now reserved for timeouts, explicit cancellation, or abnormal displacement.
5. **Teleport escalation and incomplete collision validation**: a successful bypass now clears the consecutive-failure counter. Validation sweeps a body-width travel corridor, rejects actor floors, and checks headroom plus eight-direction body-radius clearance at the destination.
6. **Incomplete failed-start cleanup**: a graph that rejected or failed to enter parkour now receives the same state cleanup as other ended jobs.
7. **Unsafe form downcasts**: MCM globals and the follower faction now use CommonLib's type-checked form casts.
8. **ESP HEDR metadata**: the next-object field used a load-order-prefixed FormID. It now stores the correct local next object ID, `0x813`.
9. **Version drift**: CMake and vcpkg still reported 1.0.0 while the DLL and documentation reported 2.4.0. All 2.4.1 source metadata is synchronized.
10. **Documentation drift**: the README claimed a 250-unit default climb height while the runtime, ESP, INI, and MCM use 130.
11. **Build portability drift**: the only CMake preset targeted Visual Studio 2026, and CI assumed `VCPKG_ROOT` even when hosted runners expose `VCPKG_INSTALLATION_ROOT`. The preset now targets Visual Studio 2022 and CI resolves either variable.
12. **Release pollution**: the 403 MB development ZIP included compiler output, a nested dependency repository, installed libraries, caches, and another ZIP. The clean source release excludes all of it.

## Validation added

- Deterministic ESP and SEQ regeneration.
- ESP header, ESL flag, record count, FormID, next-object, and SEQ checks.
- MCM GlobalValue reference validation.
- Version synchronization checks.
- Regression guards for the corrected runtime state paths.
- PE x64, SKSE export, and embedded-version validation before a Nexus ZIP can be created.
- Deterministic Nexus and source archive builders.
- A pinned Windows GitHub Actions build and a local `build_release.ps1` path.

## Current release state

The source and generated plugin data are ready for 2.4.1. The supplied DLL is still the original 2.4.0 binary. The release builder intentionally refuses to label or package that stale DLL as 2.4.1.

A production SKSE DLL must be rebuilt with the MSVC ABI and Windows SDK. The current sandbox does not contain that proprietary toolchain. The included Windows build script and CI workflow compile the corrected source and only then create the Nexus archive.

GitHub read access succeeded, but repository writes returned HTTP 403 because the connected GitHub app has no repository installation. No remote changes were made.
