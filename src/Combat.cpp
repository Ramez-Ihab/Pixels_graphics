// Owner: Person 1 (gameplay and combat)
#include "Combat.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {
constexpr float CONTACT_DISTANCE = 20.f;
constexpr float WALK_SPEED = 100.f;
constexpr float HARD_ENEMY_SPEED = 120.f;
constexpr float SHOT_SPEED = 320.f;
constexpr float ATTACK_COOLDOWN = 1.f;
constexpr float ATTACK_ANIMATION_TIME = 0.25f;
constexpr float POSE_DURATION = 1.0f;

// Place a unit at its side of the field.
void addUnit(GameState& state, UnitType type, int lane, bool isPlayer, int hp) {
    if (lane < 0 || lane >= LANES) {
        return;
    }

    Unit unit{};
    unit.type = type;
    unit.isPlayer = isPlayer;
    unit.lane = lane;
    unit.x = isPlayer ? GRID_X0 : GRID_X1;
    unit.hp = unit.maxHp = hp;
    state.units.push_back(unit);
}

// Tells me the number of steps for each unit to move.
std::vector<float> planSteps(const std::vector<Unit>& units, float dt,
                                 std::vector<float>& startingPositions) {
    std::vector<float> wantedSteps(units.size(), 0.f); //Gets me the needed steps until meeting the other unit/or reaching the base
    for (std::size_t i = 0; i < units.size(); ++i) {
        const Unit& unit = units[i];
        startingPositions[i] = unit.x;
        if (unit.hp <= 0 || unit.type == UnitType::Defense) {
            continue;
        }

        const float speed = unit.type == UnitType::EnemyMelee ? HARD_ENEMY_SPEED : WALK_SPEED; //This is an if-statement to choose the speed of the unit.
        wantedSteps[i] = speed * dt; //Updates the steps needed per frame/second.
        if (unit.isPlayer) {
            const float roomToEdge = std::max(0.f, GRID_X1 - unit.x);
            wantedSteps[i] = std::min(wantedSteps[i], roomToEdge);
        }
    }

    return wantedSteps;
}

// Takes the steps and gets the tightest distance between enemy & player. RETUNS allowed steps!
std::vector<float> stopUnitsAtContact(const std::vector<Unit>& units,
                                      const std::vector<float>& wantedSteps) {
    std::vector<float> allowedSteps = wantedSteps;
    for (std::size_t i = 0; i < units.size(); ++i) {
        const Unit& player = units[i];
        if (!player.isPlayer || player.hp <= 0) {
            continue;
        }

        for (std::size_t j = 0; j < units.size(); ++j) {
            const Unit& enemy = units[j];
            if (enemy.isPlayer || enemy.hp <= 0 || player.lane != enemy.lane ||
                enemy.x < player.x) {
                continue;
            }

            const float gapBeforeContact = std::max(0.f, enemy.x - player.x - CONTACT_DISTANCE);
            const float combinedStep = wantedSteps[i] + wantedSteps[j];
            if (combinedStep > gapBeforeContact) {
                const float allowedFraction = gapBeforeContact / combinedStep;
                allowedSteps[i] = std::min(allowedSteps[i], wantedSteps[i] * allowedFraction);
                allowedSteps[j] = std::min(allowedSteps[j], wantedSteps[j] * allowedFraction);
            }
        }
    }

    return allowedSteps;
}

// Move units, expire poses, and update animations.
void moveAndAnimateUnits(std::vector<Unit>& units, float dt, const std::vector<float>& allowedSteps) {
    for (std::size_t i = 0; i < units.size(); ++i) {
        Unit& unit = units[i];
        unit.x += unit.isPlayer ? allowedSteps[i] : -allowedSteps[i];
        unit.poseTimer = std::max(0.f, unit.poseTimer - dt);
        if (unit.poseTimer == 0.f) {
            unit.pose = Pose::Normal;
        }

        unit.animTime += dt;
        if (unit.anim != Anim::Attack || unit.animTime >= ATTACK_ANIMATION_TIME) {
            const Anim nextAnimation = allowedSteps[i] > 0.f ? Anim::Walk : Anim::Idle;
            if (unit.anim != nextAnimation) {
                unit.anim = nextAnimation;
                unit.animTime = 0.f;
            }
        }
    }
}

void moveUnits(GameState& state, float dt, std::vector<float>& startingPositions) {
    const auto wantedSteps = planSteps(state.units, dt, startingPositions);
    const auto allowedSteps = stopUnitsAtContact(state.units, wantedSteps);
    moveAndAnimateUnits(state.units, dt, allowedSteps);
}

// Find the closest opponent in front of the attacker.
std::size_t findMeleeTarget(const std::vector<Unit>& units, const Unit& attacker) {
    std::size_t target = units.size();
    // Allow a tiny rounding margin when both units moved to contact.
    float closestDistance = CONTACT_DISTANCE + 0.001f;
    for (std::size_t i = 0; i < units.size(); ++i) {
        const Unit& candidate = units[i];
        if (candidate.hp <= 0 || candidate.isPlayer == attacker.isPlayer ||
            candidate.lane != attacker.lane) {
            continue;
        }

        const float distance = attacker.isPlayer
            ? candidate.x - attacker.x : attacker.x - candidate.x;
        if (distance >= 0.f && distance < closestDistance) {
            closestDistance = distance;
            target = i;
        }
    }

    return target;
}

