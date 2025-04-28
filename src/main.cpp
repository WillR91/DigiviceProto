#include "Game.h" // Include the main Game class header
#include <SDL_log.h> // For logging start/end
#include <exception> // For exception handling

int main(int /*argc*/, char* /*argv*/[]) { // Mark args as unused if needed
    SDL_Log("--- Application Entry Point ---");
    Game digiviceGame; // Create the Game object on the stack

    try {
        if (digiviceGame.initialize()) { // Initialize systems
            digiviceGame.run(); // Start the main game loop
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game initialization failed!");
            // No cleanup needed here as destructor handles it if init fails partially
            return 1; // Indicate failure
        }
    } catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unhandled exception caught: %s", e.what());
         // Game object goes out of scope here, destructor calls cleanup
         return 1; // Indicate failure
    } catch (...) {
         SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unknown unhandled exception caught!");
          // Game object goes out of scope here, destructor calls cleanup
         return 1; // Indicate failure
    }

    // Game object goes out of scope here, destructor calls cleanup
    // digiviceGame.cleanup(); // Cleanup is now handled by Game's destructor

    SDL_Log("--- Application Exiting Normally ---");
    return 0; // Indicate success
}