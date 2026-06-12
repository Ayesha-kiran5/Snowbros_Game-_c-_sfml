//#pragma once
//#pragma once
//
//#include <SFML/Graphics.hpp>
//#include "level_config.h"
//
//// Forward declare — full definition in game.h
//struct Platform;
//
//// =============================================================
////  MAX PLATFORMS PER LEVEL
//// =============================================================
//static const int LM_MAX_PLATFORMS = 32;
//
//// =============================================================
////  LevelData  — one loaded level's runtime state
////
////  Holds everything that changes per level:
////    • config data  (name, bg file, enemy types, etc.)
////    • background texture + sprite
////    • platform array loaded from platformFile
//// =============================================================
//struct LevelData {
//    LevelConfig      config;
//
//    // Background — loaded from config.bgFile
//    sf::Texture      bgTex;
//    sf::Sprite       bgSprite;
//    bool             hasBg;
//
//    // Platforms — loaded from config.platformFile
//    sf::RectangleShape platforms[LM_MAX_PLATFORMS];
//    int                platformCount;
//
//    LevelData() : hasBg(false), platformCount(0) {}
//};
//
//// =============================================================
////  LevelManager
////
////  Owns all LevelData objects.
////  Game.cpp calls only these 5 functions:
////
////    load()           — reads levels.cfg, loads all LevelData
////    startLevel(n)    — activates level n (0-based index)
////    drawBackground() — draws current level's bg image
////    drawPlatforms()  — draws current level's platforms
////    goNextLevel()    — advances to next level
////
////  To add more levels: add lines to levels.cfg. Zero code change.
//// =============================================================
//class LevelManager {
//public:
//    LevelManager();
//    ~LevelManager();
//
//    // Read levels.cfg — creates all LevelData dynamically
//    // platformTex = shared platform texture from Game
//    bool load(const char* cfgFile, sf::Texture& platformTex,
//        int screenW, int screenH);
//
//    // Activate level by 0-based index
//    void startLevel(int idx);
//
//    // Draw current level's background image
//    void drawBackground(sf::RenderWindow& window, float alpha = 1.f) const;
//
//    // Draw current level's platforms
//    void drawPlatforms(sf::RenderWindow& window) const;
//
//    // Advance to next level — wraps around to 0 after last
//    void goNextLevel();
//
//    // Go to previous level (for debug L key)
//    void goPrevLevel();
//
//    // Getters
//    int         getCurrentIndex()  const { return currentIdx; }
//    int         getTotalLevels()   const { return levelCount; }
//    LevelData* getCurrent()       const;
//    const LevelConfig& getCurrentConfig() const;
//
//    // Platform collision access for resolvePlayerCollisions()
//    sf::RectangleShape* getPlatforms();
//    int                 getPlatformCount() const;
//
//    // Unlock next level after completing current
//    void unlockNext();
//
//private:
//    // Parse one line of levels.cfg into a LevelConfig
//    bool parseLine(const char* line, LevelConfig& cfg) const;
//
//    // Parse one platform file into a LevelData
//    void loadPlatforms(LevelData& ld, sf::Texture& platformTex,
//        int screenW, int screenH) const;
//
//    // Count non-comment lines in a file (to allocate array)
//    int countDataLines(const char* filePath) const;
//
//    // Split a char[] by a delimiter into parts[]
//    // Returns number of parts found
//    int splitChar(const char* src, char delim,
//        char parts[][128], int maxParts) const;
//
//    LevelData* levels;     // dynamic array — new LevelData[levelCount]
//    int        levelCount; // set from cfg file — never hardcoded
//    int        currentIdx; // which level is active right now
//};