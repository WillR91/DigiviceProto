#ifndef IINPUT_H
#define IINPUT_H

#include <SDL_keycode.h>

// Define generic input actions
enum class InputAction {
    QUIT,
    STEP, // Simulate shaking/pedometer step
    SELECT_DIGI_1,
    SELECT_DIGI_2,
    SELECT_DIGI_3,
    SELECT_DIGI_4,
    SELECT_DIGI_5,
    SELECT_DIGI_6,
    SELECT_DIGI_7,
    SELECT_DIGI_8,
    UNKNOWN // Placeholder
};

// Interface definition for input operations
class IInput {
public:
    virtual ~IInput() = default;

    virtual void update() = 0; // Poll hardware events
    virtual bool wasActionPressed(InputAction action) const = 0; // Check press this frame
    virtual bool isQuitRequested() const = 0;
    // virtual int getShakeCount() = 0; // Add later
    // virtual bool getTouchPosition(int& x, int& y) = 0; // Add later
};

#endif // IINPUT_H