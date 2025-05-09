#include <GLFont/FTLabel.h>
#include <GLFont/GLUtils.h>
#include <GLFont/FontAtlas.h>
#include <GLFont/GLFont.h>

#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

// GLM
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef RAD_TO_DEG
#define RAD_TO_DEG 180.0 / M_PI;
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD M_PI / 180.0
#endif

FTLabel::FTLabel(std::shared_ptr<GLFont> ftFace, int windowWidth, int windowHeight) :
  _isInitialized(false),
  _text(""),
  _alignment(FontFlags::LeftAligned),
  _textColor(0, 0, 0, 1),
  _uniformMVPHandle(-1),
  _indentationPix(0),
  _x(0),
  _y(0),
  _maxWidth(0),
  _maxHeight(0),
  _actualHeight(0),
  _actualWidth(0),
  _arsx(1.0),
  _arsy(1.0)
{
    setFont(ftFace);
    setWindowSize(windowWidth, windowHeight);

    // Intially enabled flags
    _flags = FontFlags::LeftAligned | FontFlags::WordWrap;

    // Since we are dealing with 2D text, we will use an orthographic projection matrix
    _projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, // View: left, right, bottom, top (we are using normalized coords)
                             0.1f,                     // Near clipping distance
                             100.0f);                  // Far clipping distance

    _view = glm::lookAt(glm::vec3(0, 0, 1),  // Camera position in world space (doesn't really apply when using an ortho projection matrix)
                        glm::vec3(0, 0, 0),  // look at origin
                        glm::vec3(0, 1, 0)); // Head is up (set to 0, -1, 0 to look upside down)

    _model = glm::mat4(1.0f); // Identity matrix

    recalculateMVP();

    // Load the shaders
    _programId = glCreateProgram();

    const std::string fontVertexSource =
        #include <GLFont/shaders/fontVertex.shader>
        ;

    const std::string fontFragmentSource =
        #include <GLFont/shaders/fontFragment.shader>
        ;

    GLUtils::loadShader(fontVertexSource, GL_VERTEX_SHADER, _programId);
    GLUtils::loadShader(fontFragmentSource, GL_FRAGMENT_SHADER, _programId);

    glUseProgram(_programId);

    // Create and bind the vertex array object
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    // Set default pixel size and create the texture
    setPixelSize(48); // default pixel size

    // Get shader handles
    _uniformTextureHandle = glGetUniformLocation(_programId, "tex");
    _uniformTextColorHandle = glGetUniformLocation(_programId, "textColor");
    _uniformMVPHandle = glGetUniformLocation(_programId, "mvp");

    GLuint curTex = _fontAtlas[_pixelSize]->getTexId(); // get texture ID for this pixel size

    glActiveTexture(GL_TEXTURE0 + curTex);
    glBindTexture(GL_TEXTURE_2D, curTex);

    // Set our uniforms
    glUniform1i(_uniformTextureHandle, curTex);
    glUniform4fv(_uniformTextColorHandle, 1, glm::value_ptr(_textColor));
    glUniformMatrix4fv(_uniformMVPHandle, 1, GL_FALSE, glm::value_ptr(_mvp));

    // Create the vertex buffer object
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindVertexArray(0);

    glUseProgram(0);

    _isInitialized = true;
}

FTLabel::FTLabel(GLFont* ftFace, int windowWidth, int windowHeight) :
  FTLabel(std::shared_ptr<GLFont>(new GLFont(*ftFace)), windowWidth, windowHeight)
{}

FTLabel::FTLabel(std::shared_ptr<GLFont> ftFace, const std::string& text, float x, float y, int maxWidth, int maxHeight, int windowWidth, int windowHeight) :
  FTLabel(ftFace, text, x, y, windowWidth, windowHeight)
{
    _maxWidth = maxWidth;
    _maxHeight = maxHeight;

    recalculateVertices(text, x, y, maxWidth, maxHeight);
}

