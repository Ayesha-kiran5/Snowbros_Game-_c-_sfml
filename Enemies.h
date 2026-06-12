#pragma once
#include <SFML/Graphics.hpp>
#include <string>
using namespace std;

enum class Snow_State {
    NONE, PARTIAL, FULLY_COVERED
};
enum class Foe_Variant {
    RED, GREEN, BLUE
};
enum class Direction {
    LEFT, RIGHT
};
struct HitBox {
    sf::FloatRect rect;

    HitBox() : rect(0.f, 0.f, 0.f, 0.f) {}
    HitBox(float x, float y, float w, float h) : rect(x, y, w, h) {}

    // Move the rect to match the owner's new position each frame
    void update(float x, float y) { rect.left = x; rect.top = y; }

    bool intersects(const HitBox& other) const { return rect.intersects(other.rect); }
};

// ============================================================
//  PROJECTILE  (abstract)
//  Base for anything that flies through the air and can expire.
//  Knife is the only concrete type right now.
// ============================================================
class Projectile {
public:
    HitBox hitBox;

    Projectile(float x, float y, float w, float h) : hitBox(x, y, w, h) {}

    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& w) = 0;
    virtual bool is_expired() const = 0;
    virtual ~Projectile() = default;
};

// ============================================================
//  KNIFE
//  Thrown by Tornado toward the player's last known position.
//  Travels in a straight line and expires when it leaves the
//  screen bounds.
// ============================================================
class Knife : public Projectile {
    sf::RectangleShape shape;
    float vx, vy;   // velocity components, set in constructor
    float px, py;   // current position, updated every frame
    bool  expired;

public:
    // Fires from (sx, sy) toward target (tx, ty) at fixed speed
    Knife(float sx, float sy, float tx, float ty);
    void update(float dt)  override;
    void draw(sf::RenderWindow& w)override;
    bool is_expired() const override;
};

class Foe {
protected:
    // Invisible box ---> all physics and collision checks use this
    sf::RectangleShape shape;
    sf::Vector2f  velocity;
    bool  onGround;

    // The actual sprite drawn to the screen each frame
    sf::Sprite  sprite;
    sf::Texture texture;

    // Animation state ==> points at a static frame array in the .cpp,
    // so never heap-allocated and never deleted here
    const sf::IntRect* animFrames;
    int  animFrameCount;
    int curFrame;
    float frameTimer;
    float frameSPF;   // how long each frame stays visible

    // Kept in sync with animFrames[curFrame] by tick_anim()
    int frameW;
    int frameH;

    // Movement
    float baseSpeed;
    float currentSpeed;
    Direction facing;
    float dirTimer;
    float dirInterval;

    // Patrol bounds==> set by Game after spawn, -1 means no limit
    float patrolMinX;
    float patrolMaxX;
    float floorY;   // shape.top Y when the enemy is standing still

    // Health and snow cover tracking
    int maxHealth;
    int currentHealth;
    int hitsToEncase;         // snowballs needed to fully cover
    int snowballHitsReceived;
    Snow_State snowState;

    bool alive;
    bool  dead;
    Foe_Variant variant;
    int  gemDropAmount;

    // Physics constants --->same window dimensions as the rest of the game
    static constexpr float GRAVITY_ACC = 900.f;
    static constexpr float WIN_W = 600.f;
    static constexpr float WIN_H = 600.f;

    // Recalculates snow state after each hit --> partial slow or full freeze
    void check_encase_threshold();

    // Switches to a new animation frame array and resets curFrame to 0.
    // Pass force=true when you need to override what was set earlier
    // in the same tick (Tornado does this to stomp FlyingFooga's choice).
    void set_anim_frames(const sf::IntRect* frames, int count, float spf,
        bool force = false);

    // Advances the animation by dt --> call once per update()
    void tick_anim(float dt);

    // Moves the enemy left or right and clamps to screen + patrol edges
    void walk_horizontal(float dt);

    // Pulls the enemy down and lands it when it reaches floorY
    void apply_gravity(float dt);

    // Snaps the sprite over the collision box--> mirrors player.cpp exactly:
    //   sprite bottom == shape bottom
    //   sprite centred on shape horizontally
    //   left-facing enemies flip via setScale(-1, 1)
    void position_sprite_over_shape();

public:
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& w) = 0;
    virtual string get_name() const = 0;
    virtual int  get_min_score() const = 0;
    virtual int get_max_score() const = 0;

    // Called by Game right after spawning the enemy.
    // platformTop is the Y of the surface it should stand on.
    void set_base_y(float platformTop);

    // Restricts horizontal movement to the platform it lives on
    void set_patrol_bounds(float minX, float maxX);

    // Hit by a snowball — updates snow state and may slow/freeze the enemy
    void snowball_received();

    bool is_fully_covered() const;
    bool is_alive()  const;

    // Instantly kills the enemy (used when a rolled snowball lands)
    void roll_hit();

    // Applies RED/GREEN/BLUE stat differences--> speed, health, gem drops
    void apply_variant(Foe_Variant v);

    // Returns a copy of the hitbox --> safe to store for one frame
    HitBox get_hit_box_value() const;

    // Returns a reference -- kept for legacy game.cpp code that expects this
    HitBox& get_hit_box();

    void get_position(float& x, float& y) const;
    Snow_State  get_snow_state()const;
    Foe_Variant get_variant()const;
    bool is_dead() const;
    int get_gem_drop() const;
    void set_position(float x, float y);

    virtual ~Foe() = default;

