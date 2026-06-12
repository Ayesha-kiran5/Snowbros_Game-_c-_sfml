

#include "game.h"
#include <cstring>
#include <sstream>
#include "level_config.h"

// =============================================================
//  FREE HELPERS  (defined once here, declared in game.h)
// =============================================================
float frand(float lo, float hi) {
    return lo + (hi - lo) * (static_cast<float>(rand()) / RAND_MAX);
}
void spawnFlake(Snowflake& f, bool anywhere) {
    f.x = frand(0, WIN_W);
    f.y = anywhere ? frand(0, WIN_H) : frand(-20, -2);
    f.speed = frand(40, 110);
    f.radius = frand(1.5f, 4.5f);
    f.drift = frand(8, 25);
    f.driftOffset = frand(0, 6.28f);
    f.opacity = frand(140, 255);
}

// =============================================================
//  CONSTRUCTOR
// =============================================================
Game::Game()
    : window(sf::VideoMode(WIN_W, WIN_H), "Snow Bros Arcade Game",
        sf::Style::Close | sf::Style::Titlebar),
    state(GameState::SPLASH), nextState(GameState::SPLASH),
    transAlpha(0), transitioning(false), totalTime(0), tick(0),
    hasSplashBg(false), hasSplashSound(false), hasMenuMusic(false),
    topCount(0), botCount(0), logoPulse(0), glowPulse(0),
    menuSelected(0), hoveredLevel(-1), levelSelectHovered(-1),
    selectedChar(0), platformCount(0),
    currentLevel(1), currentLevelIdx(0),
    hasLevelBg(false),
    enemyCount(0), playerWorldX(0.f), playerWorldY(0.f),
    debugHitbox(false),
    playerScore(0), playerLives(2), levelScore(0),
    hasLogo(false), hasEndBg(false),
    GRAVITY(GRAVITY_DEFAULT),
    MOVE_SPEED(MOVE_SPEED_DEFAULT),
    JUMP_SPEED(JUMP_SPEED_DEFAULT)
{
    srand(static_cast<unsigned>(time(0)));
    for (int i = 0; i < MAX_ENEMIES; ++i) enemies[i] = nullptr;

    // Font
    if (!font.loadFromFile("arial.ttf"))
        if (!font.loadFromFile("arial.TTF"))
            if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
                font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf");

    // Splash background
    hasSplashBg = splashTex.loadFromFile("snowbros.png");
    if (!hasSplashBg) hasSplashBg = splashTex.loadFromFile("snowbros.jpeg");
    if (hasSplashBg) {
        splashTex.setSmooth(false);
        splashSprite.setTexture(splashTex);
        splashSprite.setScale(
            (float)WIN_W / splashTex.getSize().x,
            (float)WIN_H / splashTex.getSize().y);
    }

    // Menu background
    if (!bgTexture.loadFromFile("background.png"))
        bgTexture.loadFromFile("background.jpeg");
    bgTexture.setSmooth(false);
    bgSprite.setTexture(bgTexture);
    if (bgTexture.getSize().x > 0)
        bgSprite.setScale(
            (float)WIN_W / bgTexture.getSize().x,
            (float)WIN_H / bgTexture.getSize().y);

    // Logo
    hasLogo = logoTex.loadFromFile("logo.png");
    if (hasLogo) {
        logoTex.setSmooth(true);
        logoSprite.setTexture(logoTex);
    }

    // End screen background
    hasEndBg = endBgTex.loadFromFile("background.png");
    if (!hasEndBg) hasEndBg = endBgTex.loadFromFile("background.jpeg");
    if (hasEndBg) {
        endBgTex.setSmooth(false);
        endBgSprite.setTexture(endBgTex);
        endBgSprite.setScale(
            (float)W / endBgTex.getSize().x,
            (float)H / endBgTex.getSize().y);
    }

    // Audio
    hasSplashSound = splashBuf.loadFromFile("mainstartsound.flac");
    if (hasSplashSound) {
        splashSound.setBuffer(splashBuf);
        splashSound.setLoop(false);
        splashSound.setVolume(70);
        splashSound.play();
    }
    hasMenuMusic = menuMusic.openFromFile("snow_bros_theme_02.ogg");
    if (hasMenuMusic) { menuMusic.setLoop(true); menuMusic.setVolume(55); }

    // Platform texture
    if (platformTex.loadFromFile("Platform.png")) {
        platformTex.setSmooth(false);
        platformTex.setRepeated(true);
    }
    if (platformTex2.loadFromFile("platform2.png")) {
        platformTex2.setSmooth(false);
        platformTex2.setRepeated(true);
    }
    // Character sprite sheets
    hasCharTex[0] = charTex[0].loadFromFile("Player_Blue.png");
    hasCharTex[1] = charTex[1].loadFromFile("Player_Red.png");
    hasCharTex[2] = charTex[2].loadFromFile("Player_Green.png");
    for (int i = 0; i < 3; ++i)
        if (hasCharTex[i]) charTex[i].setSmooth(false);

    // Splash snowflakes
    for (int i = 0; i < MAX_FLAKES; ++i) spawnFlake(sFlakes[i], true);
    sFlakeShape.setPointCount(8);
    sOverlay.setSize(sf::Vector2f(WIN_W, WIN_H));
    sOverlay.setFillColor(sf::Color(0, 10, 30, 110));

    buildLevelConfigs(levelConfigs, W, H);
    initSplash();
    buildLevels();
    spawnGFlakes();
}

