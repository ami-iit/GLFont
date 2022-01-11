#include <GLFont/GLUtils.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>

GLUtils::GLUtils() {}


GLUtils::~GLUtils() {}

void GLUtils::loadShader(const std::string& shaderSource, GLenum shaderType, GLuint &programId) {
    GLuint shaderId = glCreateShader(shaderType);

    GLint result = GL_FALSE; // compilation result
    int infoLogLength; // length of info log

    const char* shader;
    shader = shaderSource.c_str();

    // Compile shader
    glShaderSource(shaderId,        // Shader handle
        1,               // Number of files
        &shader,  // Shader source code
        NULL);           // NULL terminated string
    glCompileShader(shaderId);

    // Check shader
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> errorMessage(std::max(infoLogLength, int(1)));
    glGetShaderInfoLog(shaderId, infoLogLength, NULL, errorMessage.data());
    if (errorMessage[0] != '\0')
        fprintf(stdout, "Error while compiling shader: %s\n", errorMessage.data());

    // Link the program
    glAttachShader(programId, shaderId);
    glLinkProgram(programId);

    // Check the program
    glGetProgramiv(programId, GL_LINK_STATUS, &result);
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> programErrorMessage(std::max(infoLogLength, int(1)));
    glGetProgramInfoLog(programId, infoLogLength, NULL, programErrorMessage.data());
    if (programErrorMessage[0] != '\0')
        fprintf(stdout, "Program error message: %s\n", programErrorMessage.data());

    glDeleteShader(shaderId);
}
