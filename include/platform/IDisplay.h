#ifndef IDISPLAY_H
#define IDISPLAY_H

#include <stdint.h> // For uint16_t

// Interface definition for display operations
class IDisplay {
public:
    virtual ~IDisplay() = default; // Virtual destructor

    virtual bool init(const char* title, int windowWidth, int windowHeight) = 0;
    virtual void close() = 0;
    virtual void clear(uint16_t color) = 0;
    virtual void drawPixels(int destX, int destY, int width, int height,
                            const uint16_t* pixelData,
                            int sourceBufferWidth, int sourceBufferHeight,
                            int sourceX, int sourceY) = 0;
    virtual void present() = 0; // Show the drawn buffer on screen
};

#endif // IDISPLAY_H