#ifndef GLFONT_H
#define GLFONT_H

#include <GLFont/GLConfig.h>
#include <GLFont/FTLabel.h>
#include <string>

class GLFont {
public:
    GLFont(const std::string& fontFile);
    ~GLFont();

    void setFontFile(const std::string& fontFile);

    FT_Face getFaceHandle();

    static inline std::string DefaultFontsPathPrefix()
    {
        return std::string(GLFont_DEFAULT_FONTS_PATH) + "/";
    }


private:
    std::string _fontFile;
    FT_Error _error;
    FT_Library _ft;
    FT_Face _face;

};

#endif //GLFONT_H
