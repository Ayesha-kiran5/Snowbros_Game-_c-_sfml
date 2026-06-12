#include "auth.h"
#include "database.h"
#include "game.h"
#include "level_config.h"
#include <cmath>

static inline float fmin2(float a, float b) { return a < b ? a : b; }
static inline float fmax2(float a, float b) { return a > b ? a : b; }

// =============================================================
//  BUILD LEVELS  
// =============================================================
void Game::buildLevels() {
    for (int i = 0; i < MAX_LEVELS; ++i) {
        const LevelConfig& cfg = levelConfigs[i];
        levels[i].number = cfg.number;
        levels[i].isBoss = cfg.isBoss;
        levels[i].isBonus = cfg.isBonus;
        levels[i].locked = false;   // all levels unlocked

        int ni = 0;
        while (cfg.name[ni] && ni < 31) { levels[i].name[ni] = cfg.name[ni]; ++ni; }
        levels[i].name[ni] = '\0';

        int di = 0;
        while (cfg.description[di] && di < 63) { levels[i].description[di] = cfg.description[di]; ++di; }
        levels[i].description[di] = '\0';
    }
}

// =============================================================
//  LOAD LEVEL ASSETS
// =============================================================
void Game::loadLevelAssets(int levelIdx) {
    const LevelConfig& cfg = levelConfigs[levelIdx];

    hasLevelBg = false;
    if (cfg.bgFile[0] != '\0') {
        hasLevelBg = levelBgTex.loadFromFile(cfg.bgFile);
        if (hasLevelBg) {
            levelBgTex.setSmooth(false);
            levelBgSprite.setTexture(levelBgTex, true);
            levelBgSprite.setPosition(0.f, 0.f);
            levelBgSprite.setScale(
                (float)W / levelBgTex.getSize().x,
                (float)H / levelBgTex.getSize().y);
        }
    }

    if (cfg.musicFile[0] != '\0') {
        if (levelMusic.openFromFile(cfg.musicFile)) {
            levelMusic.setLoop(true);
            levelMusic.setVolume(55);
            levelMusic.play();
        }
    }

    GRAVITY = cfg.gravityOverride > 0.f ? cfg.gravityOverride : GRAVITY_DEFAULT;
    MOVE_SPEED = cfg.moveSpeedOverride > 0.f ? cfg.moveSpeedOverride : MOVE_SPEED_DEFAULT;
    JUMP_SPEED = cfg.jumpSpeedOverride > 0.f ? cfg.jumpSpeedOverride : JUMP_SPEED_DEFAULT;
}

// =============================================================
//  BUILD LEVEL PLATFORMS
// =============================================================
void Game::buildLevelPlatforms(int levelIdx) {
    const PlatformLayout& layout = levelConfigs[levelIdx].platforms;
    platformCount = layout.count;
    if (platformCount > MAX_PLATFORMS) platformCount = MAX_PLATFORMS;
    sf::Texture& activeTex = (levelIdx >= 5 && levelIdx < 9) ? platformTex2 : platformTex;

    for (int i = 0; i < platformCount; ++i) {
        const PlatformDef& d = layout.defs[i];
        platforms[i].shape.setSize(sf::Vector2f(d.w, d.h));
        platforms[i].shape.setPosition(sf::Vector2f(d.x, d.y));
        if (activeTex.getSize().x > 0) {
            platforms[i].shape.setTexture(&activeTex);
            platforms[i].shape.setTextureRect(sf::IntRect(0, 0, (int)d.w, (int)d.h));
            platforms[i].shape.setFillColor(sf::Color::White);
        }
        else {
            platforms[i].shape.setFillColor(sf::Color(80, 160, 220));
        }
        platforms[i].shape.setOutlineThickness(2);
        platforms[i].shape.setOutlineColor(sf::Color(180, 230, 255));
    }
}

// =============================================================
//  ENEMY MANAGEMENT
// =============================================================
void Game::cleanupEnemies() {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        delete enemies[i];
        enemies[i] = nullptr;
    }
    enemyCount = 0;
}

