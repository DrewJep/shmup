# Isometric Shmup

An isometric shoot 'em up arcade-style game written in C++ with SFML.

## Prerequisites

- C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake 3.15 or higher
- SFML 3.0 or higher (Note: This project uses SFML 3.0 API which has breaking changes from 2.5)

### Installing SFML

#### macOS (using Homebrew)
```bash
brew install sfml
```

**Note:** After installing SFML via Homebrew, if CMake still can't find it, you may need to specify the SFML path manually:
```bash
export CMAKE_PREFIX_PATH="/opt/homebrew/opt/sfml:$CMAKE_PREFIX_PATH"
```

Or if you're using an Intel Mac:
```bash
export CMAKE_PREFIX_PATH="/usr/local/opt/sfml:$CMAKE_PREFIX_PATH"
```

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get install libsfml-dev
```

#### Windows
Download SFML from [sfml-dev.org](https://www.sfml-dev.org/download.php) and follow the installation instructions.

## Building

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Configure with CMake:
```bash
cmake ..
```

3. Build the project:
```bash
cmake --build .
```

Or on Unix-like systems:
```bash
make
```

4. Run the game:
```bash
./Shmup
```

## Controls

- **W / Up Arrow**: Move up
- **S / Down Arrow**: Move down
- **A / Left Arrow**: Move left
- **D / Right Arrow**: Move right
- **ESC / Close Window**: Exit game

## Project Structure

```
shmup/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── .gitignore             # Git ignore rules
├── include/                # Header files
│   ├── Ship.h             # Ship class header
│   ├── Game.h             # Game class header
│   └── IsometricUtils.h   # Isometric projection utilities
├── src/                    # Source files
│   ├── main.cpp           # Entry point
│   ├── Game.cpp           # Game class implementation
│   ├── Ship.cpp           # Ship class implementation
│   └── IsometricUtils.cpp # Isometric utilities implementation
└── assets/                # Game assets
    ├── images/            # Sprite images and textures
    ├── sounds/            # Sound effects and music
    └── fonts/             # Font files
```

## Current Features

- Basic ship object that can be controlled by the player
- Smooth movement with delta time
- Screen boundary collision
- Simple visual representation (triangle shape)
- Game class for organized game loop management
- Isometric utility functions for future isometric rendering
- Clean separation of concerns (Game, Ship, utilities)

## Roadmap

- [ ] Isometric rendering
- [ ] Sprite-based graphics
- [ ] Shooting mechanics
- [ ] Enemy AI
- [ ] Multiple levels
- [ ] Power-ups
- [ ] Sound effects and music
- [ ] Particle effects
- [ ] Score system

## License

[Add your license here]

