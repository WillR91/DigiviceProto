#ifndef PC_INPUT_H
#define PC_INPUT_H

#include "platform/IInput.h" // <<< Include the interface
#include <SDL2/SDL.h>
#include <unordered_map>
#include <unordered_set>

class PCInput : public IInput { // <<< Inherit from IInput
public:
    PCInput();
    ~PCInput() override = default;

    // --- IInput Interface Implementation ---
    void update() override;
    bool wasActionPressed(InputAction action) const override;
    bool isQuitRequested() const override;

private:
    std::unordered_map<SDL_Keycode, InputAction> keyActionMap;
    std::unordered_set<InputAction> pressedActions; // Actions pressed this frame
    bool quitRequested;
};

#endif // PC_INPUT_H