#pragma once

#include "common.hpp"
#include "packet.hpp"
#include "log.hpp"

#include <random>
#include <entt/entt.hpp>

using Entity = entt::entity;
using EntityId = entt::id_type;

struct EntityRegistry {
    template<typename ...TemplateArgs, typename ...Args>
    decltype(auto) Create(Args &&...args) {
        return this->impl.create(std::forward<Args>(args)...);
    }

    template<typename ...TemplateArgs, typename ...Args>
    decltype(auto) Add(Args &&...args) {
        return this->impl.emplace<TemplateArgs...>(std::forward<Args>(args)...);
    }

    template<typename ...TemplateArgs, typename ...Args>
    decltype(auto) Remove(Args &&...args) {
        return this->impl.remove<TemplateArgs...>(std::forward<Args>(args)...);
    }

    template<typename ...Args>
    decltype(auto) Destroy(Args &&...args) {
        return this->impl.destroy(std::forward<Args>(args)...);
    }

    template<typename ...Args>
    decltype(auto) IsValid(Args &&...args) {
        return this->impl.valid(std::forward<Args>(args)...);
    }

    template<typename ...TemplateArgs, typename ...Args>
    decltype(auto) View(Args &&...args) {
        return this->impl.view<TemplateArgs...>(std::forward<Args>(args)...);
    }

    template<typename ...TemplateArgs, typename ...Args>
    decltype(auto) Each(Args &&...args) const {
        return this->impl.each<TemplateArgs...>(std::forward<Args>(args)...);
    }

    template<typename ...TemplateArgs, typename ...Args>
    decltype(auto) Get(Args &&...args) {
        return this->impl.get<TemplateArgs...>(std::forward<Args>(args)...);
    }

    template<typename ...TemplateArgs, typename ...Args>
    decltype(auto) TryGet(Args &&...args) {
        return this->impl.try_get<TemplateArgs...>(std::forward<Args>(args)...);
    }

    template<typename ...TemplateArgs, typename ...Args>
    decltype(auto) Get(Args &&...args) const {
        return this->impl.get<TemplateArgs...>(std::forward<Args>(args)...);
    }

    template<typename ...TemplateArgs, typename ...Args>
    decltype(auto) TryGet(Args &&...args) const {
        return this->impl.try_get<TemplateArgs...>(std::forward<Args>(args)...);
    }

    entt::registry impl;
};

struct CPosition {
    Vec2 value;
};

struct CVelocity {
    Vec2 value;
};

struct CMass {
    f32 value;
};

struct CHealth {
    f32 value;
    f32 max;
};

struct CPlanet {
    Vec2 initial_position;
    f32 radius;
    f32 orbital_velocity = 0.001f;
};

struct Weapon {
    enum class Type {
        SHOTGUN,
        MACHINEGUN,
        MISSILE,
        MORTAR,
        COUNT,
    };

    constexpr static f32 MAX_CHARGE = 60.0f;

    f32 projectile_mass;
    f32 cooldown;
    f32 damage;
    f32 speed;
    f32 projectile_ttl;
    f32 spread;
    f32 speed_spread;
    u32 burst;
    StringView name;
};

constexpr Weapon g_weapons[static_cast<size_t>(Weapon::Type::COUNT)] {
    /* WEAPON TABLE
     |mass    |cooldown |damage  |speed  |ttl     |spread |speed_spread |burst |name */
    {7.0f,    60.0f,    3.5f,    25.0f,  100.0f,  3.0f,   1.0f,         10,    "Shotgun", },
    {5.0f,    4.5f,     6.0f,    50.0f,  60.0f,   8.0f,   2.0f,         1,     "Machinegun", },
    {10.0f,   150.0f,   40.0f,   17.0f,  300.0f,  0.0f,   0.0f,         1,     "Missile launcher", },
    {12.0f,   270.0f,   50.0f,   3.0f,   400.0f,  0.0f,   0.0f,         1,     "Mortar", },
};

struct CTank {
    constexpr static f32 BASE_HEIGHT = 30.0f;
    constexpr static f32 TURRET_HEIGHT = 40.0f;
    constexpr static f32 MAX_FUEL = 1000.0f;
    constexpr static u32 ROTATE_TURRET_LEFT  = 1 << 0;
    constexpr static u32 ROTATE_TURRET_RIGHT = 1 << 1;
    f32 turret_rotation;
    f32 target_turret_rotation;
    u32 flags = 0;
    Entity planet_id;
    f32 fuel = MAX_FUEL;
    Weapon::Type weapon_type = Weapon::Type::MORTAR;
    f32 last_fire_time = 0.0f;
};

struct CPlanetPosition {
    f32 delta;
    f32 value;
};

struct CCharging {
    f32 start_time = 0.0f;
};

