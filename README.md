# Dot-and-Boxes
## 1. Description
Dots and Boxes is a classic pen-and-paper game. In this project, you will develop a digital version of Dots and Boxes using the C programming language. This version will initially include a two-player mode and then add bot functionality. You will also analyze the algorithmic complexity of key operations, such as determining if a square has been completed, and evaluate the complexity of the strategies used by the bot.

## 2. Sprint 1: Two-Player Game
### 2.1. Description
Implement a two-player version of Dots and Boxes (Players A and B), where both players interact through a console-based interface. Players take turns drawing a line between two dots. If a player completes a box, they are awarded that box, and their initials will appear inside it (A or B). The game continues until all boxes have been claimed. The winner is the player with the most boxes.

### 2.2. Requirement
- Display a 4x5 grid of boxes.
- Allow two players to take turns connecting dots.
- Update the board after each move.
- Check if a move completes a square and award it to the player.
- Declare the winner when all squares have been claimed.
- Run the game on a small Linux distribution without a graphical user interface (GUI), such as Alpine Linux.
- The Linux OS will run inside a Virtual Machine (VM), and your game should launch automatically as the first thing displayed after starting the VM. I.e., when the Alpine Linux VM starts, it should immediately take you to the game without even asking you for a username and password to login.
- Write your code in multiple .h and .c files and write a Makefile that compiles your code.
- Debug you code using Valgrind and using GDB
- Ensure that everything is found on Github
