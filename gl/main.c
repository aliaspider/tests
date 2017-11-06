#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#define GL_GLEXT_PROTOTYPES
#define WGL_WGLEXT_PROTOTYPES
#include <GL/GL.h>
#include <GL/glext.h>
#include <GL/wglext.h>

#include "common.h"
#include "video.h"
#include "input.h"


UINT uiVBO[2];

HGLRC hRC;
HDC hDC;


void* glGetProcAddress(const char* name);

static void video_init()
{
   DEBUG_LINE();
   hDC = GetDC(video.screens[0].hwnd);
   PIXELFORMATDESCRIPTOR pfd =
   {
      sizeof(PIXELFORMATDESCRIPTOR),
      .nVersion   = 1,
      .dwFlags    = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW,
      .iPixelType = PFD_TYPE_RGBA,
      .cColorBits = 32,
      .cDepthBits = 32,
      .iLayerType = PFD_MAIN_PLANE,
   };
   int iPixelFormat = ChoosePixelFormat(hDC, &pfd);
   assert(iPixelFormat);
   (void)iPixelFormat;

   SetPixelFormat(hDC, iPixelFormat, &pfd);

   hRC = wglCreateContext(hDC);
   wglMakeCurrent(hDC, hRC);
   glGetProcAddress(NULL);


      if(0)
   {
      const int iPixelFormatAttribList[] =
      {
         WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
         WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
         WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
         WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
         WGL_COLOR_BITS_ARB, 32,
         WGL_DEPTH_BITS_ARB, 24,
         WGL_STENCIL_BITS_ARB, 8,
         0 // End of attributes list
      };
      int iContextAttribs[] =
      {
         WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
         WGL_CONTEXT_MINOR_VERSION_ARB, 0,
         WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
         0 // End of attributes list
      };

      int iPixelFormat, iNumFormats;
      wglChoosePixelFormatARB(hDC, iPixelFormatAttribList, NULL, 1, &iPixelFormat, (UINT*)&iNumFormats);

      SetPixelFormat(hDC, iPixelFormat, &pfd);

      wglMakeCurrent(NULL, NULL);
      wglDeleteContext(hRC);
      hRC = wglCreateContextAttribsARB(hDC, 0, iContextAttribs);
      assert(hRC);
      wglMakeCurrent(hDC, hRC);
//      GL_PROCS
   }
   wglMakeCurrent(hDC, hRC);

   float fTriangle[9]; // Data to render triangle (3 vertices, each has 3 floats)
   float fQuad[12]; // Data to render quad using triangle strips (4 vertices, each has 3 floats)

      glClearColor(0.0f, 0.5f, 1.0f, 1.0f);

      // Setup triangle vertices
      fTriangle[0] = -0.4f; fTriangle[1] = 0.1f; fTriangle[2] = 0.0f;
      fTriangle[3] = 0.4f; fTriangle[4] = 0.1f; fTriangle[5] = 0.0f;
      fTriangle[6] = 0.0f; fTriangle[7] = 0.7f; fTriangle[8] = 0.0f;

      // Setup quad vertices

      fQuad[0] = -0.2f; fQuad[1] = -0.1f; fQuad[2] = 0.0f;
      fQuad[3] = -0.2f; fQuad[4] = -0.6f; fQuad[5] = 0.0f;
      fQuad[6] = 0.2f; fQuad[7] = -0.1f; fQuad[8] = 0.0f;
      fQuad[9] = 0.2f; fQuad[10] = -0.6f; fQuad[11] = 0.0f;

      // Now we create two VBOs
      glGenBuffers(2, uiVBO);

      glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
      glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), fTriangle, GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, uiVBO[1]);
      glBufferData(GL_ARRAY_BUFFER, 12*sizeof(float), fQuad, GL_STATIC_DRAW);

      wglSwapIntervalEXT(2);
}


static void video_render()
{
//   DEBUG_LINE();
   // We just clear color
	glClear(GL_COLOR_BUFFER_BIT);

	glEnableVertexAttribArray(0);
	// Triangle render
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Quad render using triangle strip
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   SwapBuffers(hDC);
}

static void video_destroy()
{
   DEBUG_LINE();
   wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
}

static void video_register_draw_command(int screen_id, draw_command_t fn)
{
}

const video_t video_gl =
{
   .init                  = video_init,
   .render                = video_render,
   .destroy               = video_destroy,
   .register_draw_command = video_register_draw_command,
};

