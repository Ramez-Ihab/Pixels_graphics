// Owner: Person 1 (gameplay and combat)
#include "Combat.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {
constexpr float CONTACT_DISTANCE = 20.f;
constexpr float WALK_SPEED = 100.f;
constexpr float HARD_ENEMY_SPEED = 150.f;
constexpr float SHOT_SPEED = 320.f;
constexpr float ATTACK_COOLDOWN = 1.f;
constexpr float ATTACK_ANIMATION_TIME = 0.25f;
constexpr float POSE_DURATION = 0.5f;
constexpr float PICKUP_DISTANCE = 12.f;

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

void moveUnits(GameState& state, float dt, std::vector<float>& startX) {
    auto& units = state.units;
    std::vector<float> planned(units.size(), 0.f);

    for (std::size_t i = 0; i < units.size(); ++i) {
        Unit& unit = units[i];
        startX[i] = unit.x;
        if (unit.hp <= 0 || unit.type == UnitType::Defense) {
            continue;
        }

        const float speed = unit.type == UnitType::EnemyMelee ? HARD_ENEMY_SPEED : WALK_SPEED;
        planned[i] = unit.isPlayer
            ? std::min(speed * dt, std::max(0.f, GRID_X1 - unit.x))
            : speed * dt;
    }

    // Check encounters against the same starting positions for both sides.
    // Each pair shares the available gap in proportion to its intended movement.
    std::vector<float> movement = planned;
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

            const float room = std::max(0.f, enemy.x - player.x - CONTACT_DISTANCE);
            const float approach = planned[i] + planned[j];
            if (approach > room) {
                const float fraction = room / approach;
                movement[i] = std::min(movement[i], planned[i] * fraction);
                movement[j] = std::min(movement[j], planned[j] * fraction);
            }
        }
    }

    for (std::size_t i = 0; i < units.size(); ++i) {
        Unit& unit = units[i];
        unit.x += unit.isPlayer ? movement[i] : -movement[i];
        unit.poseTimer = std::max(0.f, unit.poseTimer - dt);
        if (unit.poseTimer == 0.f) {
            unit.pose = Pose::Normal;
        }

        unit.animTime += dt;
        if (unit.anim != Anim::Attack || unit.animTime >= ATTACK_ANIMATION_TIME) {
            const Anim next = movement[i] > 0.f ? Anim::Walk : Anim::Idle;
            if (unit.anim != next) {
                unit.anim = next;
                unit.animTime = 0.f;
            }
        }
    }
}

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
            std::size_t target = units.size();
            // Allow a tiny rounding margin when both units moved to contact.
            float closest = CONTACT_DISTANCE + 0.001f;
            for (std::size_t i = 0; i < units.size(); ++i) {
                const Unit& candidate = units[i];
                if (candidate.hp <= 0 || candidate.isPlayer == attacker.isPlayer ||
                    candidate.lane != attacker.lane) {
                    continue;
                }

                const float distance = attacker.isPlayer
                    ? candidate.x - attacker.x : attacker.x - candidate.x;
                if (distance >= 0.f && distance < closest) {
                    closest = distance;
                    target = i;
                }
            }

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

