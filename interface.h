
#pragma once
#pragma once
/**
  ============================================================================
  INTERFACE CONTRACT -- Snow Bros Project
  ============================================================================

 * This file defines the exact API surface shared between Student A and Student B.
 *  NEITHER student modifies the other's internal implementation.
 * They ONLY call the functions listed here.
 *  "No runtime integration errors" .
============================================================================
|                    STUDENT A PROVIDES                                    |
|               (Student B calls these functions)                          |
=============================================================================
GameLoop::getWindowSize()    ----> sf::Vector2u
GameLoop::getDeltaTime()     ---->float
GameLoop::getPlayerPosition(int playerIndex) --->sf::Vector2f
CollisionDetector::check(FloatRect a, FloatRect b) ---> bool
LevelManager::getCurrentLevel() ---> int
LevelManager::isBonus()         --->bool

============================================================================
|                    STUDENT B PROVIDES                                    |
|               (Student Acalls these functions)                          |
=============================================================================


User_Auth::isLoggedIn()    --->bool
User_Auth::getLoggedInUser() --->  const UserAccount&
Db_manager::loadprogress(userId) --> Player_progress
Db_manager::saveprogress(progress)
Foe_Factory::createForLevel(type, pos, level) -->unique_ptr<Enemy>
EventBus::fire(event, data)
User_interf::set_activescrn(screen)
User_interf::draw_HUD(window, score, lives, gems, level, ...)
Score_tracker::track_CurrentScore() --> int

============================================================================
|                    SHARED EVENT BUS EVENTS                               |
|               (Student A fires; Student B listens)                       |
============================================================================
 ENEMY_HIT        data.intVal = damage done
 ENEMY_ENCASED    data.strVal = enemy type
 ENEMY_KILLED     data.intVal = score value, data.posVal = position
 CHAIN_KILL       data.intVal = chain count
 PLAYER_HIT       (no extra data needed)
 PLAYER_DIED      (no extra data needed)
 LEVEL_COMPLETE   data.intVal = level number
 BONUS_LEVEL_STARTED (no extra data)
 GEM_COLLECTED    data.intVal = gem amount
 GAME_OVER        data.intVal = final level reached

 *
 *  QUICK RULES (from spec, enforced here):
 *   - All collisions go through CollisionDetector — no ad-hoc overlap checks.
 *   - Events fire from Student A's engine; Student B's systems listen.
 *     Never the other way around for gameplay events.
 *   - Both students must understand the full codebase for the viva.
 *   - No hardcoded level counts anywhere.
 *   - Never store plain-text passwords.
 */

