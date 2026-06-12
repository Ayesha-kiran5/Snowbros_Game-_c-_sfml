#include "Enemies.h"
#include <cstdlib>
#include <cmath>
using namespace std;

Knife::Knife(float sx, float sy, float tx, float ty)
    : Projectile(sx, sy, 12.f, 6.f),
    px(sx), py(sy), expired(false)
{
    // Normalise the direction vector then scale to a fixed speed
    float dx = tx - sx, dy = ty - sy;
    float len = sqrtf(dx * dx + dy * dy);
    float spd = 300.f;
    if (len > 0.f) { vx = dx / len * spd; vy = dy / len * spd; }
    else { vx = spd; vy = 0.f; }

    shape.setSize(sf::Vector2f(12.f, 6.f));
    shape.setFillColor(sf::Color(220, 220, 60));
    shape.setPosition(px, py);
}

void Knife::update(float dt) {
    if (expired) return;
    px += vx * dt;
    py += vy * dt;
    shape.setPosition(px, py);
    hitBox.update(px, py);
    // Anything that flies this far off screen is never coming back
    if (px < -20 || px > 620 || py < -20 || py > 620) expired = true;
}

void Knife::draw(sf::RenderWindow& w) { if (!expired) w.draw(shape); }
bool Knife::is_expired() const { return expired; }
// set_anim_frames==> Switches to a new animation and resets the counter
// Skips if already active unless force=true (used by Tornado).
void Foe::set_anim_frames(const sf::IntRect* frames, int count, float spf, bool force) {
    if (!force && animFrames == frames) return;
    animFrames = frames;
    animFrameCount = count;
    frameSPF = spf;
    curFrame = 0;
    frameTimer = 0.f;
    frameW = frames[0].width;
    frameH = frames[0].height;
    sprite.setTextureRect(frames[0]);
}

// tick_anim ===> 
// Steps to the next frame once enough time has passed.
// Wraps around to 0 when it reaches the end of the array.
void Foe::tick_anim(float dt) {
    frameTimer += dt;
    if (frameTimer >= frameSPF) {
        frameTimer = 0.f;
        curFrame = (curFrame + 1) % animFrameCount;
        frameW = animFrames[curFrame].width;
        frameH = animFrames[curFrame].height;
        sprite.setTextureRect(animFrames[curFrame]);
    }
}

// position_sprite_over_shape==> 
// Keeps sprite aligned with the collision box bottom.
// Flips horizontally by scaling X to -1 and offsetting by frame width.
void Foe::position_sprite_over_shape() {
    sf::Vector2f boxPos = shape.getPosition();
    sf::Vector2f boxSize = shape.getSize();

    float scaleX = boxSize.x / (float)frameW;
    float scaleY = boxSize.y / (float)frameH;

    if (facing == Direction::LEFT) {
        sprite.setScale(-scaleX, scaleY);
        sprite.setPosition(boxPos.x + boxSize.x, boxPos.y);
    }
    else {
        sprite.setScale(scaleX, scaleY);
        sprite.setPosition(boxPos.x, boxPos.y);
    }
}

// walk_horizontal==> 
// Moves the enemy at currentSpeed in the direction it's facing,
// then clamps to both the screen edges and the patrol bounds.
// patrolMinX/MaxX are -1 when no patrol limit is set, so we
// guard against that before using them as real coordinates.
void Foe::walk_horizontal(float dt) {
    sf::Vector2f pos = shape.getPosition();
    sf::Vector2f size = shape.getSize();

    float mv = currentSpeed * dt;
    float newX = pos.x + (facing == Direction::LEFT ? -mv : mv);

    if (newX < 0.f) { newX = 0.f;             facing = Direction::RIGHT; }
    if (newX + size.x > WIN_W) { newX = WIN_W - size.x;  facing = Direction::LEFT; }

    if (patrolMinX >= 0.f && newX < patrolMinX)
    {
        newX = patrolMinX;            facing = Direction::RIGHT;
    }
    if (patrolMaxX >= 0.f && newX + size.x > patrolMaxX)
    {
        newX = patrolMaxX - size.x;   facing = Direction::LEFT;
    }

    shape.setPosition(newX, pos.y);
}

