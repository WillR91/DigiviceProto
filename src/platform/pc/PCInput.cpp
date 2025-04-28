#include "platform/pc/PCInput.h" // <<< Include correct header
#include <SDL_log.h>

PCInput::PCInput() : quitRequested(false) {
    // --- Define Key Mappings ---
    keyActionMap[SDLK_ESCAPE] = InputAction::QUIT;
    keyActionMap[SDLK_SPACE] = InputAction::STEP; // Use spacebar to simulate a shake/step
    keyActionMap[SDLK_1] = InputAction::SELECT_DIGI_1;
    keyActionMap[SDLK_2] = InputAction::SELECT_DIGI_2;
    keyActionMap[SDLK_3] = InputAction::SELECT_DIGI_3;
    keyActionMap[SDLK_4] = InputAction::SELECT_DIGI_4;
    keyActionMap[SDLK_5] = InputAction::SELECT_DIGI_5;
    keyActionMap[SDLK_6] = InputAction::SELECT_DIGI_6;
    keyActionMap[SDLK_7] = InputAction::SELECT_DIGI_7;
    keyActionMap[SDLK_8] = InputAction::SELECT_DIGI_8;
    // Add more mappings here later (arrows, enter, etc.)
}

void PCInput::update() {
    // Clear the per-frame action set
    pressedActions.clear();

    SDL_Event e;
    // Process all pending events this frame
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quitRequested = true;
        } else if (e.type == SDL_KEYDOWN) {
            // Only register the first press (repeat == 0)
            if (e.key.repeat == 0) {
                auto it = keyActionMap.find(e.key.keysym.sym);
                if (it != keyActionMap.end()) {
                    InputAction action = it->second;
                    pressedActions.insert(action);
                    // If the action is QUIT, also set the flag immediately
                    if(action == InputAction::QUIT) {
                        quitRequested = true;
                    }
                }
            }
        }
        // We don't need key up handling for now, but could add it here
        // else if (e.type == SDL_KEYUP) { ... }
        // Add Touch Input handling here later if needed
        // else if (e.type == SDL_FINGERDOWN ...) { ... }
    }
}

bool PCInput::wasActionPressed(InputAction action) const {
    // Check if the action exists in the set of actions pressed this frame
    return pressedActions.count(action);
}

bool PCInput::isQuitRequested() const {
    return quitRequested;
}