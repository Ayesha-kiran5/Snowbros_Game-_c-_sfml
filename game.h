#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "database.h"
#include "auth.h"
#include "player.h"
#include "snowball.h"
#include "Enemies.h"
#include "level_config.h"
static const int MAX_FLAKES = 180;
static const int G_MAX_FLAKES = 120;
static const int MAX_PLATFORMS = 32;
static const int MAX_LETTERS = 12;
static const int MENU_COUNT = 6;
static const int WIN_W = 600;
static const int WIN_H = 600;
static const int W = 600;
static const int H = 600;
// =============================================================
//  LIGHT-BLUE THEME PALETTE
// =============================================================
namespace Theme {
    static const sf::Color BG_DARK(8, 28, 55, 255);
    static const sf::Color BTN_FILL(18, 70, 120, 255);
    static const sf::Color BTN_BORDER(80, 180, 240, 255);
    static const sf::Color BONUS_COL(255, 210, 50, 255);
    static const sf::Color LOCKED_COL(100, 130, 160, 255);
    static const sf::Color BTN_TEXT(180, 225, 255, 255);
    static const sf::Color BTN_FILL_SEL(30, 110, 180, 255);
    static const sf::Color BTN_BDR_SEL(0, 220, 255, 255);
    static const sf::Color BTN_TEXT_SEL(220, 245, 255, 255);
    static const sf::Color ACCENT(0, 210, 255, 255);
    static const sf::Color ACCENT_DIM(100, 180, 220, 255);
    static const sf::Color BOSS_COL(255, 100, 100, 255);
}

// =============================================================
//  APP STATES
// =============================================================
enum class GameState {
    SPLASH,
    MAIN_MENU,
    CHARACTER_SELECT,
    LEVEL_SELECT,
    INSTRUCTIONS,
    PLAYING,
    LEADERBOARD,
    LEVEL_COMPLETE    
};

// =============================================================
//  STRUCTS
// =============================================================
struct Snowflake { float x, y, speed, radius, drift, driftOffset, opacity; };
struct GSnowflake { float x, y, speed, radius, opacity; };
struct Platform { sf::RectangleShape shape; };

struct Level {
    int  number;
    char name[32];
    char description[64];
    bool isBoss;
    bool isBonus;
    bool locked;
};

// =============================================================
//  FREE HELPERS
// =============================================================
float frand(float lo, float hi);
void  spawnFlake(Snowflake& f, bool anywhere);

// =============================================================
//  GAME CLASS
// =============================================================
class Game {
public:
    Game();
    ~Game();
    void run();

private:
    // ── Core loop 
    void handleEvents();
    void update(float dt);
    void render();

    
    void initSplash();
    void updateSplash(float dt);
    void drawSplash();
    void drawSplashButton();
    void drawArcText();

    // screens
    void drawInstructions(float alpha = 1.f);
    void drawLeaderboard(float alpha = 1.f);
    void draw_mainmenu(float alpha = 1.f);
    void drawPlaying(float alpha = 1.f);
    void initCharacters();
    void draw_characterselect(float alpha = 1.f);
    void drawLevelSelect(float alpha = 1.f);
  
    void runLoginScreen();   
    void drawLevelComplete();   
    void drawTransition();
    void drawBackground_Menu(float alpha);

    // ui
      void drawText(const char* str, float x, float y,
        unsigned int size, sf::Color color, bool center = false);
    void drawCornerButton(const char* label, float x, float y,
        float w, float h, sf::Uint8 alpha);

    void pixelBox(float x, float y, float w, float h,
        sf::Color fill, sf::Color border, float thick = 1.5f);
  
    // game
    void buildLevels();
    void loadLevelAssets(int levelIdx);
    void buildLevelPlatforms(int levelIdx);
    void spawnEnemies(int levelIdx);
    void cleanupEnemies();
    void spawnGFlakes();
    void resolvePlayerCollisions();
    void updatePlaying(float dt);
    static bool intersects(const sf::FloatRect& a, const sf::FloatRect& b);
    void startTransitionTo(GameState target);
    void updateEnemyMotion(float dt);

    
    sf::RenderWindow window;
    sf::Font         font;
    sf::Clock        clock;

    // ── State machine
    GameState state, nextState;
    float    transAlpha;
    bool     transitioning;
    float    totalTime;
    int      tick;

    // ── Audio 
    sf::SoundBuffer splashBuf;
    sf::Sound       splashSound;
    bool            hasSplashSound;
    sf::Music       menuMusic;
    bool            hasMenuMusic;
    sf::Music       levelMusic;

    // ── Splash visuals 
    sf::Texture        splashTex;
    sf::Sprite         splashSprite;
    bool               hasSplashBg;
    Snowflake          sFlakes[MAX_FLAKES];
    sf::CircleShape    sFlakeShape;
    sf::RectangleShape sOverlay;
    sf::Text           topLetters[MAX_LETTERS]; int topCount;
    sf::Text           botLetters[MAX_LETTERS]; int botCount;
    float              logoPulse, glowPulse;
    sf::CircleShape    stars[6];
    sf::RectangleShape btnBody;
    sf::Text           btnText, btnSub;
    static const int MAX_ENEMIES = 32;
    Foe* enemies[MAX_ENEMIES];
    int  enemyCount;

    float playerWorldX;
    float playerWorldY;

    //logoo
    sf::Texture        logoTex;
    sf::Sprite         logoSprite;
    bool               hasLogo;
    sf::Texture        endBgTex;
    sf::Sprite         endBgSprite;
    bool               hasEndBg;

    // Menu
    sf::Texture bgTexture;
    sf::Sprite  bgSprite;
    int         menuSelected;
      const char* menuLabels[MENU_COUNT] = {
        "NEW GAME","CONTINUE","INSTRUCTIONS","LEVEL NAVIGATOR","LEADERBOARD","EXIT"
    };
    int        hoveredLevel;
    Level      levels[MAX_LEVELS];
    LevelConfig levelConfigs[MAX_LEVELS];
    GSnowflake gFlakes[G_MAX_FLAKES];
    int        levelSelectHovered;
    static constexpr float GRAVITY_DEFAULT = 900.f;
    static constexpr float MOVE_SPEED_DEFAULT = 130.f;
    static constexpr float JUMP_SPEED_DEFAULT = 420.f;
    // ── Character select 
    sf::Texture charTex[3];
    bool        hasCharTex[3];
    int         selectedChar;
    const char* charNames[3] = { "NICK", "TOM", "MYSTERY" };

    // ── Playing 
    sf::Texture platformTex;
    sf::Texture platformTex2;
    sf::Texture levelBgTex;
    sf::Sprite  levelBgSprite;
    bool        hasLevelBg;

    Platform    platforms[MAX_PLATFORMS];
    int         platformCount;
    int         currentLevel;
    int         currentLevelIdx;

    PlayerController playerCtrl;
    SnowballManager  snowballMgr;

    // Debug
    bool debugHitbox;

    // player stats
    int   playerScore;
    int   playerLives;
    int   levelScore;   

    // enemy motion
    float enemySpeed[MAX_ENEMIES];
    float enemyDir[MAX_ENEMIES];
    float enemyVelY[MAX_ENEMIES];
    int   enemyPlatIdx[MAX_ENEMIES];
    float GRAVITY;
    float MOVE_SPEED;
    float JUMP_SPEED;
};