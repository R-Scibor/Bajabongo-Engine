# Audio System

The Bajabongo Engine features a robust, event-driven audio system built on top of SFML. It supports pooled sound effects, streaming background music, and hierarchical volume control.

## 1. Overview

The audio system is fully decoupled from gameplay logic. Systems request audio playback by dispatching events, which the central `AudioSystem` handles. This ensures that gameplay code doesn't need to manage `sf::Sound` or `sf::Music` instances directly.

## 2. Asset Pipeline

### Directory Structure
*   **Sound Effects**: Place short audio files (`.wav`, `.ogg`) in `assets/sounds/`.
*   **Music**: Place long tracks (`.ogg`, `.mp3`) in `assets/music/`.

### Naming Conventions
*   Filenames become IDs (minus extension).
*   Example: `assets/sounds/gunshot.wav` -> ID: `"gunshot"`
*   Example: `assets/music/main_menu.ogg` -> ID: `"main_menu"`

### Updating the Manifest
After adding new files, run the asset baker tool to update `resources.json`:

```bash
cd tools
python AssetBaker.py
```

This will automatically populate the `sounds` and `music` sections in the manifest.

## 3. Usage

### Playing Sound Effects
Dispatch a `PlaySoundEvent` to play a one-shot sound effect. The system uses a pool of pre-allocated sound sources to prevent runtime allocations.

```cpp
#include "engine/events/AudioEvents.hpp"

// ... inside a system or state
dispatcher.enqueue<engine::PlaySoundEvent>({
    .id = "gunshot",
    .volume = 100.0f, // 0-100
    .pitch = 1.0f,    // 1.0 is normal speed
    .loop = false
});
```

### Playing Music
Dispatch a `PlayMusicEvent` to stream background music. Only one music track can play at a time. The system handles crossfading if specified.

```cpp
dispatcher.enqueue<engine::PlayMusicEvent>({
    .id = "ambient_track_1",
    .volume = 50.0f,
    .loop = true,
    .crossfadeDuration = 2.0f // Fade out old, fade in new over 2 seconds
});
```

### Stopping Music
To stop the current music track:

```cpp
dispatcher.enqueue<engine::StopMusicEvent>({
    .fadeOutDuration = 1.5f // Fade out over 1.5 seconds
});
```

### Volume Control
Volume can be controlled globally or per-category (Music/SFX). The Master volume scales both Music and SFX.

```cpp
// Set Master Volume to 80%
dispatcher.enqueue<engine::SetVolumeEvent>({
    .category = engine::AudioCategory::Master,
    .volume = 80.0f
});

// Set SFX Volume to 100% (scaled by Master)
dispatcher.enqueue<engine::SetVolumeEvent>({
    .category = engine::AudioCategory::SFX,
    .volume = 100.0f
});
```

## 4. Architecture

### AudioSystem
The `AudioSystem` class (in `src/engine/audio/`) is responsible for:
*   Listening to audio events.
*   Managing a fixed pool of `sf::Sound` instances (default 64) to avoid allocation overhead.
*   Managing a single `sf::Music` instance for streaming.
*   Handling volume mixing and fading logic in its `update()` loop.

### Resource Management
*   **Sounds**: Loaded into `sf::SoundBuffer` and cached in memory by `SFMLResourceManager`.
*   **Music**: Not cached in memory. The system stores file paths and streams them from disk on demand.

### Threading
The system runs on the main thread but delegates decoding and mixing to SFML's internal audio thread. This prevents audio processing from stalling the game loop, though event processing happens in the `update` phase.