FTLabel::FTLabel(std::shared_ptr<GLFont> ftFace, const std::string &text, float x, float y, int windowWidth, int windowHeight) :
FTLabel(ftFace, windowWidth, windowHeight)
{
    _text = text;
    _x = x;
    _y = y;
}

FTLabel::~FTLabel() {
    glDeleteBuffers(1, &_vbo);
    glDeleteVertexArrays(1, &_vao);
}

void FTLabel::recalculateVertices(const std::string& text, float x, float y, int maxWidth, int maxHeight) {

    _coords.clear(); // case there are any existing coords

    // Break the text into individual words
    std::vector<std::string> words = splitText(_text);

    std::vector<std::string> lines;
    int widthRemaining = maxWidth;
    int spaceWidth = calcWidth(" ");
    int indent = (_flags & FontFlags::Indented) && _alignment != FontFlags::CenterAligned ? _pixelSize : 0;

    // Create lines from our text, each containing the maximum amount of words we can fit within the given width
    std::string curLine = "";
    for(const std::string &word : words) {
        int wordWidth = calcWidth(word.c_str());

        if(wordWidth - spaceWidth > widthRemaining && maxWidth /* make sure there is a width specified */) {

            // If we have passed the given width, add this line to our collection and start a new line
            lines.push_back(curLine);
            widthRemaining = maxWidth - wordWidth;
            curLine = "";

            // Start next line with current word
            curLine.append(word.c_str());
        }
        else {
            // Otherwise, add this word to the current line
            curLine.append(word.c_str());
            widthRemaining = widthRemaining - wordWidth;
        }
    }

    // Add the last line to lines
    if(curLine != "")
        lines.push_back(curLine);

    // Print each line, increasing the y value as we go
    float startY = y - (_face->size->metrics.height >> 6) * _arsy;
    int lineWidth;
    _actualWidth = 0;
    for(const std::string &line : lines) {
        // If we go past the specified height, stop drawing
        if(y - startY > maxHeight && maxHeight)
            break;

        recalculateVertices(line.c_str(), x + indent, y);
        y += (_face->size->metrics.height >> 6)  * _arsy;
        indent = 0;

        lineWidth = calcWidth(line.c_str());
        if (lineWidth > _actualWidth)
            _actualWidth = lineWidth;
    }

    _actualHeight = static_cast<int>(std::ceil(y - startY));

    glBindVertexArray(_vao);
    glUseProgram(_programId);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint curTex = _fontAtlas[_pixelSize]->getTexId();
    glActiveTexture(GL_TEXTURE0 + curTex);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glBindTexture(GL_TEXTURE_2D, curTex);
    glUniform1i(_uniformTextureHandle, curTex);

    glEnableVertexAttribArray(0);

    // Send the data to the gpu
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Point), 0);
    glBufferData(GL_ARRAY_BUFFER, _coords.size() * sizeof(Point), _coords.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, _coords.size());
    _numVertices = _coords.size();

    glDisableVertexAttribArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDisable(GL_BLEND);
    glUseProgram(0);
    glBindVertexArray(0);

}

