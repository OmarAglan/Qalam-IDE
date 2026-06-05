# Windows Test Build Fix — 2026-06-05

## Fixed

- Added the missing `tests/TestWorkspaceIndexer.cpp` test source.
- Made tests opt-in for the normal Windows app build by setting `QALAM_BUILD_TESTS=OFF` by default.
- Updated `scripts/build-windows.ps1` to pass `-DQALAM_BUILD_TESTS=OFF` unless `-BuildTests` is explicitly used.
- Updated `scripts/build-windows.ps1` to build the app target (`Qalam`) for normal packaging instead of building every test target.
- Set test executables to `WIN32_EXECUTABLE OFF`, keeping Qt Test binaries as console apps on Windows/MinGW.

## Commands

Normal app build:

```powershell
.\build-qalam-windows.cmd
```

Optional test build:

```powershell
.\scripts\build-windows.ps1 -Configuration Release -BuildTests
```
