# PROJECT OUTLINE - Ultimate Turkey Donkey

## Overview

Purpose of the project:

This is my project for the course DV1633 - scripting and other languages.

It is meant to show an understanding of Lua and scripting in a game engine context
by constructing a game engine using ECS on the C/C++ side and then 
using Lua to manage scripting. As such the project uses Lua for scripting and 
binds the engine API to Lua using static bindings with the raw Lua C API. 
Furthermore it uses entt for ECS management and raylib for graphics, input, etc.

### Solution Projects

External Libs:

- entt : for ECS
- raylib : for graphics and other low level API management.
- Lua : for scripting.

Own libs:

- GameEngine : Core engine code, ECS management, input, raylib and so on.
- LuaBinding : Binding interface for the Engine, creating separation of responsibilities
			   with the core engine.

Main project:

- Application: The actual game application, using the GameEngine and LuaBinding libs, invoking
               lua scripts to start and manage game logic.

Other files:

- Scripts: In the project root, all the scripts used for the project.

### Solution Structure

The solution projects are set up like this:

0. External projects are compiled once and then used as `.lib` files. Lua, entt and raylib.
1. GameEngine defines the game engine, the scene object handler, as well as all the ECS components and systems.
2. LuaBinding is used to bind all the relevant functionality from the GameEngine project to the lua state.
3. Application uses LuaBinding and initializes the game loop, then hands over control to the Lua environment and scripts.

