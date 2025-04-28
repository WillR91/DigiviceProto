#include "Game.h"
#include "platform/IDisplay.h" // Include interfaces
#include "platform/IInput.h"
#include "platform/pc/PCDisplay.h" // Include PC implementations FOR NOW
#include "platform/pc/PCInput.h"   // to allow creating them

#include <SDL.h> // Still need SDL for GetTicks, Delay etc. FOR NOW
#include <SDL_log.h>
#include <cmath> // For fmod
#include <stdexcept>

// --- Game Constructor ---
Game::Game() :
    display(nullptr),
    input(nullptr),
    isRunning(false),
    bg_data_0(castlebackground0_data),
    bg_data_1(castlebackground1_data),
    bg_data_2(castlebackground2_data),
    bg_scroll_offset_0(0.0f),
    bg_scroll_offset_1(0.0f),
    bg_scroll_offset_2(0.0f),
    current_state(STATE_IDLE),
    current_digimon(DIGI_AGUMON),
    active_anim(nullptr),
    current_anim_frame_idx(0),
    last_anim_update_time(0),
    queued_steps(0)
{
    // Create the platform-specific objects using concrete types for now
    display = new PCDisplay();
    input = new PCInput();
}

// --- Game Destructor ---
Game::~Game() {
    delete display;
    delete input;
}

// --- Initialize Game Systems ---
bool Game::initialize() {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    SDL_Log("--- Game Initialization ---");

    if (!display || !input) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Display or Input not created!");
        return false;
    }

    // Init display using the interface
    if (!display->init("Digivice Sim - Refactored", WINDOW_WIDTH, WINDOW_HEIGHT)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Display initialization failed!");
        return false;
    }
    // Note: Input doesn't have an init method currently

    // Set up initial game state (moved from old main)
    setupAnimations(); // Setup animation objects first
    current_state = STATE_IDLE;
    current_digimon = DIGI_AGUMON;
    queued_steps = 0;
    selectActiveAnimation(true); // Then select the starting animation

    last_anim_update_time = SDL_GetTicks(); // Initialize time

    isRunning = true;
    SDL_Log("--- Game Initialized Successfully ---");
    return true;
}

// --- Main Game Loop ---
void Game::run() {
    SDL_Log("--- Entering Game Loop ---");
    while (isRunning) {
        Uint32 currentTime = SDL_GetTicks(); // Use SDL_GetTicks for now

        handleInput();       // Process inputs
        update(currentTime); // Update game logic
        render();            // Draw the frame

        // Frame Limiter
        SDL_Delay(16); // Aim for ~60 FPS
    }
     SDL_Log("--- Exited Game Loop ---");
}

// --- Handle User Input ---
void Game::handleInput() {
    if (!input) return;

    input->update(); // This now polls SDL events inside PCInput

    if (input->isQuitRequested()) {
        isRunning = false;
        return;
    }

    // Check for step action (using the interface method)
    if (input->wasActionPressed(InputAction::STEP)) {
        if (queued_steps < MAX_QUEUED_STEPS) {
            queued_steps++;
             SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "Step Action Pressed (%d queued)", queued_steps);
        }
    }

    // Check for Digimon selection (using the interface method)
    InputAction selections[] = {
        InputAction::SELECT_DIGI_1, InputAction::SELECT_DIGI_2, InputAction::SELECT_DIGI_3,
        InputAction::SELECT_DIGI_4, InputAction::SELECT_DIGI_5, InputAction::SELECT_DIGI_6,
        InputAction::SELECT_DIGI_7, InputAction::SELECT_DIGI_8
    };
    bool character_changed_this_frame = false; // Track if change happened
    for(int i = 0; i < DIGI_COUNT; ++i) {
        if (input->wasActionPressed(selections[i])) {
            DigimonType selected_digi = static_cast<DigimonType>(i);
             if (selected_digi != current_digimon) {
                 current_digimon = selected_digi; SDL_Log("Switched character to %d", current_digimon);
                 current_state = STATE_IDLE; // Force idle on switch
                 queued_steps = 0; // Reset steps on switch
                 character_changed_this_frame = true; // Flag for animation reset
             }
             break; // Only process one selection per frame
        }
    }
    // Reset animation if character changed
    if (character_changed_this_frame) {
        selectActiveAnimation(true);
    }
}