// =============================================================
//  DESTRUCTOR
// =============================================================
Game::~Game() {
    if (hasSplashSound) splashSound.stop();
    if (hasMenuMusic)   menuMusic.stop();
    cleanupEnemies();
}

// =============================================================
//  MAIN LOOP
// =============================================================
void Game::run() {
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;
        totalTime += dt;
        handleEvents();
        update(dt);
        render();
    }
}

// =============================================================
//  UPDATE
// =============================================================
void Game::update(float dt) {
    logoPulse = glowPulse = totalTime;
    ++tick;

    if (state == GameState::SPLASH || transitioning) updateSplash(dt);

    // Advance menu snowflakes
    for (int i = 0; i < G_MAX_FLAKES; ++i) {
        gFlakes[i].y += gFlakes[i].speed * dt;
        if (gFlakes[i].y > H + 5) {
            gFlakes[i].y = -5;
            gFlakes[i].x = (float)(rand() % W);
        }
    }

    // Menu music control
    if (state == GameState::MAIN_MENU && hasMenuMusic) {
        if (menuMusic.getStatus() != sf::Music::Playing) {
            if (hasSplashSound) splashSound.stop();
            menuMusic.play();
        }
    }
    if (state != GameState::MAIN_MENU && hasMenuMusic
        && menuMusic.getStatus() == sf::Music::Playing)
        menuMusic.stop();

    if (state == GameState::PLAYING) updatePlaying(dt);

    // Transition fade
    if (transitioning) {
        transAlpha += dt * 300;
        if (transAlpha >= 300) {
            transAlpha = 0;
            state = nextState;
            transitioning = false;
        }
    }
}
void Game::runLoginScreen() {
    LoginScreen loginScreen(window);
    LoginScreenResult result = loginScreen.run();

    if (result == LoginScreenResult::EXIT) {
        window.close();
        return;
    }

    // Restore saved progress into game state
    if (SessionManager::getInstance().isLoggedIn()) {
        PlayerProgress p = SessionManager::getInstance().getLoadedProgress();
        playerScore = p.high_Score;
        playerLives = p.lives_Remaining;
        currentLevelIdx = p.current_Level - 1;
        if (currentLevelIdx < 0 || currentLevelIdx >= MAX_LEVELS)
            currentLevelIdx = 0;
        currentLevel = levelConfigs[currentLevelIdx].number;
    }

    state = GameState::MAIN_MENU;
    nextState = GameState::MAIN_MENU;
    transitioning = false;
}
// =============================================================
//  RENDER
// =============================================================
void Game::render() {
    window.clear(sf::Color(Theme::BG_DARK.r, Theme::BG_DARK.g, Theme::BG_DARK.b));

    if (transitioning)                              drawTransition();
    else if (state == GameState::SPLASH)             drawSplash();
    else if (state == GameState::MAIN_MENU)          draw_mainmenu();
    else if (state == GameState::CHARACTER_SELECT)   draw_characterselect();
    else if (state == GameState::LEVEL_SELECT)       drawLevelSelect();
    else if (state == GameState::INSTRUCTIONS)       drawInstructions();
    else if (state == GameState::PLAYING)            drawPlaying();
    else if (state == GameState::LEADERBOARD)        drawLeaderboard();
    else if (state == GameState::LEVEL_COMPLETE)     drawLevelComplete();

    window.display();
}

// =============================================================
//  TRANSITION
// =============================================================
void Game::startTransitionTo(GameState t) {
    if (transitioning) return;
    nextState = t;
    transAlpha = 0;
    transitioning = true;
}

void Game::drawTransition() {
    auto ds = [&](GameState s) {
        switch (s) {
        case GameState::SPLASH:           drawSplash();          break;
        case GameState::MAIN_MENU:        draw_mainmenu();        break;
        case GameState::CHARACTER_SELECT: draw_characterselect(); break;
        case GameState::LEVEL_SELECT:     drawLevelSelect();     break;
        case GameState::INSTRUCTIONS:     drawInstructions();    break;
        case GameState::PLAYING:          drawPlaying();         break;
        case GameState::LEADERBOARD:      drawLeaderboard();     break;
        case GameState::LEVEL_COMPLETE:   drawLevelComplete();   break;
        default: break;
        }
        };

    sf::RectangleShape ov(sf::Vector2f((float)WIN_W, (float)WIN_H));
    if (transAlpha < 150) {
        ds(state);
        ov.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(transAlpha / 150 * 255)));
    }
    else {
        ds(nextState);
        float p = (transAlpha - 150) / 150;
        ov.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)((1 - p) * 255)));
    }
    window.draw(ov);
}