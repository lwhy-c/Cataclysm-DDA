#pragma once
#ifndef ITEM_POCKET_H
#define ITEM_POCKET_H

#include <list>

#include "enums.h"
#include "enum_traits.h"
#include "optional.h"
#include "type_id.h"
#include "units.h"

class Character;
class item;
class item_location;
class player;

struct itype;
struct tripoint;

using itype_id = std::string;

class item_pocket
{
    public:
        enum pocket_type {
            // this is to aid the transition from the previous way item contents were handled.
            // this will have the rules that previous contents would have
            LEGACY_CONTAINER,
            CONTAINER,
            MAGAZINE,
            LAST
        };

        item_pocket() = default;
        item_pocket( pocket_type ptype ) : type( ptype ) {}

        bool stacks_with( const item_pocket &rhs ) const;

        bool is_type( pocket_type ptype ) const;
        bool empty() const;

        std::list<item> all_items();
        std::list<item> all_items() const;
        std::list<item *> all_items_ptr( pocket_type pk_type );

        item &back();
        const item &back() const;
        item &front();
        const item &front() const;
        size_t size() const;
        void pop_back();

        bool can_contain( const item &it ) const;

        // combined volume of contained items
        units::volume contains_volume() const;
        units::volume remaining_volume() const;
        // combined weight of contained items
        units::mass contains_weight() const;

        units::volume item_size_modifier() const;
        units::mass item_weight_modifier() const;

        item *magazine_current();
        void casings_handle( const std::function<bool( item & )> &func );
        bool use_amount( const itype_id &it, int &quantity, std::list<item> &used );
        bool will_explode_in_a_fire() const;
        bool detonate( const tripoint &p, std::vector<item> &drops );
        bool process( const itype &type, player *carrier, const tripoint &pos, bool activate,
                      float insulation, const temperature_flag flag );
        bool legacy_unload( player *guy, bool &changed );
        void remove_all_ammo( Character &guy );
        void remove_all_mods( Character &guy );

        // removes and returns the item from the pocket.
        cata::optional<item> remove_item( const item &it );
        cata::optional<item> remove_item( const item_location &it );
        bool spill_contents( const tripoint &pos );
        void clear_items();
        bool has_item( const item &it ) const;
        item *get_item_with( const std::function<bool( const item & )> &filter );
        void remove_items_if( const std::function<bool( item & )> &filter );
        void has_rotten_away( const tripoint &pnt );

        // tries to put an item in the pocket. returns false if failure
        bool insert_item( const item &it );
        void add( const item &it );

        // only available to help with migration from previous usage of std::list<item>
        std::list<item> &edit_contents();

        // cost of getting an item from this pocket
        // @TODO: make move cost vary based on other contained items
        int obtain_cost( const item &it ) const;

        // this is used for the visitable interface. returns true if no further visiting is required
        bool remove_internal( const std::function<bool( item & )> &filter,
                              int &count, std::list<item> &res );

        void load( const JsonObject &jo );
        void serialize( JsonOut &json ) const;
        void deserialize( JsonIn &jsin );

        bool was_loaded;
    private:
        pocket_type type = CONTAINER;
        // max volume of stuff the pocket can hold
        units::volume max_contains_volume = 0_ml;
        // min volume of item that can be contained, otherwise it spills
        units::volume min_item_volume = 0_ml;
        // max weight of stuff the pocket can hold
        units::mass max_contains_weight = 0_gram;
        // multiplier for spoilage rate of contained items
        float spoil_multiplier = 1.0f;
        // items' weight in this pocket are modified by this number
        float weight_multiplier = 1.0f;
        // base time it takes to pull an item out of the pocket
        int moves = 100;
        // protects contents from exploding in a fire
        bool fire_protection = false;
        // can hold liquids
        bool watertight = false;
        // can hold gas
        bool gastight = false;
        // the pocket will spill its contents if placed in another container
        bool open_container = false;
        // allows only items that can be stored on a hook to be contained in this pocket
        bool hook = false;
        // container's size and encumbrance does not change based on contents.
        bool rigid = false;
        // the items inside the pocket
        std::list<item> contents;
};

template<>
struct enum_traits<item_pocket::pocket_type> {
    static constexpr auto last = item_pocket::pocket_type::LAST;
};

#endif