// --- Update Game Logic ---
void Game::update(Uint32 currentTime) {
    bool needsAnimReset = false; // Track if animation needs changing this frame

    // --- State Transitions based on Input/Queue ---
    if (current_state == STATE_IDLE && queued_steps > 0) {
        current_state = STATE_WALKING;
        needsAnimReset = true;
        SDL_Log("State changed to WALKING");
    }

    // --- Update Scrolling based on State ---
    if (current_state == STATE_WALKING) {
        // Layer 0
        bg_scroll_offset_0 -= SCROLL_SPEED_0;
        while (bg_scroll_offset_0 < 0.0f) { bg_scroll_offset_0 += effectiveW_float_0; }
        bg_scroll_offset_0 = std::fmod(bg_scroll_offset_0, effectiveW_float_0);
        // Layer 1
        bg_scroll_offset_1 -= SCROLL_SPEED_1;
        while (bg_scroll_offset_1 < 0.0f) { bg_scroll_offset_1 += effectiveW_float_1; }
        bg_scroll_offset_1 = std::fmod(bg_scroll_offset_1, effectiveW_float_1);
        // Layer 2
        bg_scroll_offset_2 -= SCROLL_SPEED_2;
        while (bg_scroll_offset_2 < 0.0f) { bg_scroll_offset_2 += effectiveW_float_2; }
        bg_scroll_offset_2 = std::fmod(bg_scroll_offset_2, effectiveW_float_2);
    }

    // --- Animation Logic ---
    bool animation_cycle_finished = false;
    if (active_anim && !active_anim->frames.empty() && !active_anim->frame_durations_ms.empty()) {
         if (current_anim_frame_idx >= active_anim->frames.size() || current_anim_frame_idx >= active_anim->frame_durations_ms.size()) {
             current_anim_frame_idx = 0; // Reset if out of bounds
         }

         Uint32 current_frame_duration = active_anim->frame_durations_ms[current_anim_frame_idx];
         if (currentTime >= last_anim_update_time + current_frame_duration) {
             current_anim_frame_idx++;
             last_anim_update_time = currentTime;

             if (current_anim_frame_idx >= active_anim->frames.size()) {
                 animation_cycle_finished = true;
                 if (active_anim->loops) {
                     current_anim_frame_idx = 0; // Loop
                 } else {
                     current_anim_frame_idx = active_anim->frames.size() - 1; // Stay on last frame
                     if (current_anim_frame_idx < 0) current_anim_frame_idx = 0;
                 }
             }
         }
    } else {
        current_anim_frame_idx = 0;
    }

    // --- State Transitions based on Animation ---
     if (current_state == STATE_WALKING && animation_cycle_finished && !active_anim->loops) {
         queued_steps--; SDL_Log("Walk cycle finished. Steps remaining: %d", queued_steps);
         if (queued_steps > 0) { // Still steps left, restart walk animation
             current_anim_frame_idx = 0;
             last_anim_update_time = currentTime; // Reset timer for new cycle
             animation_cycle_finished = false;
             SDL_Log("Starting next queued walk cycle.");
             // Needs animation reset, but state stays WALKING
             needsAnimReset = true; // Signal to re-select walk animation (and reset its frame)
         } else { // No steps left, transition to idle
             SDL_Log("Switching to IDLE state.");
             current_state = STATE_IDLE;
             needsAnimReset = true; // Signal to select idle animation
         }
     }

    // --- Select correct animation if needed ---
    if (needsAnimReset) {
        selectActiveAnimation(true); // Force reset timing etc.
    }
}

