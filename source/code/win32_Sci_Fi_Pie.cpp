#include <windows.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <XInput.h>

#define internal static;
#define GLOBAL_VARIABLE static;

#define KiloBytes(value) (value * 1024)
#define MegaBytes(value) (KiloBytes(value) * 1024)
#define GigaBytes(value) (MegaBytes(value) * 1024)

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define s8 int8_t
#define s16  int16_t
#define s32  int32_t
#define s64  int64_t
#define f32 float
#define d64 double

#include "Sci_Fi_Pie.cpp"

#pragma pack(push, 1)
struct BitmapHeader
{
	u16   FileType;    
	u32   FileSize;    
	u16   Reserved1;   
	u16   Reserved2;   
	u32   BitmapOffset;
	u32   Size;            
	s32    Width;           
	s32    Height;          
	u16   Planes;          
	u16   BitsPerPixel;    
};
#pragma pack(pop)

struct Win32BackBuffer
{
	BITMAPINFO Info;
	void* Memory;
	int Pitch;
	int BytesPerPixel;
	int Width;
	int Height;
	int Size;
};

struct WindowDimensions
{
	int Width;
	int Height;
};

struct LoadedBitmap
{
	int Width;
	int Height;
	int Pitch;
	u8* Memory;
};

GLOBAL_VARIABLE Win32BackBuffer GLOBAL_BACK_BUFFER;
GLOBAL_VARIABLE bool RUNNING;
GLOBAL_VARIABLE s64 COUNTER_FREQUENCY;
GLOBAL_VARIABLE f32 TARGET_MS_PER_FRAME;

internal void Win32ProcessControllerMessage(GameButton* button, bool isDown)
{	
	button->EndedDown = isDown;
}

