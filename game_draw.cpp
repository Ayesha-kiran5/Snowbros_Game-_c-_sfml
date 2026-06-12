#include "game.h"
#include <sstream>
#include <cmath>

static const float STAR_X[6] = { 60,540,40,560,120,480 };
static const float STAR_Y[6] = { 100,90,190,200,60,170 };
static const float BTN_CX = 300.f, BTN_CY = 440.f;
static const float BTN_W = 200.f, BTN_H = 50.f;

// =============================================================
//  UI HELPERS
// =============================================================
void Game::pixelBox(float x, float y, float w, float h,
    sf::Color fill, sf::Color border, float thick)
{
    sf::RectangleShape box(sf::Vector2f(w, h));
    box.setPosition(x, y);
    box.setFillColor(fill);
    box.setOutlineColor(border);
    box.setOutlineThickness(thick);
    window.draw(box);

    sf::RectangleShape dot(sf::Vector2f(3, 3));
    dot.setFillColor(border);
    float pos[4][2] = { {x,y},{x + w - 3,y},{x,y + h - 3},{x + w - 3,y + h - 3} };
    for (int i = 0; i < 4; ++i) { dot.setPosition(pos[i][0], pos[i][1]); window.draw(dot); }
}

void Game::drawText(const char* str, float x, float y,
    unsigned int size, sf::Color color, bool center)
{
    sf::Text t;
    t.setFont(font);
    t.setString(str);
    t.setCharacterSize(size);
    t.setFillColor(color);
    if (center) {
        sf::FloatRect r = t.getLocalBounds();
        t.setOrigin(r.left + r.width / 2, r.top);
    }
    t.setPosition(x, y);
    window.draw(t);
}

void Game::drawCornerButton(const char* label, float x, float y,
    float w, float h, sf::Uint8 alpha)
{
    pixelBox(x, y, w, h,
        sf::Color(Theme::BTN_FILL.r, Theme::BTN_FILL.g, Theme::BTN_FILL.b, alpha),
        sf::Color(Theme::BTN_BORDER.r, Theme::BTN_BORDER.g, Theme::BTN_BORDER.b, alpha), 1.5f);
    drawText(label, x + w / 2, y + h / 2 - 6, 9,
        sf::Color(Theme::BTN_TEXT.r, Theme::BTN_TEXT.g, Theme::BTN_TEXT.b, alpha), true);
}

// =============================================================
//  BACKGROUND HELPERS
// =============================================================
void Game::drawBackground_Menu(float alpha) {
    bgSprite.setColor(sf::Color(255, 255, 255, (sf::Uint8)(alpha * 255)));
    window.draw(bgSprite);

    sf::RectangleShape scan(sf::Vector2f((float)WIN_W, 1));
    scan.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(28 * alpha)));
    for (int y = 0; y < WIN_H; y += 2) { scan.setPosition(0, (float)y); window.draw(scan); }

    for (int i = 0; i < G_MAX_FLAKES; ++i) {
        sf::CircleShape c(gFlakes[i].radius);
        c.setPosition(gFlakes[i].x - gFlakes[i].radius, gFlakes[i].y - gFlakes[i].radius);
        c.setFillColor(sf::Color(180, 210, 255, (sf::Uint8)(gFlakes[i].opacity * alpha * 255)));
        window.draw(c);
    }
}

void Game::spawnGFlakes() {
    for (int i = 0; i < G_MAX_FLAKES; ++i) {
        gFlakes[i].x = (float)(rand() % W);
        gFlakes[i].y = (float)(rand() % H);
        gFlakes[i].speed = 25 + (float)(rand() % 55);
        gFlakes[i].radius = 1.5f + (float)(rand() % 30) / 10.f;
        gFlakes[i].opacity = 0.1f + (float)(rand() % 5) / 10.f;
    }
}