// apply_gravity
// Accumulates downward velocity while airborne and stops the
// enemy when it reaches floorY. The 1-pixel epsilon on the
// onGround check stops a one-frame flicker where enemies at
// rest were briefly marked as falling.
void Foe::apply_gravity(float dt) {
    sf::Vector2f pos = shape.getPosition();

    if (!onGround) velocity.y += GRAVITY_ACC * dt;

    float newY = pos.y + velocity.y * dt;

    if (newY >= floorY) {
        newY = floorY;
        velocity.y = 0.f;
        onGround = true;
    }
    else {
        onGround = (pos.y >= floorY - 1.f);
    }

    shape.setPosition(pos.x, newY);
}

// set_base_y
// Called by Game after spawning. Positions the enemy so its
// feet sit exactly on the platform surface.
void Foe::set_base_y(float platformTop) {
    sf::Vector2f size = shape.getSize();
    float restY = platformTop - size.y;
    floorY = restY;
    shape.setPosition(shape.getPosition().x, restY);
    velocity.y = 0.f;
    onGround = true;
}

void Foe::set_patrol_bounds(float minX, float maxX) {
    patrolMinX = minX;
    patrolMaxX = maxX;
}

void Foe::set_position(float x, float y) {
    shape.setPosition(x, y);
    legacyHB.rect.left = x;
    legacyHB.rect.top = y;
}


void Foe::snowball_received() {
    if (!alive) return;
    ++snowballHitsReceived;
    check_encase_threshold();
}

// check_encase_threshold ==> 
// One hit past the halfway mark gives a PARTIAL slow.
// Hitting hitsToEncase fully freezes the enemy.
void Foe::check_encase_threshold() {
    int half = hitsToEncase / 2;
    if (half < 1) half = 1;

    if (snowballHitsReceived >= hitsToEncase) {
        snowState = Snow_State::FULLY_COVERED;
        currentSpeed = 0.f;
    }
    else if (snowballHitsReceived >= half) {
        snowState = Snow_State::PARTIAL;
        currentSpeed = baseSpeed * 0.4f;
    }
}

bool Foe::is_fully_covered() const {
    return snowState == Snow_State::FULLY_COVERED;
}
bool Foe::is_alive() const {
    return alive;
}

void Foe::roll_hit() {
    currentHealth = 0;
    alive = false;
    dead = true;
}

// apply_variant
void Foe::apply_variant(Foe_Variant v) {
    variant = v;
    switch (v) {
    case Foe_Variant::RED:
        currentSpeed = baseSpeed * 1.00f;
        hitsToEncase = 2;
        maxHealth = 1;
        gemDropAmount = 2;
        break;
    case Foe_Variant::GREEN:
        currentSpeed = baseSpeed * 1.25f;
        hitsToEncase = 3;
        maxHealth = 2;
        gemDropAmount = 4;
        break;
    case Foe_Variant::BLUE:
        currentSpeed = baseSpeed * 1.50f;
        hitsToEncase = 4;
        maxHealth = 3;
        gemDropAmount = 6;
        break;
    }
    currentHealth = maxHealth;
}

//Getters

HitBox Foe::get_hit_box_value() const {
    sf::FloatRect gb = shape.getGlobalBounds();
    return HitBox(gb.left, gb.top, gb.width, gb.height);
}

// get_hit_box  legacy ref version; legacyHB is refreshed every call
HitBox& Foe::get_hit_box() {
    legacyHB.rect = shape.getGlobalBounds();
    return legacyHB;
}

void Foe::get_position(float& x, float& y) const {
    x = shape.getPosition().x;
    y = shape.getPosition().y;
}
Snow_State Foe::get_snow_state() const {
    return snowState;
}
Foe_Variant Foe::get_variant() const {
    return variant;
}
bool Foe::is_dead()const {
    return dead;
}
int Foe::get_gem_drop()const {
    return gemDropAmount;
}


static const sf::IntRect BOT_FRAMES_IDLE[] = { {20,135,81,85} };
static const sf::IntRect BOT_FRAMES_CHANGE[] = { {20,240,187,85} };
static const sf::IntRect BOT_FRAMES_WALK[] = { {20,345,183,84}, {223,345,93,84} };
static const sf::IntRect BOT_FRAMES_TUMP[] = { {20,449,88,98} };
static const sf::IntRect BOT_FRAMES_FALL[] = { {20,567,86,97} };
static const sf::IntRect BOT_FRAMES_TRAPPED[] = { {20,684,90,86}, {130,684,87,86} };
static const sf::IntRect BOT_FRAMES_UNLSH[] = { {20,790,84,86}, {124,790,84,86}, {228,790,88,86} };
static const sf::IntRect BOT_FRAMES_STARE[] = { {20,896,93,87} };
static const sf::IntRect BOT_FRAMES_DIE[] = { {20,1003,274,98}, {314,1003,89,98},
                                                    {423,1003,176,98}, {619,1003,278,98} };
