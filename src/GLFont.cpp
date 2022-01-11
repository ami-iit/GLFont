#include <GLFont/GLFont.h>
#include <fstream>
#include <stdexcept>

GLFont::GLFont(const std::string &fontFile) {
    // Initialize FreeType

    _error = FT_Init_FreeType(&_ft);
    if(_error) {
        throw std::runtime_error("Failed to initialize FreeType");
    }
    setFontFile(fontFile);
}

GLFont::~GLFont() {
    FT_Done_FreeType(_ft);
}

void GLFont::setFontFile(const std::string &fontFile) {
    _fontFile = fontFile;

    // Create a new font
    _error = FT_New_Face(_ft,       // FreeType instance handle
                         _fontFile.c_str(), // Font family to use
                         0,         // index of font (in case there are more than one in the file)
                         &_face);   // font face handle

    if(_error == FT_Err_Unknown_File_Format) {
        throw std::runtime_error("Failed to open font: unknown font format");
    }
    else if(_error) {
        throw std::runtime_error("Failed to open font");
    }
}

FT_Face GLFont::getFaceHandle() {
    return _face;
}
