# N-Queens Game using C++ and SFML

A polished desktop version of the classic N-Queens puzzle built with C++ and SFML. The game lets you place queens on a dynamic board, track conflicts, undo moves, request hints, and reset the puzzle when needed.

## Features

- Interactive SFML interface with a custom board layout
- Dynamic board sizing for different `N` values
- Conflict highlighting for invalid queen placements
- Undo support for backtracking moves
- Hint support for guided play
- Reset option to start a fresh puzzle
- Win state feedback and visual effects

## Controls

- Click a board cell to place or remove a queen
- Use **Undo** to revert the last move
- Use **Hint** to reveal a helpful move
- Use **Reset** to restart the current puzzle

## Build Requirements

- C++17 compatible compiler
- SFML 2.5+ development files
- CMake 3.14 or newer

This repository includes SFML folders for Windows builds, but you can also point CMake to your own SFML installation.

## Build With CMake

From the project directory:

```bash
cmake -S . -B build
cmake --build build
```

If CMake cannot find SFML, set `SFML_DIR` in `CMakeLists.txt` to the path of your SFML CMake package, for example:

```cmake
set(SFML_DIR "C:/SFML-2.6.1/lib/cmake/SFML")
```

## Manual Build Example

If you want to compile the game directly on Windows with g++, use a command similar to this:

```bash
g++ main.cpp -o NQueens.exe -IC:/SFML-2.6.1/include -LC:/SFML-2.6.1/lib -lsfml-graphics -lsfml-window -lsfml-system -std=c++17
```

## Project Files

- `main.cpp` contains the full game implementation
- `CMakeLists.txt` configures the CMake build
- `build/` stores generated build output

## Notes

- On Windows, the required SFML DLLs must be available next to the executable or on your PATH.
- The game is designed as a standalone desktop app and does not require a browser or external runtime.