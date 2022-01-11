find_package(GLFW3 REQUIRED)

set (TEST_WINDOW_SRC
    src/TestWindow.cpp
    src/GLWindow.cpp
    src/main.cpp)

set (TEST_WINDOW_HDR
    src/TestWindow.h
    src/GLWindow.h)

add_executable(test_window ${TEST_WINDOW_HDR} ${TEST_WINDOW_SRC})

target_link_libraries(test_window PRIVATE GLFont::GLFont glfw)
