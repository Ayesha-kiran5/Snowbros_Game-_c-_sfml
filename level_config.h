 #pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
// =============================================================
//  PLATFORM DEFINITION
// =============================================================
struct PlatformDef {
    float x, y, w, h;
    sf::Color color;
};

static const int MAX_PLATFORM_DEFS = 16;

struct PlatformLayout {
    PlatformDef defs[MAX_PLATFORM_DEFS];
    int         count = 0;
};

// =============================================================
//  ENEMY SPAWN DEFINITION
// =============================================================
struct EnemySpawnDef {
    char  typeStr[32];
    float x, y;
    bool  usesPlayerPtr;
};

static const int MAX_ENEMY_DEFS = 16;

struct EnemyLayout {
    EnemySpawnDef defs[MAX_ENEMY_DEFS];
    int           count = 0;
};

// =============================================================
//  LEVEL CONFIG — base class
// =============================================================
struct LevelConfig {
    int  number = 0;
    char name[32] = {};
    char description[64] = {};
    bool isBoss = false;
    bool isBonus = false;
    

    char bgFile[64] = {};
    char musicFile[64] = {};

    float gravityOverride = 0.f;
    float moveSpeedOverride = 0.f;
    float jumpSpeedOverride = 0.f;

    PlatformLayout platforms;
    EnemyLayout    enemies;

    // Simple helper — sets bgFile directly
    void setBg(const char* file) {
        int i = 0;
        while (file[i] && i < 63) { bgFile[i] = file[i]; ++i; }
        bgFile[i] = '\0';
    }

    void setMusic(const char* file) {
        int i = 0;
        while (file[i] && i < 63) { musicFile[i] = file[i]; ++i; }
        musicFile[i] = '\0';
    }

    void setName(const char* n) {
        int i = 0;
        while (n[i] && i < 31) { name[i] = n[i]; ++i; }
        name[i] = '\0';
    }

    void setDesc(const char* d) {
        int i = 0;
        while (d[i] && i < 63) { description[i] = d[i]; ++i; }
        description[i] = '\0';
    }
};

// =============================================================
//  LEVEL COUNT
// =============================================================
static const int MAX_LEVELS = 10;


struct Level1Config : public LevelConfig {
    Level1Config(float W, float H) {
        number = 1;  isBoss = false;  isBonus = false; 
        setName("SNOW VILLAGE");
        
        setBg("background.jpeg");
        setMusic("snow_bros_theme_01.ogg");

        platforms.count = 7;
        platforms.defs[0] = { 0,         H - 40,  W,   40, sf::Color(200,230,255) };
        platforms.defs[1] = { 0,         H - 160, 200, 24, sf::Color(200,230,255) };
        platforms.defs[2] = { W - 200,   H - 160, 200, 24, sf::Color(200,230,255) };
        platforms.defs[3] = { 80,        H - 280, 200, 24, sf::Color(200,230,255) };
        platforms.defs[4] = { W - 280,   H - 280, 200, 24, sf::Color(200,230,255) };
        platforms.defs[5] = { 0,         H - 400, 200, 24, sf::Color(200,230,255) };
        platforms.defs[6] = { W - 200,   H - 400, 200, 24, sf::Color(200,230,255) };

        enemies.count = 3;
        enemies.defs[0] = { "Botom", 30.f,  H - 40 - 110, false };
        enemies.defs[1] = { "Botom", 280.f, H - 40 - 110, false };
        enemies.defs[2] = { "Botom", 460.f, H - 40 - 110, false };
    }
};

