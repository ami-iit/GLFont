#ifndef GLFONT_GLUTILS_H
#define GLFONT_GLUTILS_H

#include <GLFont/GLConfig.h>

class GLUtils {
public:
    GLUtils();
    ~GLUtils();

    static void loadShader(char* shaderSource, GLenum shaderType, GLuint &programId);
};
#endif //GLFONT_GLUTILS_H

