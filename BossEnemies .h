//#pragma once
//#include "Enemies.h"
//#include <SFML/Graphics.hpp>
//#include <string>
//using namespace std;
//
//// ============================================================
////  MogeraChild
////  --> ik snowball se khtm ho jaate hain.
////  Do frames ki animation hai aur direction ke hisaab se move karte hain.
//// ============================================================
//class MogeraChild : public Foe
//{
//public:
//    MogeraChild(const sf::Vector2f& position,const sf::Texture&  texture,float direction);
//
//    ~MogeraChild() override = default;
//
//    void update(float deltaTime) override;
//    void draw(sf::RenderWindow& window) override;
//
//    void snowball_received();
//
//    string get_name()  const override { return "MogeraChild"; }
//    int get_min_score() const override { return 100; }
//    int  get_max_score() const override { return 100; }
//
//private:
//    static constexpr float MOVE_SPEED = 120.0f;
//    static constexpr float FRAME_DURATION = 0.15f;
//    static constexpr int FRAME_COUNT    = 2;
//
//    float  m_direction;
//    sf::Sprite m_sprite;
//    float m_anim_timer;
//    int  m_current_frame;
//
//    void advance_animation();
//    void recalc_hit_box();
//};
//
//// ============================================================
////  ArtilleryRocket
////  Gamakichi boss ki trf se  rocket projectile.
////  explode() call hone ke baad expired ho jaata hai.
//// ============================================================
//class ArtilleryRocket : public Projectile
//{
//public:
//    ArtilleryRocket(const sf::Vector2f& position,const sf::Vector2f& velocity,const sf::Texture&  texture);
//
//    ~ArtilleryRocket() override = default;
//
//    void update(float deltaTime) override;
//    void draw(sf::RenderWindow& window) override;
//    bool is_expired() const override;
//    void explode();
//    sf::FloatRect get_blast_zone() const;
//
//private:
//    static constexpr float BLAST_RADIUS  = 48.0f;
//    static constexpr float FRAME_DURATION = 0.12f;
//    static constexpr int   FRAME_COUNT   = 3;
//    static constexpr float ROCKET_W      = 24.0f;
//    static constexpr float ROCKET_H      = 24.0f;
//
//    sf::Vector2f m_velocity;
//    sf::Sprite  m_sprite;
//    bool m_exploded;
//    float  m_anim_timer;
//    int  m_current_frame;
//
//    void advance_animation();
//    void recalc_hit_box();
//};
//
//// ============================================================
////  BossEnemy  (abstract base)
////  Dono boss classes (Mogera aur Gamakichi) is se inherit karte hain.
////  Health bar aur takeDamage ka logic yahan se aata hai.
//// ============================================================
//class BossEnemy : public Foe
//{
//public:
//    virtual int get_phase()      const = 0;
//    virtual int get_health()     const = 0;
//    virtual int get_max_health() const = 0;
//    virtual int get_gem_drop()   const override = 0;
//    virtual int get_score_reward() const = 0;
//
//    void snowball_received();
//
//    string get_name()  const override { return "BossEnemy"; }
//    int get_min_score() const override { return 0; }
//    int get_max_score() const override { return 0; }
//
//    virtual ~BossEnemy() = default;
//
//protected:
//    sf::RectangleShape m_health_bar_bg;
//    sf::RectangleShape m_health_bar_fg;
//
//    virtual void update_health_bar() = 0;
//    virtual bool take_damage() { return false; }
//};
//
///
//// ============================================================
//class Mogera : public BossEnemy
//{
//public:
//    static constexpr int MAX_CHILDREN = 16;
//
//    Mogera(const sf::Vector2f& position,const sf::Texture&  texture, const sf::Texture&  child_tex);
//    ~Mogera() override;
//    void update(float deltaTime) override;
//    void draw(sf::RenderWindow& window) override;
//
//    string get_name() const override { return "Mogera"; }
//    int get_min_score() const override { return 3000; }
//    int get_max_score() const override { return 5000; }
//    int get_phase() const override;
//    int  get_health() const override;
//    int get_max_health()  const override;
//    int  get_gem_drop()  const override;
//    int get_score_reward() const override;
//
//    MogeraChild* const* get_children()  const;
//    int  get_child_array_size() const;
//
//    void prune_dead_children();
//
//private:
//    static constexpr int   MAX_HEALTH          = 20;
//    static constexpr int   GEM_REWARD          = 200;
//    static constexpr int   SCORE_REWARD        = 5000;
//    static constexpr float SPAWN_INTERVAL_P1   = 3.0f;
//    static constexpr float SPAWN_INTERVAL_P2   = 1.8f;
//    static constexpr int   PHASE2_HEALTH_THRESH = 10;
//    static constexpr int   FRAME_COUNT         = 4;
//    static constexpr float FRAME_DURATION      = 0.18f;
//    static constexpr float HITBOX_MARGIN       = 8.0f;
//    static constexpr float SPRITE_W            = 96.0f;
//    static constexpr float SPRITE_H            = 96.0f;
//
//    int   m_health;
//    int   m_phase;
//    float m_spawn_timer;
//    float m_anim_timer;
//    int   m_current_frame;
//
//    MogeraChild* m_children[MAX_CHILDREN];
//    const sf::Texture& m_child_tex;
//    sf::Sprite m_sprite;
//    sf::FloatRect  m_hit_box;
//
//    void spawn_children();
//    void advance_animation();
//    void update_health_bar() override;
//    bool take_damage()  override;
//    void enter_phase2();
//    void recalc_hit_box();
//};
//
//class Gamakichi : public BossEnemy
//{
//public:
//    static constexpr int MAX_ROCKETS  = 24;
//    static constexpr int MAX_CHILDREN = 12;
//
//    Gamakichi(const sf::Vector2f& position,const sf::Texture&  body_texture, const sf::Texture&  rocket_texture,
//              const sf::Texture&  child_texture);
//    ~Gamakichi() override;
//
//    void update(float deltaTime) override;
//    void draw(sf::RenderWindow& window) override;
//
//    string get_name()  const override { return "Gamakichi"; }
//    int  get_min_score()  const override { return 6000; }
//    int get_max_score()  const override { return 10000; }
//    int get_phase() const override;
//    int get_health() const override;
//    int get_max_health()  const override;
//    int get_gem_drop() const override;
//    int get_score_reward() const override;
//
//    ArtilleryRocket* const* get_rockets()  const;
//    int  get_rocket_array_size() const;
//    MogeraChild* const* get_children() const;
//    int get_child_array_size()  const;
//
//    
//    void prune_dead_projectiles();
//    
//    void prune_dead_children();
//
//private:
//    static constexpr int   MAX_HEALTH           = 40;
//    static constexpr int   GEM_REWARD           = 500;
//    static constexpr int   SCORE_REWARD         = 10000;
//    static constexpr int   PHASE2_THRESH        = 27;
//    static constexpr int   PHASE3_THRESH        = 13;
//    static constexpr int   ROCKETS_P1           = 2;
//    static constexpr int   ROCKETS_P2           = 4;
//    static constexpr int   ROCKETS_P3           = 6;
//    static constexpr float ATTACK_INTERVAL_P1   = 3.5f;
//    static constexpr float ATTACK_INTERVAL_P2   = 2.2f;
//    static constexpr float ATTACK_INTERVAL_P3   = 1.4f;
//    static constexpr float CHILD_SPAWN_INTERVAL = 4.0f;
//    static constexpr int   FRAME_COUNT          = 4;
//    static constexpr float FRAME_DURATION       = 0.20f;
//    static constexpr float SHAKE_DURATION       = 0.25f;
//    static constexpr float SHAKE_AMPLITUDE      = 6.0f;
//    static constexpr float HITBOX_MARGIN        = 12.0f;
//    static constexpr float SPRITE_W             = 128.0f;
//    static constexpr float SPRITE_H             = 128.0f;
//
//    int   m_health;
//    int   m_phase;
//    float m_attack_timer;
//    float m_child_spawn_timer;
//    float m_anim_timer;
//    int   m_current_frame;
//    bool  m_shaking;
//    float m_shake_timer;
//    float m_shake_offset;
//
//    ArtilleryRocket*   m_rockets[MAX_ROCKETS];
//    MogeraChild*       m_children[MAX_CHILDREN];
//    sf::Sprite         m_sprite;
//    sf::FloatRect      m_hit_box;
//
//    const sf::Texture& m_rocket_texture;
//    const sf::Texture& m_child_texture;
//
//    void fire_rocket_burst();
//    void spawn_children();
//    void advance_animation();
//    void update_health_bar() override;
//    bool take_damage()       override;
//    void enter_phase2();
//    void enter_phase3();
//    void update_shake(float dt);
//    void recalc_hit_box();
//};
