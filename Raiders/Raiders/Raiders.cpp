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
#include <dmusici.h>
#include <dmusicc.h>
#include <dmusicf.h>
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
void Init_Tie(int index);



//============================================================================|
//			GLOBALS
//============================================================================|
HWND main_window_handle = NULL;
HINSTANCE main_instance = NULL;
char buffer[256];


POINT3D tie_vlist[NUM_TIE_VERTS];		// vertex list for tie fighter model
LINE3D tie_shape[NUM_TIE_EDGES];		// edge list for tie fighter model
TIE ties[NUM_TIES];						// tie fighters
POINT3D stars[NUM_STARS];				// the star field

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
int window_closed = 0;


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
/**
 * Start's a tie fighter at far end of the universe so as to send it towards 
 *	the screen simulating 3D
 */
void Init_Tie(int index)
{
	// position each tie in the viewing volume
	ties[index].x = -WINDOW_WIDTH + rand() % (2 * WINDOW_WIDTH);
	ties[index].y = -WINDOW_HEIGHT + rand() % (2 * WINDOW_HEIGHT);
	ties[index].z = 4 * FAR_Z;

	// initalize velocity of tie fighter
	ties[index].vx = -4 + rand() % 8;
	ties[index].vy = -4 + rand() % 8;
	ties[index].vz = -4 - rand() % 64;

	// turn tie fighter on
	ties[index].state = 1;
} // end Init_Tie



//============================================================================|
/**
 * Moves each alive tie fighter towards the screen i.e. the near plane from
 *	the far plane of the universe. If tie crosses past the near plane it's reset
 *	back to the far side
 */
void Process_Ties()
{
	int index;

	for (index = 0; index < NUM_TIES; index++)
	{
		if (ties[index].state == 0)
			continue;		// is dead

		ties[index].x += ties[index].vx;
		ties[index].y += ties[index].vy;
		ties[index].z += ties[index].vz;

		if (ties[index].z <= NEAR_Z)
		{
			Init_Tie(index);
			misses++;
		} // end if clip
	} // end for
} // end Process_Ties



//============================================================================|
void Draw_Ties()
{
	int index;
	int bmin_x, bmin_y, bmax_x, bmax_y;		// bounding box collosion detection

	// draw each tie fighter
	for (index = 0; index < NUM_TIES; index++)
	{
		if (ties[index].state == 0)
			continue;		// dead one.

		bmin_x = bmin_y = 100'000;
		bmax_x = bmax_y = -100'000;

		// based on z distance shade the tie fighter
		UINT rgba_tie = _RGB32BIT(0, 0, (255 - 255 * (ties[index].z / (4 * FAR_Z))), 0);
		
		// as this is a wireframe stuff a tie fighter is 
		//	made up of bunch of edges and stuff
		for (int edge = 0; edge < NUM_TIE_EDGES; edge++)
		{
			POINT3D p1_per, p2_per;

			p1_per.x = VIEW_DISTANCE * (ties[index].x + tie_vlist[tie_shape[edge].v1].x) /
				(tie_vlist[tie_shape[edge].v1].z + ties[index].z);
			p1_per.y = VIEW_DISTANCE * (ties[index].y + tie_vlist[tie_shape[edge].v1].y) /
				(tie_vlist[tie_shape[edge].v1].z + ties[index].z);
			p2_per.x = VIEW_DISTANCE * (ties[index].x + tie_vlist[tie_shape[edge].v2].x) /
				(tie_vlist[tie_shape[edge].v1].z + ties[index].z);
			p2_per.y = VIEW_DISTANCE * (ties[index].y + tie_vlist[tie_shape[edge].v2].y) /
				(tie_vlist[tie_shape[edge].v1].z + ties[index].z);

			int p1_screen_x = WINDOW_WIDTH / 2 + p1_per.x;
			int p1_screen_y = WINDOW_HEIGHT / 2 - p1_per.y;
			int p2_screen_x = WINDOW_WIDTH / 2 + p2_per.x;
			int p2_screen_y = WINDOW_HEIGHT / 2 - p2_per.y;

			Draw_Clip_Line32(p1_screen_x, p1_screen_y, p2_screen_x, p2_screen_y, rgba_tie,
				back_buffer, back_lpitch);
			
			int min_x = min(p1_screen_x, p2_screen_x);
			int max_x = max(p1_screen_x, p2_screen_x);
			int min_y = min(p1_screen_y, p2_screen_y);
			int max_y = max(p1_screen_y, p2_screen_y);

			bmin_x = min(bmin_x, min_x);
			bmin_y = min(bmin_y, min_y);
			bmax_x = max(bmax_x, max_x);
			bmax_y = max(bmax_y, max_y);
		} // end for edge

		// test hit with cannon
		if (cannon_state == 1)
		{
			if (target_x_screen > bmin_x && target_x_screen < bmax_x &&
				target_y_screen > bmin_x && target_y_screen < bmax_y)
			{
				Start_Explosion(index);
				DSound_Play(explosion_id);

				score += ties[index].z;
				hits++;
				Init_Tie(index);
			} // end if inbound
		} // end if cannon_state
	} // end for
} // end Draw_Tie



//============================================================================|
/**
 * Initializes the direct draw, sound, and input as well as game elements
 *	such as starfields and crosshair, and tie fighters.
 */
