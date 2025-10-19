// PC IO simulator.
// Take Pico outputs and show them on a PC
// WILL ONLY BUILD ON x86 due to 32-bit pico memory arithmetic

#include <iostream>
#include <fstream> // for file I/O
#include "framework.h"

#include <windows.h>
#include <time.h>
#include <stdio.h>
#include "oleddisplaysim.h" // need to rename all this OLED stuff at some point
#include "pc_bitmap_to_text.h"
#include "pc_audio.h"
#include "pc_threads.h"
#include "..\..\App\app_inputs.h"
#include "..\..\App\App_main.h"
#include "..\..\hw_defs.h"

#define MAX_LOADSTRING 100
#define MY_TIMER 1

#define SCALE_SCREEN 3
#define BMP_WIDTH (TV_WIDTH*SCALE_SCREEN)
#define BMP_HEIGHT (TV_HEIGHT*SCALE_SCREEN)


/* Big RAM variables */
AppMain app_main; // C++ class with all the RP2040 application code

// Global Variables:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
int nTimer;
char str[255] = "";
unsigned char* dataval;
unsigned long lpBitmapBits[BMP_WIDTH * BMP_HEIGHT];
static volatile int testkey = 0;

static int inputword = 0xFFFFFFFF;
static bool g_linebuffer = 0;


void Pixel_to_Bitmap(int x, int y, int colourcode);

// Local functions
static void simulate_screen_linebuffer(void);
static void simulate_screen_framebuffer(void);

static void simulate_inputs(AppInputs::inputs_t button, BOOL pressed);
static void colour_palette(void);
static void toggle_inputs(AppInputs::inputs_t button);

