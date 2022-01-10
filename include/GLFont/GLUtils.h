#ifndef GLFONT_GLUTILS_H
#define GLFONT_GLUTILS_H

#include <GLFont/GLConfig.h>
#include <string>

class GLUtils {
public:
    GLUtils();
    ~GLUtils();

    static void loadShader(const std::string &shaderSource, GLenum shaderType, GLuint &programId);
};
#endif //GLFONT_GLUTILS_H

