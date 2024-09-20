/**
 * Breif:
 *	Implementation of Life object member functions
 *
 * Program Author:
 *	Rediet Worku aka Aethiops II ben Zahab
 *
 * Date Created:
 *	19th of Sepetemeber 2024, Thursday
 */
#pragma once



 //==========================================================================================|
 //			INCLUDES
 //==========================================================================================|
#include <fstream>

#include "Life.h"





//==========================================================================================|
//			CLASS IMPLEMENTATION
//==========================================================================================|
/**
 * Initializes the grid object from one of the two possible sources, if the file "configuration.txt"
 *	exists within the directory then, it initalizes the grid from the file alas it uses
 *	a pre-set kooked value to show the simulation
 */
void Life::initialize()
{
	int row, col;
	for (row = 1; row <= LIFE_ROWS; row++)
		for (col = 1; col <= LIFE_COLS; col++)
			grid[row][col] = 0;

	std::ifstream infile{ "configuration.txt" };
	if (infile.is_open())
	{
		while (infile >> row >> col)
			grid[row][col] = 1;
	} // end if
	else
	{
		// use predefined shape
		grid[3][3] = grid[3][4] = grid[3][5] = 1;
		grid[15][15] = grid[14][15] = grid[16][15] = 1;
	} // end else
} // end initalize



//==========================================================================================|
/**
 * @Breif updates the grid object according to the rules of the game. After the function is
 *	finished grid now contains a new configuration.
 * Function uses, neighbourCount() utility memeber to count the alive cells from all the 6
 *	neighbours of a cell.
 */
void Life::update()
{
	int row, col;
	int newGrid[LIFE_ROWS + 2][LIFE_COLS + 2]{ 0 };

	for (row = 1; row <= LIFE_ROWS; row++) {
		for (col = 1; col <= LIFE_COLS; col++) {
			switch (neighbourCount(row, col))
			{
			case 2:
				newGrid[row][col] = grid[row][col];		// status stays same
				break;

			case 3:
				newGrid[row][col] = 1;					// becomes alive
				break;

			default:
				newGrid[row][col] = 0;					// cell-dies; overcrowding, lonelyness, etc
			} // end switch
		} // end for
	} // end outer for

	for (row = 1; row <= LIFE_ROWS; row++)
		for (col = 1; col <= LIFE_COLS; col++)
			grid[row][col] = newGrid[row][col];
} // end update




//==========================================================================================|
/**
 * Draw's the current configuration of the grid on the device context passed as it's parameter
 *	and using the cell color
 *
 * @param hdc window device context
 * @param color cell internal color
 * @param border frame color for cell
 */
void Life::draw(HDC hdc, HBRUSH color, HBRUSH border)
{
	//HDC hdcMem = CreateCompatibleDC(hdc);

	for (int row = 1; row <= LIFE_ROWS; row++)
	{
		for (int col = 1; col <= LIFE_COLS; col++)
		{
			if (grid[row][col])
			{
				RECT rc;
				rc.left = (col - 1) * CELL_WIDTH;
				rc.top = (row - 1) * CELL_HEIGHT;
				rc.right = rc.left + CELL_WIDTH;
				rc.bottom = rc.top + CELL_HEIGHT;
				
				FrameRect(hdc, &rc, border);

				rc.left += 1;
				rc.top += 1;
				rc.bottom -= 1;
				rc.right -= 1;
				FillRect(hdc, &rc, color);
			} // end if drawing
		} // end nested for
	} // end for

} // end draw



//==========================================================================================|
/**
 * @Breif counts all the 6 neighbours of a grid cell.
 *
 * @param row
 * @param col
 *
 * returns int count of all the alive neighbouring cells
 */
int Life::neighbourCount(const int row, const int col)
{
	int count{ 0 };

	for (int i = row - 1; i <= row + 1; i++)
	{
		for (int j = col - 1; j <= col + 1; j++)
			count += grid[i][j];
	} // end outer for

	count -= grid[row][col];		// a cell is not it's own neighbour
	return count;
} // end neighboutCount