// =============================================================
//  SPLASH
// =============================================================
void Game::initSplash() {
    for (int i = 0; i < 6; ++i) {
        float r = frand(3, 7);
        stars[i].setRadius(r);
        stars[i].setPointCount(4);
        stars[i].setFillColor(sf::Color(200, 240, 255, 220));
        stars[i].setOrigin(r, r);
        stars[i].setPosition(STAR_X[i], STAR_Y[i]);
    }
    btnBody.setSize(sf::Vector2f(BTN_W, BTN_H));
    btnBody.setOrigin(BTN_W / 2, BTN_H / 2);
    btnBody.setPosition(BTN_CX, BTN_CY);
    btnBody.setFillColor(Theme::BTN_FILL_SEL);
    btnBody.setOutlineThickness(2);
    btnBody.setOutlineColor(Theme::BTN_BDR_SEL);

    btnText.setFont(font); btnText.setString("START");
    btnText.setCharacterSize(22); btnText.setStyle(sf::Text::Bold);
    btnText.setFillColor(Theme::BTN_TEXT_SEL);
    sf::FloatRect tb = btnText.getLocalBounds();
    btnText.setOrigin(tb.left + tb.width / 2, tb.top + tb.height / 2);
    btnText.setPosition(BTN_CX, BTN_CY - 4);
}
void Game::updateSplash(float dt) {
    for (int i = 0; i < MAX_FLAKES; ++i) {
        sFlakes[i].y += sFlakes[i].speed * dt;
        if (sFlakes[i].y > WIN_H + 5) {
            sFlakes[i].y = -5;
            sFlakes[i].x = (float)(rand() % W);
        }
    }
    for (int i = 0; i < 6; ++i) {
        float t = .5f + .5f * sinf(totalTime * 3.f + i * 1.1f);
        stars[i].setScale(t, t);
    }
}
void Game::drawSplashButton() {
    window.draw(btnBody);
    window.draw(btnText);
    window.draw(btnSub);
}

void Game::drawSplash() {
    window.clear(sf::Color(5, 15, 40));
    if (hasSplashBg) window.draw(splashSprite);
    window.draw(sOverlay);
    for (int i = 0; i < 6; ++i) window.draw(stars[i]);
    drawSplashButton();
    for (int i = 0; i < MAX_FLAKES; ++i) {
        sFlakeShape.setRadius(sFlakes[i].radius);
        sFlakeShape.setOrigin(sFlakes[i].radius, sFlakes[i].radius);
        sFlakeShape.setPosition(sFlakes[i].x, sFlakes[i].y);
        sFlakeShape.setFillColor(sf::Color(220, 235, 255, (sf::Uint8)sFlakes[i].opacity));
        window.draw(sFlakeShape);
    }
}

// =============================================================
//  MAIN MENU
// =============================================================
void Game::draw_mainmenu(float alpha) {
    drawBackground_Menu(alpha);
    sf::Uint8 a = (sf::Uint8)(alpha * 255);

    // Logo
    if (hasLogo) {
        sf::Vector2u sz = logoTex.getSize();
        float maxW = 260.f, maxH = 100.f;
        float scaleX = maxW / sz.x, scaleY = maxH / sz.y;
        float scale = (scaleX < scaleY) ? scaleX : scaleY;
        logoSprite.setScale(scale, scale);
        logoSprite.setPosition((float)W / 2 - sz.x * scale / 2, 50.f);
        logoSprite.setColor(sf::Color(255, 255, 255, a));
        window.draw(logoSprite);
    }
    else {
        drawText("SNOW BROS", (float)W / 2, 60, 28,
            sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, a), true);
    }
    const float bW = 260, bH = 62, gX = 16, gY = 18, sY = 170;
    float sX = (W - (bW * 2 + gX)) / 2;
    const char* labels[MENU_COUNT] = {
        "NEW GAME", "CONTINUE", "INSTRUCTIONS",
        "LEVEL NAVIGATOR", "LEADERBOARD", "EXIT"
    };
    for (int i = 0; i < MENU_COUNT; ++i) {
        float bx = sX + (i % 2) * (bW + gX);
        float by = sY + (i / 2) * (bH + gY);
        bool  sel = (i == menuSelected);
        sf::Color fill = sel
            ? sf::Color(Theme::BTN_FILL_SEL.r, Theme::BTN_FILL_SEL.g, Theme::BTN_FILL_SEL.b, a)
            : sf::Color(Theme::BTN_FILL.r, Theme::BTN_FILL.g, Theme::BTN_FILL.b, a);
        sf::Color border = sel
            ? sf::Color(Theme::BTN_BDR_SEL.r, Theme::BTN_BDR_SEL.g, Theme::BTN_BDR_SEL.b, a)
            : sf::Color(Theme::BTN_BORDER.r, Theme::BTN_BORDER.g, Theme::BTN_BORDER.b, a);
        pixelBox(bx, by, bW, bH, fill, border, sel ? 2.f : 1.5f);
        drawText(labels[i], bx + bW / 2, by + bH / 2 - 7, 12,
            sel ? sf::Color(Theme::BTN_TEXT_SEL.r, Theme::BTN_TEXT_SEL.g, Theme::BTN_TEXT_SEL.b, a)
            : sf::Color(Theme::BTN_TEXT.r, Theme::BTN_TEXT.g, Theme::BTN_TEXT.b, a),
            true);
    }
}

