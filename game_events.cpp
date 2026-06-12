

#include "game.h"
#include "level_config.h"
#include "auth.h"
#include "database.h"

static const float BTN_CX = 300.f, BTN_CY = 440.f;
static const float BTN_W = 200.f, BTN_H = 50.f;

// =============================================================
//  HELPER — loads a saved level from SessionManager into game state
// =============================================================
static void loadContinue(Game* g,
    int& currentLevelIdx, int& currentLevel,
    int& playerLives, int& playerScore,
    LevelConfig levelConfigs[],
    Platform platforms[], int& platformCount,
    sf::Texture charTex[], bool hasCharTex[],
    int selectedChar,
    PlayerController& playerCtrl,
    SnowballManager& snowballMgr,
    int W, int H)
{
    // This is just a flag — actual work done inline below
}

void Game::handleEvents() {
    sf::Event ev;
    while (window.pollEvent(ev)) {
        if (ev.type == sf::Event::Closed) { window.close(); return; }

        // ── SPLASH ───────────────────────────────────────────
        if (state == GameState::SPLASH && !transitioning) {
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Escape) window.close();
                if (ev.key.code == sf::Keyboard::Return
                    || ev.key.code == sf::Keyboard::Space) {
                    if (hasSplashSound) splashSound.stop();
                    runLoginScreen();   // <-- login before going to main menu
                }
            }
            if (ev.type == sf::Event::MouseButtonPressed
                && ev.mouseButton.button == sf::Mouse::Left) {
                float mx = (float)ev.mouseButton.x, my = (float)ev.mouseButton.y;
                if (mx >= BTN_CX - BTN_W / 2 && mx <= BTN_CX + BTN_W / 2
                    && my >= BTN_CY - BTN_H / 2 && my <= BTN_CY + BTN_H / 2) {
                    if (hasSplashSound) splashSound.stop();
                    runLoginScreen();   // <-- login before going to main menu
                }
            }
        }

        // ── MAIN MENU ────────────────────────────────────────
        if (state == GameState::MAIN_MENU && !transitioning) {
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Escape) window.close();
                if (ev.key.code == sf::Keyboard::Up)
                    menuSelected = (menuSelected - 1 + MENU_COUNT) % MENU_COUNT;
                if (ev.key.code == sf::Keyboard::Down)
                    menuSelected = (menuSelected + 1) % MENU_COUNT;
                if (ev.key.code == sf::Keyboard::Return) {
                    // NEW GAME
                    if (menuSelected == 0) startTransitionTo(GameState::CHARACTER_SELECT);

                    // CONTINUE — restore saved progress
                    if (menuSelected == 1) {
                        if (SessionManager::getInstance().isLoggedIn()) {
                            PlayerProgress p = SessionManager::getInstance().getLoadedProgress();
                            currentLevelIdx = p.current_Level - 1;
                            if (currentLevelIdx < 0 || currentLevelIdx >= MAX_LEVELS) currentLevelIdx = 0;
                            currentLevel = levelConfigs[currentLevelIdx].number;
                            playerLives = p.lives_Remaining;
                            playerScore = p.high_Score;
                            loadLevelAssets(currentLevelIdx);
                            buildLevelPlatforms(currentLevelIdx);
                            spawnEnemies(currentLevelIdx);
                            sf::Texture& tex = hasCharTex[selectedChar] ? charTex[selectedChar] : charTex[0];
                            playerCtrl.reset(tex, (float)W / 2, (float)H - 120);
                            snowballMgr.clear();
                            startTransitionTo(GameState::PLAYING);
                        }
                    }

                    if (menuSelected == 2) startTransitionTo(GameState::INSTRUCTIONS);
                    if (menuSelected == 3) startTransitionTo(GameState::LEVEL_SELECT);
                    if (menuSelected == 4) startTransitionTo(GameState::LEADERBOARD);
                    if (menuSelected == 5) window.close();
                }
            }
            if (ev.type == sf::Event::MouseButtonPressed
                && ev.mouseButton.button == sf::Mouse::Left) {
                float mx = (float)ev.mouseButton.x, my = (float)ev.mouseButton.y;
                const float bW = 260, bH = 62, gX = 16, gY = 18, sY = 170;
                float sX = (W - (bW * 2 + gX)) / 2;
                for (int i = 0; i < MENU_COUNT; ++i) {
                    float bx = sX + (i % 2) * (bW + gX);
                    float by = sY + (i / 2) * (bH + gY);
                    if (mx >= bx && mx <= bx + bW && my >= by && my <= by + bH) {
                        menuSelected = i;

                        // NEW GAME
                        if (i == 0) startTransitionTo(GameState::CHARACTER_SELECT);

                        // CONTINUE — restore saved progress
                        if (i == 1) {
                            if (SessionManager::getInstance().isLoggedIn()) {
                                PlayerProgress p = SessionManager::getInstance().getLoadedProgress();
                                currentLevelIdx = p.current_Level - 1;
                                if (currentLevelIdx < 0 || currentLevelIdx >= MAX_LEVELS) currentLevelIdx = 0;
                                currentLevel = levelConfigs[currentLevelIdx].number;
                                playerLives = p.lives_Remaining;
                                playerScore = p.high_Score;
                                loadLevelAssets(currentLevelIdx);
                                buildLevelPlatforms(currentLevelIdx);
                                spawnEnemies(currentLevelIdx);
                                sf::Texture& tex = hasCharTex[selectedChar] ? charTex[selectedChar] : charTex[0];
                                playerCtrl.reset(tex, (float)W / 2, (float)H - 120);
                                snowballMgr.clear();
                                startTransitionTo(GameState::PLAYING);
                            }
                        }

                        if (i == 2) startTransitionTo(GameState::INSTRUCTIONS);
                        if (i == 3) startTransitionTo(GameState::LEVEL_SELECT);
                        if (i == 4) startTransitionTo(GameState::LEADERBOARD);
                        if (i == 5) window.close();
                    }
                }
            }
        }

        // ── CHARACTER SELECT ─────────────────────────────────
        if (state == GameState::CHARACTER_SELECT && !transitioning) {
            if (ev.type == sf::Event::KeyPressed
                && ev.key.code == sf::Keyboard::Escape)
                startTransitionTo(GameState::MAIN_MENU);

            if (ev.type == sf::Event::MouseButtonPressed
                && ev.mouseButton.button == sf::Mouse::Left) {
                float mx = (float)ev.mouseButton.x, my = (float)ev.mouseButton.y;
                float bW = 160, bH = 200, gap = 20;
                float sx = (W - (bW * 3 + gap * 2)) / 2, by2 = 140;
                for (int i = 0; i < 3; ++i) {
                    float bx = sx + i * (bW + gap);
                    if (mx >= bx && mx <= bx + bW && my >= by2 && my <= by2 + bH) {
                        selectedChar = i;
                        startTransitionTo(GameState::LEVEL_SELECT);
                    }
                }
            }
        }

        // ── LEVEL SELECT ─────────────────────────────────────
        if (state == GameState::LEVEL_SELECT && !transitioning) {
            if (ev.type == sf::Event::KeyPressed
                && ev.key.code == sf::Keyboard::Escape)
                startTransitionTo(GameState::CHARACTER_SELECT);

            if (ev.type == sf::Event::MouseMoved) {
                float mx = (float)ev.mouseMove.x, my = (float)ev.mouseMove.y;
                const float cW = 265, cH = 68, gX = 14, gY = 10;
                float sx = (W - (cW * 2 + gX)) / 2, sy = 78;
                levelSelectHovered = -1;
                for (int i = 0; i < MAX_LEVELS; ++i) {
                    float bx = sx + (i % 2) * (cW + gX);
                    float by = sy + (i / 2) * (cH + gY);
                    if (mx >= bx && mx <= bx + cW && my >= by && my <= by + cH)
                        levelSelectHovered = i;
                }
            }

            if (ev.type == sf::Event::MouseButtonPressed
                && ev.mouseButton.button == sf::Mouse::Left) {
                float mx = (float)ev.mouseButton.x, my = (float)ev.mouseButton.y;
                const float cW = 265, cH = 68, gX = 14, gY = 10;
                float sx = (W - (cW * 2 + gX)) / 2, sy = 78;
                for (int i = 0; i < MAX_LEVELS; ++i) {
                    float bx = sx + (i % 2) * (cW + gX);
                    float by = sy + (i / 2) * (cH + gY);
                    if (!levels[i].locked
                        && mx >= bx && mx <= bx + cW
                        && my >= by && my <= by + cH)
                    {
                        currentLevelIdx = i;
                        currentLevel = levels[i].number;
                        loadLevelAssets(i);
                        buildLevelPlatforms(i);
                        spawnEnemies(i);
                        playerScore = 0; playerLives = 2; levelScore = 0;
                        sf::Texture& tex = hasCharTex[selectedChar]
                            ? charTex[selectedChar] : charTex[0];
                        playerCtrl.reset(tex, (float)W / 2, (float)H - 120);
                        snowballMgr.clear();
                        startTransitionTo(GameState::PLAYING);
                    }
                }
            }
        }

        // ── INSTRUCTIONS ─────────────────────────────────────
        if (state == GameState::INSTRUCTIONS && !transitioning) {
            if (ev.type == sf::Event::KeyPressed
                && ev.key.code == sf::Keyboard::Escape)
                startTransitionTo(GameState::MAIN_MENU);
            if (ev.type == sf::Event::MouseButtonPressed)
                startTransitionTo(GameState::MAIN_MENU);
        }

        // ── LEADERBOARD ──────────────────────────────────────
        if (state == GameState::LEADERBOARD && !transitioning)
            if (ev.type == sf::Event::KeyPressed
                && ev.key.code == sf::Keyboard::Escape)
                startTransitionTo(GameState::MAIN_MENU);

        // ── PLAYING ──────────────────────────────────────────
        if (state == GameState::PLAYING && !transitioning) {
            if (ev.type == sf::Event::KeyPressed) {

                // ESC — back to menu
                if (ev.key.code == sf::Keyboard::Escape) {
                    cleanupEnemies();
                    startTransitionTo(GameState::MAIN_MENU);
                }
                // B — toggle hitbox
                if (ev.key.code == sf::Keyboard::B)
                    debugHitbox = !debugHitbox;

                // N — next level (debug)
                if (ev.key.code == sf::Keyboard::N) {
                    currentLevelIdx = (currentLevelIdx + 1) % MAX_LEVELS;
                    currentLevel = levelConfigs[currentLevelIdx].number;
                    loadLevelAssets(currentLevelIdx);
                    buildLevelPlatforms(currentLevelIdx);
                    spawnEnemies(currentLevelIdx);
                    sf::Texture& tex = hasCharTex[selectedChar]
                        ? charTex[selectedChar] : charTex[0];
                    playerCtrl.reset(tex, (float)W / 2, (float)H - 120);
                    snowballMgr.clear();
                }
                // P — previous level (debug)
                if (ev.key.code == sf::Keyboard::P) {
                    currentLevelIdx = (currentLevelIdx - 1 + MAX_LEVELS) % MAX_LEVELS;
                    currentLevel = levelConfigs[currentLevelIdx].number;
                    loadLevelAssets(currentLevelIdx);
                    buildLevelPlatforms(currentLevelIdx);
                    spawnEnemies(currentLevelIdx);
                    sf::Texture& tex = hasCharTex[selectedChar]
                        ? charTex[selectedChar] : charTex[0];
                    playerCtrl.reset(tex, (float)W / 2, (float)H - 120);
                    snowballMgr.clear();
                }
            }
        }

        // ── LEVEL COMPLETE ────────────────────────────────────
        if (state == GameState::LEVEL_COMPLETE && !transitioning) {
            if (ev.type == sf::Event::MouseButtonPressed
                && ev.mouseButton.button == sf::Mouse::Left) {
                float mx = (float)ev.mouseButton.x, my = (float)ev.mouseButton.y;

                // NEXT LEVEL button — pixelBox(200, 330, 200, 46)
                if (mx >= 200.f && mx <= 400.f && my >= 330.f && my <= 376.f) {
                    int nextIdx = currentLevelIdx + 1;
                    if (nextIdx < MAX_LEVELS) {
                        currentLevelIdx = nextIdx;
                        currentLevel = levelConfigs[nextIdx].number;
                        loadLevelAssets(nextIdx);
                        buildLevelPlatforms(nextIdx);
                        spawnEnemies(nextIdx);
                        levelScore = 0;
                        sf::Texture& tex = hasCharTex[selectedChar]
                            ? charTex[selectedChar] : charTex[0];
                        playerCtrl.reset(tex, (float)W / 2, (float)H - 120);
                        snowballMgr.clear();
                        startTransitionTo(GameState::PLAYING);
                    }
                    else {
                        cleanupEnemies();
                        startTransitionTo(GameState::MAIN_MENU);
                    }
                }

                // LEVEL SELECT button — pixelBox(200, 390, 200, 46)
                if (mx >= 200.f && mx <= 400.f && my >= 390.f && my <= 436.f) {
                    cleanupEnemies();
                    startTransitionTo(GameState::LEVEL_SELECT);
                }
            }
        }

    } 
}