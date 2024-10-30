#ifndef __OPENGL_INCLUDE_H__
#define __OPENGL_INCLUDE_H__

#if defined(__APPLE__)
  #define GL_SILENCE_DEPRECATION
  #include <OpenGL/gl3.h>
#elif defined(linux)
  #include <GL/gl.h>
  #define GL_GLEXT_PROTOTYPES
  #include <GL/glext.h>
#else
  #error "Why are you compiling this on windows?"
#endif

#endif
