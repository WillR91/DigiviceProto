#ifndef PC_DISPLAY_H // Changed header guard
#define PC_DISPLAY_H

#include "platform/IDisplay.h" // <<< Include the interface
#include <SDL2/SDL.h>
#include <vector>
#include <stdint.h> // Ensure uint types are included

class PCDisplay : public IDisplay { // <<< Inherit from IDisplay
public:
    PCDisplay();
    ~PCDisplay() override; // <<< Use override

    // --- IDisplay Interface Implementation ---
    bool init(const char* title, int windowWidth, int windowHeight) override;
    void close() override;
    void clear(uint16_t color) override;
    void drawPixels(int destX, int destY, int width, int height,
                    const uint16_t* pixelData,
                    int sourceBufferWidth, int sourceBufferHeight,
                    int sourceX, int sourceY) override;
    void present() override;

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int screenWidth;
    int screenHeight;
    // std::vector<uint32_t> pixelBuffer; // We write directly to texture now

    uint32_t convertRGB565toARGB8888(uint16_t color); // Keep helper
};

#endif // PC_DISPLAY_H