private:
    // Backing storage for get_hit_box()--> avoids dangling refs
    mutable HitBox legacyHB;
};

class Botom : public Foe {
protected:
    float dirChangeTimer;
    float dirChangeInterval;

    // Death plays out in two animation passes (DIE then AAAHH)
    bool dyingStarted;
    int  dyingPhase;   // 0 = DIE frames, 1 = AAAHH frame

    void randomize_direction();

public:
    Botom(float x, float y, Foe_Variant v = Foe_Variant::RED);

    void update(float dt)  override;
    void draw(sf::RenderWindow& w) override;
    string get_name() const override { return "Botom"; }
    int  get_min_score() const override { return 100; }
    int  get_max_score() const override { return 500; }
};

class GreenBotom : public Botom {
public:
    GreenBotom(float x, float y);
    string get_name() const override { return "Botom (Green)"; }
    int  get_min_score() const override { return 150; }
    int get_max_score() const override { return 600; }
};

class BlueBotom : public Botom {
public:
    BlueBotom(float x, float y);
    string get_name() const override { return "Botom (Blue)"; }
    int get_min_score() const override { return 200; }
    int  get_max_score() const override { return 700; }
};

class FlyingFooga_Foe : public Botom {
protected:
    bool  inFlightMode;
    float groundTimer;
    float flightTimer;
    float groundDuration;   // how long it walks before lifting off
    float flightDuration;   // how long it stays airborne
    float flightDirX;
    float flightDirY;

    // Death is two separate frame sets played back to back
    bool dyingFF;
    int  dyingFFPhase;   // 0 = DYING1, 1 = DYING2

    void enter_flight_mode();
    void exit_flight_mode();
    void update_flight(float dt);

public:
    FlyingFooga_Foe(float x, float y, Foe_Variant v = Foe_Variant::RED);
    void update(float dt) override;
    void draw(sf::RenderWindow& w) override;
    string get_name() const override { return "Flyng Fooga Foog"; }
    int get_min_score() const override { return 200; }
    int get_max_score() const override { return 800; }
};

class GreenFlyingFooga : public FlyingFooga_Foe {
public:
    GreenFlyingFooga(float x, float y);
    string get_name()  const override { return "Flyng Fooga Foog (Green)"; }
    int get_min_score() const override { return 300; }
    int get_max_score() const override { return 1000; }
};

class BlueFlyingFooga : public FlyingFooga_Foe {
public:
    BlueFlyingFooga(float x, float y);
    string get_name() const override { return "Flyng Fooga Foog (Blue)"; }
    int  get_min_score() const override { return 400; }
    int get_max_score() const override { return 1200; }
};

class Tornado : public FlyingFooga_Foe {
protected:
    // Pointer to the player ---> needed to aim knives
    const float* playerX;
    const float* playerY;

    float knifeTimer;
    float knifeInterval;   // seconds between knife throws

    float minFlightSpeed;
    float maxFlightSpeed;

    static const int MAX_KNIVES = 10;
    Knife* knives[MAX_KNIVES];
    int knifeCount;

    void throw_knife();
    void randomize_flight_speed();
    void update_knives(float dt);
    void draw_knives(sf::RenderWindow& w);
    void clean_expired_knives();

    // Death uses two sprite rows — track which one we're on
    int  diePhase;    // 0 = DIE1, 1 = DIE2
    bool dieStarted;

public:
    Tornado(float x, float y, const float* px, const float* py, Foe_Variant v = Foe_Variant::RED);
    ~Tornado() override;   // deletes any live knives

    void update(float dt) override;
    void draw(sf::RenderWindow& w) override;
    string get_name()const override { return "Tornado"; }
    int get_min_score() const override { return 300; }
    int  get_max_score() const override { return 1200; }

    // Game uses these to check knife collisions against the player
    Knife** get_knives()const;
    int get_knife_count() const;
};

class GreenTornado : public Tornado {
public:
    GreenTornado(float x, float y, const float* px, const float* py);
    string get_name()  const override { return "Tornado (Green)"; }
    int get_min_score() const override { return 450; }
    int get_max_score() const override { return 1600; }
};

class BlueTornado : public Tornado {
public:
    BlueTornado(float x, float y, const float* px, const float* py);
    string get_name()  const override { return "Tornado (Blue)"; }
    int    get_min_score() const override { return 600; }
    int    get_max_score() const override { return 2000; }
};

// ============================================================
//  FOE FACTORY
//  Pass a type string and spawn coordinates; get back a Foe*.
//  Caller owns the returned pointer. playerX/Y are only needed
//  for Tornado types.
// ============================================================
class FoeFactory {
public:
    static Foe* create(
        const string& type, float x, float y,
        const float* playerX = nullptr,
        const float* playerY = nullptr);
};