// =============================================================
//  CHARACTER SELECT
// =============================================================
void Game::initCharacters() {}

void Game::draw_characterselect(float alpha) {
    drawBackground_Menu(alpha);
    sf::Uint8 a = (sf::Uint8)(alpha * 255);

    drawText("CHOOSE YOUR HERO", 300, 50, 20,
        sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, a), true);
   
    const float bW = 160.f, bH = 200.f, gap = 20.f;
    float sx = (W - (bW * 3 + gap * 2)) / 2.f, by = 140.f;
    const int IDLE_Y = 146, IDLE_H = 98, IDLE_FW = SPRITE_FRAME_W;

    for (int i = 0; i < 3; ++i) {
        float bx = sx + i * (bW + gap);
        bool  sel = (selectedChar == i);
        sf::Color fill = sel
            ? sf::Color(Theme::BTN_FILL_SEL.r, Theme::BTN_FILL_SEL.g, Theme::BTN_FILL_SEL.b, a)
            : sf::Color(Theme::BTN_FILL.r, Theme::BTN_FILL.g, Theme::BTN_FILL.b, a);
        sf::Color border = sel
            ? sf::Color(255, 220, 0, a)
            : sf::Color(Theme::BTN_BORDER.r, Theme::BTN_BORDER.g, Theme::BTN_BORDER.b, a);
        pixelBox(bx, by, bW, bH, fill, border, sel ? 3.f : 1.5f);

        const float padX = 10.f, padTop = 10.f, padBot = 30.f;
        float previewW = bW - padX * 2, previewH = bH - padTop - padBot;

        if (hasCharTex[i]) {
            sf::Sprite prev; prev.setTexture(charTex[i]);
            prev.setTextureRect(sf::IntRect(0, IDLE_Y, IDLE_FW, IDLE_H));
            float scaleX = previewW / (float)IDLE_FW, scaleY = previewH / (float)IDLE_H;
            float scale = (scaleX < scaleY) ? scaleX : scaleY;
            prev.setScale(scale, scale);
            prev.setPosition(bx + padX + (previewW - IDLE_FW * scale) / 2.f,
                by + padTop + (previewH - IDLE_H * scale) / 2.f);
            prev.setColor(sf::Color(255, 255, 255, a));
            window.draw(prev);
        }
        else {
            sf::Color ph[3] = {
                sf::Color(80,160,255,a), sf::Color(255,130,80,a), sf::Color(120,200,80,a)
            };
            sf::RectangleShape img(sf::Vector2f(previewW, previewH));
            img.setPosition(bx + padX, by + padTop);
            img.setFillColor(ph[i]);
            window.draw(img);
        }
        drawText(charNames[i], bx + bW / 2.f, by + bH - 22.f, 13,
            sf::Color(255, 255, 255, a), true);
    }
    drawText("ESC = BACK", 300, 470, 10,
        sf::Color(Theme::BTN_TEXT.r, Theme::BTN_TEXT.g, Theme::BTN_TEXT.b, a), true);
}

