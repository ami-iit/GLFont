cmake_minimum_required(VERSION 3.14)
project(GLFont VERSION 0.0.1 LANGUAGES C CXX)

find_package(YCM REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
## This is to select the newest version of OpenGL
if (POLICY CMP0072)
  cmake_policy (SET CMP0072 NEW)
endif(POLICY CMP0072)

find_package(PkgConfig REQUIRED)
pkg_check_modules(freetype2 REQUIRED IMPORTED_TARGET freetype2)

find_package(OpenGL REQUIRED)

# Defines the CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_BINDIR and many other useful macros.
# See https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

# Control where libraries and executables are placed during the build.
# With the following settings executables are placed in <the top level of the
# build tree>/bin and libraries/archives in <top level of the build tree>/lib.
# This is particularly useful to run ctests on libraries built on Windows
# machines: tests, which are executables, are placed in the same folders of
# dlls, which are treated as executables as well, so that they can properly
# find the libraries to run. This is a because of missing RPATH on Windows.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

# To build shared libraries in Windows, we set CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS to TRUE.
# See https://cmake.org/cmake/help/v3.4/variable/CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS.html
# See https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Under MSVC, we set CMAKE_DEBUG_POSTFIX to "d" to add a trailing "d" to library
# built in debug mode. In this Windows user can compile, build and install the
# library in both Release and Debug configuration avoiding naming clashes in the
# installation directories.
if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
endif()

# Build position independent code.
# Position Independent Code (PIC) is commonly used for shared libraries so that
# the same shared library code can be loaded in each program address space in a
# location where it will not overlap with any other uses of such memory.
# In particular, this option avoids problems occurring when a process wants to
# load more than one shared library at the same virtual address.
# Since shared libraries cannot predict where other shared libraries could be
# loaded, this is an unavoidable problem with the traditional shared library
# concept.
# Generating position-independent code is often the default behavior for most
# modern compilers.
# Moreover linking a static library that is not built with PIC from a shared
# library will fail on some compiler/architecture combinations.
# Further details on PIC can be found here:
# https://eli.thegreenplace.net/2011/11/03/position-independent-code-pic-in-shared-libraries/
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Disable C and C++ compiler extensions.
# C/CXX_EXTENSIONS are ON by default to allow the compilers to use extended
# variants of the C/CXX language.
# However, this could expose cross-platform bugs in user code or in the headers
# of third-party dependencies and thus it is strongly suggested to turn
# extensions off.
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

# Shared/Dynamic or Static library?
option(BUILD_SHARED_LIBS "Build libraries as shared as opposed to static" ON)

# Build test related commands?
option(BUILD_TESTING "Create tests using CMake" OFF)
if(BUILD_TESTING)
    enable_testing()
endif()

# Enable RPATH support for installed binaries and libraries
include(AddInstallRPATHSupport)
add_install_rpath_support(BIN_DIRS "${CMAKE_INSTALL_FULL_BINDIR}"
                          LIB_DIRS "${CMAKE_INSTALL_FULL_LIBDIR}"
                          INSTALL_NAME_DIR "${CMAKE_INSTALL_FULL_LIBDIR}"
                          USE_LINK_PATH)

# Encourage user to specify a build type (e.g. Release, Debug, etc.), otherwise set it to Release.
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "Setting build type to 'Release' as none was specified.")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
    endif()
endif()


set (${PROJECT_NAME}_HDR
    include/GLFont/FTLabel.h
    include/GLFont/FontAtlas.h
    include/GLFont/GLFont.h
    include/GLFont/GLUtils.h
    include/GLFont/GLConfig.h)

set (${PROJECT_NAME}_SHADERS
    include/GLFont/shaders/fontFragment.shader
    include/GLFont/shaders/fontVertex.shader)

set (${PROJECT_NAME}_SRC
    src/FTLabel.cpp
    src/FontAtlas.cpp
    src/GLFont.cpp
    src/GLUtils.cpp)

source_group("Shader Files" FILES ${${PROJECT_NAME}_SHADERS})

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_HDR} ${${PROJECT_NAME}_SHADERS} ${${PROJECT_NAME}_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                                                  "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries(${PROJECT_NAME} PUBLIC GLEW::GLEW PkgConfig::freetype2 OpenGL::GL glm)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

target_compile_definitions(${PROJECT_NAME} PUBLIC GLFont_DEFAULT_FONTS_PATH="${CMAKE_CURRENT_LIST_DIR}/fonts")

set_target_properties(${PROJECT_NAME} PROPERTIES
  OUTPUT_NAME ${PROJECT_NAME}
  VERSION ${${PROJECT_NAME}_VERSION}
  PUBLIC_HEADER "${${PROJECT_NAME}_HDR}")

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

## Specify installation targets, typology and destination folders.
install(TARGETS    ${PROJECT_NAME}
  EXPORT           ${PROJECT_NAME}Targets
  COMPONENT        runtime
  LIBRARY          DESTINATION "${CMAKE_INSTALL_LIBDIR}"                      COMPONENT shlib
  ARCHIVE          DESTINATION "${CMAKE_INSTALL_LIBDIR}"                      COMPONENT lib
  RUNTIME          DESTINATION "${CMAKE_INSTALL_BINDIR}"                      COMPONENT bin
  PUBLIC_HEADER    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/" COMPONENT dev)

install(FILES ${${PROJECT_NAME}_SHADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/shaders")

include(CMakePackageConfigHelpers)

install(EXPORT ${PROJECT_NAME}Targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        NAMESPACE ${PROJECT_NAME}::)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/GLFontConfig.cmake.in
                              ${CMAKE_BINARY_DIR}/GLFontConfig.cmake
                              INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/GLFontConfigVersion.cmake VERSION ${${PROJECT_NAME}_VERSION} COMPATIBILITY SameMajorVersion )

install(FILES ${CMAKE_BINARY_DIR}/GLFontConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/GLFontConfigVersion.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(DIRECTORY fonts DESTINATION "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}")

# Add the uninstall target
include(AddUninstallTarget)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
