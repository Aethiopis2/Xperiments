/**
 * @Brief Radiers is a wireframe 3D game, meant to ease into my journey to 3D 
 *	game programming.
 *
 * @date 3rd of January 2025, Friday.
 */
#define WIN32_LEAN_AND_MEAN
#define INITGUID


//============================================================================|
//			INCLUDES
//============================================================================|
#include <windows.h>   // include important windows stuff
#include <windowsx.h> 
#include <mmsystem.h>
#include <iostream> // include important C/C++ stuff
#include <conio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <io.h>
#include <fcntl.h>
#include <sys/timeb.h>
#include <time.h>


#include <ddraw.h>    // directX includes
#include <dsound.h>
#include <dmksctrl.h>
//#include <dmusici.h>
#include <dmusicc.h>
//#include <dmusicf.h>
#include <dinput.h>

#include "t3dlib1.h"
#include "t3dlib2.h"
#include "t3dlib3.h"



//============================================================================|
//			DEFINES
//============================================================================|
#define WINDOW_CLASS_NAME	  "WIN3DCLASS"
#define WINDOW_TITLE		  "Raiders a wireframe 3D game"
#define WINDOW_WIDTH		  640
#define WINDOW_HEIGHT		  480
#define WINDOW_BPP			  32	/* updated for 32-bit color mode */
#define WINDOWED_APP		  1		// 0 for fullscreen, 1 for windowed


#define NUM_STARS			  250	// number of stars in the sim
#define NUM_TIES			  10	// number of tie fighters in the sim

#define NEAR_Z				  10	// near clipping plane
#define FAR_Z				  2000	// far clipping plane
#define VIEW_DISTANCE		  320	// viewing distance with 90 deg fov

// player constants
#define CROSS_VEL			  8		// speed of crosshair mov't
#define PLAYER_Z_VEL		  8		// virtual z velocity to simulate motion

// tie fighter model constants
#define NUM_TIE_VERTS		  10
#define NUM_TIE_EDGES		  8

// explosion
#define NUM_EXPLOSIONS		  NUM_TIES

// game states
#define GAME_RUNNING		  1
#define GAME_OVER			  0


//============================================================================|
//			TYPES
//============================================================================|
// 3D point structure
typedef struct POINT3D_TYP
{
	UINT color;
	float x, y, z;
} POINT3D, *POINT3D_PTR;

// 3D line with two indices into a vertex list
typedef struct LINE3D_TYP
{
	UINT color;
	int v1, v2;
} LINE3D, *LINE3D_PTR;

// 3D vector used for velocity
typedef struct VEC3D_TYP
{
	float x, y, z;
} VEC3D, *VEC3D_PTR;

// tie fighter
typedef struct TIE_TYP
{
	int state;		// state of the tie 0 = dead, 1 = alive
	float x, y, z;	// position of the tie fighter
	float vx, vy, vz;	// velocity of tie
} TIE, TIE_PTR;

// a wireframe explosion
typedef struct EXPL_TYP
{
	int state;		// state of explosion
	int counter;	// counter for explosion
	UINT color;

	// an explosion is a collection of edges and lines
	//	based on the 3D model of the tie that is exploding
	POINT3D p1[NUM_TIE_EDGES];		// start point of edge n
	POINT3D p2[NUM_TIE_EDGES];		// ending point
	VEC3D vel[NUM_TIE_EDGES];		// velocity of sharpnel
} EXPL, *EXPL_PTR;



//============================================================================|
//			PROTOTYPES
//============================================================================|
int Game_Init(void* parms = NULL);
int Game_Main(void* parms = NULL);
int Game_Shutdown(void* parms = NULL);
int Init_Tie(int index);



//============================================================================|
//			GLOBALS
//============================================================================|
HWND main_window_handle = NULL;
HINSTANCE main_instance = NULL;
char buffer[256];


POINT3D tie_vlist[NUM_TIE_VERTS];		// vertex list for tie fighter model
LINE3D tie_shape[NUM_TIE_EDGES];		// edge list for tie fighter model
TIE ties[NUM_TIES];						// tie fighters
POINT3D start[NUM_STARS];				// the star field

UINT rgba_green,
rgba_white,
rgba_red,
rgba_blue;


