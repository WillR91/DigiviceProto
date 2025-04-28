#include "platform/pc/PCDisplay.h" // <<< Include the correct header
#include <SDL_log.h>
#include <stdexcept>

PCDisplay::PCDisplay() : window(nullptr), renderer(nullptr), texture(nullptr), screenWidth(0), screenHeight(0) {}

// Destructor needs to clean up
PCDisplay::~PCDisplay() {
    close();
}

bool PCDisplay::init(const char* title, int windowWidth, int windowHeight) {
    // Keep your original init logic, it looks fine
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // Create texture with ARGB8888 format for easier pixel manipulation
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, windowWidth, windowHeight);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Texture could not be created! SDL Error: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    screenWidth = windowWidth;
    screenHeight = windowHeight;
    // pixelBuffer.resize(screenWidth * screenHeight); // No longer needed if writing direct

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay Initialized (%dx%d)", screenWidth, screenHeight);
    return true;
}

void PCDisplay::close() {
    // Keep your original close logic
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    // SDL_Quit(); // Consider quitting SDL only once at the very end in main/Game::cleanup
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay Closed resources");
}

// Keep your original helper function
uint32_t PCDisplay::convertRGB565toARGB8888(uint16_t color) {
    uint32_t r = (color & 0xF800) >> 11;
    uint32_t g = (color & 0x07E0) >> 5;
    uint32_t b = (color & 0x001F);
    r = (r * 255) / 31;
    g = (g * 255) / 63;
    b = (b * 255) / 31;
    return (0xFF << 24) | (r << 16) | (g << 8) | b; // Alpha = FF (opaque)
}

void PCDisplay::clear(uint16_t color) {
    // Keep your original clear logic
    uint32_t sdlColor = convertRGB565toARGB8888(color);
    uint8_t r = (sdlColor >> 16) & 0xFF;
    uint8_t g = (sdlColor >> 8) & 0xFF;
    uint8_t b = sdlColor & 0xFF;
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderClear(renderer);
}

void PCDisplay::drawPixels(int destX, int destY, int width, int height,
                           const uint16_t* pixelData,
                           int sourceBufferWidth, int sourceBufferHeight,
                           int sourceX, int sourceY)
{
    // Your existing drawPixels logic using SDL_LockTexture looks correct, keep it.
    // Just ensure it matches the function signature from the header.
    if (!pixelData || !texture) return;

    // Clip drawing rect (same clipping logic as before is fine)
    int drawW = width;
    int drawH = height;
    int dX = destX;
    int dY = destY;
    int sX = sourceX;
    int sY = sourceY;

    if (dX < 0) { drawW += dX; sX -= dX; dX = 0; }
    if (dY < 0) { drawH += dY; sY -= dY; dY = 0; }
    if (dX + drawW > screenWidth) { drawW = screenWidth - dX; }
    if (dY + drawH > screenHeight) { drawH = screenHeight - dY; }

    if (drawW <= 0 || drawH <= 0) return; // Nothing to draw

    // Lock texture for writing
    void* texturePixels;
    int pitch;
    if (SDL_LockTexture(texture, NULL, &texturePixels, &pitch) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to lock texture: %s", SDL_GetError());
        return;
    }

    uint32_t* texBuffer = static_cast<uint32_t*>(texturePixels);
    int texPitchInPixels = pitch / sizeof(uint32_t);

    // Copy pixel data row by row, converting format
    for (int y = 0; y < drawH; ++y) {
        if (sY + y < 0 || sY + y >= sourceBufferHeight) continue;
        if (dY + y < 0 || dY + y >= screenHeight) continue;

        const uint16_t* srcRow = pixelData + (sY + y) * sourceBufferWidth + sX;
        uint32_t* destRow = texBuffer + (dY + y) * texPitchInPixels + dX;

        for (int x = 0; x < drawW; ++x) {
            if (sX + x < 0 || sX + x >= sourceBufferWidth) continue;
            if (dX + x < 0 || dX + x >= screenWidth) continue;

            uint16_t srcColor = srcRow[x];
             // Your original magenta key check:
             if (srcColor != 0xF81F) { // Magenta Check (0b1111100000011111)
                 destRow[x] = convertRGB565toARGB8888(srcColor);
             } else {
                 // Optional: If clearing wasn't perfect, explicitly set transparent pixels
                 // destRow[x] = 0x00000000; // Fully transparent black
             }
        }
    }
    SDL_UnlockTexture(texture);
}


void PCDisplay::present() {
    // Keep your original present logic
    if (!renderer || !texture) return;
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}