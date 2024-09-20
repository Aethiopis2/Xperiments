# Conway's Game of Life
Program to simulate the game of Life in Windows app using standard GDI functions. The simulation
takes place in a rectangular grid of 20x20; i.e. 400x400 window size as each cell is 20 px.
The program considers the entire window as a grid world.

## Usage
Create a file named "configuration.txt" and write down the positions of the cells to turn alive;
as YX format making sure that the index does not exceed 20 for both row and column.
a sample configuration may be given as follows (as alternating values of rows and columns)
`5 5 5 6 5 7 11 12 11 13 11 14 15 15 19 1 19 2 18 1 18 2`