VOID CALLBACK MyTimerProc(HWND hWnd, UINT uTimerMsg, UINT uTimerID, DWORD dwTime);

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_OLEDDISPLAYSIM, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OLEDDISPLAYSIM));

	MSG msg;

	//audio_main();
	app_threads();

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OLEDDISPLAYSIM));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_OLEDDISPLAYSIM);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 1024, 800, nullptr, nullptr, hInstance, nullptr);
	/*
	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
*/
	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);


	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		//sets up timer function
		nTimer = SetTimer(hWnd, MY_TIMER, 1, (TIMERPROC)MyTimerProc);

		// Could put init code here

		// Init OLED emulator
		//oled_app_init();
	}
	break;

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			import_bitmap();
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_ERASEBKGND:
	{
		//Do nothing. Need this here or it'll flash white as it auto-clears each frame.
	}
	break;
	case WM_PAINT:
	{

	    app_main.frame_interrupt();

		//#endif
		if (g_linebuffer)
		{
			simulate_screen_linebuffer();
		}
		else
		{
			simulate_screen_framebuffer();
		}
		//colour_palette();

		// Draw the bitmap, using global 'lpBitmapBits'

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		BITMAPINFO mybmi = { 0 };


		mybmi.bmiHeader.biSize = sizeof(mybmi);
		mybmi.bmiHeader.biWidth = BMP_WIDTH;
		mybmi.bmiHeader.biHeight = BMP_HEIGHT;
		mybmi.bmiHeader.biPlanes = 1;
		mybmi.bmiHeader.biBitCount = 32;
		mybmi.bmiHeader.biCompression = BI_RGB;
		mybmi.bmiHeader.biSizeImage = ((BMP_WIDTH * (mybmi.bmiHeader.biBitCount / 8) + 3) & -4) * BMP_HEIGHT;

		static int testcount = 0;

		int result = SetDIBitsToDevice(hdc, 0, 0,
			mybmi.bmiHeader.biWidth,
			mybmi.bmiHeader.biHeight,
			0, 0, 0,
			mybmi.bmiHeader.biHeight,
			lpBitmapBits,
			&mybmi,
			DIB_RGB_COLORS);

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_KEYDOWN:
		
		if (wParam == 0x53)  //S
		{
			simulate_inputs(AppInputs::B_SERVICE, 1);
		}

		if (wParam == 0x54)  //T
		{
			simulate_inputs(AppInputs::B_TEST, 1);
		}
		if (wParam == 0x30) // '0'
		{
			toggle_inputs(AppInputs::B_DIP0);
		}
		if (wParam == 0x31) // '1'
		{
			toggle_inputs(AppInputs::B_DIP1);
		}
		if (wParam == 'Z')
		{
			simulate_inputs(AppInputs::B_1P_1, 1);
		}
		if (wParam == 'X')
		{
			simulate_inputs(AppInputs::B_1P_2, 1);
		}
		if (wParam == 'C')
		{
			simulate_inputs(AppInputs::B_1P_3, 1);
		}

		if (wParam == VK_UP) // 'Up'
		{
			simulate_inputs(AppInputs::B_UP1, 1);
		}
		if (wParam == VK_DOWN) // 'Down'
		{
			simulate_inputs(AppInputs::B_DOWN1,1);
		}
		if (wParam == VK_LEFT)
		{
			simulate_inputs(AppInputs::B_LEFT1, 1);
		}
		if (wParam == VK_RIGHT)
		{
			simulate_inputs(AppInputs::B_RIGHT1, 1);
		}

		if (wParam == 'Z')
		{
			simulate_inputs(AppInputs::B_1P_1, 1);
		}
		if (wParam == 'X')
		{
			simulate_inputs(AppInputs::B_1P_2, 1);
		}
		if (wParam == 'C')
		{
			simulate_inputs(AppInputs::B_1P_3, 1);
		}
		if (wParam == 'V')
		{
			simulate_inputs(AppInputs::B_1P_4, 1);
		}
		if (wParam == 'B')
		{
			simulate_inputs(AppInputs::B_1P_5, 1);
		}
		if (wParam == 'N')
		{
			simulate_inputs(AppInputs::B_1P_6, 1);
		}
		// oled_app_button_press(1);
		break;
	case WM_KEYUP:
		if (wParam == 0x53)
		{
			simulate_inputs(AppInputs::B_SERVICE, 0);
		}
		if (wParam == 0x54)  //T
		{
			simulate_inputs(AppInputs::B_TEST, 0);
		}
		if (wParam == VK_UP) // 'Up'
		{
			simulate_inputs(AppInputs::B_UP1, 0);
		}
		if (wParam == VK_DOWN) // 'Down'
		{
			simulate_inputs(AppInputs::B_DOWN1, 0);
		}
		if (wParam == VK_LEFT) 
		{
			simulate_inputs(AppInputs::B_LEFT1, 0);
		}
		if (wParam == VK_RIGHT) 
		{
			simulate_inputs(AppInputs::B_RIGHT1, 0);
		}

		if (wParam == 'Z')
		{
			simulate_inputs(AppInputs::B_1P_1,0);
		}
		if (wParam == 'X')
		{
			simulate_inputs(AppInputs::B_1P_2,0);
		}
		if (wParam == 'C')
		{
			simulate_inputs(AppInputs::B_1P_3,0);
		}
		if (wParam == 'V')
		{
			simulate_inputs(AppInputs::B_1P_4, 0);
		}
		if (wParam == 'B')
		{
			simulate_inputs(AppInputs::B_1P_5, 0);
		}
		if (wParam == 'N')
		{
			simulate_inputs(AppInputs::B_1P_6, 0);
		}

	/*	if (wParam == 0x30) // '0'
		{
			simulate_inputs(B_DIP0, 0);
		}
		if (wParam == 0x31) // '0'
		{
			simulate_inputs(B_DIP1, 0);
		}*/
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}




static int test_counter = 0;
VOID CALLBACK MyTimerProc(HWND hWnd, UINT uTimerMsg, UINT uTimerID, DWORD dwTime)
{
	test_counter++;

	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	InvalidateRect(hWnd, &rcClient, true);
	UpdateWindow(hWnd);

}



