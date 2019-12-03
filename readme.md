# Network of the damned!
This is a fork of [Catacombs of the damned](https://github.com/jhhoward/Arduboy3D) which is an experimental 2 player networked multiplayer version.

Catacombs of the damned! is a first person shooter / dungeon crawler for the [Arduboy miniature game system](https://www.arduboy.com) where you navigate 10 floors of a randomly generated dungeon dispatching monsters with your magic fireballs whilst collecting as much loot as possible. 

It was partly inspired by the [Catacomb 3D series](https://www.gog.com/game/catacombs_pack) of games.

## Build instructions
To compile from source you will need the [Arduboy2 library](https://github.com/MLXXXp/Arduboy2) and the [ArduboyTones library](https://github.com/MLXXXp/ArduboyTones) as well as the Arduino IDE

Open **Source/Arduboy3D/Arduboy3D.ino** in the Arduino IDE and hit build

## Multiplayer setup
You will need:
- 2 Arduboys flashed with the sketch
- A computer with 2xUSB ports with Python installed to act as the server

### Steps:
- Flash the 2 Arduboys with the sketch (flash **Arduboy3D.hex** binary or alternatively build from source: **Source/Arduboy3D/Arduboy3D.ino**)
- On the server computer, make sure that you have Python installed
- Ensure you have installed the **pyserial** and **keyboard** Python modules by running:
    - python -m pip install pyserial keyboard
- Start the relay script
    - python Relay/relay.py
- Switch on both Arduboys, the game should start automatically once they have found each other

## Notes on implementation
The game works by using the USB serial interface. The Python relay script simply forwards any traffic between the connected Arduboys - it doesn't have any game logic itself. Each game tick, the 2 Arduboys exchange their current button state and move in lockstep.

## Windows client
It is possible to build a Windows client version of the game if you don't have 2 Arduboys to hand. The relay script will accept TCP connections from port 9999. To build from source, you will need Visual Studio 2015 and open the solution **Source/Windows/Arduboy3D/Arduboy3D.sln**. The build configuration has only been set up for Debug/x86.

You will need to run the Windows client on the same machine as the relay script.