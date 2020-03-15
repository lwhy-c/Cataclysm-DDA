#pragma once
#ifndef ITEM_CONTENTS_H
#define ITEM_CONTENTS_H

#include "enums.h"
#include "item_pocket.h"
#include "optional.h"
#include "ret_val.h"
#include "type_id.h"
#include "units.h"
#include "visitable.h"

#include <list>

class Character;
class item;

using itype_id = std::string;

class item;
class item_location;
class player;

struct iteminfo;
struct tripoint;

class item_contents
{
    public:
        item_contents() = default;
        // used for migration only
        item_contents( const std::list<item> &loaded );
        // used for loading itype
        item_contents( const std::vector<pocket_data> &pockets )
        {
            for( const pocket_data &data : pockets ) {
                contents.push_back( item_pocket( &data ) );
            }
        }

        /**
          * returns a pointer to the best pocket that can contain the item @it 
          * only checks CONTAINER pocket type
          */
        item_pocket *best_pocket( const item &it, bool nested );
        ret_val<bool> can_contain_rigid( const item &it ) const;
        ret_val<bool> can_contain( const item &it ) const;
        bool can_contain_liquid( bool held_or_ground ) const;
        bool empty() const;
        // number of pockets
        size_t size() const;

        /** returns a list of pointers to all top-level items */
        std::list<item *> all_items_top( item_pocket::pocket_type pk_type );
        /** returns a list of pointers to all top-level items */
        std::list<const item *> all_items_top( item_pocket::pocket_type pk_type ) const;

        // returns a list of pointers to all items inside recursively
        std::list<item *> all_items_ptr( item_pocket::pocket_type pk_type );
        // returns a list of pointers to all items inside recursively
        std::list<const item *> all_items_ptr( item_pocket::pocket_type pk_type ) const;

        /** gets all gunmods in the item */
        std::vector<item *> gunmods();
        /** gets all gunmods in the item */
        std::vector<const item *> gunmods() const;

        units::volume item_size_modifier() const;
        units::mass item_weight_modifier() const;
        /** 
          * gets the total volume available to be used.
          * does not guarantee that an item of that size can be inserted.
          */
        units::volume total_container_capacity() const;
        units::volume remaining_container_capacity() const;
        // gets the number of charges of liquid that can fit into the rest of the space
        int remaining_capacity_for_liquid( const item &liquid ) const;

        /** returns the best quality of the id that's contained in the item in CONTAINER pockets */
        int best_quality( const quality_id &id ) const;

        ret_val<bool> insert_item( const item &it, item_pocket::pocket_type pk_type );
        // fills the contents to the brim of this item
        void fill_with( const item &contained );
        bool can_unload_liquid() const;

        /**
         * returns the number of items stacks in contents
         * each item that is not count_by_charges,
         * plus whole stacks of items that are
         */
        size_t num_item_stacks() const;

        void on_pickup( Character &guy );
        bool spill_contents( const tripoint &pos );
        void clear_items();

        // heats up the contents if they have temperature
        void heat_up();
        // returns qty - need
        int ammo_consume( int qty );
        item *magazine_current();
        // spills all liquid from the container. removing liquid from a magazine requires unload logic.
        void handle_liquid_or_spill( Character &guy );
        void casings_handle( const std::function<bool( item & )> &func );

        item *get_item_with( const std::function<bool( const item & )> &filter );

        bool has_any_with( const std::function<bool( const item & )> &filter, item_pocket::pocket_type pk_type ) const;

        void migrate_item( item &obj, const std::set<itype_id> &migrations );
        bool item_has_uses_recursive() const;
        bool stacks_with( const item_contents &rhs ) const;
        // can this item be used as a funnel?
        bool is_funnel_container( units::volume &bigger_than ) const;
        /**
         * @relates visitable
         * NOTE: upon expansion, this may need to be filtered by type enum depending on accessibility
         */
        VisitResponse visit_contents( const std::function<VisitResponse( item *, item * )> &func,
                                      item *parent = nullptr );
        bool remove_internal( const std::function<bool( item & )> &filter,
                              int &count, std::list<item> &res );

        void info( std::vector<iteminfo> &info ) const;

        void serialize( JsonOut &json ) const;
        void deserialize( JsonIn &jsin );
    private:
        // finds the pocket the item will fit in, given the pocket type.
        // this will be where the algorithm picks the best pocket in the contents
        // returns nullptr if none is found
        ret_val<item_pocket *> find_pocket_for( const item &it,
                                                item_pocket::pocket_type pk_type = item_pocket::pocket_type::CONTAINER );

        std::list<item_pocket> contents;
};

#endif