struct Level2Config : public LevelConfig {
    Level2Config(float W, float H) {
        number = 2;  isBoss = false;  isBonus = false;  
        setName("ICE CAVES");
       
        setBg("background.jpeg");
        setMusic("snow_bros_theme_01.ogg");

        platforms.count = 6;
        platforms.defs[0] = { 0,        H - 40,  W,   40, sf::Color(180,220,255) };
        platforms.defs[1] = { 0,        H - 160, 200, 24, sf::Color(180,220,255) };
        platforms.defs[2] = { W - 200,  H - 160, 200, 24, sf::Color(180,220,255) };
        platforms.defs[3] = { 100,      H - 280, 200, 24, sf::Color(180,220,255) };
        platforms.defs[4] = { W - 300,  H - 280, 200, 24, sf::Color(180,220,255) };
        platforms.defs[5] = { 160,      H - 400, 280, 24, sf::Color(180,220,255) };

        enemies.count = 4;
        enemies.defs[0] = { "Botom",       30.f,  H - 40 - 110,  false };
        enemies.defs[1] = { "Botom",       420.f, H - 40 - 110,  false };
        enemies.defs[2] = { "Botom",       30.f,  H - 160 - 110, false };
        enemies.defs[3] = { "FlyingFooga", 160.f, H - 280 - 145, false };
    }
};

struct Level3Config : public LevelConfig {
    Level3Config(float W, float H) {
        number = 3;  isBoss = false;  isBonus = false;  
        setName("FROZEN FOREST");
        
        setBg("background.jpeg");
        setMusic("snow_bros_theme_01.ogg");

        platforms.count = 6;
        platforms.defs[0] = { 0,        H - 40,  W,   40, sf::Color(100,160,100) };
        platforms.defs[1] = { 0,        H - 160, 200, 24, sf::Color(100,160,100) };
        platforms.defs[2] = { W - 200,  H - 160, 200, 24, sf::Color(100,160,100) };
        platforms.defs[3] = { 120,      H - 280, 360, 24, sf::Color(100,160,100) };
        platforms.defs[4] = { 0,        H - 400, 200, 24, sf::Color(100,160,100) };
        platforms.defs[5] = { W - 200,  H - 400, 200, 24, sf::Color(100,160,100) };

        enemies.count = 3;
        enemies.defs[0] = { "Botom",       30.f,  H - 40 - 110,  false };
        enemies.defs[1] = { "FlyingFooga", 120.f, H - 280 - 145, false };
        enemies.defs[2] = { "Tornado",     30.f,  H - 400 - 120, true };
    }
};

struct Level4Config : public LevelConfig {
    Level4Config(float W, float H) {
        number = 4;  isBoss = false;  isBonus = true;  
        setName("BLIZZARD PASS");
       
        setBg("background.jpeg");
        setMusic("snow_bros_theme_01.ogg");

        platforms.count = 7;
        platforms.defs[0] = { 0,        H - 40,  W,   40, sf::Color(140,150,170) };
        platforms.defs[1] = { 0,        H - 160, 200, 24, sf::Color(140,150,170) };
        platforms.defs[2] = { W - 200,  H - 160, 200, 24, sf::Color(140,150,170) };
        platforms.defs[3] = { 0,        H - 280, 200, 24, sf::Color(140,150,170) };
        platforms.defs[4] = { W - 200,  H - 280, 200, 24, sf::Color(140,150,170) };
        platforms.defs[5] = { 160,      H - 400, 200, 24, sf::Color(140,150,170) };
        platforms.defs[6] = { 160,      H - 520, 280, 24, sf::Color(140,150,170) };

        enemies.count = 5;
        enemies.defs[0] = { "Botom",       30.f,         H - 40 - 110,  false };
        enemies.defs[1] = { "Botom",       460.f,        H - 40 - 110,  false };
        enemies.defs[2] = { "FlyingFooga", 30.f,         H - 280 - 145, false };
        enemies.defs[3] = { "FlyingFooga", W - 200 + 10.f, H - 280 - 145, false };
        enemies.defs[4] = { "Tornado",     160.f,        H - 520 - 120, true };
    }
};

struct Level5Config : public LevelConfig {
    Level5Config(float W, float H) {
        number = 5;  isBoss = true;  isBonus = false; 
        setName("UNDERGROUND LAIR");
      
        setBg("level2.png");
        setMusic("level1.ogg");

        platforms.count = 5;
        platforms.defs[0] = { 0,        H - 40,  W,   40, sf::Color(180,60,40) };
        platforms.defs[1] = { 0,        H - 160, 250, 24, sf::Color(180,60,40) };
        platforms.defs[2] = { W - 250,  H - 160, 250, 24, sf::Color(180,60,40) };
        platforms.defs[3] = { 0,        H - 280, 250, 24, sf::Color(180,60,40) };
        platforms.defs[4] = { W - 250,  H - 280, 250, 24, sf::Color(180,60,40) };
    }
};

