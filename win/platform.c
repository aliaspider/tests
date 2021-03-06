
#include <windows.h>

#include "platform.h"
#include "input.h"
#include "video.h"

platform_t platform;

bool created;

static LRESULT CALLBACK wndproc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch (msg)
   {
   case WM_CREATE:
      created = true;
      printf("create\n"); fflush(stdout);
      break;

   case WM_DESTROY:
       DestroyWindow(hWnd);
      input.pad.meta.exit = true;
      PostQuitMessage(0);
      break;

   case WM_PAINT:
      ValidateRect(hWnd, NULL);
      break;

   case WM_SIZE:
      for(int i = 0; i < video.screen_count; i++)
      {
         if (video.screens[i].hwnd == hWnd)
         {
            video.screens[i].width = LOWORD(lParam);
            video.screens[i].height = HIWORD(lParam);
            break;
         }
      }
      break;

   default:
      break;
   }

   return DefWindowProc(hWnd, msg, wParam, lParam);
}

void platform_init()
{
   platform.hInstance = GetModuleHandle(NULL);
   WNDCLASSEX wndclass =
   {
      .cbSize = sizeof(WNDCLASSEX),
      .style = CS_HREDRAW | CS_VREDRAW,
      .lpfnWndProc = wndproc,
      .cbClsExtra = 0,
      .cbWndExtra = 0,
      .hInstance = platform.hInstance,
      .hIcon = LoadIcon(platform.hInstance, MAKEINTRESOURCE(IDI_APPLICATION)),
      .hCursor = LoadCursor(NULL, IDC_ARROW),
      .hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
//      .lpszMenuName = NULL,
      .lpszClassName = "myWndClass",
      .hIconSm = LoadIcon(platform.hInstance, MAKEINTRESOURCE(IDI_APPLICATION)),
   };
   RegisterClassEx(&wndclass);

   for (int i = video.screen_count - 1; i >= 0; i--)
   {
      video.screens[i].hinstance = platform.hInstance;
      video.screens[i].hwnd = CreateWindow(wndclass.lpszClassName, "Vulkan Test",
                                           WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
                                           video.screens[i].x, video.screens[i].y, video.screens[i].width, video.screens[i].height,
                                           NULL, NULL, platform.hInstance, NULL);
   }
}

void platform_destroy()
{
   for (int i = 0; i < video.screen_count; i++)
   {
      video.screens[i].hinstance = NULL;
//      DestroyWindow(video.screens[i].hwnd);
      video.screens[i].hwnd = NULL;
   }
}

void platform_update()
{
   MSG msg;

   while (PeekMessage(&msg, NULL, 0, 0, 1))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
}