// --- Render the Game Frame ---
void Game::render() {
    if (!display) return;

    display->clear(0x0000); // Use interface pointer

    // --- Draw Background Layers ---
    int draw2_x1 = -static_cast<int>(bg_scroll_offset_2);
    int draw2_x2 = draw2_x1 + EFFECTIVE_BG_WIDTH_2;
    drawClippedTile(draw2_x1, bg_data_2, TILE_WIDTH_2, TILE_HEIGHT_2);
    drawClippedTile(draw2_x2, bg_data_2, TILE_WIDTH_2, TILE_HEIGHT_2);

    int draw1_x1 = -static_cast<int>(bg_scroll_offset_1);
    int draw1_x2 = draw1_x1 + EFFECTIVE_BG_WIDTH_1;
    drawClippedTile(draw1_x1, bg_data_1, TILE_WIDTH_1, TILE_HEIGHT_1);
    drawClippedTile(draw1_x2, bg_data_1, TILE_WIDTH_1, TILE_HEIGHT_1);

    // --- Draw Character Sprite ---
    if (active_anim && current_anim_frame_idx < active_anim->frames.size()) {
        const SpriteFrame& frame = active_anim->frames[current_anim_frame_idx];
        if (frame.data) {
            int draw_x = (WINDOW_WIDTH / 2) - (frame.width / 2);
            int draw_y = (WINDOW_HEIGHT / 2) - (frame.height / 2);
            // int draw_y = WINDOW_HEIGHT - frame.height - 10; // Align bottom example
            display->drawPixels(draw_x, draw_y, frame.width, frame.height,
                               frame.data, frame.width, frame.height, 0, 0);
        }
    }

    // --- Draw Foreground Layer ---
    int draw0_x1 = -static_cast<int>(bg_scroll_offset_0);
    int draw0_x2 = draw0_x1 + EFFECTIVE_BG_WIDTH_0;
    drawClippedTile(draw0_x1, bg_data_0, TILE_WIDTH_0, TILE_HEIGHT_0);
    drawClippedTile(draw0_x2, bg_data_0, TILE_WIDTH_0, TILE_HEIGHT_0);

    // --- Present the final frame ---
    display->present(); // Use interface pointer
}

// --- Cleanup Game Systems ---
void Game::cleanup() {
     SDL_Log("--- Cleaning up Game ---");
    if (display) {
        display->close(); // Close display via interface
    }
    // Input cleanup might be added later if needed
    SDL_Quit(); // Quit SDL subsystems here, after display is closed
     SDL_Log("--- Game Cleanup Finished ---");
}

