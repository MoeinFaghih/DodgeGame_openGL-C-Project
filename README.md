

# DodgeGame

**DodgeGame** is an interactive game written in **C** using **OpenGL**, where players dodge randomly generated objects by moving their character. Players can navigate menus, adjust settings, and aim to achieve high scores through quick reactions and strategic dodging.

## Features

- **Random Object Firing & Animation**: Objects are fired at the player’s character from random coordinates, requiring quick reactions to dodge.
- **Manual UI Elements**: Custom-built UI for settings and menus, including range bars, text boxes, and buttons for easy navigation and customization.
- **Score Multiplier System**: Adjusts score based on player-defined settings (e.g., fire rate, object speed) for a customized challenge and reward balance.
- **High Scores with Bubble Sort**: High scores are recorded and displayed in a sorted list maintained by a bubble sort algorithm.

## Technical Concepts

- **Data Structures**: Defined multiple data structures, including `point` and nested structures, to handle object coordinates and game states.
- **Finite State Machine (FSM)**: Used an FSM to manage the game's flow, enabling smooth transitions between states (e.g., Menu, Gameplay, Game Over).
- **Collision Detection**: Implemented collision detection to check if the player’s character has collided with incoming objects.
- **Customizable Gameplay Settings**: Allows players to adjust game parameters (like fire rate and object speed), which influences gameplay difficulty.

## Controls

- **Movement**: Move your character to dodge objects by **right-clicking**.

