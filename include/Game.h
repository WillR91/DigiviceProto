#ifndef GAME_H
#define GAME_H

#include <vector>
#include <stdint.h> // For Uint32 etc.

// Forward declarations
class IDisplay;
class IInput;
struct Animation; // Assuming animation.h defines this
struct SpriteFrame; // Assuming animation.h defines this
enum class InputAction; // From IInput.h
enum PlayerState; // From old main.cpp
enum DigimonType; // From old main.cpp

// Include necessary headers for data ONLY (minimal includes here)
// Adjust path based on where you put the asset files
#include "animation.h"
#include "castlebackground0.h"
#include "castlebackground1.h"
#include "castlebackground2.h"
// Include one Digimon header just to get the enums if they aren't separate
#include "Agumon_Idle_0.h"
#include "Agumon_Idle_1.h"
#include "Agumon_Walk_0.h"
#include "Agumon_Walk_1.h"
#include "Gabumon_Idle_0.h"
#include "Gabumon_Idle_1.h"
#include "Gabumon_Walk_0.h"
#include "Gabumon_Walk_1.h"
#include "Biyomon_Idle_0.h"
#include "Biyomon_Idle_1.h"
#include "Biyomon_Walk_0.h"
#include "Biyomon_Walk_1.h"
#include "Gatomon_Idle_0.h"
#include "Gatomon_Idle_1.h"
#include "Gatomon_Walk_0.h"
#include "Gatomon_Walk_1.h"
#include "Gomamon_Idle_0.h"
#include "Gomamon_Idle_1.h"
#include "Gomamon_Walk_0.h"
#include "Gomamon_Walk_1.h"
#include "Palmon_Idle_0.h"
#include "Palmon_Idle_1.h"
#include "Palmon_Walk_0.h"
#include "Palmon_Walk_1.h"
#include "Tentomon_Idle_0.h"
#include "Tentomon_Idle_1.h"
#include "Tentomon_Walk_0.h"
#include "Tentomon_Walk_1.h"
#include "Patamon_Idle_0.h"
#include "Patamon_Idle_1.h"
#include "Patamon_Walk_0.h"
#include "Patamon_Walk_1.h"


// Define enums directly here if they are not in a separate header
enum PlayerState { STATE_IDLE, STATE_WALKING };
enum DigimonType { DIGI_AGUMON, DIGI_GABUMON, DIGI_BIYOMON, DIGI_GATOMON, DIGI_GOMAMON, DIGI_PALMON, DIGI_TENTOMON, DIGI_PATAMON, DIGI_COUNT };


class Game {
public:
    Game();
    ~Game();

    bool initialize();
    void run();
    void cleanup();

private:
    // --- Core Systems ---
    IDisplay* display; // Pointer to the display interface
    IInput* input;     // Pointer to the input interface

    // --- Game Loop Control ---
    bool isRunning;

    // --- Game State Variables (from old main) ---
    const uint16_t* bg_data_0;
    const uint16_t* bg_data_1;
    const uint16_t* bg_data_2;
    float bg_scroll_offset_0;
    float bg_scroll_offset_1;
    float bg_scroll_offset_2;

    PlayerState current_state;
    DigimonType current_digimon;
    Animation* active_anim;
    int current_anim_frame_idx;
    Uint32 last_anim_update_time;
    int queued_steps;

    // Digimon Animations (Declare them here for now)
     Animation agumon_idle_anim, agumon_walk_anim;
     Animation gabumon_idle_anim, gabumon_walk_anim;
     Animation biyomon_idle_anim, biyomon_walk_anim;
     Animation gatomon_idle_anim, gatomon_walk_anim;
     Animation gomamon_idle_anim, gomamon_walk_anim;
     Animation palmon_idle_anim, palmon_walk_anim;
     Animation tentomon_idle_anim, tentomon_walk_anim;
     Animation patamon_idle_anim, patamon_walk_anim;

    // --- Private Helper Methods ---
    void handleInput();
    void update(Uint32 currentTime); // Pass current time from loop
    void render();

    void drawClippedTile(int dest_x_unclipped, const uint16_t* tile_data,
                         int layer_tile_width, int layer_tile_height);
    void setupAnimations();
    void selectActiveAnimation(bool forceReset);

    // --- Constants (copied from old main) ---
    const int WINDOW_WIDTH = 466;
    const int WINDOW_HEIGHT = 466;
    const int MAX_QUEUED_STEPS = 2;

    const int TILE_WIDTH_0 = CASTLEBACKGROUND0_WIDTH;
    const int TILE_HEIGHT_0 = CASTLEBACKGROUND0_HEIGHT;
    const int EFFECTIVE_BG_WIDTH_0 = 947;
    const float effectiveW_float_0 = static_cast<float>(EFFECTIVE_BG_WIDTH_0);
    const float SCROLL_SPEED_0 = 3.0f;

    const int TILE_WIDTH_1 = CASTLEBACKGROUND1_WIDTH;
    const int TILE_HEIGHT_1 = CASTLEBACKGROUND1_HEIGHT;
    const int EFFECTIVE_BG_WIDTH_1 = 947;
    const float effectiveW_float_1 = static_cast<float>(EFFECTIVE_BG_WIDTH_1);
    const float SCROLL_SPEED_1 = 1.0f;

    const int TILE_WIDTH_2 = CASTLEBACKGROUND2_WIDTH;
    const int TILE_HEIGHT_2 = CASTLEBACKGROUND2_HEIGHT;
    const int EFFECTIVE_BG_WIDTH_2 = 947;
    const float effectiveW_float_2 = static_cast<float>(EFFECTIVE_BG_WIDTH_2);
    const float SCROLL_SPEED_2 = 0.5f;
};

#endif // GAME_H