void moveProjectiles(GameState& state, float dt, const std::vector<float>& startX,
                     std::vector<int>& damage) {
    auto& shots = state.projectiles;
    const float distance = SHOT_SPEED * dt;
    shots.erase(std::remove_if(shots.begin(), shots.end(), [&](const Projectile& shot) {
        float firstImpact = 1.f;
        std::size_t target = state.units.size();

        // Compare the shot's path with each unit's movement during this frame.
        for (std::size_t i = 0; i < state.units.size(); ++i) {
            const Unit& unit = state.units[i];
            if (unit.hp <= 0 || unit.lane != shot.lane || unit.isPlayer == shot.fromPlayer) {
                continue;
            }

            // A Defense player always blocks. Other players can dodge enemy shots.
            if (unit.isPlayer && unit.type != UnitType::Defense &&
                unit.pose == (shot.high ? Pose::Duck : Pose::Jump)) {
                continue;
            }

            const float startGap = (startX[i] - shot.x) * shot.dir;
            const float endGap = (unit.x - shot.x) * shot.dir - distance;
            if (startGap < 0.f || endGap > 0.f) {
                continue;
            }

            const float impact = startGap == 0.f ? 0.f : startGap / (startGap - endGap);
            const bool shieldsAtSamePosition = impact == firstImpact && target != state.units.size() &&
                unit.type == UnitType::Defense && state.units[target].type != UnitType::Defense;
            if (target == state.units.size() || impact < firstImpact || shieldsAtSamePosition) {
                firstImpact = impact;
                target = i;
            }
        }

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

void collectDrops(GameState& state, const std::vector<float>& startX) {
    auto& drops = state.collectibles;
    drops.erase(std::remove_if(drops.begin(), drops.end(), [&](const Collectible& drop) {
        for (std::size_t i = 0; i < state.units.size(); ++i) {
            const Unit& unit = state.units[i];
            if (unit.isPlayer && unit.hp > 0 && unit.lane == drop.lane &&
                drop.x >= startX[i] - PICKUP_DISTANCE && drop.x <= unit.x + PICKUP_DISTANCE) {
                ++state.coins;
                return true;
            }
        }
        return false;
    }), drops.end());
}
}  // namespace

void combatReset(GameState& state) {
    // Reset the battlefield. Flow controls the screen and progression.
    state.units.clear();
    state.projectiles.clear();
    state.collectibles.clear();
}

void combatUpdate(GameState& state, float dt) {
    if (!std::isfinite(dt) || dt < 0.f || enemyReachedBase(state)) {
        return;
    }

    // Save the original positions so pickup checks cover the whole movement step.
    std::vector<float> startX(state.units.size());
    moveUnits(state, dt, startX);
    if (enemyReachedBase(state)) {
        return;
    }

    // Queue damage before applying it so ready units can trade hits.
    std::vector<int> damage(state.units.size(), 0);
    moveProjectiles(state, dt, startX, damage);
    queueAttacks(state, dt, damage);

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

    // The indices in startX are valid until the defeated units are erased.
    collectDrops(state, startX);
    auto& units = state.units;
    units.erase(std::remove_if(units.begin(), units.end(),
                              [](const Unit& unit) { return unit.hp <= 0; }), units.end());
}

void spawnPlayer(GameState& state, UnitType type, int lane) {
    // Only player types can use the left-side spawn.
    if (type == UnitType::Melee || type == UnitType::Ranged || type == UnitType::Defense) {
        addUnit(state, type, lane, true, type == UnitType::Defense ? 7 : 5);
    }
}

void spawnEnemy(GameState& state, UnitType type, int lane) {
    // Flow chooses the lane. Combat initializes the enemy at the right edge.
    if (type == UnitType::EnemyUpper || type == UnitType::EnemyLower ||
        type == UnitType::EnemyMelee) {
        addUnit(state, type, lane, false, type == UnitType::EnemyMelee ? 7 : 5);
    }
}

void setLanePose(GameState& state, int lane, Pose pose) {
    // Flow calls this for each selected lane. Defense players stay in place.
    for (Unit& unit : state.units) {
        if (unit.isPlayer && unit.hp > 0 && unit.lane == lane && unit.type != UnitType::Defense) {
            unit.pose = pose;
            unit.poseTimer = pose == Pose::Normal ? 0.f : POSE_DURATION;
        }
    }
}

bool enemyReachedBase(const GameState& state) {
    // The base begins strictly to the left of the playfield.
    return std::any_of(state.units.begin(), state.units.end(), [](const Unit& unit) {
        return !unit.isPlayer && unit.hp > 0 && unit.x < GRID_X0;
    });
}