// player vars
int cross_x = 0,
cross_y = 0;

int cross_x_screen = WINDOW_WIDTH / 2,
cross_y_screen = WINDOW_HEIGHT / 2,
target_x_screen = WINDOW_WIDTH / 2,
target_y_screen = WINDOW_HEIGHT / 2;	// used for targeter

int player_z_vel = 4;		// virtual speed of ship/viewport
int cannon_state = 0;		
int cannon_count = 0;

EXPL explosions[NUM_EXPLOSIONS];	// the explosions

int misses = 0;
int hits = 0;
int score = 0;


// music stuff
int main_track_id = -1,	// main music id
laser_id = -1,			// sound of laser
explosion_id,			// blast sounds
flyby_id;				// tie fighter flying by 

int game_state = GAME_RUNNING;



#define EEXIT(h, m)		{ snprintf(buffer, 256, "%s failed with error code %d", m, GetLastError()); \
	MessageBox(h, m, "Err Box", MB_ICONERROR); return(0); }


//============================================================================|
//			FUNCTIONS
//============================================================================|
/**
 * Main window event handler
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static PAINTSTRUCT ps;
	static HDC hdc;

	switch (msg)
	{
	case WM_CREATE:
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default: break;
	} // end switch

	return DefWindowProc(hwnd, msg, wparam, lparam);
} // end WindowProc



//============================================================================|
/**
 * Program entry point
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX win;
	MSG msg;

	main_instance = hInstance;

	win.cbClsExtra = 0;
	win.cbSize = sizeof(win);
	win.cbWndExtra = 0;
	win.hbrBackground = GetStockBrush(BLACK_BRUSH);
	win.hCursor = LoadCursor(NULL, IDC_ARROW);
	win.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	win.hInstance = hInstance;
	win.lpfnWndProc = WindowProc;
	win.lpszClassName = WINDOW_CLASS_NAME;
	win.lpszMenuName = NULL;
	win.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&win))
		EEXIT(NULL, (char*)"RegisterClassEx()");

	if (!(main_window_handle = CreateWindowEx(
		0,
		WINDOW_CLASS_NAME, WINDOW_TITLE,
		(WINDOWED_APP ? WS_OVERLAPPED | WS_SYSMENU : WS_POPUP),
		CW_USEDEFAULT, CW_USEDEFAULT,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL)))
		EEXIT(NULL, (char*)"CreateWindowEx");

	if (WINDOWED_APP)
	{
		RECT rc{ 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };
		AdjustWindowRectEx(&rc,
			GetWindowStyle(main_window_handle),
			GetMenu(main_window_handle) != NULL,
			GetWindowExStyle(main_window_handle));

		// save th golbal client offsets
		window_client_x0 = -rc.left;
		window_client_y0 = -rc.top;

		// now resize
		MoveWindow(main_window_handle, 
			100, 
			100, 
			rc.right - rc.left, 
			rc.bottom - rc.top, 
			FALSE);
	} // end if WINDOWED_APP

	ShowWindow(main_window_handle, nCmdShow);
	UpdateWindow(main_window_handle);

	Game_Init();

	// disable CTRL+ALT+DEL, ALT+TAB
	SystemParametersInfo(SPI_SCREENSAVERRUNNING, TRUE, NULL, 0);
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} // end if

		Game_Main();
	} // end while

	Game_Shutdown();
	SystemParametersInfo(SPI_SCREENSAVERRUNNING, FALSE, NULL, 0);

	return (int)msg.wParam;
} // end WinMain



//============================================================================|
int Init_Tie(int index)
{
	return 0;
} // end Init_Tie



//============================================================================|
int Game_Init(void* parms)
{
	int index;
	Open_Error_File((char*)"error.txt");

	// intialize directX
	DDraw_Init(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BPP, WINDOWED_APP);
	DInput_Init();
	DInput_Init_Keyboard();
	DSound_Init();

	return 0;
} // end Game_Init



//============================================================================|
int Game_Main(void* parms)
{
	return 0;
} // end Game_Main



//============================================================================|
int Game_Shutdown(void* parms)
{
	// KILL ddraw
	DDraw_Shutdown();

	return 0;
} // end Game_Shutdown