internal void Win32ProcessMessages(GameInput* gameInput)
{

	DWORD controllerResult;    
	for (DWORD i=0; i < MAX_CONTROLLERS; i++)
	{
     	XINPUT_STATE state;
     	controllerResult = XInputGetState(i, &state);

      if(controllerResult == ERROR_SUCCESS)
	  {
        // Controller is connected 
     	gameInput->GameControllers[i].isAnalogue = true;

		gameInput->GameControllers[i].StickX = state.Gamepad.sThumbLX;
		gameInput->GameControllers[i].StickY = state.Gamepad.sThumbLY;

	  	XINPUT_KEYSTROKE button = {};
       	XInputGetKeystroke(i, 0, &button);

       	bool isDown = (button.Flags == XINPUT_KEYSTROKE_KEYDOWN);
       	bool wasDown = (button.Flags == XINPUT_KEYSTROKE_KEYUP);

       	if(wasDown != isDown)
       	{
	        switch(button.VirtualKey)
	        {
	        	case VK_PAD_DPAD_UP:
	        	{
	        		Win32ProcessControllerMessage(&gameInput->GameControllers[i].Up, isDown);
	        		break;
	        	}	
				case VK_PAD_DPAD_DOWN:	
	        	{
	        		Win32ProcessControllerMessage(&gameInput->GameControllers[i].Down, isDown);
	        		break;
	        	}	
				case VK_PAD_DPAD_LEFT:	
	        	{
	        		Win32ProcessControllerMessage(&gameInput->GameControllers[i].Left, isDown);
	        		break;
	        	}
				case VK_PAD_DPAD_RIGHT:	
	        	{
	        		Win32ProcessControllerMessage(&gameInput->GameControllers[i].Right, isDown);
	        		break;
	        	}
	        	case VK_PAD_A:
	        	{
	        		Win32ProcessControllerMessage(&gameInput->GameControllers[i].Action1, isDown);
	        		int dummy = 0;
	        		break;
	        	}	
				case VK_PAD_B:	
	        	{
	        		Win32ProcessControllerMessage(&gameInput->GameControllers[i].Action2, isDown);
	        		break;
	        	}	
				case VK_PAD_X:	
	        	{
	        		Win32ProcessControllerMessage(&gameInput->GameControllers[i].Action3, isDown);
	        		break;
	        	}
				case VK_PAD_Y:	
	        	{
	        		Win32ProcessControllerMessage(&gameInput->GameControllers[i].Action4, isDown);
	        		break;
	        	}	
	
	        }
       	}
	  }
      else
	  {
        // Controller is not connected 
	  }
	}

	MSG message;
	while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		switch(message.message)
		{
			case WM_QUIT:
			{
			RUNNING = false;
			break;				
			}

			case WM_LBUTTONUP:
			{
				gameInput->Mouse.LMB.EndedDown = false;
			}

			case WM_RBUTTONUP:
			{
				gameInput->Mouse.RMB.EndedDown = false;
			}

			case WM_LBUTTONDOWN:
			{
				u32 mousePosition = (u32)message.lParam;

				gameInput->Mouse.PosX = (u16)mousePosition;
				gameInput->Mouse.PosY = (u16)(mousePosition >> 16);
				gameInput->Mouse.LMB.EndedDown = true;				
			}
			case WM_RBUTTONDOWN:
			{
				u32 mousePosition = (u32)message.lParam;
					
				gameInput->Mouse.PosX = (u16)mousePosition;
				gameInput->Mouse.PosY = (u16)(mousePosition >> 16);
				gameInput->Mouse.RMB.EndedDown = true;
			}

			case WM_SYSKEYUP:
			case WM_KEYUP:
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				u32 VKCode = (u32)message.wParam;
				bool wasDown = (((u32)message.lParam & (1 << 30)) != 0);
				bool isDown = (((u32)message.lParam & (1 << 31)) == 0);
				
				if(wasDown != isDown)
				{
					switch(VKCode)
					{

						case 'W':
						case VK_UP:
						{
							Win32ProcessControllerMessage(&gameInput->GameControllers[KEYBOARD_CONTROLLER].Up, isDown);
							break;					
						}

						case 'S':
						case VK_DOWN:
						{
							Win32ProcessControllerMessage(&gameInput->GameControllers[KEYBOARD_CONTROLLER].Down, isDown);
							break;					
						}

						case 'A':
						case VK_LEFT:
						{
							Win32ProcessControllerMessage(&gameInput->GameControllers[KEYBOARD_CONTROLLER].Left, isDown);
							break;					
						}

						case 'D':
						case VK_RIGHT:
						{
							Win32ProcessControllerMessage(&gameInput->GameControllers[KEYBOARD_CONTROLLER].Right, isDown);
							break;					
						}

						case 'Q':
						{
							Win32ProcessControllerMessage(&gameInput->GameControllers[KEYBOARD_CONTROLLER].Action1, isDown);
							break;					
						}

						case 'E':
						{
							Win32ProcessControllerMessage(&gameInput->GameControllers[KEYBOARD_CONTROLLER].Action2, isDown);
							break;					
						}

						case 'R':
						{
							Win32ProcessControllerMessage(&gameInput->GameControllers[KEYBOARD_CONTROLLER].Action3, isDown);
							break;					
						}

						case VK_SPACE:
						{
							Win32ProcessControllerMessage(&gameInput->GameControllers[KEYBOARD_CONTROLLER].Action4, isDown);
							break;					
						}

						case VK_SHIFT:
						{
							OutputDebugStringA("SHIFT\n");
							break;					
						}

						case VK_CONTROL:
						{
							OutputDebugStringA("CTRL\n");
							break;					
						}

						default:
						{

						}
					}
				}
			}
		}
		
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

}