// --- Helper: Setup Animation Objects ---
void Game::setupAnimations() {
     // This logic is identical to your old main.cpp, just inside the Game class
     SpriteFrame agumon_idle_0_sf = {AGUMON_IDLE_0_WIDTH, AGUMON_IDLE_0_HEIGHT, Agumon_Idle_0_data};
     SpriteFrame agumon_idle_1_sf = {AGUMON_IDLE_1_WIDTH, AGUMON_IDLE_1_HEIGHT, Agumon_Idle_1_data};
     SpriteFrame agumon_walk_0_sf = {AGUMON_WALK_0_WIDTH, AGUMON_WALK_0_HEIGHT, Agumon_Walk_0_data};
     SpriteFrame agumon_walk_1_sf = {AGUMON_WALK_1_WIDTH, AGUMON_WALK_1_HEIGHT, Agumon_Walk_1_data};
     SpriteFrame gabumon_idle_0_sf = {GABUMON_IDLE_0_WIDTH, GABUMON_IDLE_0_HEIGHT, Gabumon_Idle_0_data};
     SpriteFrame gabumon_idle_1_sf = {GABUMON_IDLE_1_WIDTH, GABUMON_IDLE_1_HEIGHT, Gabumon_Idle_1_data};
     SpriteFrame gabumon_walk_0_sf = {GABUMON_WALK_0_WIDTH, GABUMON_WALK_0_HEIGHT, Gabumon_Walk_0_data};
     SpriteFrame gabumon_walk_1_sf = {GABUMON_WALK_1_WIDTH, GABUMON_WALK_1_HEIGHT, Gabumon_Walk_1_data};
     SpriteFrame biyomon_idle_0_sf = {BIYOMON_IDLE_0_WIDTH, BIYOMON_IDLE_0_HEIGHT, Biyomon_Idle_0_data};
     SpriteFrame biyomon_idle_1_sf = {BIYOMON_IDLE_1_WIDTH, BIYOMON_IDLE_1_HEIGHT, Biyomon_Idle_1_data};
     SpriteFrame biyomon_walk_0_sf = {BIYOMON_WALK_0_WIDTH, BIYOMON_WALK_0_HEIGHT, Biyomon_Walk_0_data};
     SpriteFrame biyomon_walk_1_sf = {BIYOMON_WALK_1_WIDTH, BIYOMON_WALK_1_HEIGHT, Biyomon_Walk_1_data};
     SpriteFrame gatomon_idle_0_sf = {GATOMON_IDLE_0_WIDTH, GATOMON_IDLE_0_HEIGHT, Gatomon_Idle_0_data};
     SpriteFrame gatomon_idle_1_sf = {GATOMON_IDLE_1_WIDTH, GATOMON_IDLE_1_HEIGHT, Gatomon_Idle_1_data};
     SpriteFrame gatomon_walk_0_sf = {GATOMON_WALK_0_WIDTH, GATOMON_WALK_0_HEIGHT, Gatomon_Walk_0_data};
     SpriteFrame gatomon_walk_1_sf = {GATOMON_WALK_1_WIDTH, GATOMON_WALK_1_HEIGHT, Gatomon_Walk_1_data};
     SpriteFrame gomamon_idle_0_sf = {GOMAMON_IDLE_0_WIDTH, GOMAMON_IDLE_0_HEIGHT, Gomamon_Idle_0_data};
     SpriteFrame gomamon_idle_1_sf = {GOMAMON_IDLE_1_WIDTH, GOMAMON_IDLE_1_HEIGHT, Gomamon_Idle_1_data};
     SpriteFrame gomamon_walk_0_sf = {GOMAMON_WALK_0_WIDTH, GOMAMON_WALK_0_HEIGHT, Gomamon_Walk_0_data};
     SpriteFrame gomamon_walk_1_sf = {GOMAMON_WALK_1_WIDTH, GOMAMON_WALK_1_HEIGHT, Gomamon_Walk_1_data};
     SpriteFrame palmon_idle_0_sf = {PALMON_IDLE_0_WIDTH, PALMON_IDLE_0_HEIGHT, Palmon_Idle_0_data};
     SpriteFrame palmon_idle_1_sf = {PALMON_IDLE_1_WIDTH, PALMON_IDLE_1_HEIGHT, Palmon_Idle_1_data};
     SpriteFrame palmon_walk_0_sf = {PALMON_WALK_0_WIDTH, PALMON_WALK_0_HEIGHT, Palmon_Walk_0_data};
     SpriteFrame palmon_walk_1_sf = {PALMON_WALK_1_WIDTH, PALMON_WALK_1_HEIGHT, Palmon_Walk_1_data};
     SpriteFrame tentomon_idle_0_sf = {TENTOMON_IDLE_0_WIDTH, TENTOMON_IDLE_0_HEIGHT, Tentomon_Idle_0_data};
     SpriteFrame tentomon_idle_1_sf = {TENTOMON_IDLE_1_WIDTH, TENTOMON_IDLE_1_HEIGHT, Tentomon_Idle_1_data};
     SpriteFrame tentomon_walk_0_sf = {TENTOMON_WALK_0_WIDTH, TENTOMON_WALK_0_HEIGHT, Tentomon_Walk_0_data};
     SpriteFrame tentomon_walk_1_sf = {TENTOMON_WALK_1_WIDTH, TENTOMON_WALK_1_HEIGHT, Tentomon_Walk_1_data};
     SpriteFrame patamon_idle_0_sf = {PATAMON_IDLE_0_WIDTH, PATAMON_IDLE_0_HEIGHT, Patamon_Idle_0_data};
     SpriteFrame patamon_idle_1_sf = {PATAMON_IDLE_1_WIDTH, PATAMON_IDLE_1_HEIGHT, Patamon_Idle_1_data};
     SpriteFrame patamon_walk_0_sf = {PATAMON_WALK_0_WIDTH, PATAMON_WALK_0_HEIGHT, Patamon_Walk_0_data};
     SpriteFrame patamon_walk_1_sf = {PATAMON_WALK_1_WIDTH, PATAMON_WALK_1_HEIGHT, Patamon_Walk_1_data};

    agumon_idle_anim.addFrame(agumon_idle_0_sf, 1000); agumon_idle_anim.addFrame(agumon_idle_1_sf, 1000); agumon_idle_anim.loops = true;
    agumon_walk_anim.addFrame(agumon_walk_0_sf, 300); agumon_walk_anim.addFrame(agumon_walk_1_sf, 300); agumon_walk_anim.addFrame(agumon_walk_0_sf, 300); agumon_walk_anim.addFrame(agumon_walk_1_sf, 300); agumon_walk_anim.loops = false;
    gabumon_idle_anim.addFrame(gabumon_idle_0_sf, 1100); gabumon_idle_anim.addFrame(gabumon_idle_1_sf, 1100); gabumon_idle_anim.loops = true;
    gabumon_walk_anim.addFrame(gabumon_walk_0_sf, 320); gabumon_walk_anim.addFrame(gabumon_walk_1_sf, 320); gabumon_walk_anim.addFrame(gabumon_walk_0_sf, 320); gabumon_walk_anim.addFrame(gabumon_walk_1_sf, 320); gabumon_walk_anim.loops = false;
    biyomon_idle_anim.addFrame(biyomon_idle_0_sf, 960); biyomon_idle_anim.addFrame(biyomon_idle_1_sf, 960); biyomon_idle_anim.loops = true;
    biyomon_walk_anim.addFrame(biyomon_walk_0_sf, 280); biyomon_walk_anim.addFrame(biyomon_walk_1_sf, 280); biyomon_walk_anim.addFrame(biyomon_walk_0_sf, 280); biyomon_walk_anim.addFrame(biyomon_walk_1_sf, 280); biyomon_walk_anim.loops = false;
    gatomon_idle_anim.addFrame(gatomon_idle_0_sf, 1200); gatomon_idle_anim.addFrame(gatomon_idle_1_sf, 1200); gatomon_idle_anim.loops = true;
    gatomon_walk_anim.addFrame(gatomon_walk_0_sf, 340); gatomon_walk_anim.addFrame(gatomon_walk_1_sf, 340); gatomon_walk_anim.addFrame(gatomon_walk_0_sf, 340); gatomon_walk_anim.addFrame(gatomon_walk_1_sf, 340); gatomon_walk_anim.loops = false;
    gomamon_idle_anim.addFrame(gomamon_idle_0_sf, 1040); gomamon_idle_anim.addFrame(gomamon_idle_1_sf, 1040); gomamon_idle_anim.loops = true;
    gomamon_walk_anim.addFrame(gomamon_walk_0_sf, 310); gomamon_walk_anim.addFrame(gomamon_walk_1_sf, 310); gomamon_walk_anim.addFrame(gomamon_walk_0_sf, 310); gomamon_walk_anim.addFrame(gomamon_walk_1_sf, 310); gomamon_walk_anim.loops = false;
    palmon_idle_anim.addFrame(palmon_idle_0_sf, 1080); palmon_idle_anim.addFrame(palmon_idle_1_sf, 1080); palmon_idle_anim.loops = true;
    palmon_walk_anim.addFrame(palmon_walk_0_sf, 330); palmon_walk_anim.addFrame(palmon_walk_1_sf, 330); palmon_walk_anim.addFrame(palmon_walk_0_sf, 330); palmon_walk_anim.addFrame(palmon_walk_1_sf, 330); palmon_walk_anim.loops = false;
    tentomon_idle_anim.addFrame(tentomon_idle_0_sf, 920); tentomon_idle_anim.addFrame(tentomon_idle_1_sf, 920); tentomon_idle_anim.loops = true;
    tentomon_walk_anim.addFrame(tentomon_walk_0_sf, 290); tentomon_walk_anim.addFrame(tentomon_walk_1_sf, 290); tentomon_walk_anim.addFrame(tentomon_walk_0_sf, 290); tentomon_walk_anim.addFrame(tentomon_walk_1_sf, 290); tentomon_walk_anim.loops = false;
    patamon_idle_anim.addFrame(patamon_idle_0_sf, 1060); patamon_idle_anim.addFrame(patamon_idle_1_sf, 1060); patamon_idle_anim.loops = true;
    patamon_walk_anim.addFrame(patamon_walk_0_sf, 300); patamon_walk_anim.addFrame(patamon_walk_1_sf, 300); patamon_walk_anim.addFrame(patamon_walk_0_sf, 300); patamon_walk_anim.addFrame(patamon_walk_1_sf, 300); patamon_walk_anim.loops = false;

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Digimon animations setup complete.");
}