void FTLabel::recalculateVertices(const char* text, float x, float y) {

    // Coordinates passed in should specify where to start drawing from the top left of the text,
    // but FreeType starts drawing from the bottom-right, therefore move down one line
    y += (_face->size->metrics.height >> 6) * _arsy;

    // Calculate alignment (if applicable)
    int textWidth = calcWidth(text);
    if(_alignment == FontFlags::CenterAligned)
        x -= textWidth / 2.0;
    else if(_alignment == FontFlags::RightAligned)
        x -= textWidth;

    // Normalize window coordinates
    x = -1 + x * _sx * _arsx;
    y = 1 - y * _sy;


    int atlasWidth = _fontAtlas[_pixelSize]->getAtlasWidth();
    int atlasHeight = _fontAtlas[_pixelSize]->getAtlasHeight();

    FontAtlas::Character* chars = _fontAtlas[_pixelSize]->getCharInfo();

    for(const char *p = text; *p; ++p) {
        float x2 = x + chars[*p].bitmapLeft * _sx * _arsx; // scaled x coord
        float y2 = -y - chars[*p].bitmapTop * _sy * _arsy; // scaled y coord
        float w = chars[*p].bitmapWidth * _sx * _arsx;     // scaled width of character
        float h = chars[*p].bitmapHeight * _sy * _arsy;    // scaled height of character

        // Calculate kerning value
        FT_Vector kerning;
        FT_Get_Kerning(_face,              // font face handle
                       *p,                 // left glyph
                       *(p + 1),           // right glyph
                       FT_KERNING_DEFAULT, // kerning mode
                       &kerning);          // variable to store kerning value

        // Advance cursor to start of next character
        x += (chars[*p].advanceX + (kerning.x >> 6)) * _sx * _arsx;
        y += chars[*p].advanceY * _sy * _arsy;

        // Skip glyphs with no pixels (e.g. spaces)
        if(!w || !h)
            continue;

        _coords.push_back(Point(x2,                // window x
                                -y2,               // window y
                                chars[*p].xOffset, // texture atlas x offset
                                0));               // texture atlas y offset

        _coords.push_back(Point(x2 + w,
                                -y2,
                                chars[*p].xOffset + chars[*p].bitmapWidth / atlasWidth,
                                0));

        _coords.push_back(Point(x2,
                                -y2 - h,
                                chars[*p].xOffset,
                                chars[*p].bitmapHeight / atlasHeight));

        _coords.push_back(Point(x2 + w,
                                -y2,
                                chars[*p].xOffset + chars[*p].bitmapWidth / atlasWidth,
                                0));

        _coords.push_back(Point(x2,
                                -y2 - h,
                                chars[*p].xOffset,
                                chars[*p].bitmapHeight / atlasHeight));

        _coords.push_back(Point(x2 + w,
                                -y2 - h,
                                chars[*p].xOffset + chars[*p].bitmapWidth / atlasWidth,
                                chars[*p].bitmapHeight / atlasHeight));
    }

}

void FTLabel::render() {
    glBindVertexArray(_vao);
    glUseProgram(_programId);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint curTex = _fontAtlas[_pixelSize]->getTexId();
    glActiveTexture(GL_TEXTURE0 + curTex);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glBindTexture(GL_TEXTURE_2D, curTex);
    glUniform1i(_uniformTextureHandle, curTex);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Point), 0);
    glDrawArrays(GL_TRIANGLES, 0, _numVertices);

    glDisableVertexAttribArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDisable(GL_BLEND);
    glUseProgram(0);
    glBindVertexArray(0);
}

std::vector<std::string> FTLabel::splitText(const std::string& text) {
    std::vector<std::string> words;
    size_t startPos = 0; // start position of current word
    size_t endPos = text.find(' '); // end position of current word

    if(endPos == -1) {
        // There is only one word, so return early
        words.push_back(text);
        return words;
    }

    // Find each word in the text (delimited by spaces) and add it to our std::vector of words
    while(endPos != std::string::npos) {
        words.push_back(text.substr(startPos, endPos  - startPos + 1));
        startPos = endPos + 1;
        endPos = text.find(' ', startPos);
    }

    // Add last word
    words.push_back(text.substr(startPos, std::min(endPos, text.size()) - startPos + 1));

    return words;
}

int FTLabel::calcWidth(const char* text) {
    int width = 0;
    FontAtlas::Character* chars = _fontAtlas[_pixelSize]->getCharInfo();
    for(const char* p = text; *p; ++p) {
        width += static_cast<int>(std::ceil(chars[*p].advanceX));
    }

    return width  * _arsx;
}

void FTLabel::setText(const std::string& text) {
    _text = text;

    recalculateVertices(_text, _x, _y, _maxWidth, _maxHeight);
}

std::string FTLabel::getText() {
    return std::string(_text);
}

void FTLabel::setPosition(float x, float y) {
    _x = x;
    _y = y;

    if(_text != "") {
        recalculateVertices(_text, _x, _y, _maxWidth, _maxHeight);
    }
}

float FTLabel::getX() {
    return _x;
}

float FTLabel::getY() {
    return _y;
}

