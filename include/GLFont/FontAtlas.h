#ifndef GLFONT_FONTATLAS_H
#define GLFONT_FONTATLAS_H

#include <GLFont/GLConfig.h>

class FontAtlas {
public:
    struct Character {
        float advanceX;
        float advanceY;

        float bitmapWidth;
        float bitmapHeight;

        float bitmapLeft;
        float bitmapTop;

        float xOffset;
    };

    FontAtlas(FT_Face face, int pixelSize);
    ~FontAtlas();

    inline GLuint getTexId() { return _tex; }
    inline int getAtlasWidth() { return _width; }
    inline int getAtlasHeight() { return _height; }
    inline Character* getCharInfo() { return _chars; }

private:
    FT_Face _face;
    FT_GlyphSlot _slot;
    GLuint _tex;

    Character _chars[128];

    int _width;  // width of texture
    int _height; // height of texture
};

#endif //GLFONT_FONTATLAS_H

