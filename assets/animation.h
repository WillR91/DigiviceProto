#pragma once

#include <cstdint>
#include <vector>

// Holds data for a single sprite frame
struct SpriteFrame {
    int width = 0;
    int height = 0;
    const uint16_t* data = nullptr; // Pointer to pixel data array
};

// Represents a sequence of frames for an animation
struct Animation {
    std::vector<SpriteFrame> frames;
    std::vector<uint32_t> frame_durations_ms; // Duration for each frame in milliseconds
    bool loops = true; // Does the animation loop?
    uint32_t total_duration = 0; // Can be calculated or set manually

    // Helper to add a frame easily
    void addFrame(const SpriteFrame& frame, uint32_t duration_ms) {
        frames.push_back(frame);
        frame_durations_ms.push_back(duration_ms);
        total_duration += duration_ms;
    }
}