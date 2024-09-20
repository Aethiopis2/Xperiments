/**
 * Breif:
 *	Definition for Life object that simulates the game over a gird of 20x20
 *	array of ints
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
#include <Windows.h>
#include <stdio.h>





//==========================================================================================|
//			GLOBALS
//==========================================================================================|
const int LIFE_ROWS{ 20 };			// grid dimensions
const int LIFE_COLS{ 20 };


// cell dimensions
const int CELL_WIDTH{ 20 };
const int CELL_HEIGHT{ 20 };




//==========================================================================================|
//			CLASS
//==========================================================================================|
class Life
{
public:

	void initialize();
	void update();
	void draw(HDC hdc, HBRUSH color, HBRUSH border);

private:

	int grid[LIFE_ROWS + 2][LIFE_COLS + 2];	// +2 on each side to act as boundaries

	int neighbourCount(const int row, const int col);
};