// =============================================================
//  LEVEL SELECT
// =============================================================
void Game::drawLevelSelect(float alpha) {
    drawBackground_Menu(alpha);
    sf::Uint8 a = (sf::Uint8)(alpha * 255);
    sf::Uint8 a2 = (sf::Uint8)(alpha * 180);

    drawText("SELECT LEVEL", 300, 18, 22,
        sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, a), true);

    const float cW = 265, cH = 68, gX = 14, gY = 10;
    float sx = (W - (cW * 2 + gX)) / 2, sy = 78;

    for (int i = 0; i < MAX_LEVELS; ++i) {
        float bx = sx + (i % 2) * (cW + gX);
        float by = sy + (i / 2) * (cH + gY);
        bool  hov = (levelSelectHovered == i);

        sf::Color fill = hov
            ? sf::Color(Theme::BTN_FILL_SEL.r, Theme::BTN_FILL_SEL.g, Theme::BTN_FILL_SEL.b, a)
            : sf::Color(Theme::BTN_FILL.r, Theme::BTN_FILL.g, Theme::BTN_FILL.b, a);
        sf::Color border = hov
            ? sf::Color(Theme::BTN_BDR_SEL.r, Theme::BTN_BDR_SEL.g, Theme::BTN_BDR_SEL.b, a)
            : sf::Color(Theme::BTN_BORDER.r, Theme::BTN_BORDER.g, Theme::BTN_BORDER.b, a);
        pixelBox(bx, by, cW, cH, fill, border, hov ? 2.f : 1.5f);

        std::ostringstream ns; ns << levels[i].number;
        drawText(ns.str().c_str(), bx + 20, by + 14, 18,
            sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, a));
        drawText(levels[i].name, bx + 44, by + 12, 11,
            sf::Color(Theme::BTN_TEXT_SEL.r, Theme::BTN_TEXT_SEL.g, Theme::BTN_TEXT_SEL.b, a));
        drawText(levels[i].description, bx + 44, by + 32, 9,
            sf::Color(Theme::ACCENT_DIM.r, Theme::ACCENT_DIM.g, Theme::ACCENT_DIM.b, a));

        if (levels[i].isBoss)
            drawText("BOSS", bx + cW - 48, by + 14, 9,
                sf::Color(Theme::BOSS_COL.r, Theme::BOSS_COL.g, Theme::BOSS_COL.b, a));
        else if (levels[i].isBonus)
            drawText("BONUS", bx + cW - 52, by + 14, 9,
                sf::Color(Theme::BONUS_COL.r, Theme::BONUS_COL.g, Theme::BONUS_COL.b, a));
    }
    drawText("ESC = BACK", 300, 558, 10,
        sf::Color(Theme::BTN_TEXT.r, Theme::BTN_TEXT.g, Theme::BTN_TEXT.b, a), true);
}