// --- Helper: Select Correct Animation Based on State/Digimon ---
void Game::selectActiveAnimation(bool forceReset) {
     Animation* previous_anim = active_anim;

     if (current_state == STATE_IDLE) {
         switch(current_digimon) { // Copied from old main
            case DIGI_AGUMON:   active_anim = &agumon_idle_anim; break;
            case DIGI_GABUMON:  active_anim = &gabumon_idle_anim; break;
            case DIGI_BIYOMON:  active_anim = &biyomon_idle_anim; break;
            case DIGI_GATOMON:  active_anim = &gatomon_idle_anim; break;
            case DIGI_GOMAMON:  active_anim = &gomamon_idle_anim; break;
            case DIGI_PALMON:   active_anim = &palmon_idle_anim; break;
            case DIGI_TENTOMON: active_anim = &tentomon_idle_anim; break;
            case DIGI_PATAMON:  active_anim = &patamon_idle_anim; break;
            default:            active_anim = &agumon_idle_anim;
         }
     } else { // STATE_WALKING
          switch(current_digimon) { // Copied from old main
            case DIGI_AGUMON:   active_anim = &agumon_walk_anim; break;
            case DIGI_GABUMON:  active_anim = &gabumon_walk_anim; break;
            case DIGI_BIYOMON:  active_anim = &biyomon_walk_anim; break;
            case DIGI_GATOMON:  active_anim = &gatomon_walk_anim; break;
            case DIGI_GOMAMON:  active_anim = &gomamon_walk_anim; break;
            case DIGI_PALMON:   active_anim = &palmon_walk_anim; break;
            case DIGI_TENTOMON: active_anim = &tentomon_walk_anim; break;
            case DIGI_PATAMON:  active_anim = &patamon_walk_anim; break;
            default:            active_anim = &agumon_walk_anim;
         }
     }

     // Reset frame index and timer if the animation changed OR if forced
     if (forceReset || active_anim != previous_anim) {
         current_anim_frame_idx = 0;
         // Always reset timer when animation changes/is forced for simplicity here
         last_anim_update_time = SDL_GetTicks();
         SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Animation selected/reset.");
     }
}