void Game::spawnEnemies(int levelIdx) {
    cleanupEnemies();
    const EnemyLayout& layout = levelConfigs[levelIdx].enemies;

    for (int i = 0; i < layout.count && enemyCount < MAX_ENEMIES; ++i) {
        const EnemySpawnDef& def = layout.defs[i];
        Foe* foe = nullptr;
        if (def.usesPlayerPtr)
            foe = FoeFactory::create(def.typeStr, def.x, def.y, &playerWorldX, &playerWorldY);
        else
            foe = FoeFactory::create(def.typeStr, def.x, def.y);
        if (!foe) continue;

        // Speed scales with level
        float speedLo = 40.f + levelIdx * 12.f;
        float speedHi = 80.f + levelIdx * 14.f;
        if (speedLo > 160.f) speedLo = 160.f;
        if (speedHi > 220.f) speedHi = 220.f;

        enemySpeed[enemyCount] = frand(speedLo, speedHi);
        enemyDir[enemyCount] = (rand() % 2 == 0) ? 1.f : -1.f;
        enemyVelY[enemyCount] = 0.f;
        enemyPlatIdx[enemyCount] = -1;

        // Find platform enemy stands on
        float bestDist = 1e9f;
        int   bestPlat = -1;
        for (int p = 0; p < platformCount; ++p) {
            sf::FloatRect pr = platforms[p].shape.getGlobalBounds();
            bool xOverlap = (def.x + 20.f > pr.left) && (def.x < pr.left + pr.width);
            if (!xOverlap) continue;
            float dist = pr.top - def.y;
            if (dist >= 0.f && dist < bestDist) { bestDist = dist; bestPlat = p; }
        }
        if (bestPlat >= 0) {
            sf::FloatRect pr = platforms[bestPlat].shape.getGlobalBounds();
            foe->set_base_y(pr.top);
            enemyPlatIdx[enemyCount] = bestPlat;
            foe->set_patrol_bounds(pr.left, pr.left + pr.width);
        }

        // Detect flyers
        bool isFlyer = false;
        {
            const char* t = def.typeStr;
            int len = 0; while (t[len]) ++len;
            for (int c = 0; c < len && !isFlyer; ++c) {
                if (c + 2 < len && t[c] == 'F' && t[c + 1] == 'l' && t[c + 2] == 'y') isFlyer = true;
                if (c + 3 < len && t[c] == 'F' && t[c + 1] == 'o' && t[c + 2] == 'o' && t[c + 3] == 'g') isFlyer = true;
                if (c + 2 < len && t[c] == 'T' && t[c + 1] == 'o' && t[c + 2] == 'r') isFlyer = true;
            }
        }
        if (isFlyer) enemyVelY[enemyCount] = frand(20.f, 60.f);

        enemies[enemyCount] = foe;
        ++enemyCount;
    }
}

// =============================================================
//  UPDATE ENEMY MOTION
// =============================================================
void Game::updateEnemyMotion(float dt) {
    for (int i = 0; i < enemyCount; ++i) {
        if (!enemies[i] || enemies[i]->is_dead()) continue;

        HitBox& hb = enemies[i]->get_hit_box();
        float ex = hb.rect.left, ey = hb.rect.top;
        float ew = hb.rect.width, eh = hb.rect.height;
        bool isFlyer = (enemyVelY[i] != 0.f);

        if (isFlyer) {
            float newX = ex + enemyDir[i] * enemySpeed[i] * dt;
            float newY = ey + sinf(totalTime * enemyVelY[i] * 0.05f) * enemyVelY[i] * dt;
            if (newX < 0.f) { newX = 0.f;             enemyDir[i] = 1.f; }
            if (newX + ew > (float)W) { newX = (float)W - ew;   enemyDir[i] = -1.f; }
            if (newY < 0.f)             newY = 0.f;
            if (newY + eh > (float)H)   newY = (float)H - eh;
            enemies[i]->set_position(newX, newY);
        }
        else {
            int   pidx = enemyPlatIdx[i];
            float newX = ex + enemyDir[i] * enemySpeed[i] * dt;

            if (pidx >= 0 && pidx < platformCount) {
                sf::FloatRect pr = platforms[pidx].shape.getGlobalBounds();
                if (newX < pr.left) { newX = pr.left;                enemyDir[i] = 1.f; }
                if (newX + ew > pr.left + pr.width) { newX = pr.left + pr.width - ew; enemyDir[i] = -1.f; }
                enemies[i]->set_position(newX, pr.top - eh);
            }
            else {
                enemyVelY[i] += GRAVITY * dt;
                float newY = ey + enemyVelY[i] * dt;
                for (int p = 0; p < platformCount; ++p) {
                    sf::FloatRect pr = platforms[p].shape.getGlobalBounds();
                    float feetY = newY + eh;
                    if (feetY >= pr.top && feetY <= pr.top + pr.height
                        && newX + ew > pr.left && newX < pr.left + pr.width)
                    {
                        newY = pr.top - eh; enemyVelY[i] = 0.f; enemyPlatIdx[i] = p; break;
                    }
                }
                if (newY + eh > (float)H) { newY = (float)H - eh; enemyVelY[i] = 0.f; }
                enemies[i]->set_position(newX, newY);
            }
        }

        // Random direction flip
        float flipChance = 0.002f + currentLevelIdx * 0.001f;
        if (frand(0.f, 1.f) < flipChance) enemyDir[i] = -enemyDir[i];
    }
}

// =============================================================
//  PHYSICS
// =============================================================
bool Game::intersects(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.left < b.left + b.width && a.left + a.width  > b.left
        && a.top  < b.top + b.height && a.top + a.height > b.top;
}

