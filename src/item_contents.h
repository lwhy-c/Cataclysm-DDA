#pragma once
#ifndef ITEM_CONTENTS_H
#define ITEM_CONTENTS_H

#include "item_pocket.h"
#include "optional.h"
#include "units.h"

class item;
class item_location;

class item_contents
{
    public:
        bool is_nestable() const {
            return nestable;
        }

        bool can_contain( const item &it ) const;

        // total size the parent item needs to be modified based on rigidity of pockets
        units::volume item_size_modifier() const;
        // total weight the parent item needs to be modified based on weight modifiers of pockets
        units::mass item_weight_modifier() const;

        // removes and returns the item from the pocket.
        cata::optional<item> remove_item( const item &it );
        cata::optional<item> remove_item( const item_location &it );

        // tries to put an item in a pocket. returns false on failure
        bool insert_item( const item &it );
        // finds or makes a fake pocket and puts this item into it
        void insert_legacy( const item &it );

        void load( const JsonObject &jo );
        void serialize( JsonOut &json ) const;
        void deserialize( JsonIn &jsin );

        bool was_loaded;
    private:
        // container can be placed into other containers
        bool nestable = true;

        std::list<item_pocket> contents;
};

#endif