internal LoadedBitmap Win32LoadBitmap(char* fileName)
{	                         
	void* fileHandle = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0,
								   OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	u32 fileSize = GetFileSize(fileHandle, 0);
	void* fileMemory = VirtualAlloc(0, fileSize, MEM_COMMIT, PAGE_READWRITE);
	bool readResult = ReadFile(fileHandle, fileMemory, fileSize, 0, 0);	
	assert(readResult);

	BitmapHeader* header = (BitmapHeader*)fileMemory;
	LoadedBitmap result;
	result.Height = header->Height;
	result.Width = header->Width;
	result.Memory = (u8*)fileMemory + header->BitmapOffset;
	result.Pitch = (header->BitsPerPixel/8)*(header->Width);

	u32* Pixel = (u32*)fileMemory;
	int bitmapSize = header->Height * header->Width;

	for(int i = 0; i < bitmapSize; i++)
	{
		*Pixel++ = ((*Pixel >> 8) | (*Pixel << 24));		
	}
	
	return result;
}

internal void Win32DrawBitmap(Win32BackBuffer* buffer, LoadedBitmap bitmap, int xCoord, int yCoord)
{
		if(bitmap.Width > buffer->Width)
		{
			bitmap.Width = buffer->Width;
		}
		else if(bitmap.Height > buffer->Height)
		{
			bitmap.Height = buffer->Height;
		}

		int x1 = xCoord;
		int x2 = xCoord + bitmap.Width;

		int y1 = yCoord;
		int y2 = yCoord + bitmap.Height;

		int yOffset = 0;
		if(y1 < 0)
		{
			yOffset = -y1;
			y1 = 0;
		}
		if(y2 > buffer->Height)
		{
			y2 = buffer->Height;
		}

		int xOffset = 0;
		if(x1 < 0)
		{
			xOffset = -x1;
			x1 = 0;
		}
		if(x2 > buffer->Width)
		{
			x2 = buffer->Width;
		}

		u8* bufferRow = (u8*)buffer->Memory + (x1 * buffer->BytesPerPixel) + (y1 * buffer->Pitch); 
		u8* bitmapRow = (u8*)bitmap.Memory + (xOffset * buffer->BytesPerPixel) + (yOffset * bitmap.Pitch);

		for(int y = y1; y < y2; y++)
		{
			u32* bufferPixel = (u32*)bufferRow;
			u32* bitmapPixel = (u32*)bitmapRow;

			for(int x = x1; x < x2; x++)
			{

				f32 BA= (*bitmapPixel >> 24);
				f32 T = BA/255;

				u8 alphaB = (*bitmapPixel << 0 >> 24);				
				u8 redB = (*bitmapPixel << 8 >> 24);
				u8 greenB = (*bitmapPixel << 16 >> 24);
				u8 blueB = (*bitmapPixel << 24 >> 24);

				u8 alphaA = (*bufferPixel << 0 >> 24);				
				u8 redA = (*bufferPixel << 8 >> 24);
				u8 greenA = (*bufferPixel << 16 >> 24);
				u8 blueA = (*bufferPixel << 24 >> 24);

				u8 alphaAB = alphaA + ((alphaB - alphaA) * T ); 
				u8 redAB = redA + ((redB - redA) * T ); 
				u8 greenAB = greenA + ((greenB - greenA) * T);
				u8 blueAB = blueA + ((blueB - blueA) * T);

				*bufferPixel = ((alphaAB << 24) | (redAB << 16) | (greenAB << 8) | (blueAB << 0));					
		
				bitmapPixel++;
				bufferPixel++;
			}
			bufferRow += buffer->Pitch;
			bitmapRow += bitmap.Pitch;
		}
}

internal void Win32ClearWin32BackBuffer(Win32BackBuffer* buffer)
{
	u32* Pixel = (u32*)(buffer->Memory) + buffer->Size;
	for(s32 i = 0; i < buffer->Size; i++)
	{
		*Pixel-- = 0;
	}
}

internal void  Win32DisplayWin32BackBuffer(Win32BackBuffer buffer, HDC deviceContext, int width, int height)
{
	u32 offSetX = 20;
	u32 offSetY = 0;

	BitBlt(deviceContext, 0, 0, offSetX, height, deviceContext, 0, 0, BLACKNESS);
	BitBlt(deviceContext, 0, 0, width, offSetY * 2, deviceContext, 0, 0, BLACKNESS);
	BitBlt(deviceContext, buffer.Width, 0, width, height, deviceContext, 0, 0, BLACKNESS);
	BitBlt(deviceContext, 0, buffer.Height, width, height, deviceContext, 0, 0, BLACKNESS);

	StretchDIBits(deviceContext, offSetX, offSetY, buffer.Width, buffer.Height,
								 0, 0, buffer.Width, buffer.Height,
								 buffer.Memory, &buffer.Info, DIB_RGB_COLORS, SRCCOPY);
}

