#ifndef GLFONT_GLCONFIG_H
#define GLFONT_GLCONFIG_H

#define GL_GLEXT_PROTOTYPES
#define GL3_PROTOTYPES
#define NOMINMAX //On windows this avoids issues when using min and max

#include <GL/glew.h>
#include <GL/gl.h>

#if defined(_WIN32)
 #define GLFW_EXPOSE_NATIVE_WIN32
 #define GLFW_EXPOSE_NATIVE_WGL
#elif defined(__APPLE__)
 #define GLFW_EXPOSE_NATIVE_COCOA
 #define GLFW_EXPOSE_NATIVE_NSGL
#include <GL/glext.h>
 #include <GL/glx.h>
#elif defined(__linux__)
 #define GLFW_EXPOSE_NATIVE_X11
 #define GLFW_EXPOSE_NATIVE_GLX
#include <GL/glext.h>
 #include <GL/glx.h>
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#ifdef Success
  #undef Success
#endif //See https://gitlab.com/libeigen/eigen/-/issues/253


#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>

#endif // GLFONT_GLCONFIG_H
