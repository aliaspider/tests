#define GL_GLEXT_PROTOTYPES
#define WGL_WGLEXT_PROTOTYPES

#include <GL/GL.h>
#include <GL/glext.h>
#include <GL/wglext.h>

#define CNT_ARGS(...) CNT_ARGS_(__VA_ARGS__,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)
#define CNT_ARGS_(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,n,...) n


#define DROP_TYPE(...) DROP_TYPE_(CNT_ARGS(__VA_ARGS__),__VA_ARGS__)
#define DROP_TYPE_(n,...) DROP_TYPE__(n,__VA_ARGS__)
#define DROP_TYPE__(n,...) DROP_TYPE_##n(__VA_ARGS__)
#define DROP_TYPE_1()
#define DROP_TYPE_2(ptype,pname)      pname
#define DROP_TYPE_4(ptype,pname,...)  pname, DROP_TYPE_2(__VA_ARGS__)
#define DROP_TYPE_6(ptype,pname,...)  pname, DROP_TYPE_4(__VA_ARGS__)
#define DROP_TYPE_8(ptype,pname,...)  pname, DROP_TYPE_6(__VA_ARGS__)
#define DROP_TYPE_10(ptype,pname,...) pname, DROP_TYPE_8(__VA_ARGS__)
#define DROP_TYPE_12(ptype,pname,...) pname, DROP_TYPE_10(__VA_ARGS__)
#define DROP_TYPE_14(ptype,pname,...) pname, DROP_TYPE_12(__VA_ARGS__)
#define DROP_TYPE_16(ptype,pname,...) pname, DROP_TYPE_14(__VA_ARGS__)
#define DROP_TYPE_18(ptype,pname,...) pname, DROP_TYPE_16(__VA_ARGS__)

#define MERGE_TYPE(...) MERGE_TYPE_(CNT_ARGS(__VA_ARGS__),__VA_ARGS__)
#define MERGE_TYPE_(n,...) MERGE_TYPE__(n,__VA_ARGS__)
#define MERGE_TYPE__(n,...) MERGE_TYPE_##n(__VA_ARGS__)
#define MERGE_TYPE_1()
#define MERGE_TYPE_2(ptype,pname)      ptype pname
#define MERGE_TYPE_4(ptype,pname,...)  ptype pname, MERGE_TYPE_2(__VA_ARGS__)
#define MERGE_TYPE_6(ptype,pname,...)  ptype pname, MERGE_TYPE_4(__VA_ARGS__)
#define MERGE_TYPE_8(ptype,pname,...)  ptype pname, MERGE_TYPE_6(__VA_ARGS__)
#define MERGE_TYPE_10(ptype,pname,...) ptype pname, MERGE_TYPE_8(__VA_ARGS__)
#define MERGE_TYPE_12(ptype,pname,...) ptype pname, MERGE_TYPE_10(__VA_ARGS__)
#define MERGE_TYPE_14(ptype,pname,...) ptype pname, MERGE_TYPE_12(__VA_ARGS__)
#define MERGE_TYPE_16(ptype,pname,...) ptype pname, MERGE_TYPE_14(__VA_ARGS__)
#define MERGE_TYPE_18(ptype,pname,...) ptype pname, MERGE_TYPE_16(__VA_ARGS__)


#define GL_PROCS \
   GL_PROC(BOOL,   WINAPI,   wglSwapIntervalEXT, int, interval);\
   GL_PROC(BOOL,   WINAPI,   wglChoosePixelFormatARB, HDC, hdc, const int *,piAttribIList, const FLOAT *,pfAttribFList, UINT, nMaxFormats, int *,piFormats, UINT *,nNumFormats);\
   GL_PROC(HGLRC,  WINAPI,   wglCreateContextAttribsARB, HDC, hDC, HGLRC, hShareContext, const int *,attribList);\
   GL_PROC(void,   APIENTRY, glGenBuffers ,GLsizei, n, GLuint*,buffers);\
   GL_PROC(void,   APIENTRY, glBindBuffer, GLenum, target, GLuint, buffer);\
   GL_PROC(void,   APIENTRY, glBufferData, GLenum, target, GLsizeiptr, size, const void *,data, GLenum, usage); \
   GL_PROC(void,   APIENTRY, glVertexAttribPointer, GLuint, index, GLint, size, GLenum, type, GLboolean, normalized, GLsizei, stride, const void *,pointer); \
   GL_PROC(void,   APIENTRY, glEnableVertexAttribArray, GLuint, index); \
   GL_PROC(void,   APIENTRY, glGenVertexArrays, GLsizei, n, GLuint*, arrays); \
   GL_PROC(void,   APIENTRY, glBindVertexArray, GLuint, array); \
   GL_PROC(void,   APIENTRY, glShaderSource, GLuint, shader, GLsizei, count, const GLchar *const*,string, const GLint *,length); \
   GL_PROC(GLuint, APIENTRY, glCreateShader, GLenum, type); \
   GL_PROC(void,   APIENTRY, glCompileShader, GLuint, shader); \
   GL_PROC(void,   APIENTRY, glGetShaderiv, GLuint, shader, GLenum, pname, GLint*, params); \
   GL_PROC(void,   APIENTRY, glAttachShader, GLuint, program, GLuint, shader); \
   GL_PROC(GLuint, APIENTRY, glCreateProgram); \
   GL_PROC(void,   APIENTRY, glLinkProgram, GLuint, program); \
   GL_PROC(void,   APIENTRY, glGetProgramiv, GLuint, program, GLenum, pname, GLint*, params); \
   GL_PROC(void,   APIENTRY, glUseProgram, GLuint, program); \
   GL_PROC(void,   APIENTRY, glGetShaderInfoLog, GLuint, shader, GLsizei, bufSize, GLsizei*, length, GLchar*, infoLog);



#if defined(GL_GLEXT_PROTOTYPES) || defined(WGL_WGLEXT_PROTOTYPES)
#define PFN_NAME(fn) p##fn
#else
#define PFN_NAME(fn) fn
#endif

#define GL_PROC(type,api,fn,...) static type (api * PFN_NAME(fn))(MERGE_TYPE(__VA_ARGS__))
GL_PROCS
#undef GL_PROC

void* glGetProcAddress(const char* name)
{
   if(!name)
   {
#define GL_PROC(type,api,fn,...) PFN_NAME(fn) = (void*)wglGetProcAddress(#fn)
  GL_PROCS
#undef GL_PROC
        return NULL;
   }
   return wglGetProcAddress(name);
}


#if defined(GL_GLEXT_PROTOTYPES) || defined(WGL_WGLEXT_PROTOTYPES)
#define GL_PROC(type,api,fn,...) \
   type api fn (MERGE_TYPE(__VA_ARGS__)) \
{\
   return PFN_NAME(fn)(DROP_TYPE( __VA_ARGS__));\
}
GL_PROCS
#undef GL_PROC
#endif