internal WindowDimensions Win32GetWindowDimensions(HWND windowHandle)
{
	WindowDimensions result;
	RECT clientRect; 
	GetClientRect(windowHandle, &clientRect);
	result.Width = clientRect.right - clientRect.left;
	result.Height = clientRect.bottom - clientRect.top;

	return result;
}

internal void Win32InitializeWin32BackBuffer(Win32BackBuffer* buffer, int width, int height)
{
	if(buffer->Memory)
	{
		VirtualFree(buffer->Memory, 0, MEM_RELEASE);
	}

  	buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);;
	buffer->Info.bmiHeader.biWidth = width;
	buffer->Info.bmiHeader.biHeight = -height;
	buffer->Info.bmiHeader.biPlanes = 1;
	buffer->Info.bmiHeader.biBitCount = 32;
	buffer->Info.bmiHeader.biCompression = BI_RGB;

	buffer->BytesPerPixel = 4;
	buffer->Pitch = buffer->Info.bmiHeader.biWidth * buffer->BytesPerPixel;

	buffer->Width = width;
	buffer->Height = height;
	buffer->Size = buffer->Width * buffer->Height;

	int bufferSize = (buffer->Info.bmiHeader.biWidth * buffer->Info.bmiHeader.biHeight * -1) * buffer->BytesPerPixel; 
	buffer->Memory = VirtualAlloc(0, bufferSize, MEM_COMMIT, PAGE_READWRITE);
}

internal GameBackBuffer Win32CreateGameBackBuffer(Win32BackBuffer* buffer)
{
	GameBackBuffer gameBuffer = {};
	gameBuffer.Pitch = GLOBAL_BACK_BUFFER.Pitch;
	gameBuffer.Memory = GLOBAL_BACK_BUFFER.Memory;
	gameBuffer.BytesPerPixel = GLOBAL_BACK_BUFFER.BytesPerPixel;
	gameBuffer.Width = GLOBAL_BACK_BUFFER.Width;
	gameBuffer.Height = GLOBAL_BACK_BUFFER.Height;

	return gameBuffer;
}

internal GameMemory Win32CreateGameMemory()
{
	GameMemory gameMemory = {};
	gameMemory.MainMemorySize = MegaBytes((u64)64);
	gameMemory.TempMemorySize = MegaBytes((u64)256); 
	gameMemory.MainMemory = VirtualAlloc(0, gameMemory.MainMemorySize, MEM_COMMIT, PAGE_READWRITE);
	gameMemory.TempMemory = VirtualAlloc(0, gameMemory.TempMemorySize, MEM_COMMIT, PAGE_READWRITE);

	return gameMemory;
}

inline LARGE_INTEGER Win32GetClockCounter()
{
	LARGE_INTEGER endCounter;
	QueryPerformanceCounter(&endCounter); 

	return endCounter;
}

inline f32 Win32GetMiliSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
	f32 result = (1000.0f * ((f32)(End.QuadPart - Start.QuadPart) / (f32)(COUNTER_FREQUENCY))); 
	return result;
}


LRESULT CALLBACK WndProc(  
   HWND   windowHandle,  
   UINT   message,  
   WPARAM wParam,  
   LPARAM lParam  
)
{
	//Message handling
	LRESULT result = 0;

	switch(message)
	{
		case WM_SIZE:
		{
			break;
		}

		case WM_DESTROY:
		{
			//handle this with error to user
			RUNNING = false;
			break;
		}

		case WM_CLOSE:
		{
			//handle this with message to user
			RUNNING = false;
			break;
		}

		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
			break;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			WindowDimensions Dimensions = Win32GetWindowDimensions(windowHandle);

			HDC DeviceContext = BeginPaint(windowHandle, &Paint);

			Win32DisplayWin32BackBuffer(GLOBAL_BACK_BUFFER, DeviceContext, Dimensions.Width, Dimensions.Height);

			EndPaint(windowHandle, &Paint);
		}

		case WM_INPUT:
		{
			OutputDebugStringA("WM_INPUT\n");
		}
		default:
		{
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		}

		return result;
	}
}  

