# DV1633 Project

This started as an attempt to learn how to use CMake and maintaining different build systems, but that fell through.

## Build

1. From the project root, go into `buildsystems>MSVC` and open `DV1633GameProject.sln`.
2. Build the solution, it should compile Lua and raylib. It should also copy the scripts folder to be next to the bin folder
3. Should now be possible to run either from the visual studio
4. If raylib or lua is missing, run `git submodule update --init --recursive` to get the submodules

## Game

The game is an MVP because I did not have time to finish everything I wanted to do. Like Ultimate Chicken Horse it is possible to place objects between rounds, players take turns and whoever touches the goal the most times wins.

Game runs over 3 rounds.

### Controls

**Placement**:

- numbers 1-2 to select an object
- Left Mouse Button to place object
- S to save current placement to file
- L to load object placements from file

To verify that file loading works, try placing objects in round 1, quickly die twice, place one more object in round 2, save. Start the `.exe` again and press L to load objects from the file.

**Platforming**:

- A to move left
- D to move right
- Spacebar to jump