#include "item_contents.h"

#include "generic_factory.h"
#include "item.h"
#include "item_pocket.h"
#include "optional.h"
#include "point.h"
#include "units.h"

namespace fake_item
{
static item_pocket none_pocket( item_pocket::pocket_type::LAST );
} // namespace fake_item

void item_contents::load( const JsonObject &jo )
{
    optional( jo, was_loaded, "nestable", nestable, true );
    mandatory( jo, was_loaded, "contents", contents );
}

void item_contents::serialize( JsonOut &json ) const
{
    json.start_object();

    json.member( "nestable", nestable );
    json.member( "contents", contents );

    json.end_object();
}

void item_contents::deserialize( JsonIn &jsin )
{
    JsonObject data = jsin.get_object();
    load( data );
}

bool item_contents::stacks_with( const item_contents &rhs ) const
{
    if( contents.size() != rhs.contents.size() ) {
        return false;
    }
    return std::equal( contents.begin(), contents.end(),
                       rhs.contents.begin(),
    []( const item_pocket & a, const item_pocket & b ) {
        return a.stacks_with( b );
    } );
}

bool item_contents::can_contain( const item &it ) const
{
    for( const item_pocket &pocket : contents ) {
        if( pocket.can_contain( it ) ) {
            return true;
        }
    }
    return false;
}

item *item_contents::magazine_current()
{
    for( item_pocket &pocket : contents ) {
        item *mag = pocket.magazine_current();
        if( mag != nullptr ) {
            return mag;
        }
    }
    return nullptr;
}

void item_contents::casings_handle( const std::function<bool( item & )> &func )
{
    for( item_pocket &pocket : contents ) {
        pocket.casings_handle( func );
    }
}

bool item_contents::use_amount( const itype_id &it, int &quantity, std::list<item> &used )
{
    bool used_item = false;
    for( item_pocket &pocket : contents ) {
        used_item = pocket.use_amount( it, quantity, used ) || used_item;
    }
    return used_item;
}

bool item_contents::will_explode_in_a_fire() const
{
    for( const item_pocket &pocket : contents ) {
        if( pocket.will_explode_in_a_fire() ) {
            return true;
        }
    }
    return false;
}

bool item_contents::detonate( const tripoint &p, std::vector<item> &drops )
{
    bool detonated = false;
    for( item_pocket &pocket : contents ) {
        detonated = pocket.detonate( p, drops ) || detonated;
    }
    return detonated;
}

bool item_contents::process( const itype &type, player *carrier, const tripoint &pos, bool activate,
                             float insulation, const temperature_flag flag )
{
    for( item_pocket &pocket : contents ) {
        pocket.process( type, carrier, pos, activate, insulation, flag );
    }
    return true;
}

bool item_contents::legacy_unload( player *guy, bool &changed )
{
    for( item_pocket &pocket : contents ) {
        pocket.legacy_unload( guy, changed );
    }
    return true;
}

void item_contents::remove_all_ammo( Character &guy )
{
    for( item_pocket &pocket : contents ) {
        pocket.remove_all_ammo( guy );
    }
}

void item_contents::remove_all_mods( Character &guy )
{
    for( item_pocket &pocket : contents ) {
        pocket.remove_all_mods( guy );
    }
}

bool item_contents::empty() const
{
    if( contents.empty() ) {
        return true;
    }
    for( const item_pocket &pocket : contents ) {
        if( pocket.empty() ) {
            return true;
        }
    }
    return false;
}

item &item_contents::legacy_back()
{
    return legacy_pocket().back();
}

const item &item_contents::legacy_back() const
{
    return legacy_pocket().back();
}

item &item_contents::legacy_front()
{
    return legacy_pocket().front();
}

const item &item_contents::legacy_front() const
{
    return legacy_pocket().front();
}

size_t item_contents::legacy_size() const
{
    return legacy_pocket().size();
}

void item_contents::legacy_pop_back()
{
    legacy_pocket().pop_back();
}