static void colour_palette(void)
{
	static unsigned int x = 0;

	for (x = 0; x < (BMP_HEIGHT * BMP_WIDTH); x++)
	{
		unsigned short colourcode = x & 0xff;

		uint8_t red8;
		uint8_t green8;
		uint8_t blue8;


		app_global_utils::unpackRGB555_dualmodes(colourcode, &red8, &green8, &blue8);

		uint32_t red = red8;
		uint32_t green = green8;
		uint32_t blue = blue8;

		unsigned int pixel = red << 16 | green << 8 | blue;

		// Fill the bitmap. Invert the vertical as bitmaps are bottom-left first, TV is top right
		lpBitmapBits[x] = pixel;


	}

}

/*
	TV simulator.
	Take the image, scale it up and stick it on a bitmap
	Don't forget that bitmaps are upside down. Grr.
*/

static void simulate_screen_linebuffer(void)
{
#if 0
	int scanline_sim = 0;

	for (int h = 0; h <= BMP_HEIGHT; h++)
	{
		if (scanline_sim == SCALE_SCREEN)
		{
			scanline_sim = 0;

			// Linebuffer calcs for the game don't run at 3x resolution, obviously!
			
			app_main.line_interrupt(); // Execute this code every line
			
		}


		for (int w = 0; w <= BMP_WIDTH; w++)
		{
			unsigned int index_x = w / SCALE_SCREEN;
			unsigned int index_xy = index_x + (TV_WIDTH * (h / SCALE_SCREEN));
			if (index_xy >= PIXEL_COUNT)
			{
				index_xy = PIXEL_COUNT - 1;
			}

			uint16_t colourcode2 = 0;
			uint16_t colourcode1 = 0;
			uint16_t colourcode0 = 0;


			/* Todo */
			if (app_main.m_doublebuffer_line == 1)
			{				
				colourcode0 = app_main.m_scratchram.layered.layers.top_a[64 + index_x];
				colourcode2 = app_main.m_scratchram.layered.layers.bottom_a[64 + index_x];
			}
			else
			{
				colourcode0 = app_main.m_scratchram.layered.layers.top_b[64 + index_x];
				colourcode2 = app_main.m_scratchram.layered.layers.bottom_b[64 + index_x];
			}

			// 3-layer transparency (0 is top, 2 is bottom)
			uint16_t colourcode = colourcode0;

			// Transparency code is 0x0000
			/*if (colourcode == 0x0000)
			{
				colourcode = colourcode1;
			}*/

			if (colourcode == 0x0000)
			{
				colourcode = colourcode2;
			}

			if (colourcode == 0x0000)
			{
				colourcode = 0x7777; // Error - no background colour
			}


			uint8_t red8;
			uint8_t green8;
			uint8_t blue8;

			app_global_utils::unpackRGB555_dualmodes(colourcode, &red8, &green8, &blue8);

			uint32_t red = red8;
			uint32_t green = green8;
			uint32_t blue = blue8;
			// For a bit of fun, for every rescaled TV pixel, darken it for one line, then over-brighten it afterwards
			// Gives crude representation of TV scanlines
			if (scanline_sim == 0)
			{
				red = (red * 64) >> 8;
				green = (green * 64) >> 8;
				blue = (blue * 64) >> 8;

			}
			else if (scanline_sim == 1)
			{
				red = (red * 40) >> 5;
				green = (green * 40) >> 5;
				blue = (blue * 40) >> 5;

				if (red > 255)
				{
					red = 255;
				}
				if (green > 255)
				{
					green = 255;
				}
				if (blue > 255)
				{
					blue = 255;
				}

			}

			unsigned int pixel = red << 16 | green << 8 | blue;

			// Fill the bitmap. Invert the vertical as bitmaps are bottom-left first, TV is top right
			lpBitmapBits[w + ((BMP_HEIGHT - 1) * BMP_WIDTH) - (h * BMP_WIDTH)] = pixel;

		}
		scanline_sim++;
		
	}
#endif
}