static const sf::IntRect BOT_FRAMES_AAAHH[] = { {20,1121,88,86} };

#define BOT_ANIM(arr)  arr, (int)(sizeof(arr)/sizeof(arr[0]))

static const float BOT_BOX_W = 60.f;
static const float BOT_BOX_H = 80.f;

// ============================================================
//  BOTOM constructor
// ============================================================
Botom::Botom(float x, float y, Foe_Variant v)
    : dirChangeTimer(0.f), dirChangeInterval(1.5f),
    dyingStarted(false), dyingPhase(0)
{
    shape.setSize(sf::Vector2f(BOT_BOX_W, BOT_BOX_H));
    shape.setFillColor(sf::Color::Transparent);
    shape.setPosition(x, y);

    velocity = sf::Vector2f(0.f, 0.f);
    onGround = false;
    floorY = y;   // corrected by Game via set_base_y()

    animFrames = nullptr;
    animFrameCount = 0;
    curFrame = 0;
    frameTimer = 0.f;
    frameSPF = 0.15f;
    frameW = (int)BOT_BOX_W;
    frameH = (int)BOT_BOX_H;

    baseSpeed = 80.f;
    facing = Direction::RIGHT;
    dirTimer = 0.f;
    dirInterval = 0.f;
    patrolMinX = -1.f;
    patrolMaxX = -1.f;

    snowState = Snow_State::NONE;
    snowballHitsReceived = 0;
    alive = true; dead = false;
    apply_variant(v);

    string path;
    switch (v) {
    case Foe_Variant::GREEN:
        path = "Botom_Green.png";
        break;
    case Foe_Variant::BLUE:
        path = "Botom_Blue.png";
        break;
    default:
        path = "Botom_Orange.png";
        break;
    }
    if (!texture.loadFromFile(path)) {
        // Fallback solid colour so a missing asset doesn't crash
        sf::Image img; img.create(100, 100, sf::Color(200, 100, 30));
        texture.loadFromImage(img);
    }
    sprite.setTexture(texture);

    set_anim_frames(BOT_ANIM(BOT_FRAMES_IDLE), 0.18f);
    randomize_direction();
}

void Botom::randomize_direction() {
    facing = (rand() % 2 == 0) ? Direction::LEFT : Direction::RIGHT;
}

void Botom::update(float dt) {
    // Handle death before anything else so a dying enemy stops moving
    if (!alive) {
        if (!dead) {
            if (!dyingStarted) {
                set_anim_frames(BOT_ANIM(BOT_FRAMES_DIE), 0.08f);
                dyingStarted = true;
                dyingPhase = 0;
            }
            tick_anim(dt);
            position_sprite_over_shape();

            // Switch to AAAHH once DIE finishes, then mark dead
            if (dyingPhase == 0 && curFrame == animFrameCount - 1) {
                dyingPhase = 1;
                set_anim_frames(BOT_ANIM(BOT_FRAMES_AAAHH), 0.08f, true);
            }
            if (dyingPhase == 1 && curFrame == animFrameCount - 1) dead = true;
        }
        return;
    }

    // Fully covered — just stand there and loop the trapped animation
    if (snowState == Snow_State::FULLY_COVERED) {
        set_anim_frames(BOT_ANIM(BOT_FRAMES_TRAPPED), 0.15f);
        tick_anim(dt);
        position_sprite_over_shape();
        return;
    }

    // Occasionally pick a new random walk direction
    dirChangeTimer += dt;
    if (dirChangeTimer >= dirChangeInterval) {
        dirChangeTimer = 0.f;
        randomize_direction();
    }

    walk_horizontal(dt);
    apply_gravity(dt);

    // Choose animation based on current state
    if (snowState == Snow_State::PARTIAL) {
        set_anim_frames(BOT_ANIM(BOT_FRAMES_CHANGE), 0.15f);
    }
    else if (!onGround) {
        set_anim_frames(BOT_ANIM(BOT_FRAMES_FALL), 0.12f);
    }
    else {
        set_anim_frames(BOT_ANIM(BOT_FRAMES_WALK), 0.15f);
    }

    tick_anim(dt);
    position_sprite_over_shape();
}