void FTLabel::setMaxSize(int width, int height) {
    _maxWidth = width;
    _maxHeight = height;

    if(_text != "") {
        recalculateVertices(_text, _x, _y, width, height);
    }
}

int FTLabel::getWidth() {
    return _maxWidth;
}

int FTLabel::getHeight() {
    return _maxHeight;
}

void FTLabel::setFontFlags(int flags) {
    _flags = flags;
}

void FTLabel::setFontAspectRatio(float aspectRatio)
{
    if (aspectRatio >= 1.0)
    {
        _arsx = aspectRatio;
        _arsy = 1.0;
    }
    else
    {
        _arsx = 1.0;
        _arsy = 1.0 / aspectRatio;
    }

    if(_text != "") {
        recalculateVertices(_text, _x, _y, _maxWidth, _maxHeight);
    }
}

void FTLabel::appendFontFlags(int flags) {
    _flags |= flags;
}

int FTLabel::getFontFlags() {
    return _flags;
}

int FTLabel::getCurrentLabelHeight()
{
    if(_text != "") {
        recalculateVertices(_text, _x, _y, _maxWidth, _maxHeight);
    }

    return _actualHeight;
}

int FTLabel::getCurrentLabelWidth()
{
    if(_text != "") {
        recalculateVertices(_text, _x, _y, _maxWidth, _maxHeight);
    }

    return _actualWidth;
}

void FTLabel::setIndentation(int pixels) {
    _indentationPix = pixels;
}

int FTLabel::getIndentation() {
    return _indentationPix;
}

void FTLabel::setColor(float r, float b, float g, float a) {
    _textColor = glm::vec4(r, b, g, a);

    // Update the textColor uniform
    if(_programId != -1) {
        glUseProgram(_programId);
        glUniform4fv(_uniformTextColorHandle, 1, glm::value_ptr(_textColor));
        glUseProgram(0);
    }
}

glm::vec4 FTLabel::getColor() {
    return _textColor;
}

void FTLabel::setFont(std::shared_ptr<GLFont> ftFace) {
    _ftFace = ftFace;
    _face = _ftFace->getFaceHandle(); // shortcut
}

char* FTLabel::getFont() {
    return _font;
}

void FTLabel::setAlignment(FTLabel::FontFlags alignment) {
    _alignment = alignment;

    if(_isInitialized) {
        recalculateVertices(_text, _x, _y, _maxWidth, _maxHeight);
    }
}

FTLabel::FontFlags FTLabel::getAlignment() {
    return _alignment;
}

void FTLabel::setPixelSize(int size) {
    _pixelSize = size;

    // Create texture atlas for characters of this pixel size if there isn't already one
    if(!_fontAtlas[_pixelSize])
        _fontAtlas[_pixelSize] = std::shared_ptr<FontAtlas>(new FontAtlas(_face, _pixelSize));

    if(_isInitialized) {
        recalculateVertices(_text, _x, _y, _maxWidth, _maxHeight);
    }
}

void FTLabel::setWindowSize(int width, int height) {
    _windowWidth = width;
    _windowHeight = height;

    // Recalculate sx and sy
    _sx = 2.0 / _windowWidth;
    _sy = 2.0 / _windowHeight;

    if(_isInitialized) {
        recalculateVertices(_text, _x, _y, _maxWidth, _maxHeight);
    }
}

void FTLabel::rotate(float degrees, float x, float y, float z) {
    float rad = degrees * DEG_TO_RAD;
    _model = glm::rotate(_model, rad, glm::vec3(x, y, z));
    recalculateMVP();
}

void FTLabel::scale(float x, float y, float z) {
    _model = glm::scale(_model, glm::vec3(x, y, z));
    recalculateMVP();
}

void FTLabel::recalculateMVP() {
    _mvp = _projection * _view * _model;

    if(_uniformMVPHandle != -1) {
        glUseProgram(_programId);
        glUniformMatrix4fv(_uniformMVPHandle, 1, GL_FALSE, glm::value_ptr(_mvp));
        glUseProgram(0);
    }
}
