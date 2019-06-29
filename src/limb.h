#pragma once
#ifndef LIMB_H
#define LIMB_H

#include <list>
#include <vector>

#include "enum_bitset.h"
#include "optional.h"
#include "type_id.h"

class Character;
class JsonObject;
class nc_color;

enum limb_category {
    HEAD,
    TORSO,
    ARM,
    LEG,
    HAND,
    FOOT,
    EYE,
    MOUTH,
    TAIL
};

// the limb data that is read from json
class limb_type {
public:
    // all the limbs attached to this limb.
    std::list<limb_id> attached_limbs;
    // the limb this limb is attached to
    cata::optional<limb_id> parent_limb;
    // if on, damages the parent_limb instead
    // must be false if no parent limb exists
    bool redirect_damage = false;

    limb_id id;
    // untranslated english name
    std::string name;
    // what type of limb is this?
    limb_category category;

    void load_limb( JsonObject &jo, const std::string &src );
};

class limb {
private:
    const limb_type *type;
    // how much damage this limb has accrued.
    // is opposite of hp because in order to get the hp of the limb,
    // we need to interface with the owner of the limb.
    int dmg = 0;

    int cur_health( const Character &owner ) const;
    int max_health( const Character &owner ) const;
    nc_color health_color( const Character &owner ) const;
public:
    // cause damage to this limb.
    // eventually to be used with the wound system, 
    // which is why it has its own function encapsulated
    void damage( const Character &owner, int amount );
    void heal( const Character &owner, int amount );
    // returns a colorized string
    std::string health_bar( const Character &owner ) const;
};

#endif
