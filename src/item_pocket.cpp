#include "item_pocket.h"

#include "assign.h"
#include "enums.h"
#include "generic_factory.h"
#include "item.h"
#include "json.h"
#include "units.h"

namespace io
{
// *INDENT-OFF*
template<>
std::string enum_to_string<item_pocket::pocket_type>( item_pocket::pocket_type data )
{
    switch ( data ) {
    case item_pocket::pocket_type::CONTAINER: return "CONTAINER";
    case item_pocket::pocket_type::MAGAZINE: return "MAGAZINE";
    case item_pocket::pocket_type::LEGACY_CONTAINER: return "LEGACY_CONTAINER";
    case item_pocket::pocket_type::LAST: break;
    }
    debugmsg( "Invalid valid_target" );
    abort();
}
// *INDENT-ON*
} // namespace io

void item_pocket::load( const JsonObject &jo )
{
    optional( jo, was_loaded, "pocket_type", type, CONTAINER );
    optional( jo, was_loaded, "min_item_volume", min_item_volume, volume_reader(), 0_ml );
    mandatory( jo, was_loaded, "max_contains_volume", max_contains_volume, volume_reader() );
    mandatory( jo, was_loaded, "max_contains_weight", max_contains_weight, mass_reader() );
    optional( jo, was_loaded, "spoil_multiplier", spoil_multiplier, 1.0f );
    optional( jo, was_loaded, "weight_multiplier", weight_multiplier, 1.0f );
    optional( jo, was_loaded, "moves", moves, 100 );
    optional( jo, was_loaded, "watertight", watertight, false );
    optional( jo, was_loaded, "gastight", gastight, false );
    optional( jo, was_loaded, "open_container", open_container, false );
    optional( jo, was_loaded, "hook", hook, false );
    optional( jo, was_loaded, "rigid", rigid, false );
}

void item_pocket::serialize( JsonOut &json ) const
{
    json.start_object();

    json.member( "pocket_type", type );
    json.member( "min_item_volume", min_item_volume );
    json.member( "max_contains_volume", max_contains_volume );
    json.member( "max_contains_weight", max_contains_weight );
    json.member( "spoil_multiplier", spoil_multiplier );
    json.member( "weight_multiplier", weight_multiplier );
    json.member( "moves", moves );
    json.member( "watertight", watertight );
    json.member( "gastight", gastight );
    json.member( "open_container", open_container );
    json.member( "hook", hook );
    json.member( "rigid", rigid );

    json.member( "contents", contents );

    json.end_object();
}

void item_pocket::deserialize( JsonIn &jsin )
{
    JsonObject data = jsin.get_object();
    load( data );
    optional( data, was_loaded, "contents", contents );
}

units::volume item_pocket::remaining_volume() const
{
    return max_contains_volume - contains_volume();
}

units::volume item_pocket::item_size_modifier() const
{
    if( rigid ) {
        return 0_ml;
    }
    units::volume total_vol = 0_ml;
    for( const item &it : contents ) {
        total_vol += it.volume();
    }
    return total_vol;
}

units::mass item_pocket::item_weight_modifier() const
{
    units::mass total_mass = 0_gram;
    for( const item &it : contents ) {
        total_mass += it.weight() * weight_multiplier;
    }
    return total_mass;
}

bool item_pocket::can_contain( const item &it ) const
{
    if( type == LEGACY_CONTAINER ) {
        return true;
    }
    if( it.made_of( LIQUID ) && !watertight ) {
        return false;
    }
    if( it.made_of( GAS ) && !gastight ) {
        return false;
    }
    if( hook ) {
        return it.has_flag( "BELT_CLIP" ) && it.volume() >= min_item_volume;
    } else {
        return it.volume() <= remaining_volume() && it.volume() >= min_item_volume;
    }
}

cata::optional<item> item_pocket::remove_item( const item &it )
{
    for( auto iter = contents.begin(); iter != contents.end(); iter++ ) {
        if( &*iter == &it ) {
            item ret = *iter;
            contents.erase( iter );
            return ret;
        }
    }
    return cata::nullopt;
}

cata::optional<item> item_pocket::remove_item( const item_location &it )
{
    if( !it ) {
        return cata::nullopt;
    }
    return remove_item( *it );
}

void item_pocket::add( const item &it )
{
    contents.emplace_back( it );
}

bool item_pocket::insert_item( const item &it )
{
    if( type == LEGACY_CONTAINER ) {
        // putting an item into a legacy container needs to be explicit
        return false;
    }
    if( can_contain( it ) ) {
        contents.emplace_back( it );
        return true;
    }
    return false;
}

bool item_pocket::is_type( pocket_type ptype ) const
{
    return ptype == type;
}

units::volume item_pocket::contains_volume() const
{
    units::volume vol = 0_ml;
    for( const item &it : contents ) {
        vol += it.volume();
    }
    return vol;
}

units::mass item_pocket::contains_weight() const
{
    units::mass weight = 0_gram;
    for( const item &it : contents ) {
        weight += it.weight();
    }
    return weight;
}
