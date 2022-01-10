#ifndef GLFONT_H
#define GLFONT_H

#include <GLFont/GLConfig.h>

class GLFont {
public:
    GLFont(const char* fontFile);
    ~GLFont();

    void setFontFile(const char* fontFile);

    FT_Face getFaceHandle();

private:
    char* _fontFile;
    FT_Error _error;
    FT_Library _ft;
    FT_Face _face;

};

#endif //GLFONT_H