struct CProjectile {
    Entity firing_entity;
    Color color; // TODO only for debugging purposes
    f32 impact_damage = 0.0f;
    f32 hit_radius = 40.0f;
    f32 radius = 7.0f;
};

struct CTimeToLiveBeforeExplosion {
    f32 value;
};

struct CNetReplication {
    f32 last_replication;
};

enum class EntityPrefabId {
    PLANET,
    TANK,
    PROJECTILE,
};

inline Entity CreateEntity(EntityRegistry &registry, EntityPrefabId id, Optional<Entity> hint = std::nullopt) {
    Entity entity;

    if (hint.has_value()) {
        if (registry.IsValid(hint.value())) {
            LogInfo("entities", "Destroy {}"_format(entt::to_integral(hint.value())));
            registry.Destroy(hint.value());
        }

        entity = registry.Create(hint.value());
        assert(entity == hint.value());
    } else {
        entity = registry.Create();
    }

    switch (id) {
        case EntityPrefabId::PLANET: {
            registry.Add<CPlanet>(entity);
            registry.Add<CPosition>(entity);
            registry.Add<CMass>(entity);
            registry.Add<CNetReplication>(entity);
        } break;

        case EntityPrefabId::TANK: {
            registry.Add<CTank>(entity);
            registry.Add<CPlanetPosition>(entity);
            registry.Add<CHealth>(entity);
            registry.Add<CNetReplication>(entity);
        } break;

        case EntityPrefabId::PROJECTILE: {
            registry.Add<CPosition>(entity);
            registry.Add<CVelocity>(entity);
            registry.Add<CMass>(entity);
            registry.Add<CProjectile>(entity);
            registry.Add<CTimeToLiveBeforeExplosion>(entity);
            registry.Add<CNetReplication>(entity);
        } break;

        default:
            UNREACHED;
    }

    return entity;
}

inline void SerializeEntities(const EntityRegistry &registry, Packet &packet) {
    auto archive = [&](const auto &...data) {
        (packet.WriteData(&data, sizeof(data)), ...);
    };

    entt::snapshot snapshot{registry.impl};
    snapshot.entities(archive);
    snapshot.component<
        CPosition,
        CVelocity,
        CMass,
        CHealth,
        CPlanet,
        CTank,
        CPlanetPosition,
        CCharging,
        CProjectile
        >(archive);
}

inline bool DeserializeEntities(EntityRegistry &registry, Packet &packet) {
    auto archive = [&](auto &...data) {
        (packet.ReadData(&data, sizeof(data)), ...);
    };

    entt::snapshot_loader loader{registry.impl};
    loader.entities(archive);
    loader.component<
        CPosition,
        CVelocity,
        CMass,
        CHealth,
        CPlanet,
        CTank,
        CPlanetPosition,
        CCharging,
        CProjectile
        >(archive);
    loader.orphans();

    return packet.IsValidAndFinished();
}

template<typename Type>
void CloneComponents(const EntityRegistry &from, EntityRegistry &to) {
    auto data = from.impl.data<Type>();
    auto size = from.impl.size<Type>();

    if constexpr (ENTT_IS_EMPTY(Type)) {
        to.impl.insert<Type>(data, data + size);
    } else {
        auto raw = from.impl.raw<Type>();
        to.impl.insert<Type>(data, data + size, raw, raw + size);
    }
}

using clone_fn_type = void(const EntityRegistry &, EntityRegistry &);
inline const std::unordered_map<EntityId, clone_fn_type *> g_clone_functions = {
    std::make_pair(entt::type_info<CPosition>::id(), &CloneComponents<CPosition>),
    std::make_pair(entt::type_info<CVelocity>::id(), &CloneComponents<CVelocity>),
    std::make_pair(entt::type_info<CMass>::id(), &CloneComponents<CMass>),
    std::make_pair(entt::type_info<CHealth>::id(), &CloneComponents<CHealth>),
    std::make_pair(entt::type_info<CPlanet>::id(), &CloneComponents<CPlanet>),
    std::make_pair(entt::type_info<CTank>::id(), &CloneComponents<CTank>),
    std::make_pair(entt::type_info<CPlanetPosition>::id(), &CloneComponents<CPlanetPosition>),
    std::make_pair(entt::type_info<CCharging>::id(), &CloneComponents<CCharging>),
    std::make_pair(entt::type_info<CProjectile>::id(), &CloneComponents<CProjectile>),
    std::make_pair(entt::type_info<CTimeToLiveBeforeExplosion>::id(), &CloneComponents<CTimeToLiveBeforeExplosion>),
    std::make_pair(entt::type_info<CNetReplication>::id(), &CloneComponents<CNetReplication>),
};

inline void CloneRegistry(const EntityRegistry &from, EntityRegistry &to) {
    from.impl.visit([&](auto type_id) {
        g_clone_functions.at(type_id)(from, to);
    });
}