void Botom::draw(sf::RenderWindow& w) {
    if (dead) return;
    w.draw(sprite);
}

GreenBotom::GreenBotom(float x, float y) : Botom(x, y, Foe_Variant::GREEN) {}
BlueBotom::BlueBotom(float x, float y) : Botom(x, y, Foe_Variant::BLUE) {}

static const sf::IntRect FF_FRAMES_FLY0[] = { {20,20,120,204},  {160,20,199,204},  {379,20,170,204} };
static const sf::IntRect FF_FRAMES_FLY1[] = { {20,244,159,186}, {199,244,160,186}, {379,244,182,186} };
static const sf::IntRect FF_FRAMES_TRAPPED[] = { {20,450,187,154}, {227,450,189,154} };
static const sf::IntRect FF_FRAMES_WALK[] = { {20,624,192,162}, {232,624,198,162}, {450,624,176,162} };
static const sf::IntRect FF_FRAMES_DIE1[] = { {20,806,166,175}, {206,806,215,175}, {441,806,193,175} };
static const sf::IntRect FF_FRAMES_DIE2[] = { {20,1001,168,177}, {208,1001,211,177},
                                                   {439,1001,197,177}, {656,1001,206,177} };

#define FF_ANIM(arr)  arr, (int)(sizeof(arr)/sizeof(arr[0]))

static const float FF_BOX_W = 70.f;
static const float FF_BOX_H = 90.f;

// ============================================================
//  FLYINGFOOGA constructor
// ============================================================
FlyingFooga_Foe::FlyingFooga_Foe(float x, float y, Foe_Variant v)
    : Botom(x, y, v),
    inFlightMode(false), groundTimer(0.f), flightTimer(0.f),
    groundDuration(3.f), flightDuration(2.f),
    flightDirX(0.f), flightDirY(0.f),
    dyingFF(false), dyingFFPhase(0)
{
    shape.setSize(sf::Vector2f(FF_BOX_W, FF_BOX_H));
    floorY = y;
    baseSpeed = 100.f;
    apply_variant(v);

    string path;
    switch (v) {
    case Foe_Variant::GREEN: path = "FlyingFoogaFoog_Green.png"; break;
    case Foe_Variant::BLUE:  path = "FlyingFoogaFoog_Blue.png";  break;
    default:                 path = "FlyingFoogaFoog_Red.png";   break;
    }
    if (!texture.loadFromFile(path)) {
        sf::Image img; img.create(200, 200, sf::Color(60, 180, 60));
        texture.loadFromImage(img);
    }
    sprite.setTexture(texture);

    set_anim_frames(FF_ANIM(FF_FRAMES_WALK), 0.12f);
    position_sprite_over_shape();
}

void FlyingFooga_Foe::enter_flight_mode() {
    inFlightMode = true;
    flightTimer = 0.f;
    // Pick one of 8 cardinal/diagonal directions at random
    static const float AX[8] = { 0.f,  0.7f, 1.f,  0.7f,  0.f, -0.7f, -1.f, -0.7f };
    static const float AY[8] = { -1.f, -0.7f, 0.f,  0.7f,  1.f,  0.7f,  0.f, -0.7f };
    int d = rand() % 8;
    flightDirX = AX[d];
    flightDirY = AY[d];
}

void FlyingFooga_Foe::exit_flight_mode() {
    inFlightMode = false;
    groundTimer = 0.f;
    flightDirX = 0.f;
    flightDirY = 0.f;
    velocity.y = 0.f;
    onGround = true;
    // Snap back to the floor so it doesn't hover a pixel above it
    shape.setPosition(shape.getPosition().x, floorY);
}

void FlyingFooga_Foe::update_flight(float dt) {
    sf::Vector2f pos = shape.getPosition();
    sf::Vector2f size = shape.getSize();

    float nx = pos.x + flightDirX * currentSpeed * dt;
    float ny = pos.y + flightDirY * currentSpeed * dt;

    // Bounce off all four walls
    if (nx < 0.f) {
        nx = 0.f;
        flightDirX = fabsf(flightDirX);
    }
    if (nx + size.x > WIN_W) {
        nx = WIN_W - size.x;
        flightDirX = -fabsf(flightDirX);
    }
    if (ny < 0.f) {
        ny = 0.f;
        flightDirY = fabsf(flightDirY);
    }
    if (ny + size.y > WIN_H) {
        ny = WIN_H - size.y;
        flightDirY = -fabsf(flightDirY);
    }

    shape.setPosition(nx, ny);
}

