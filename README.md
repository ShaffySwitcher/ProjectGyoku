# Project Gyoku

Snippets of Project Gyoku's source code that I've open sourced.

## Requirements

- Windows
- Visual Studio 2022
- MSVC toolsets:
	- v141_xp (for Win32 builds)
	- v143 (for x64 builds)
- DxLib and zlib available in your include/library paths

## Build

1. Open ProjectGyoku.sln in Visual Studio.
2. Select a configuration (Debug or Release) and platform (Win32 or x64).
3. Build the solution.

## Run

1. Set ProjectGyoku as the startup project.
2. Run with F5 or Ctrl+F5.
3. Make sure the working directory is the ProjectGyoku folder so config and assets can be found.

## Default Keyboard Controls

- Arrow keys: Move
- Z: Fire
- X: Bomb / Cancel
- Left Shift: Focus
- C: Special
- Esc: Pause
- Left Ctrl: Skip
- Enter: Select
- Alt + Enter: Toggle fullscreen
- P: Save screenshot

## Notes

- Configuration is saved to pg01.cfg.
- The game tries to load data.dat first. If it is not available, development builds can fall back to external files.
