#ifndef GLFONT_H
#define GLFONT_H

#include <GLFont/GLConfig.h>
#include <string>

class GLFont {
public:
    GLFont(const char* fontFile);
    ~GLFont();

    void setFontFile(const std::string& fontFile);

    FT_Face getFaceHandle();

private:
    std::string _fontFile;
    FT_Error _error;
    FT_Library _ft;
    FT_Face _face;

};

#endif //GLFONT_H
