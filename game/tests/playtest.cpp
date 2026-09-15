#include "asset_manager.h"
#include "boss.h"
#include "bullet.h"
#include "collision.h"
#include "enemy.h"
#include "hud.h"
#include "input.h"
#include "menu.h"
#include "player.h"
#include "powerup.h"
#include "stage.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <filesystem>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace galaxy;

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

SDL_Event keyEvent(SDL_Scancode key, Uint8 repeat = 0) {
    SDL_Event event{};
    event.type = SDL_KEYDOWN;
    event.key.keysym.scancode = key;
    event.key.repeat = repeat;
    return event;
}

struct RenderFixture {
    SDL_Surface* surface = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    AssetManager assets;

    RenderFixture() {
        surface = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_W, SCREEN_H, 32, SDL_PIXELFORMAT_RGBA32);
        require(surface != nullptr, SDL_GetError());
        renderer = SDL_CreateSoftwareRenderer(surface);
        require(renderer != nullptr, SDL_GetError());
        font = TTF_OpenFont("assets/fonts/PressStart2P-Regular.ttf", 12);
        require(font != nullptr, TTF_GetError());
        require(assets.init(renderer), IMG_GetError());
    }

    ~RenderFixture() {
        assets.shutdown();
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_FreeSurface(surface);
    }

    void clear() {
        SDL_SetRenderDrawColor(renderer, 4, 8, 24, 255);
        SDL_RenderClear(renderer);
    }

    std::vector<Uint8> pixels(SDL_Rect rect) {
        std::vector<Uint8> data(rect.w * rect.h * 4);
        require(SDL_RenderReadPixels(renderer, &rect, SDL_PIXELFORMAT_RGBA32,
                                     data.data(), rect.w * 4) == 0, SDL_GetError());
        return data;
    }

    void save(const std::filesystem::path& path) {
        SDL_RenderPresent(renderer);
        require(IMG_SavePNG(surface, path.string().c_str()) == 0, IMG_GetError());
    }
};

void captureScreens(RenderFixture& fixture, const std::filesystem::path& output) {
    std::filesystem::create_directories(output);
    Menu menu;
    fixture.clear();
    renderMenu(fixture.renderer, fixture.assets, fixture.font, menu, 1234567);
    fixture.save(output / "title.png");

    for (int i = 0; i < 3; ++i) {
        ShipSelect selection;
        selection.selectedShip = i;
        fixture.clear();
        renderShipSelect(fixture.renderer, fixture.assets, fixture.font, selection);
        fixture.save(output / ("ship-" + std::to_string(i) + ".png"));
    }

    Player player;
    initPlayer(player);
    player.score = 1234567;
    fixture.clear();
    renderHUD(fixture.renderer, fixture.assets, fixture.font, player, 1, 7654321);
    renderBossHP(fixture.renderer, fixture.font, 600, 1000, 2, 600.0f);
    fixture.save(output / "boss-hud.png");
}