void FlyingFooga_Foe::update(float dt) {
    // Same two-pass death sequence as Botom but uses FF-specific frame sets
    if (!alive) {
        if (!dead) {
            if (!dyingFF) {
                set_anim_frames(FF_ANIM(FF_FRAMES_DIE1), 0.10f);
                dyingFF = true;
                dyingFFPhase = 0;
            }
            tick_anim(dt);
            position_sprite_over_shape();

            if (dyingFFPhase == 0 && curFrame == animFrameCount - 1) {
                dyingFFPhase = 1;
                set_anim_frames(FF_ANIM(FF_FRAMES_DIE2), 0.10f, true);
            }
            if (dyingFFPhase == 1 && curFrame == animFrameCount - 1) dead = true;
        }
        return;
    }

    if (snowState == Snow_State::FULLY_COVERED) {
        set_anim_frames(FF_ANIM(FF_FRAMES_TRAPPED), 0.15f);
        tick_anim(dt);
        position_sprite_over_shape();
        return;
    }

    if (inFlightMode) {
        flightTimer += dt;
        update_flight(dt);
        if (flightTimer >= flightDuration) exit_flight_mode();

        // Alternate between two flight rows to give the wings some life
        if (fmodf(flightTimer, 0.5f) < 0.25f)
            set_anim_frames(FF_ANIM(FF_FRAMES_FLY0), 0.12f);
        else
            set_anim_frames(FF_ANIM(FF_FRAMES_FLY1), 0.12f);
    }
    else {
        // Ground phase ---> walk around until the flight timer fires
        dirChangeTimer += dt;
        if (dirChangeTimer >= dirChangeInterval) {
            dirChangeTimer = 0.f;
            randomize_direction();
        }
        walk_horizontal(dt);
        apply_gravity(dt);

        groundTimer += dt;
        if (groundTimer >= groundDuration) enter_flight_mode();

        if (snowState == Snow_State::PARTIAL)
            set_anim_frames(FF_ANIM(FF_FRAMES_TRAPPED), 0.15f);
        else if (!onGround)
            set_anim_frames(FF_ANIM(FF_FRAMES_FLY0), 0.12f);
        else
            set_anim_frames(FF_ANIM(FF_FRAMES_WALK), 0.12f);
    }

    tick_anim(dt);
    position_sprite_over_shape();
}

void FlyingFooga_Foe::draw(sf::RenderWindow& w) {
    if (dead) return;
    w.draw(sprite);
}

GreenFlyingFooga::GreenFlyingFooga(float x, float y)
    : FlyingFooga_Foe(x, y, Foe_Variant::GREEN)
{
    // Spends a bit less time on the ground and a bit more in the air
    groundDuration = 2.5f;
    flightDuration = 2.5f;
}

BlueFlyingFooga::BlueFlyingFooga(float x, float y)
    : FlyingFooga_Foe(x, y, Foe_Variant::BLUE)
{
    // Shortest walk phase, longest flight — hardest to hit
    groundDuration = 2.0f;
    flightDuration = 3.0f;
}

static const int TOR_FW = 120;
static const int TOR_IDLE_Y = 36, TOR_IDLE_H = 120, TOR_IDLE_F = 3;
static const int TOR_TRAP_Y = 170, TOR_TRAP_H = 120, TOR_TRAP_F = 2;
static const int TOR_SPIN_Y = 841, TOR_SPIN_H = 120, TOR_SPIN_F = 2;
static const int TOR_DIE1_Y = 1010, TOR_DIE1_H = 120, TOR_DIE1_F = 5;
static const int TOR_DIE2_Y = 1178, TOR_DIE2_H = 120, TOR_DIE2_F = 5;

static sf::IntRect make_tor_frame(int col, int rowY, int rowH) {
    return sf::IntRect(col * TOR_FW, rowY, TOR_FW, rowH);
}

static const sf::IntRect TOR_FRAMES_IDLE[3] = {
    make_tor_frame(0, TOR_IDLE_Y, TOR_IDLE_H),
    make_tor_frame(1, TOR_IDLE_Y, TOR_IDLE_H),
    make_tor_frame(2, TOR_IDLE_Y, TOR_IDLE_H) };