size_t item_contents::num_item_stacks() const
{
    size_t num = 0;
    for( const item_pocket &pocket : contents ) {
        num += pocket.size();
    }
    return num;
}

item_pocket &item_contents::legacy_pocket()
{
    for( item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::LEGACY_CONTAINER ) ) {
            return pocket;
        }
    }
    debugmsg( "Tried to access non-existing legacy pocket" );
    return fake_item::none_pocket;
}

const item_pocket &item_contents::legacy_pocket() const
{
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::LEGACY_CONTAINER ) ) {
            return pocket;
        }
    }
    debugmsg( "Tried to access non-existing legacy pocket" );
    return fake_item::none_pocket;
}

std::list<item> item_contents::all_items()
{
    std::list<item> item_list;
    for( item_pocket &pocket : contents ) {
        std::list<item> contained_items = pocket.all_items();
        item_list.insert( item_list.end(), contained_items.begin(), contained_items.end() );
    }
    return item_list;
}

std::list<item> item_contents::all_items() const
{
    std::list<item> item_list;
    for( const item_pocket &pocket : contents ) {
        std::list<item> contained_items = pocket.all_items();
        item_list.insert( item_list.end(), contained_items.begin(), contained_items.end() );
    }
    return item_list;
}

units::volume item_contents::item_size_modifier() const
{
    units::volume total_vol = 0_ml;
    for( const item_pocket &pocket : contents ) {
        total_vol += pocket.item_size_modifier();
    }
    return total_vol;
}

units::mass item_contents::item_weight_modifier() const
{
    units::mass total_mass = 0_gram;
    for( const item_pocket &pocket : contents ) {
        total_mass += pocket.item_weight_modifier();
    }
    return total_mass;
}

bool item_contents::spill_contents( const tripoint &pos )
{
    for( item_pocket &pocket : contents ) {
        pocket.spill_contents( pos );
    }
    return true;
}

cata::optional<item> item_contents::remove_item( const item &it )
{
    for( item_pocket pocket : contents ) {
        cata::optional<item> ret = pocket.remove_item( it );
        if( ret ) {
            return ret;
        }
    }
    return cata::nullopt;
}

cata::optional<item> item_contents::remove_item( const item_location &it )
{
    if( !it ) {
        return cata::nullopt;
    }
    return remove_item( *it );
}

void item_contents::clear_items()
{
    for( item_pocket &pocket : contents ) {
        pocket.clear_items();
    }
}

bool item_contents::has_item( const item &it ) const
{
    for( const item_pocket &pocket : contents ) {
        if( pocket.has_item( it ) ) {
            return true;
        }
    }
    return false;
}

item *item_contents::get_item_with( const std::function<bool( const item & )> &filter )
{
    for( item_pocket &pocket : contents ) {
        item *it = pocket.get_item_with( filter );
        if( it != nullptr ) {
            return it;
        }
    }
    return nullptr;
}

void item_contents::remove_items_if( const std::function<bool( item & )> &filter )
{
    for( item_pocket &pocket : contents ) {
        pocket.remove_items_if( filter );
    }
}

void item_contents::has_rotten_away( const tripoint &pnt )
{
    for( item_pocket &pocket : contents ) {
        pocket.has_rotten_away( pnt );
    }
}

bool item_contents::insert_item( const item &it )
{
    for( item_pocket pocket : contents ) {
        if( pocket.insert_item( it ) ) {
            return true;
        }
    }
    return false;
}

void item_contents::insert_legacy( const item &it )
{
    for( item_pocket pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::LEGACY_CONTAINER ) ) {
            pocket.add( it );
            return;
        }
    }
    item_pocket fake_pocket( item_pocket::pocket_type::LEGACY_CONTAINER );
    fake_pocket.add( it );
    contents.emplace_front( fake_pocket );
}

std::list<item> &item_contents::legacy_items()
{
    return legacy_pocket().edit_contents();
}