// =============================================================
//  INSTRUCTIONS
// =============================================================
void Game::drawInstructions(float alpha) {
    drawBackground_Menu(alpha);
    sf::Uint8 a = (sf::Uint8)(alpha * 255);

    drawText("INSTRUCTIONS", 300, 20, 22,
        sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, a), true);

    float bX = 60, bY = 70, bW2 = 480, bH2 = 160;
    pixelBox(bX, bY, bW2, bH2,
        sf::Color(Theme::BTN_FILL.r, Theme::BTN_FILL.g, Theme::BTN_FILL.b, a),
        sf::Color(Theme::BTN_BORDER.r, Theme::BTN_BORDER.g, Theme::BTN_BORDER.b, a), 1.5f);
    drawText("CONTROLS", 300, bY + 12, 14,
        sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, a), true);

    struct Row { const char* act, * keys; };
    Row rows[] = {
        {"MOVE",  "ARROW KEYS  or  A / D"},
        {"JUMP",  "SPACE  or  UP  or  W"},
        {"SHOOT", "Z  or  LEFT CTRL"},
        {"PAUSE", "ESC"},
        {"DEBUG", "B=hitbox | N=next level | P=prev level"}
    };
    for (int i = 0; i < 5; ++i) {
        float ry = bY + 42 + i * 22;
        drawText(rows[i].act, bX + 20, ry, 11,
            sf::Color(Theme::ACCENT_DIM.r, Theme::ACCENT_DIM.g, Theme::ACCENT_DIM.b, a));
        drawText(rows[i].keys, bX + 120, ry, 11,
            sf::Color(Theme::BTN_TEXT.r, Theme::BTN_TEXT.g, Theme::BTN_TEXT.b, a));
    }

    float tY = bY + bH2 + 24, tH = 90;
    pixelBox(bX, tY, bW2, tH,
        sf::Color(Theme::BTN_FILL.r, Theme::BTN_FILL.g, Theme::BTN_FILL.b, a),
        sf::Color(Theme::BTN_BORDER.r, Theme::BTN_BORDER.g, Theme::BTN_BORDER.b, a), 1.5f);
    drawText("TIPS", 300, tY + 10, 14,
        sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, a), true);
    drawText("Cover enemies in snow, then kick them to defeat!", 300, tY + 36, 10,
        sf::Color(Theme::BTN_TEXT.r, Theme::BTN_TEXT.g, Theme::BTN_TEXT.b, a), true);
    drawText("Collect gems for bonus points. Boss levels are harder!", 300, tY + 56, 10,
        sf::Color(Theme::BTN_TEXT.r, Theme::BTN_TEXT.g, Theme::BTN_TEXT.b, a), true);
    drawText("ESC = BACK", 300, tY + tH + 16, 10,
        sf::Color(Theme::BTN_TEXT.r, Theme::BTN_TEXT.g, Theme::BTN_TEXT.b, a), true);
}

// =============================================================
//  LEADERBOARD  
// =============================================================
void Game::drawLeaderboard(float alpha) {
    drawBackground_Menu(alpha);
    sf::Uint8 a = (sf::Uint8)(alpha * 255);

    drawText("LEADERBOARD", 300, 20, 22,
        sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, a), true);

    // Column headers
    float headerY = 60.f;
    pixelBox(40.f, headerY, 520.f, 26.f,
        sf::Color(Theme::BTN_FILL_SEL.r, Theme::BTN_FILL_SEL.g,
            Theme::BTN_FILL_SEL.b, a),
        sf::Color(Theme::BTN_BDR_SEL.r, Theme::BTN_BDR_SEL.g,
            Theme::BTN_BDR_SEL.b, a), 1.5f);

    drawText("RANK", 65, headerY + 6, 10,
        sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, a));
    drawText("PLAYER", 160, headerY + 6, 10,
        sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, a));
    drawText("SCORE", 330, headerY + 6, 10,
        sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, a));
    drawText("LEVEL", 430, headerY + 6, 10,
        sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, a));
    drawText("DATE", 490, headerY + 6, 10,
        sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, a));

    // Load top 10
    LeaderboardEntry entries[10];
    int count = LeaderboardManager::getInstance().loadTopN(entries, 10);

    if (count == 0) {
        drawText("No scores recorded yet. Complete a level to appear here!",
            300, 280, 11, sf::Color(80, 140, 180, a), true);
    }
    else {
        float rowY = headerY + 34.f;
        for (int i = 0; i < count; ++i) {
            // Alternating row background
            sf::Color rowFill = (i % 2 == 0)
                ? sf::Color(Theme::BTN_FILL.r, Theme::BTN_FILL.g,
                    Theme::BTN_FILL.b, (sf::Uint8)(a * 0.8f))
                : sf::Color(Theme::BTN_FILL.r, Theme::BTN_FILL.g,
                    Theme::BTN_FILL.b, (sf::Uint8)(a * 0.5f));

            pixelBox(40.f, rowY, 520.f, 28.f, rowFill,
                sf::Color(Theme::BTN_BORDER.r, Theme::BTN_BORDER.g,
                    Theme::BTN_BORDER.b, (sf::Uint8)(a * 0.4f)), 1.f);

            // Gold/silver/bronze for top 3
            sf::Color rankCol = sf::Color(Theme::BTN_TEXT.r,
                Theme::BTN_TEXT.g,
                Theme::BTN_TEXT.b, a);
            if (i == 0) rankCol = sf::Color(255, 215, 0, a);  // gold
            if (i == 1) rankCol = sf::Color(192, 192, 192, a);  // silver
            if (i == 2) rankCol = sf::Color(205, 127, 50, a);  // bronze

            // Build strings
            std::ostringstream rankStr;  rankStr << (i + 1);
            std::ostringstream scoreStr; scoreStr << entries[i].score;
            std::ostringstream lvlStr;   lvlStr << entries[i].level_Reached;

            // Trim date to just YYYY-MM-DD (first 10 chars)
            char dateShort[11] = {};
            for (int c = 0; c < 10 && entries[i].recorded_At[c]; ++c)
                dateShort[c] = entries[i].recorded_At[c];

            drawText(rankStr.str().c_str(), 65, rowY + 8, 11, rankCol);
            drawText(entries[i].username, 160, rowY + 8, 11,
                sf::Color(255, 255, 255, a));
            drawText(scoreStr.str().c_str(), 330, rowY + 8, 11,
                sf::Color(255, 255, 100, a));
            drawText(lvlStr.str().c_str(), 440, rowY + 8, 11,
                sf::Color(Theme::ACCENT_DIM.r, Theme::ACCENT_DIM.g,
                    Theme::ACCENT_DIM.b, a));
            drawText(dateShort, 490, rowY + 8, 9,
                sf::Color(Theme::ACCENT_DIM.r, Theme::ACCENT_DIM.g,
                    Theme::ACCENT_DIM.b, a));

            rowY += 32.f;
        }
    }

    drawText("ESC = BACK", 300, 555, 10,
        sf::Color(Theme::BTN_TEXT.r, Theme::BTN_TEXT.g, Theme::BTN_TEXT.b, a), true);
}

