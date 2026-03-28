#pragma once
#include "types.h"
#include <SDL.h>
#include <SDL_ttf.h>

// Forward-declare subsystem structs to avoid pulling in every header here.
namespace galaxy {
    struct Player;
    struct BulletPool;
    struct EnemyPool;
    struct Boss;
    struct PowerUpPool;
    struct Stage;
    struct Background;
    struct Menu;
    struct ShipSelect;
    struct AssetManager;
    struct AudioManager;
}

namespace galaxy {

class Game {
public:
    Game();
    ~Game();

    // Initialise SDL2, SDL2_image, SDL2_mixer, SDL2_ttf, create the 480×640
    // window and renderer, load assets, allocate all subsystems.
    // Returns false on any failure (caller should exit).
    bool init();

    // Enter the fixed-timestep main loop (60 FPS).
    // Blocks until the player closes the window.
    void run();

    // Destroy all subsystems, renderer, window, and call SDL_Quit().
    void shutdown();

private:
    // ── SDL handles ──────────────────────────────────────────────────────────
    SDL_Window*   window_    = nullptr;
    SDL_Renderer* renderer_  = nullptr;

    // ── Game state ────────────────────────────────────────────────────────────
    GameState state_          = GameState::TITLE;
    bool      running_        = false;
    int       stageNum_       = 1;   // 1-based current stage (1–3)
    ShipType  selectedShip_   = ShipType::BAGON;

    // ── Subsystems ────────────────────────────────────────────────────────────
    Player*       player_       = nullptr;
    BulletPool*   bullets_      = nullptr;
    EnemyPool*    enemies_      = nullptr;
    Boss*         boss_         = nullptr;
    PowerUpPool*  powerUps_     = nullptr;
    Stage*        stage_        = nullptr;
    Background*   background_   = nullptr;
    Menu*         menu_         = nullptr;
    ShipSelect*   shipSelect_   = nullptr;
    AssetManager* assets_       = nullptr;
    AudioManager* audio_        = nullptr;
    TTF_Font*     font_         = nullptr;
    bool          bossMusicPlaying_ = false;

    // ── Per-frame helpers ─────────────────────────────────────────────────────

    // Poll and dispatch SDL events; updates running_ and state_ as needed.
    void handleEvents();

    // Update all active subsystems for dt seconds.
    void update(float dt);

    // Render one full frame: background → enemies → boss → powerups
    //   → player → bullets → HUD/overlay.
    void render();

    // Allocate and initialise gameplay for the given stage number.
    void startStage(int num);

    // Called when the boss of the current stage is defeated.
    void onStageClear();

    // Called when the player's life count reaches zero.
    void onGameOver();
};

} // namespace galaxy