static void simulate_screen_framebuffer(void)
{
	int scanline_sim = 0;

	for (int h = 0; h <= BMP_HEIGHT; h++)
	{
		if (scanline_sim == SCALE_SCREEN)
		{
			scanline_sim = 0;

			// Linebuffer calcs for the game don't run at 3x resolution, obviously!

			//	app_main.line_interrupt(); // Execute this code every line

		}


		for (int w = 0; w <= BMP_WIDTH; w++)
		{
			unsigned int index_x = w / SCALE_SCREEN;
			unsigned int index_xy = index_x + (TV_WIDTH * (h / SCALE_SCREEN));
			if (index_xy >= PIXEL_COUNT)
			{
				index_xy = PIXEL_COUNT - 1;
			}


			uint16_t colourcode = 0;

			colourcode = app_main.m_framebufferpointer[index_xy];
	
			
			if (colourcode == 0x0000)
			{
				colourcode = 0x7777; // Error - no background colour
			}

			uint8_t red8;
			uint8_t green8;
			uint8_t blue8;

			app_global_utils::unpackRGB555_dualmodes(colourcode, &red8, &green8, &blue8);

			uint32_t red = red8;
			uint32_t green = green8;
			uint32_t blue = blue8;
			// For a bit of fun, for every rescaled TV pixel, darken it for one line, then over-brighten it afterwards
			// Gives crude representation of TV scanlines
			if (scanline_sim == 0)
			{
				red = (red * 64) >> 8;
				green = (green * 64) >> 8;
				blue = (blue * 64) >> 8;

			}
			else if (scanline_sim == 1)
			{
				red = (red * 40) >> 5;
				green = (green * 40) >> 5;
				blue = (blue * 40) >> 5;

				if (red > 255)
				{
					red = 255;
				}
				if (green > 255)
				{
					green = 255;
				}
				if (blue > 255)
				{
					blue = 255;
				}

			}

			unsigned int pixel = red << 16 | green << 8 | blue;

			// Fill the bitmap. Invert the vertical as bitmaps are bottom-left first, TV is top right
			lpBitmapBits[w + ((BMP_HEIGHT - 1) * BMP_WIDTH) - (h * BMP_WIDTH)] = pixel;

		}
		scanline_sim++;

	}

}




// The 'inputword'. Comes from JAMMA inputs via SPI on Pico
// This lets keyboard presses simulate JAMMA buttons being pressed
static void simulate_inputs(AppInputs::inputs_t button, BOOL pressed)
{
	if (pressed)
	{
		inputword &= ~(1u << button);
	}
	else
	{
		inputword |= (1u << button);

	}
}

static void toggle_inputs(AppInputs::inputs_t button)
{
	inputword ^= (1u << button);
}



// The 'inputword'. Comes from JAMMA inputs via SPI on Pico
uint32_t hal_spi_get_inputs(void)
{
	return (inputword);
}



// Simulate analogue channels (for measuring voltages)
int16_t hal_adc_get(int gpio)
{
	int16_t result = 0;
	switch (gpio)
	{
	case ADC_12V:
		result = (short)(1024.0f * 12.0f);
		break;
	case ADC_NEG5V:
		result = (short)(1024.0f * -5.0f);
		break;
	case ADC_5V:
		result = (short)(1024.0f * 5.0f);
		break;

	default:
		//NOT AN ADC PIN
		break;
	}

	return (result);

}


#if 0

void setVideoMode(vid_mode_t mode)
{

	vidmode = mode;
}
#endif

void HAL_video_setLinebuffer(void)
{
	g_linebuffer = 1;
}


void HAL_video_setFramebuffer(void)
{
	g_linebuffer = 0;
}

// Called from a thread to emulate CPU2
void hal_idle_cpu2(void)
{
	while (1)
	{
		app_main.subcpuloop();
	}
}