int CALLBACK WinMain(
   HINSTANCE instance,
   HINSTANCE prevInstance,
   LPSTR     commandLine,
   int       commandShow
)
{
	WNDCLASS windowClass = {};

	windowClass.style = CS_VREDRAW | CS_HREDRAW;
	windowClass.lpfnWndProc = WndProc;
	windowClass.hInstance = instance;
	//windowClass.hIcon;
	windowClass.lpszClassName = "windowClass";

	Win32InitializeWin32BackBuffer(&GLOBAL_BACK_BUFFER, 980, 560);

	GameBackBuffer gameBuffer = Win32CreateGameBackBuffer(&GLOBAL_BACK_BUFFER);;

	GameMemory gameMemory = Win32CreateGameMemory();

	GameInput gameInput = {};
	// LoadedBitmap background = Win32LoadBitmap("background_test.bmp");
	
	//TIMING CODE
	LARGE_INTEGER counterFrequencyResult;
	QueryPerformanceFrequency(&counterFrequencyResult);
	COUNTER_FREQUENCY = counterFrequencyResult.QuadPart;
	TARGET_MS_PER_FRAME = 33.33f;
	u8 TARGET_MS_OF_SCHEDULER = 1;
	MMRESULT schedulerTimeResult = timeBeginPeriod(TARGET_MS_OF_SCHEDULER);

	if(RegisterClass(&windowClass))	
	{
		HWND window = CreateWindowEx(
			0,
			windowClass.lpszClassName,
			"Sci-Fi-Pie",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			instance,
			0
			);

		if(window)
		{
			LARGE_INTEGER lastCounter = Win32GetClockCounter();

			RUNNING = true;
			while(RUNNING)
			{
				Win32ProcessMessages(&gameInput);

				WindowDimensions windowDimension = Win32GetWindowDimensions(window);
				// RECT windowRect;
				// GetWindowRect(window, &windowRect);
				// ClipCursor(&windowRect);

				HDC deviceContext = GetDC(window);
				Win32ClearWin32BackBuffer(&GLOBAL_BACK_BUFFER);

				GameLoop(&gameBuffer, &gameInput, &gameMemory);

				f32 secondsElapsed = Win32GetMiliSecondsElapsed(lastCounter, Win32GetClockCounter());

				if(secondsElapsed < TARGET_MS_PER_FRAME)
				{					
					while(secondsElapsed < (TARGET_MS_PER_FRAME - 5))
					{
						if (schedulerTimeResult)
						{
							DWORD timeToSleep = TARGET_MS_PER_FRAME - secondsElapsed;
							if (timeToSleep > 0)
							{
								Sleep(timeToSleep);
							}
						}

						secondsElapsed = Win32GetMiliSecondsElapsed(lastCounter, Win32GetClockCounter());
					}										
				}
				else
				{
					//missed target frame rate
				}

				Win32DisplayWin32BackBuffer(GLOBAL_BACK_BUFFER, deviceContext, windowDimension.Width, windowDimension.Height);
				ReleaseDC(window, deviceContext);

				f32 mspf = Win32GetMiliSecondsElapsed(lastCounter, Win32GetClockCounter());
				char charBuffer[256] = {};
				sprintf(charBuffer, "%.2f: ms\n", mspf);
				OutputDebugStringA(charBuffer);

				lastCounter = Win32GetClockCounter();
			}
		}
		{
			//Error Creating Window 
		}
	}
	else
	{
		//Error Registering window Class
	}

	return(0);
}