void Game::resolvePlayerCollisions() {
    Player& pl = playerCtrl.getPlayer();
    pl.onGround = false;
    sf::FloatRect pb = pl.shape.getGlobalBounds();

    for (int i = 0; i < platformCount; ++i) {
        sf::FloatRect rb = platforms[i].shape.getGlobalBounds();
        if (!intersects(pb, rb)) continue;

        float ox = fmin2(pb.left + pb.width, rb.left + rb.width) - fmax2(pb.left, rb.left);
        float oy = fmin2(pb.top + pb.height, rb.top + rb.height) - fmax2(pb.top, rb.top);

        if (ox < oy) {
            pl.shape.move(sf::Vector2f(pb.left < rb.left ? -ox : ox, 0));
            pl.velocity.x = 0;
        }
        else {
            if (pb.top < rb.top) {
                pl.shape.move(sf::Vector2f(0, -oy));
                pl.velocity.y = 0;
                pl.onGround = true;
            }
            else {
                pl.shape.move(sf::Vector2f(0, oy));
                pl.velocity.y = 0;
            }
        }
        pb = pl.shape.getGlobalBounds();
    }

    // Screen boundaries
    sf::Vector2f p = pl.shape.getPosition();
    if (p.x < 0.f)                 pl.shape.setPosition(0.f, p.y);
    if (p.x + PLAYER_W > (float)W) pl.shape.setPosition((float)W - PLAYER_W, p.y);
    p = pl.shape.getPosition();
    if (p.y < 0.f) { pl.shape.setPosition(p.x, 0.f); pl.velocity.y = 0.f; }
    p = pl.shape.getPosition();
    if (p.y + PLAYER_H > (float)H) {
        pl.shape.setPosition(p.x, (float)H - PLAYER_H);
        pl.velocity.y = 0;
        pl.onGround = true;
    }

    if (pl.onGround) playerCtrl.resetCoyote();
}
     //=============================================================
//  UPDATE PLAYING
// =============================================================
void Game::updatePlaying(float dt) {
    playerCtrl.update(dt);
    resolvePlayerCollisions();

    const Player& pl = playerCtrl.getPlayer();
    sf::Vector2f boxPos = pl.shape.getPosition();
    playerWorldX = boxPos.x + PLAYER_W / 2.f;
    playerWorldY = boxPos.y + PLAYER_H / 2.f;

    // Snowball shooting
    if (playerCtrl.wantsShoot()) {
        float facing = playerCtrl.getFacing();
        sf::Vector2f handOrigin;
        if (facing > 0.f)
            handOrigin = sf::Vector2f(boxPos.x + PLAYER_W + 2.f, boxPos.y + PLAYER_H * 0.4f);
        else
            handOrigin = sf::Vector2f(boxPos.x - 2.f, boxPos.y + PLAYER_H * 0.4f);
        snowballMgr.tryShoot(handOrigin, facing);
    }

    snowballMgr.checkEnemyHits(enemies, enemyCount);
    snowballMgr.update(dt);
    updateEnemyMotion(dt);

    for (int i = 0; i < enemyCount; ++i)
        if (enemies[i]) enemies[i]->update(dt);

    // Kill fully encased enemies
    for (int i = 0; i < enemyCount; ++i)
        if (enemies[i] && enemies[i]->is_fully_covered())
            enemies[i]->roll_hit();

    // Compact dead enemies + score
    int alive = 0;
    for (int i = 0; i < enemyCount; ++i) {
        if (!enemies[i]) continue;
        if (enemies[i]->is_dead()) {
            int lo = enemies[i]->get_min_score();
            int hi = enemies[i]->get_max_score();
            int earned = lo + (rand() % (hi - lo + 1));
            playerScore += earned;
            levelScore += earned;
            delete enemies[i];
            enemies[i] = nullptr;
        }
        else {
            if (alive != i) {
                enemySpeed[alive] = enemySpeed[i];
                enemyDir[alive] = enemyDir[i];
                enemyVelY[alive] = enemyVelY[i];
                enemyPlatIdx[alive] = enemyPlatIdx[i];
            }
            enemies[alive++] = enemies[i];
        }
    }
    for (int i = alive; i < enemyCount; ++i) enemies[i] = nullptr;
    enemyCount = alive;

    // Level complete when all enemies cleared
    if (enemyCount == 0 && !transitioning && levelConfigs[currentLevelIdx].enemies.count > 0) {
        {
            int nextIdx = currentLevelIdx + 1;
            if (nextIdx < MAX_LEVELS) levels[nextIdx].locked = false;

            if (SessionManager::getInstance().isLoggedIn()) {
                int uid = SessionManager::getInstance().getCurrentUserId();
                std::string uname = SessionManager::getInstance().getCurrentUsername();

                // Save progress as before
                SaveLoadManager::getInstance().manualSave(
                    uid, currentLevel, playerLives, 0, playerScore);

                // NEW: add leaderboard entry for this session
                LeaderboardManager::getInstance().addEntry(
                    uid, uname, playerScore, currentLevel);
            }
            startTransitionTo(GameState::LEVEL_COMPLETE);
        }
    }
}