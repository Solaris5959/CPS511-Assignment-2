# CPS511 Assignment 2

## Build Instructions
Ensure that the following are downloaded on your system:
- **CMake** (v3.21+)
- **C++17 Compiler** (Clang, GCC, etc.)
- **Raylib**

If on Mac, CMake and Raylib can be installed with `brew install cmake raylib`.

The CMake file in this project requires all source code is within the `/src/` directory, i.e.
```text
.
├── CMakeLists.txt
└── src/
    └── (.cpp source files)
```

In your terminal, navigate to the `/build/` directory:
```bash
cd build
```

Generate the build files with:
```bash
cmake ..
```

Compile the executable with:
```bash
cmake --build .
```

This will generate an executable `RaylibGame`. To run the compiled game:
```bash
./RaylibGame
```

## Controls

**Movement**
- **W:** Move Drone Forward
- **S:** Move Drone Backward
- **A:** Move Drone Left
- **D:** Move Drone Right
- **R:** Move Drone Up
- **F:** Move Drone Down

**Rotation**
- **Q:** Yaw Drone Left
- **E:** Yaw Drone Right

**Camera**
- **C:** Switch to First Person Camera
- **V:** Switch to Third Person Camera

**Environment & Toggles**
- **Z:** Switch between Daytime/Nighttime Scenes
- **X:** Toggle spotlight
- **Spacebar:** Turn Drone On/Off
- **Home:** Reset Drone Position

## Known Limitations
Shadows currently only map to the ground plane. Any other objects in the scene will not be shadowed by the drone. 
- A fix for this could be shadowmapping, but despite my best efforts I cannot get it working.


Yaw is currently not smoothed like other movement.