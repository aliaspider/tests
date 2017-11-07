#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#define GL_GLEXT_PROTOTYPES
#define WGL_WGLEXT_PROTOTYPES
#include <GL/GL.h>
#include <GL/glext.h>
#ifdef __WIN32__
#include <GL/wglext.h>
#endif

#include "common.h"
#include "video.h"
#include "input.h"

HGLRC hRC;
HDC hDC;

UINT uiVBO[4];
UINT uiVAO[2];

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
   SetPixelFormat(hDC, iPixelFormat, &pfd);

   hRC = wglCreateContext(hDC);
   wglMakeCurrent(hDC, hRC);
   glGetProcAddress(NULL); /* will init all pfns */


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
   }
   wglSwapIntervalEXT(0);
   glClearColor(0.0f, 0.0f, 0.5f, 1.0f);

   float fTriangle[9];
   float fQuad[12];
   float fTriangleColor[9];
   float fQuadColor[12];

	// Setup triangle vertices
	fTriangle[0] = -0.4f; fTriangle[1] = 0.1f; fTriangle[2] = 0.0f;
	fTriangle[3] = 0.4f; fTriangle[4] = 0.1f; fTriangle[5] = 0.0f;
	fTriangle[6] = 0.0f; fTriangle[7] = 0.7f; fTriangle[8] = 0.0f;

	// Setup triangle color

	fTriangleColor[0] = 1.0f; fTriangleColor[1] = 0.0f; fTriangleColor[2] = 0.0f;
	fTriangleColor[3] = 0.0f; fTriangleColor[4] = 1.0f; fTriangleColor[5] = 0.0f;
	fTriangleColor[6] = 0.0f; fTriangleColor[7] = 0.0f; fTriangleColor[8] = 1.0f;

	// Setup quad vertices

	fQuad[0] = -0.2f; fQuad[1] = -0.1f; fQuad[2] = 0.0f;
	fQuad[3] = -0.2f; fQuad[4] = -0.6f; fQuad[5] = 0.0f;
	fQuad[6] = 0.2f; fQuad[7] = -0.1f; fQuad[8] = 0.0f;
	fQuad[9] = 0.2f; fQuad[10] = -0.6f; fQuad[11] = 0.0f;

	// Setup quad color

	fQuadColor[0] = 1.0f; fQuadColor[1] = 0.0f; fQuadColor[2] = 0.0f;
	fQuadColor[3] = 0.0f; fQuadColor[4] = 1.0f; fQuadColor[8] = 0.0f;
	fQuadColor[6] = 0.0f; fQuadColor[7] = 0.0f; fQuadColor[5] = 1.0f;
	fQuadColor[9] = 1.0f; fQuadColor[10] = 1.0f; fQuadColor[11] = 0.0f;

	glGenVertexArrays(2, uiVAO); // Generate two VAOs, one for triangle and one for quad
	glGenBuffers(4, uiVBO); // And four VBOs

	// Setup whole triangle
	glBindVertexArray(uiVAO[0]);

	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), fTriangle, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), fTriangleColor, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Setup whole quad
	glBindVertexArray(uiVAO[1]);

	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[2]);
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(float), fQuad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[3]);
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(float), fQuadColor, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Load shaders and create shader program

   GLint success;

   GLuint vs = glCreateShader(GL_VERTEX_SHADER);
#define SHADER_SOURCE(x) "#version 330\n" #x
   const char* vs_code = SHADER_SOURCE(
                              layout (location = 0) in vec3 inPosition;
                              layout (location = 1) in vec3 inColor;
                              smooth out vec3 theColor;
                              void main()
                              {
                                 gl_Position = vec4(inPosition.yxz, 1.0);
                                 theColor = inColor;
                              }
                           );
   glShaderSource(vs, 1, &vs_code, NULL);
   glCompileShader(vs);
   glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
   if(!success)
   {
      char infolog[512];
      glGetShaderInfoLog(vs, sizeof(infolog), NULL, infolog);
      DEBUG_STR(infolog);
      assert(0);
   }

   GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
#define SHADER_SOURCE(x) "#version 330\n" #x
   const char* fs_code = SHADER_SOURCE(
                            smooth in vec3 theColor;
                            out vec4 outputColor;
                            void main()
                            {
                               outputColor = vec4(theColor, 1.0);
                            }
                         );
   glShaderSource(fs, 1, &fs_code, NULL);
   glCompileShader(fs);
   glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
   if(!success)
   {
      char infolog[512];
      glGetShaderInfoLog(vs, sizeof(infolog), NULL, infolog);
      DEBUG_STR(infolog);
      assert(0);
   }
   GLuint program = glCreateProgram();
   glAttachShader(program, vs);
   glAttachShader(program, fs);
   glLinkProgram(program);
   glGetProgramiv(program, GL_LINK_STATUS, &success);
   if(!success)
   {
      char infolog[512];
      glGetShaderInfoLog(vs, sizeof(infolog), NULL, infolog);
      DEBUG_STR(infolog);
      assert(0);
   }
   glUseProgram(program);
}


static void video_render()
{
//   DEBUG_LINE();
	glClear(GL_COLOR_BUFFER_BIT);

//	glEnableVertexAttribArray(0);
//	// Triangle render
//	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
//	glDrawArrays(GL_TRIANGLES, 0, 3);

//	// Quad render using triangle strip
//	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[1]);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(uiVAO[0]);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(uiVAO[1]);
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