struct Level6Config : public LevelConfig {
    Level6Config(float W, float H) {
        number = 6;  isBoss = false;  isBonus = false; 
        setName("LAVA CAVERNS");
        
        setBg("level3.png");
        setMusic("level2.ogg");

        platforms.count = 7;
        platforms.defs[0] = { 0,        H - 40,  W,   40, sf::Color(220,100,30) };
        platforms.defs[1] = { 0,        H - 160, 200, 24, sf::Color(220,100,30) };
        platforms.defs[2] = { W - 200,  H - 160, 200, 24, sf::Color(220,100,30) };
        platforms.defs[3] = { 0,        H - 280, 200, 24, sf::Color(220,100,30) };
        platforms.defs[4] = { W - 200,  H - 280, 200, 24, sf::Color(220,100,30) };
        platforms.defs[5] = { 140,      H - 400, 200, 24, sf::Color(220,100,30) };
        platforms.defs[6] = { W - 340,  H - 400, 200, 24, sf::Color(220,100,30) };

        enemies.count = 4;
        enemies.defs[0] = { "GreenBotom",       30.f,  H - 40 - 110,  false };
        enemies.defs[1] = { "GreenBotom",       460.f, H - 40 - 110,  false };
        enemies.defs[2] = { "GreenFlyingFooga", 30.f,  H - 280 - 145, false };
        enemies.defs[3] = { "GreenTornado",     140.f, H - 400 - 120, true };
    }
};

struct Level7Config : public LevelConfig {
    Level7Config(float W, float H) {
        number = 7;  isBoss = false;  isBonus = true;  
        setName("HAUNTED CASTLE");
      
        setBg("level3.png");
        setMusic("level2.ogg");

        platforms.count = 7;
        platforms.defs[0] = { 0,        H - 40,  W,   40, sf::Color(100,60,140) };
        platforms.defs[1] = { 0,        H - 160, 200, 24, sf::Color(100,60,140) };
        platforms.defs[2] = { W - 200,  H - 160, 200, 24, sf::Color(100,60,140) };
        platforms.defs[3] = { 0,        H - 280, 200, 24, sf::Color(100,60,140) };
        platforms.defs[4] = { W - 200,  H - 280, 200, 24, sf::Color(100,60,140) };
        platforms.defs[5] = { 100,      H - 400, 400, 24, sf::Color(100,60,140) };
        platforms.defs[6] = { 160,      H - 520, 280, 24, sf::Color(100,60,140) };

        enemies.count = 5;
        enemies.defs[0] = { "BlueBotom",       30.f,          H - 40 - 110,  false };
        enemies.defs[1] = { "BlueBotom",       460.f,         H - 40 - 110,  false };
        enemies.defs[2] = { "BlueFlyingFooga", 30.f,          H - 160 - 145, false };
        enemies.defs[3] = { "BlueFlyingFooga", W - 200 + 10.f,  H - 280 - 145, false };
        enemies.defs[4] = { "BlueTornado",     160.f,         H - 520 - 120, true };
    }
};

struct Level8Config : public LevelConfig {
    Level8Config(float W, float H) {
        number = 8;  isBoss = false;  isBonus = false; 
        setName("ALIEN PLANET");
        setBg("level3.png");
        setMusic("level2.ogg");

        platforms.count = 8;
        platforms.defs[0] = { 0,        H - 40,  W,   40, sf::Color(30,180,160) };
        platforms.defs[1] = { 0,        H - 160, 200, 24, sf::Color(30,180,160) };
        platforms.defs[2] = { W - 200,  H - 160, 200, 24, sf::Color(30,180,160) };
        platforms.defs[3] = { 100,      H - 280, 200, 24, sf::Color(30,180,160) };
        platforms.defs[4] = { W - 300,  H - 280, 200, 24, sf::Color(30,180,160) };
        platforms.defs[5] = { 0,        H - 400, 200, 24, sf::Color(30,180,160) };
        platforms.defs[6] = { W - 200,  H - 400, 200, 24, sf::Color(30,180,160) };
        platforms.defs[7] = { 160,      H - 520, 280, 24, sf::Color(30,180,160) };

        enemies.count = 5;
        enemies.defs[0] = { "BlueBotom",       30.f,          H - 40 - 110,  false };
        enemies.defs[1] = { "BlueBotom",       460.f,         H - 40 - 110,  false };
        enemies.defs[2] = { "BlueFlyingFooga", 100.f,         H - 280 - 145, false };
        enemies.defs[3] = { "BlueTornado",     160.f,         H - 520 - 120, true };
        enemies.defs[4] = { "BlueTornado",     W - 300 + 10.f,  H - 280 - 120, true };
    }
};