// --- Helper: Draw Background Tile Portion ---
void Game::drawClippedTile(int dest_x_unclipped, const uint16_t* tile_data,
                         int layer_tile_width, int layer_tile_height)
{
    // This logic is identical to the lambda in your old main.cpp
    if (!display || !tile_data) return;

    int src_x = 0, src_y = 0;
    int src_w = layer_tile_width, src_h = layer_tile_height;
    int dest_x = dest_x_unclipped, dest_y = 0;
    int dest_w = layer_tile_width, dest_h = layer_tile_height;

    // Clip Left
    if (dest_x < 0) {
        int clip = -dest_x; if (clip >= layer_tile_width) return;
        src_x += clip; src_w -= clip; dest_w -= clip; dest_x = 0;
    }
    // Clip Right
    if (dest_x + dest_w > WINDOW_WIDTH) {
        int clip = (dest_x + dest_w) - WINDOW_WIDTH; if (clip >= layer_tile_width) return;
        src_w -= clip; dest_w -= clip;
    }
    // Clip Bottom
    if (dest_y + dest_h > WINDOW_HEIGHT) {
         int clip = (dest_y + dest_h) - WINDOW_HEIGHT; if (clip >= layer_tile_height) return;
         src_h -= clip; dest_h -= clip;
    }

    // Draw if visible using the display interface
    if (dest_w > 0 && src_w > 0 && dest_h > 0 && src_h > 0) {
        display->drawPixels(dest_x, dest_y, dest_w, dest_h, tile_data,
                           layer_tile_width, layer_tile_height, src_x, src_y);
    }
}