// Resolve ready melee swings and spawn ranged shots.
void queueAttacks(GameState& state, float dt, std::vector<int>& damage) {
    auto& units = state.units;
    for (Unit& attacker : units) {
        if (attacker.hp <= 0 || attacker.type == UnitType::Defense) {
            continue;
        }

        attacker.attackTimer = std::max(0.f, attacker.attackTimer - dt);
        if (attacker.attackTimer > 0.f) {
            continue;
        }

        if (attacker.type == UnitType::Melee || attacker.type == UnitType::EnemyMelee) {
            const std::size_t target = findMeleeTarget(units, attacker);
            if (target == units.size()) {
                continue;
            }
            damage[target] += attacker.type == UnitType::EnemyMelee ? 2 : 1;
        } else {
            // Ranged units fire down the lane even when it is empty.
            state.projectiles.push_back({
                attacker.lane, attacker.x, attacker.isPlayer ? 1.f : -1.f,
                attacker.type == UnitType::EnemyUpper, attacker.isPlayer
            });
        }

        attacker.attackTimer = ATTACK_COOLDOWN;
        attacker.anim = Anim::Attack;
        attacker.animTime = 0.f;
    }
}

// Find the first unit hit along the shot's path.
std::size_t findProjectileTarget(const std::vector<Unit>& units, const Projectile& shot,
                                 float distance, const std::vector<float>& startingPositions) {
    float firstImpact = 1.f;
    std::size_t target = units.size();

    // Compare the shot's path with each unit's movement during this frame.
    for (std::size_t i = 0; i < units.size(); ++i) {
        const Unit& unit = units[i];
        if (unit.hp <= 0 || unit.lane != shot.lane || unit.isPlayer == shot.fromPlayer) {
            continue;
        }

        // Duck under high shots and jump over low shots.
        if (unit.isPlayer && unit.type != UnitType::Defense &&
            unit.pose == (shot.high ? Pose::Duck : Pose::Jump)) {
            continue;
        }

        const float startGap = (startingPositions[i] - shot.x) * shot.dir;
        const float endGap = (unit.x - shot.x) * shot.dir - distance;
        if (startGap < 0.f || endGap > 0.f) {
            continue;
        }

        const float impact = startGap == 0.f ? 0.f : startGap / (startGap - endGap);
        // Defense takes the hit when two units are reached together.
        const bool defenseWinsTie = impact == firstImpact && target != units.size() &&
            unit.type == UnitType::Defense && units[target].type != UnitType::Defense;
        if (target == units.size() || impact < firstImpact || defenseWinsTie) {
            firstImpact = impact;
            target = i;
        }
    }

    return target;
}

// Remove shots that hit or leave the field, then move the rest.
void moveProjectiles(GameState& state, float dt, const std::vector<float>& startingPositions,
                     std::vector<int>& damage) {
    auto& shots = state.projectiles;
    const float distance = SHOT_SPEED * dt;
    shots.erase(std::remove_if(shots.begin(), shots.end(), [&](const Projectile& shot) {
        const std::size_t target = findProjectileTarget(state.units, shot, distance, startingPositions);
        if (target != state.units.size()) {
            ++damage[target];
            return true;
        }

        const float nextX = shot.x + distance * shot.dir;
        return nextX < GRID_X0 || nextX > GRID_X1;
    }), shots.end());
    for (Projectile& shot : shots) {
        shot.x += distance * shot.dir;
    }
}

// Apply all hits and drop coins for defeated enemies.
void applyDamage(GameState& state, const std::vector<int>& damage) {
    for (std::size_t i = 0; i < state.units.size(); ++i) {
        Unit& unit = state.units[i];
        if (unit.hp <= 0) {
            continue;
        }
        unit.hp = std::max(0, unit.hp - damage[i]);
        if (!unit.isPlayer && unit.hp == 0) {
            state.collectibles.push_back({unit.lane, unit.x});
        }
    }

    // Remove fallen units after dropping coins.
    auto& units = state.units;
    units.erase(std::remove_if(units.begin(), units.end(),
                              [](const Unit& unit) { return unit.hp <= 0; }), units.end());
}

} 

void combatReset(GameState& state) {
    // Reset the battlefield.
    state.units.clear();
    state.projectiles.clear();
    state.collectibles.clear();
}

void combatUpdate(GameState& state, float dt) {
    // Update the game state.
    if (!std::isfinite(dt) || dt < 0.f || enemyReachedBase(state)) {
        return;
    }

    // Save starting positions for projectile hit checks.
    std::vector<float> startingPositions(state.units.size());
    moveUnits(state, dt, startingPositions);
    if (enemyReachedBase(state)) {
        return;
    }

    // Queue damage before applying it so ready units can trade hits.
    std::vector<int> damage(state.units.size(), 0);
    moveProjectiles(state, dt, startingPositions, damage);
    queueAttacks(state, dt, damage);

    applyDamage(state, damage);
}

void spawnPlayer(GameState& state, UnitType type, int lane) {
    // Only player types can use the left-side spawn.
    if (type == UnitType::Melee || type == UnitType::Ranged || type == UnitType::Defense) {
        addUnit(state, type, lane, true, type == UnitType::Defense ? 7 : 5);
    }
}

void spawnEnemy(GameState& state, UnitType type, int lane) {
    // Chooses the lane.
    if (type == UnitType::EnemyUpper || type == UnitType::EnemyLower ||
        type == UnitType::EnemyMelee) {
        addUnit(state, type, lane, false, type == UnitType::EnemyMelee ? 7 : 5);
    }
}

void setLanePose(GameState& state, int lane, Pose pose) {
    // Sets the lane pose.
    for (Unit& unit : state.units) {
        if (unit.isPlayer && unit.hp > 0 && unit.lane == lane && unit.type != UnitType::Defense) {
            unit.pose = pose;
            unit.poseTimer = pose == Pose::Normal ? 0.f : POSE_DURATION;
        }
    }
}

bool enemyReachedBase(const GameState& state) {
    for (const Unit& unit : state.units) {
        if (!unit.isPlayer && unit.hp > 0 && unit.x < GRID_X0) {
            return true;
        }
    }
    return false;
}