struct Level9Config : public LevelConfig {
    Level9Config(float W, float H) {
        number = 9;  isBoss = false;  isBonus = false;  
        setName("STORM CLOUDS");
     
        setBg("level3.png");
        setMusic("level2.ogg");

        platforms.count = 9;
        platforms.defs[0] = { 0,        H - 40,  W,   40, sf::Color(80,90,100) };
        platforms.defs[1] = { 0,        H - 160, 200, 24, sf::Color(80,90,100) };
        platforms.defs[2] = { W - 200,  H - 160, 200, 24, sf::Color(80,90,100) };
        platforms.defs[3] = { 100,      H - 280, 200, 24, sf::Color(80,90,100) };
        platforms.defs[4] = { W - 300,  H - 280, 200, 24, sf::Color(80,90,100) };
        platforms.defs[5] = { 0,        H - 400, 200, 24, sf::Color(80,90,100) };
        platforms.defs[6] = { W - 200,  H - 400, 200, 24, sf::Color(80,90,100) };
        platforms.defs[7] = { 140,      H - 520, 320, 24, sf::Color(80,90,100) };
        platforms.defs[8] = { 180,      H - 640, 240, 24, sf::Color(80,90,100) };

        enemies.count = 6;
        enemies.defs[0] = { "BlueBotom",       30.f,          H - 40 - 110,  false };
        enemies.defs[1] = { "BlueBotom",       460.f,         H - 40 - 110,  false };
        enemies.defs[2] = { "BlueFlyingFooga", 30.f,          H - 160 - 145, false };
        enemies.defs[3] = { "BlueFlyingFooga", W - 200 + 10.f,  H - 160 - 145, false };
        enemies.defs[4] = { "BlueTornado",     140.f,         H - 520 - 120, true };
        enemies.defs[5] = { "BlueTornado",     W - 300 + 10.f,  H - 280 - 120, true };
    }
};

struct Level10Config : public LevelConfig {
    Level10Config(float W, float H) {
        number = 10;  isBoss = true;  isBonus = false;  
        setName("FINAL ARENA");
  
        setBg("level4.png");
        setMusic("level2.ogg");

        platforms.count = 7;
        platforms.defs[0] = { 0,        H - 40,  W,   40, sf::Color(180,140,20) };
        platforms.defs[1] = { 0,        H - 160, 220, 24, sf::Color(180,140,20) };
        platforms.defs[2] = { W - 220,  H - 160, 220, 24, sf::Color(180,140,20) };
        platforms.defs[3] = { 0,        H - 280, 220, 24, sf::Color(180,140,20) };
        platforms.defs[4] = { W - 220,  H - 280, 220, 24, sf::Color(180,140,20) };
        platforms.defs[5] = { 120,      H - 400, 360, 24, sf::Color(180,140,20) };
        platforms.defs[6] = { 180,      H - 520, 240, 24, sf::Color(180,140,20) };
    }
};

// =============================================================
//  BUILD FUNCTION — called by game.cpp
// =============================================================
static inline void buildLevelConfigs(LevelConfig out[], float W, float H) {
    out[0] = Level1Config(W, H);
    out[1] = Level2Config(W, H);
    out[2] = Level3Config(W, H);
    out[3] = Level4Config(W, H);
    out[4] = Level5Config(W, H);
    out[5] = Level6Config(W, H);
    out[6] = Level7Config(W, H);
    out[7] = Level8Config(W, H);
    out[8] = Level9Config(W, H);
    out[9] = Level10Config(W, H);
}