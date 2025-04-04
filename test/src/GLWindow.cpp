#include "GLWindow.h"
#include <stdexcept>

void errorCallback(int error, const char* description) {
    // not implemented
}

void resizeCallback(GLFWwindow* window, int width, int height) {
    GLWindow* instance = (GLWindow*)glfwGetWindowUserPointer(window);
    instance->setWidth(width);
    instance->setHeight(height);
    instance->onResize(width, height);
}

void framebufferCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    GLWindow* instance = (GLWindow*)glfwGetWindowUserPointer(window);
    instance->onKey(key, scancode, action, mods);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    GLWindow* instance = (GLWindow*)glfwGetWindowUserPointer(window);
    instance->onMouseButton(button, action, mods);
}

void mouseMoveCallback(GLFWwindow* window, double x, double y) {
    GLWindow* instance = (GLWindow*)glfwGetWindowUserPointer(window);
    instance->onMouseMove(x, y);
}

void cursorEnterCallback(GLFWwindow* window, int enter) {
    GLWindow* instance = (GLWindow*)glfwGetWindowUserPointer(window);
    instance->onMouseEnter(enter);
}

void characterCallback(GLFWwindow* window, unsigned int codepoint) {
    GLWindow* instance = (GLWindow*)glfwGetWindowUserPointer(window);
    instance->onCharacter(codepoint);
}

void scrollCallback(GLFWwindow* window, double x, double y) {
    GLWindow* instance = (GLWindow*)glfwGetWindowUserPointer(window);
    instance->onScroll(x, y);
}

GLWindow::GLWindow() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW\n");
    }
    glfwWindowHint(GLFW_SAMPLES, 4); //4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Opengl 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    _windowTitle = "GLWindow";
}

GLWindow::GLWindow(int width, int height, const char* windowTitle) :
  GLWindow()
{
    _width = width;
    _height = height;
    _windowTitle = windowTitle;
}

GLWindow::~GLWindow() {
    glfwDestroyWindow(_window);
    glfwTerminate();
}

void GLWindow::run() {
    // Open a window and create a context
    _window = glfwCreateWindow(_width, _height, _windowTitle, NULL, NULL);
    if(_window == NULL) {
        glfwTerminate();
        throw std::runtime_error("Failed to open GLFW window");
    }

    glfwMakeContextCurrent(_window);

    // Initialize glew
    glewExperimental = true;
    if(glewInit() != GLEW_OK) {
        throw std::runtime_error("Failed to initialize GLEW");
    }

    glfwSetInputMode(_window, GLFW_STICKY_KEYS, GL_TRUE);

    // Set callbacks
    glfwSetWindowUserPointer(_window, this);
    glfwSetErrorCallback(errorCallback);
    glfwSetWindowSizeCallback(_window, resizeCallback);
    glfwSetFramebufferSizeCallback(_window, framebufferCallback);
    glfwSetKeyCallback(_window, keyCallback);
    glfwSetMouseButtonCallback(_window, mouseButtonCallback);
    glfwSetCursorPosCallback(_window, mouseMoveCallback);
    glfwSetCursorEnterCallback(_window, cursorEnterCallback);
    glfwSetCharCallback(_window, characterCallback);
    glfwSetScrollCallback(_window, scrollCallback);


    glGenFramebuffers(1, &_labelFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _labelFrameBuffer);
    glGenTextures(1, &_labelTexture);

    //Trying to preallocate texture for read frame buffer
    glBindTexture(GL_TEXTURE_2D, _labelTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, _labelTexture, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    init();

    double lastTime = glfwGetTime();
    while(glfwGetKey(_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(_window) == 0) {
        glfwPollEvents();

        // Calculate ms/frame
        double currentTime = glfwGetTime();
        ++_frame;
        if(currentTime - lastTime >= 2.0f) {
            printf("%0.2f ms/frame (fps: %0.2f)\n", 1000.0 / double(_frame), double(_frame) / (currentTime - lastTime));
            _frame = 0;
            lastTime += 2.0f;
        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // Render scene
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _labelFrameBuffer);
        glBindTexture(GL_TEXTURE_2D, _labelTexture);
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);

        update();
        render();

        glBlitNamedFramebuffer(_labelFrameBuffer, 0,
                               0, 0, _width, _height,
                               0, 0, _width, _height,
                               GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // Swap buffers
        glfwSwapBuffers(_window);
    }

    glDeleteFramebuffers(1, &(_labelFrameBuffer));
}

GLFWwindow* GLWindow::getWindowHandle() {
    return _window;
}

void GLWindow::onKey(int key, int scancode, int action, int mods) {}

void GLWindow::onResize(int width, int height) {
    _width = width;
    _height = height;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _labelFrameBuffer);

    //Trying to preallocate texture for read frame buffer
    glBindTexture(GL_TEXTURE_2D, _labelTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void GLWindow::onMouseButton(int button, int action, int mods) {}

void GLWindow::onMouseMove(double x, double y) {}

void GLWindow::onMouseEnter(int enter) {}

void GLWindow::onCharacter(unsigned int codepoint) {}

void GLWindow::onScroll(double x, double y) {}
