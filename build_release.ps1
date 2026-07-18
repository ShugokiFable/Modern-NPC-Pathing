[CmdletBinding()]
param(
    [string]$Generator = "",
    [string]$CommonLibCommit = "b93280e832f263dbef44e44cbe2936622a02f91a"
)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

function Resolve-CMakeGenerator {
    if ($Generator) { return $Generator }
    $help = (& cmake --help | Out-String)
    if ($help -match "Visual Studio 18 2026") { return "Visual Studio 18 2026" }
    if ($help -match "Visual Studio 17 2022") { return "Visual Studio 17 2022" }
    throw "Visual Studio 2022 or 2026 C++ build tools were not found."
}

$vcpkgRoot = $env:VCPKG_ROOT
if (-not $vcpkgRoot) { $vcpkgRoot = $env:VCPKG_INSTALLATION_ROOT }
if (-not $vcpkgRoot -or -not (Test-Path "$vcpkgRoot/scripts/buildsystems/vcpkg.cmake")) {
    throw "VCPKG_ROOT or VCPKG_INSTALLATION_ROOT must point to a working vcpkg checkout."
}
$env:VCPKG_ROOT = $vcpkgRoot

$cmakeGenerator = Resolve-CMakeGenerator
Write-Host "Using $cmakeGenerator"

python generate_esp.py
python -m unittest discover -s tests -v

$commonLib = Join-Path $PSScriptRoot "extern/CommonLibSSE"
if (-not (Test-Path (Join-Path $commonLib "CMakeLists.txt"))) {
    git clone https://github.com/CharmedBaryon/CommonLibSSE-NG.git $commonLib
}
if (-not (Test-Path (Join-Path $commonLib ".git"))) {
    throw "extern/CommonLibSSE exists but is not the pinned Git checkout. Remove it and rerun."
}
git -C $commonLib fetch --quiet origin $CommonLibCommit
git -C $commonLib checkout --quiet --detach $CommonLibCommit

$commonBuild = Join-Path $commonLib "build"
$commonInstall = Join-Path $commonLib "install"
cmake -S $commonLib -B $commonBuild -G $cmakeGenerator -A x64 `
    -DBUILD_TESTS=OFF `
    -DENABLE_SKYRIM_VR=OFF `
    -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded `
    -DCMAKE_INSTALL_PREFIX="$commonInstall" `
    -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
    -DVCPKG_TARGET_TRIPLET=x64-windows-static `
    -DVCPKG_HOST_TRIPLET=x64-windows-static
cmake --build $commonBuild --config Release --target install --parallel

cmake -S . -B build -G $cmakeGenerator -A x64
cmake --build build --config Release --parallel

python -m unittest discover -s tests -v
python tools/package_release.py
Write-Host "Release created in dist/."
