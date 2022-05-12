#ifndef GLFONT_GLCONFIG_H
#define GLFONT_GLCONFIG_H

#define GL_GLEXT_PROTOTYPES
#define GL3_PROTOTYPES
#define NOMINMAX //On windows this avoids issues when using min and max

#include <GL/glew.h>
#include <GL/gl.h>

#if defined(__APPLE__) || defined(__linux__)
 #include <GL/glx.h>
#endif

#ifdef Success
  #undef Success
#endif //See https://gitlab.com/libeigen/eigen/-/issues/253


#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>

#endif // GLFONT_GLCONFIG_H