static const sf::IntRect TOR_FRAMES_TRAP[2] = {
    make_tor_frame(0, TOR_TRAP_Y, TOR_TRAP_H),
    make_tor_frame(1, TOR_TRAP_Y, TOR_TRAP_H) };

static const sf::IntRect TOR_FRAMES_SPIN[2] = {
    make_tor_frame(0, TOR_SPIN_Y, TOR_SPIN_H),
    make_tor_frame(1, TOR_SPIN_Y, TOR_SPIN_H) };

static const sf::IntRect TOR_FRAMES_DIE1[5] = {
    make_tor_frame(0, TOR_DIE1_Y, TOR_DIE1_H), make_tor_frame(1, TOR_DIE1_Y, TOR_DIE1_H),
    make_tor_frame(2, TOR_DIE1_Y, TOR_DIE1_H), make_tor_frame(3, TOR_DIE1_Y, TOR_DIE1_H),
    make_tor_frame(4, TOR_DIE1_Y, TOR_DIE1_H) };

static const sf::IntRect TOR_FRAMES_DIE2[5] = {
    make_tor_frame(0, TOR_DIE2_Y, TOR_DIE2_H), make_tor_frame(1, TOR_DIE2_Y, TOR_DIE2_H),
    make_tor_frame(2, TOR_DIE2_Y, TOR_DIE2_H), make_tor_frame(3, TOR_DIE2_Y, TOR_DIE2_H),
    make_tor_frame(4, TOR_DIE2_Y, TOR_DIE2_H) };

#define TOR_ANIM(arr)  arr, (int)(sizeof(arr)/sizeof(arr[0]))

static const float TOR_BOX_W = 65.f;
static const float TOR_BOX_H = 85.f;

// ============================================================
//  TORNADO constructor
// ============================================================
Tornado::Tornado(float x, float y, const float* px, const float* py, Foe_Variant v)
    : FlyingFooga_Foe(x, y, v),
    playerX(px), playerY(py),
    knifeTimer(0.f), knifeInterval(2.5f),
    minFlightSpeed(0.5f), maxFlightSpeed(2.5f),
    knifeCount(0), diePhase(0), dieStarted(false)
{
    shape.setSize(sf::Vector2f(TOR_BOX_W, TOR_BOX_H));
    floorY = y;
    baseSpeed = 120.f;
    apply_variant(v);

    groundDuration = 1.5f;
    flightDuration = 3.f;

    for (int i = 0; i < MAX_KNIVES; ++i) knives[i] = nullptr;

    string path;
    switch (v) {
    case Foe_Variant::GREEN:
        path = "Tornado_Red.png";
        break;
    case Foe_Variant::BLUE:
        path = "Tornado_Blue.png";
        break;
    default:
        path = "Tornado_Green.png";
        break;
    }
    if (!texture.loadFromFile(path)) {
        sf::Image img; img.create(TOR_FW, TOR_IDLE_H, sf::Color(140, 60, 200));
        texture.loadFromImage(img);
    }
    sprite.setTexture(texture);
    set_anim_frames(TOR_ANIM(TOR_FRAMES_IDLE), 0.15f);
    position_sprite_over_shape();
}

Tornado::~Tornado() {
    // Clean up any knives that were still in flight
    for (int i = 0; i < MAX_KNIVES; ++i) { delete knives[i]; knives[i] = nullptr; }
}

void Tornado::randomize_flight_speed() {
    float r = minFlightSpeed + (rand() % 1000) / 1000.f * (maxFlightSpeed - minFlightSpeed);
    currentSpeed = baseSpeed * r;
}

void Tornado::throw_knife() {
    if (!playerX || !playerY || knifeCount >= MAX_KNIVES) return;
    sf::Vector2f pos = shape.getPosition();
    sf::Vector2f sz = shape.getSize();
    float sx = pos.x + sz.x * 0.5f;
    float sy = pos.y + sz.y * 0.5f;
    // Find the first empty slot and insert there
    for (int i = 0; i < MAX_KNIVES; ++i) {
        if (!knives[i]) {
            knives[i] = new Knife(sx, sy, *playerX, *playerY);
            ++knifeCount;
            break;
        }
    }
}

void Tornado::update_knives(float dt) {
    for (int i = 0; i < MAX_KNIVES; ++i)
        if (knives[i]) knives[i]->update(dt);
}