// =============================================================
//  PLAYING SCREEN
// =============================================================
void Game::drawPlaying(float alpha) {
    // Background
    if (hasLevelBg) window.draw(levelBgSprite);

    // Platforms
    for (int i = 0; i < platformCount; ++i)
        window.draw(platforms[i].shape);

    // Enemies
    for (int i = 0; i < enemyCount; ++i)
        if (enemies[i] && !enemies[i]->is_dead())
            enemies[i]->draw(window);

    // Snowballs + player
    snowballMgr.draw(window);
    playerCtrl.draw(window);

    // Debug hitbox overlay (B key)
    if (debugHitbox) {
        const Player& dbgPl = playerCtrl.getPlayer();
        sf::FloatRect pb = dbgPl.shape.getGlobalBounds();
        sf::RectangleShape pRect(sf::Vector2f(pb.width, pb.height));
        pRect.setPosition(pb.left, pb.top);
        pRect.setFillColor(sf::Color::Transparent);
        pRect.setOutlineColor(sf::Color(0, 255, 0, 220));
        pRect.setOutlineThickness(2.f);
        window.draw(pRect);

        for (int i = 0; i < platformCount; ++i) {
            sf::FloatRect pr = platforms[i].shape.getGlobalBounds();
            sf::RectangleShape platR(sf::Vector2f(pr.width, pr.height));
            platR.setPosition(pr.left, pr.top);
            platR.setFillColor(sf::Color::Transparent);
            platR.setOutlineColor(sf::Color(80, 80, 255, 180));
            platR.setOutlineThickness(2.f);
            window.draw(platR);
        }

        const Snowball* balls = snowballMgr.getBalls();
        int             ballCount = snowballMgr.getCount();
        for (int i = 0; i < ballCount; ++i) {
            if (!balls[i].active) continue;
            sf::CircleShape dbgC(9.f); dbgC.setOrigin(9.f, 9.f);
            dbgC.setPosition(balls[i].sprite.getPosition());
            dbgC.setFillColor(sf::Color::Transparent);
            dbgC.setOutlineColor(sf::Color(255, 255, 0, 220));
            dbgC.setOutlineThickness(2.f); dbgC.setPointCount(16);
            window.draw(dbgC);
        }

        for (int i = 0; i < enemyCount; ++i) {
            if (!enemies[i] || enemies[i]->is_dead()) continue;
            HitBox& ehb = enemies[i]->get_hit_box();
            sf::RectangleShape eRect(sf::Vector2f(ehb.rect.width, ehb.rect.height));
            eRect.setPosition(ehb.rect.left, ehb.rect.top);
            eRect.setFillColor(sf::Color::Transparent);
            eRect.setOutlineColor(sf::Color(255, 80, 80, 220));
            eRect.setOutlineThickness(2.f);
            window.draw(eRect);
        }
    }

    // HUD bar
    sf::RectangleShape hudBar(sf::Vector2f((float)W, 36.f));
    hudBar.setPosition(0.f, 0.f);
    hudBar.setFillColor(sf::Color(0, 10, 30, 210));
    window.draw(hudBar);

    std::ostringstream lvlStr;
    lvlStr << "LEVEL " << currentLevel << "/" << MAX_LEVELS;
    drawText(lvlStr.str().c_str(), 10, 8, 14,
        sf::Color(Theme::ACCENT.r, Theme::ACCENT.g, Theme::ACCENT.b, (sf::Uint8)(alpha * 255)));

    std::ostringstream scoreStr;
    scoreStr << "SCORE: " << playerScore;
    drawText(scoreStr.str().c_str(), (float)W / 2 - 60, 8, 14,
        sf::Color(255, 255, 100, (sf::Uint8)(alpha * 255)));

    std::ostringstream enemyStr;
    enemyStr << "ENEMIES: " << enemyCount;
    drawText(enemyStr.str().c_str(), (float)W / 2 + 40, 8, 14,
        sf::Color(Theme::ACCENT_DIM.r, Theme::ACCENT_DIM.g, Theme::ACCENT_DIM.b,
            (sf::Uint8)(alpha * 255)));

    drawText("ARROWS:move  SPACE:jump  Z:shoot  B:hitbox  ESC:menu",
        (float)W / 2, (float)H - 16, 8,
        sf::Color(120, 170, 210, (sf::Uint8)(alpha * 120)), true);
}