void runTests(RenderFixture& fixture) {
    int failed = 0;
    int passed = 0;
    const auto test = [&](const char* name, const std::function<void()>& body) {
        try {
            body();
            std::cout << "PASS " << name << '\n';
            ++passed;
        } catch (const std::exception& error) {
            std::cerr << "FAIL " << name << ": " << error.what() << '\n';
            ++failed;
        }
    };

    test("held confirm does not skip the title screen", [] {
        Menu menu;
        require(handleMenuEvent(menu, keyEvent(SDL_SCANCODE_RETURN, 1)) == GameState::TITLE,
                "repeat Enter must not start ship selection");
        require(handleMenuEvent(menu, keyEvent(SDL_SCANCODE_SPACE, 1)) == GameState::TITLE,
                "repeat Space must not start ship selection");
    });
    test("held confirm does not choose a ship", [] {
        ShipSelect selection;
        ShipType ship = ShipType::GUNEX;
        require(handleShipSelectEvent(selection, keyEvent(SDL_SCANCODE_RETURN, 1), ship)
                    == GameState::SHIP_SELECT, "repeat Enter must stay in ship selection");
        require(ship == ShipType::GUNEX, "ignored events must not overwrite the selected ship");
    });
    test("held navigation does not cycle menu choices", [] {
        Menu menu;
        handleMenuEvent(menu, keyEvent(SDL_SCANCODE_DOWN, 1));
        require(menu.selectedOption == 0, "repeat Down moved the selection");
        ShipSelect selection;
        ShipType ship = ShipType::BAGON;
        handleShipSelectEvent(selection, keyEvent(SDL_SCANCODE_RIGHT, 1), ship);
        require(selection.selectedShip == 0, "repeat Right moved the ship selection");
    });
    test("fresh keyboard navigation still selects all three ships", [] {
        for (int i = 0; i < 3; ++i) {
            ShipSelect selection;
            ShipType ship = ShipType::BAGON;
            for (int step = 0; step < i; ++step)
                handleShipSelectEvent(selection, keyEvent(SDL_SCANCODE_RIGHT), ship);
            require(handleShipSelectEvent(selection, keyEvent(SDL_SCANCODE_RETURN), ship)
                        == GameState::PLAYING, "fresh Enter must start the game");
            require(ship == static_cast<ShipType>(i), "incorrect ship selected");
        }
    });
    test("high score requires a fresh return key", [] {
        require(handleHighScoreEvent(keyEvent(SDL_SCANCODE_RETURN, 1)) == GameState::HIGH_SCORE,
                "repeat Enter immediately dismissed high score");
        require(handleHighScoreEvent(keyEvent(SDL_SCANCODE_ESCAPE)) == GameState::TITLE,
                "fresh Escape must return to title");
    });
    test("pause toggles only on a fresh P or Escape press", [] {
        for (const auto key : {SDL_SCANCODE_P, SDL_SCANCODE_ESCAPE}) {
            require(handlePauseEvent(GameState::PLAYING, keyEvent(key)) == GameState::PAUSED,
                    "fresh pause key did not pause");
            require(handlePauseEvent(GameState::PAUSED, keyEvent(key, 1)) == GameState::PAUSED,
                    "held key resumed gameplay");
            require(handlePauseEvent(GameState::PAUSED, keyEvent(key)) == GameState::PLAYING,
                    "fresh pause key did not resume");
        }
        require(handlePauseEvent(GameState::CONTINUE, keyEvent(SDL_SCANCODE_ESCAPE))
                    == GameState::CONTINUE, "pause policy consumed continue-screen Escape");
    });
    test("focus loss pauses gameplay without automatically resuming", [] {
        for (const auto windowEvent : {SDL_WINDOWEVENT_FOCUS_LOST, SDL_WINDOWEVENT_MINIMIZED}) {
            SDL_Event event{};
            event.type = SDL_WINDOWEVENT;
            event.window.event = windowEvent;
            require(handlePauseEvent(GameState::PLAYING, event) == GameState::PAUSED,
                    "background gameplay did not pause");
            require(handlePauseEvent(GameState::TITLE, event) == GameState::TITLE,
                    "window event incorrectly paused title screen");
        }
        SDL_Event event{};
        event.type = SDL_WINDOWEVENT;
        event.window.event = SDL_WINDOWEVENT_FOCUS_GAINED;
        require(handlePauseEvent(GameState::PAUSED, event) == GameState::PAUSED,
                "regaining focus must not resume gameplay unexpectedly");
    });
    test("boss health leaves the score row intact", [&] {
        Player player;
        initPlayer(player);
        player.score = 1234567;
        fixture.clear();
        renderHUD(fixture.renderer, fixture.assets, fixture.font, player, 1, 7654321);
        const SDL_Rect scoreRow{0, 0, SCREEN_W, 24};
        const auto before = fixture.pixels(scoreRow);
        renderBossHP(fixture.renderer, fixture.font, 600, 1000, 2, 600.0f);
        require(before == fixture.pixels(scoreRow),
                "boss health rendering overwrote score, stage or high-score pixels");
    });
    test("menu backdrop is loaded at native resolution", [&] {
        SDL_Texture* texture = fixture.assets.get(SPR_UI_MENU_BACKDROP);
        require(texture != nullptr, "menu backdrop did not load");
        int width = 0;
        int height = 0;
        require(SDL_QueryTexture(texture, nullptr, nullptr, &width, &height) == 0, SDL_GetError());
        require(width == SCREEN_W && height == SCREEN_H, "menu backdrop dimensions are incorrect");
    });
    test("menu remains usable without the optional backdrop", [&] {
        AssetManager missing;
        fixture.clear();
        renderMenu(fixture.renderer, missing, fixture.font, Menu{});
        const SDL_Rect center{200, 295, 80, 24};
        const auto menuPixels = fixture.pixels(center);
        bool hasStartText = false;
        for (std::size_t i = 0; i < menuPixels.size(); i += 4) {
            if (menuPixels[i] == 255 && menuPixels[i + 1] == 232 && menuPixels[i + 2] == 0)
                hasStartText = true;
        }
        require(hasStartText, "fallback menu has no visible START label");
    });
    test("centered text matches measured font placement", [&] {
        const char* text = "DAMUL: WIDE SPREAD FOR CROWD CONTROL";
        int width = 0;
        int height = 0;
        require(TTF_SizeUTF8(fixture.font, text, &width, &height) == 0, TTF_GetError());
        require(width <= SCREEN_W - 32, "ship description does not fit the menu margins");
        fixture.clear();
        renderTextCentered(fixture.renderer, fixture.font, text, 388, {255, 255, 255, 255});
        const SDL_Rect region{0, 388, SCREEN_W, height};
        const auto centered = fixture.pixels(region);
        fixture.clear();
        renderText(fixture.renderer, fixture.font, text, (SCREEN_W - width) / 2, 388,
                   {255, 255, 255, 255});
        require(centered == fixture.pixels(region), "menu text is not centered at the measured width");
    });
    test("bomb clears enemy bullets without deleting player shots", [] {
        Player player;
        initPlayer(player);
        BulletPool bullets{};
        fireBullet(bullets, {100, 100}, {0, 100}, BulletOwner::ENEMY);
        fireBullet(bullets, {200, 200}, {0, -100}, BulletOwner::PLAYER);
        activateBomb(player, bullets);
        int enemyCount = 0;
        int playerCount = 0;
        for (const auto& bullet : bullets.pool) {
            if (!bullet.active) continue;
            if (bullet.owner == BulletOwner::ENEMY) ++enemyCount;
            else ++playerCount;
        }
        require(enemyCount == 0 && playerCount == 1, "incorrect bullet ownership clearing");
        require(player.bombStock == 2 && player.bombActive, "bomb stock/activation mismatch");
    });
    test("power pickup changes player power", [] {
        Player player;
        initPlayer(player);
        PowerUpPool powerUps{};
        spawnPowerUp(powerUps, player.pos, PowerUpType::POWER);
        checkPowerUpPickup(player, powerUps);
        require(player.powerLevel == 2, "overlapping power pickup did not increase power");
    });
    test("final life loss and player reset clear stale input state", [] {
        Player player;
        initPlayer(player);
        player.hp = 1;
        player.lives = 1;
        player.prevFireHeld = true;
        player.prevBombHeld = true;
        player.prevLockHeld = true;
        EnemyPool enemies{};
        require(spawnEnemy(enemies, EnemyType::SMALL, player.pos) != nullptr, "enemy spawn failed");
        checkPlayerEnemyCollision(player, enemies);
        require(!player.active && player.lives == 0, "final collision did not end the player's run");
        initPlayer(player, ShipType::DAMUL);
        require(player.active && player.lives == 3 && player.hp == 3, "new run did not reset health");
        require(!player.prevFireHeld && !player.prevBombHeld && !player.prevLockHeld,
                "new run inherited held buttons");
        require(player.shipType == ShipType::DAMUL, "new run lost the selected ship");
    });
    test("boss defeat consumes its projectile and awards score once", [] {
        Player player;
        initPlayer(player);
        Boss boss{};
        initBoss(boss, 1);
        const Rect bounds = boss.worldBounds();
        BulletPool bullets{};
        fireBullet(bullets, {bounds.x, bounds.y}, {0, -100}, BulletOwner::PLAYER, boss.hp);
        checkBulletBossCollision(bullets, boss, player);
        require(!boss.active && boss.hp == 0 && player.score == 10000, "boss defeat did not register");
        checkBulletBossCollision(bullets, boss, player);
        require(player.score == 10000 && !bullets.pool[0].active, "boss score was awarded twice");
    });
    test("all five stage scripts reach a boss and a clear state", [] {
        for (int number = 1; number <= 5; ++number) {
            Stage stage{};
            EnemyPool enemies{};
            Boss boss{};
            initStage(stage, number);
            for (int frame = 0; frame < 18000 && !boss.active; ++frame) {
                // Fixture removes spawned enemies; this is not a natural clear or difficulty test.
                for (auto& enemy : enemies.pool) enemy.active = false;
                updateStage(stage, 1.0f / FPS, enemies, boss);
            }
            require(boss.active && boss.hp > 0, "stage " + std::to_string(number) + " has no boss");
            boss.active = false;
            updateStage(stage, 1.0f / FPS, enemies, boss);
            require(stage.stageCleared, "boss defeat did not clear stage " + std::to_string(number));
        }
    });

    std::cout << passed << " passed, " << failed << " failed\n";
    require(failed == 0, "playtest regression checks failed");
}
} // namespace

int main(int argc, char** argv) {
    try {
        require(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER) == 0, SDL_GetError());
        require(TTF_Init() == 0, TTF_GetError());
        {
            RenderFixture fixture;
            if (argc == 3 && std::string(argv[1]) == "--capture") {
                captureScreens(fixture, argv[2]);
            } else if (argc == 1) {
                runTests(fixture);
            } else {
                throw std::runtime_error("Usage: GalaxyStormPlaytest [--capture DIRECTORY]");
            }
        }
        TTF_Quit();
        SDL_Quit();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
}