void Tornado::clean_expired_knives() {
    for (int i = 0; i < MAX_KNIVES; ++i) {
        if (knives[i] && knives[i]->is_expired()) {
            delete knives[i];
            knives[i] = nullptr;
            --knifeCount;
        }
    }
}

void Tornado::draw_knives(sf::RenderWindow& w) {
    for (int i = 0; i < MAX_KNIVES; ++i)
        if (knives[i]) knives[i]->draw(w);
}

Knife** Tornado::get_knives()const {
    return const_cast<Knife**>(knives);
}
int Tornado::get_knife_count() const {
    return knifeCount;
}

void Tornado::update(float dt) {
    // Death plays out in DIE1 then DIE2 --->same pattern as the others
    if (!alive) {
        if (!dead) {
            if (!dieStarted) {
                set_anim_frames(TOR_ANIM(TOR_FRAMES_DIE1), 0.08f);
                dieStarted = true;
                diePhase = 0;
            }
            tick_anim(dt);
            position_sprite_over_shape();
            if (diePhase == 0 && curFrame == animFrameCount - 1) {
                diePhase = 1;
                set_anim_frames(TOR_ANIM(TOR_FRAMES_DIE2), 0.08f, true);
            }
            if (diePhase == 1 && curFrame == animFrameCount - 1) dead = true;
        }
        return;
    }
    if (snowState == Snow_State::FULLY_COVERED) return;

    // Randomise speed at the start of each new flight burst
    if (inFlightMode && flightTimer < 0.01f) randomize_flight_speed();

    // Let FlyingFooga handle movement, gravity, and ticking 
    // then we override its animation choice with Tornado-specific frames.
    // force=true is needed because FlyingFooga already called set_anim_frames
    // this tick and the same-array guard would otherwise block us.
    FlyingFooga_Foe::update(dt);

    if (snowState != Snow_State::NONE)
        set_anim_frames(TOR_ANIM(TOR_FRAMES_TRAP), 0.15f, true);
    else if (inFlightMode)
        set_anim_frames(TOR_ANIM(TOR_FRAMES_SPIN), 0.10f, true);
    else
        set_anim_frames(TOR_ANIM(TOR_FRAMES_IDLE), 0.15f, true);

    // Throw a knife every knifeInterval seconds
    knifeTimer += dt;
    if (knifeTimer >= knifeInterval) { knifeTimer = 0.f; throw_knife(); }
    update_knives(dt);
    clean_expired_knives();
}

void Tornado::draw(sf::RenderWindow& w) {
    if (dead) return;
    // Tornado spins symmetrically so we never flip it
    sprite.setScale(1.f, 1.f);
    sf::Vector2f pos = shape.getPosition();
    sf::Vector2f size = shape.getSize();
    float sprX = pos.x + size.x * 0.5f - TOR_FW * 0.5f;
    float sprY = pos.y + size.y - (float)frameH;
    sprite.setPosition(sprX, sprY);
    w.draw(sprite);
    draw_knives(w);
}

GreenTornado::GreenTornado(float x, float y, const float* px, const float* py)
    : Tornado(x, y, px, py, Foe_Variant::GREEN)
{
    knifeInterval = 2.0f;   // throws a bit faster than red
}

BlueTornado::BlueTornado(float x, float y, const float* px, const float* py)
    : Tornado(x, y, px, py, Foe_Variant::BLUE)
{
    knifeInterval = 1.5f;   // fastest knife rate of the three
}


// ============================================================
//  FOE FACTORY
// ============================================================
Foe* FoeFactory::create(const string& type, float x, float y,
    const float* px, const float* py)
{
    if (type == "Botom")
        return new Botom(x, y, Foe_Variant::RED);
    else if (type == "GreenBotom")
        return new GreenBotom(x, y);
    else if (type == "BlueBotom")
        return new BlueBotom(x, y);
    else if (type == "FlyingFooga")
        return new FlyingFooga_Foe(x, y, Foe_Variant::RED);
    else if (type == "GreenFlyingFooga")
        return new GreenFlyingFooga(x, y);
    else if (type == "BlueFlyingFooga")
        return new BlueFlyingFooga(x, y);
    else if (type == "Tornado")
        return new Tornado(x, y, px, py, Foe_Variant::RED);
    else if (type == "GreenTornado")
        return new GreenTornado(x, y, px, py);
    else if (type == "BlueTornado")
        return new BlueTornado(x, y, px, py);
    else
        return nullptr;
}