// =============================================================
//  LEVEL COMPLETE SCREEN
// =============================================================
void Game::drawLevelComplete() {
    if (hasEndBg) window.draw(endBgSprite);
    else          window.clear(sf::Color(5, 20, 10));

    sf::RectangleShape ov(sf::Vector2f((float)W, (float)H));
    ov.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(ov);

    // Logo
    if (hasLogo) {
        sf::Vector2u sz = logoTex.getSize();
        float        scale = 160.f / sz.x;
        logoSprite.setScale(scale, scale);
        logoSprite.setPosition((float)W / 2 - 80.f, 55.f);
        logoSprite.setColor(sf::Color(255, 255, 255, 255));
        window.draw(logoSprite);
    }

    drawText("LEVEL COMPLETE!", (float)W / 2, 180, 32, sf::Color(100, 255, 120), true);

    std::ostringstream s1;
    s1 << "SCORE:  " << levelScore;
    drawText(s1.str().c_str(), (float)W / 2, 242, 20, sf::Color(255, 255, 100), true);

    bool isLast = (currentLevelIdx >= MAX_LEVELS - 1);
    pixelBox(200.f, 330.f, 200.f, 46.f,
        sf::Color(Theme::BTN_FILL_SEL.r, Theme::BTN_FILL_SEL.g, Theme::BTN_FILL_SEL.b, 255),
        sf::Color(Theme::BTN_BDR_SEL.r, Theme::BTN_BDR_SEL.g, Theme::BTN_BDR_SEL.b, 255), 2.f);
    drawText(isLast ? "MAIN MENU" : "NEXT LEVEL",
        300.f, 344.f, 15, sf::Color(Theme::BTN_TEXT_SEL), true);
    pixelBox(200.f, 390.f, 200.f, 46.f,
        sf::Color(Theme::BTN_FILL.r, Theme::BTN_FILL.g, Theme::BTN_FILL.b, 255),
        sf::Color(Theme::BTN_BORDER.r, Theme::BTN_BORDER.g, Theme::BTN_BORDER.b, 255), 2.f);
    drawText("LEVEL SELECT", 300.f, 404.f, 15, sf::Color(Theme::BTN_TEXT), true);
}