int Game_Init(void* parms)
{
	int index;
	Open_Error_File((char*)"error.txt");

	// intialize directX
	DDraw_Init(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BPP, WINDOWED_APP);
	DInput_Init();
	DInput_Init_Keyboard();
	DSound_Init();

	// load in sound fx
	explosion_id = DSound_Load_WAV((char*)"exp1.wav");
	laser_id = DSound_Load_WAV((char*)"shocker.wav");

	// initalize directmusic
	DMusic_Init();

	// load and start main track
	main_track_id = DMusic_Load_MIDI((char*)"midifile2.mid");
	DMusic_Play(main_track_id);

	ShowCursor(FALSE);
	srand(Start_Clock());

	// color systems redefined to 32-bits from the original;
	//	times change...
	rgba_blue = _RGB32BIT(0, 0, 0, 255);
	rgba_green = _RGB32BIT(0, 0, 255, 0);
	rgba_red = _RGB32BIT(0, 255, 0, 0);
	rgba_white = _RGB32BIT(0, 255, 255, 255);

	// create the star field
	for (index = 0; index < NUM_STARS; index++)
	{
		stars[index].x = -WINDOW_WIDTH / 2 + rand() % WINDOW_WIDTH;
		stars[index].y = -WINDOW_HEIGHT / 2 + rand() % WINDOW_HEIGHT;
		stars[index].z = NEAR_Z + rand() % (FAR_Z - NEAR_Z);
		stars[index].color = rgba_white;
	} // end for starfeild

	// create the fighter model
	POINT3D temp_tie_vlist[NUM_TIE_VERTS] = {
		{rgba_white,-40,40,0},	// p0
		{rgba_white,-40,0,0},	// p1
		{rgba_white,-40,-40,0},	// p2
		{rgba_white,-10,0,0},	// p3
		{rgba_white,0,20,0},	// p4
		{rgba_white,10,0,0},	// p5
		{rgba_white,0,-20,0},	// p6
		{rgba_white,40,40,0},	// p7
		{rgba_white,40,0,0},	// p8
		{rgba_white,40,-40,0}	// p9
	};

	// copy model into real global arrays
	for (index = 0; index < NUM_TIE_VERTS; index++)
		tie_vlist[index] = temp_tie_vlist[index];

	LINE3D temp_tie_shapes[NUM_TIE_EDGES] = {
		{rgba_green,0,2},		// l0
		{rgba_green,1,3},		// l1
		{rgba_green,3,4},		// l2
		{rgba_green,4,5},		// l3
		{rgba_green,5,6},		// l4
		{rgba_green,6,3},		// l5
		{rgba_green,5,8},		// l6
		{rgba_green,7,9}		// l7
	};

	for (index = 0; index < NUM_TIE_EDGES; index++)
		tie_shape[index] = temp_tie_shapes[index];

	for (index = 0; index < NUM_TIES; index++)
		Init_Tie(index);

	return 0;
} // end Game_Init



//============================================================================|
/**
 * In the end we terminate the game resources proper for Windows to be cool 
 *	with it and all.
 */
int Game_Shutdown(void* parms)
{
	// KILL directX
	DSound_Stop_All_Sounds();
	DSound_Shutdown();
	DMusic_Delete_All_MIDI();
	DMusic_Shutdown();
	DInput_Shutdown();
	DDraw_Shutdown();

	return 0;
} // end Game_Shutdown



//============================================================================|
/**
 * Moves the starfeild simulating motion of the viewer into the screen
 */
void Move_Starfield()
{
	int index;
	for (index = 0; index < NUM_STARS; index++)
	{
		stars[index].z -= player_z_vel;
		if (stars[index].z <= NEAR_Z)
			stars[index].z = FAR_Z;
	} // end for index
} // end Move_Starfield



//============================================================================|
/**
 * Draw's the star feild in a cylindrical manner
 */
void Draw_Starfield()
{
	int index;
	for (index = 0; index < NUM_STARS; index++)
	{
		float x_per = VIEW_DISTANCE * stars[index].x / stars[index].z;
		float y_per = VIEW_DISTANCE * stars[index].y / stars[index].z;

		int x_screen = WINDOW_WIDTH / 2 + x_per;
		int y_screen = WINDOW_HEIGHT / 2 - y_per;

		if (x_screen >= WINDOW_WIDTH || x_screen < 0 || y_screen >= WINDOW_HEIGHT || y_screen < 0)
			continue;
		else
		{
			((UINT*)back_buffer)[x_screen + y_screen * (back_lpitch >> 2)] =
				stars[index].color;
		} // end else
	} // end for index
} // end Draw_Starfield



//============================================================================|
int Game_Main(void* parms)
{
	if (KEY_DOWN(VK_ESCAPE))
	{
		window_closed = 1;
		PostMessage(main_window_handle, WM_DESTROY, 0, 0);
	} // end if

	if (window_closed)
		return 0;

	int index;

	Start_Clock();
	DDraw_Fill_Surface(lpddsback, 0);

	Move_Starfield();
	DDraw_Lock_Back_Surface();

	Draw_Starfield();

	DDraw_Unlock_Back_Surface();

	if (DMusic_Status_MIDI(main_track_id) == MIDI_STOPPED)
		DMusic_Play(main_track_id);

	DDraw_Flip();

	Wait_Clock(30);

	return 0;
} // end Game_Main