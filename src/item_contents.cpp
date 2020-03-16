#include "item_contents.h"

#include "character.h"
#include "game.h"
#include "item.h"
#include "map.h"


static const std::vector<item_pocket::pocket_type> avail_types{
    item_pocket::pocket_type::CONTAINER,
    item_pocket::pocket_type::MAGAZINE
};

bool item_contents::empty() const
{
    if( contents.empty() ) {
        return true;
    }
    for( const item_pocket &pocket : contents ) {
        if( !pocket.empty() ) {
            return false;
        }
    }
    return true;
}

size_t item_contents::size() const
{
    return contents.size();
}

ret_val<bool> item_contents::insert_item( const item &it, item_pocket::pocket_type pk_type )
{
    ret_val<item_pocket *> pocket = find_pocket_for( it, pk_type );
    if( !pocket.success() ) {
        return ret_val<bool>::make_failure( pocket.str() );
    }
    ret_val<item_pocket::contain_code> pocket_contain_code = pocket.value()->insert_item( it );
    if( pocket_contain_code.success() ) {
        return ret_val<bool>::make_success();
    }
    return ret_val<bool>::make_failure( "No success" );
}

void item_contents::fill_with( const item &contained )
{
    for( item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::CONTAINER ) ) {
            pocket.fill_with( contained );
        }
    }
}

item_pocket *item_contents::best_pocket( const item &it, bool nested )
{
    if( !can_contain( it ).success() ) {
        return nullptr;
    }
    item_pocket *ret = nullptr;
    for( item_pocket &pocket : contents ) {
        if( nested && !pocket.rigid() ) {
            continue;
        }
        if( ret == nullptr ) {
            if( pocket.can_contain( it ).success() ) {
                ret = &pocket;
            }
        }
        else if( pocket.can_contain( it ).success() && ret->better_pocket( pocket, it ) ) {
            ret = &pocket;
            for( item *contained : all_items_top( item_pocket::pocket_type::CONTAINER ) ) {
                item_pocket *internal_pocket = contained->contents.best_pocket( it, nested );
                if( internal_pocket != nullptr && ret->better_pocket( pocket, it ) ) {
                    ret = internal_pocket;
                }
            }
        }
    }
    return ret;
}

ret_val<bool> item_contents::can_contain_rigid( const item &it ) const
{
    ret_val<bool> ret = ret_val<bool>::make_failure( _( "is not a container" ) );
    for( const item_pocket &pocket : contents ) {
        if( !pocket.rigid() ) {
            ret = ret_val<bool>::make_failure( _( "is not rigid" ) );
            continue;
        }
        const ret_val<item_pocket::contain_code> pocket_contain_code = pocket.can_contain( it );
        if( pocket_contain_code.success() ) {
            return ret_val<bool>::make_success();
        }
        ret = ret_val<bool>::make_failure( pocket_contain_code.str() );
    }
    return ret;
}

ret_val<bool> item_contents::can_contain( const item &it ) const
{
    ret_val<bool> ret = ret_val<bool>::make_failure( _( "is not a container" ) );
    for( const item_pocket &pocket : contents ) {
        const ret_val<item_pocket::contain_code> pocket_contain_code = pocket.can_contain( it );
        if( pocket_contain_code.success() ) {
            return ret_val<bool>::make_success();
        }
        ret = ret_val<bool>::make_failure( pocket_contain_code.str() );
    }
    return ret;
}

bool item_contents::can_contain_liquid( bool held_or_ground ) const
{
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::CONTAINER ) && pocket.can_contain_liquid( held_or_ground ) ) {
            return true;
        }
    }
    return false;
}

bool item_contents::can_unload_liquid() const
{
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::CONTAINER ) && pocket.can_unload_liquid() ) {
            return true;
        }
    }
    return false;
}

size_t item_contents::num_item_stacks() const
{
    size_t num = 0;
    for( const item_pocket &pocket : contents ) {
        num += pocket.size();
    }
    return num;
}

void item_contents::on_pickup( Character &guy )
{
    for( item_pocket &pocket : contents ) {
        if( !pocket.is_type( item_pocket::pocket_type::MOD ) )
        {
            pocket.on_pickup( guy );
        }
    }
}

bool item_contents::spill_contents( const tripoint &pos )
{
    bool spilled = false;
    for( item_pocket &pocket : contents ) {
        spilled = pocket.spill_contents( pos ) || spilled;
    }
    return spilled;
}

void item_contents::heat_up()
{
    for( item_pocket &pocket : contents ) {
        if( !pocket.is_type( item_pocket::pocket_type::CONTAINER ) ) {
            continue;
        }
        pocket.heat_up();
    }
}

int item_contents::ammo_consume( int qty )
{
    for( item_pocket &pocket : contents ) {
        if( !pocket.is_type( item_pocket::pocket_type::MAGAZINE ) ) {
            continue;
        }
        qty = pocket.ammo_consume( qty );
    }
    return qty;
}

item *item_contents::magazine_current()
{
    for( item_pocket &pocket : contents ) {
        if( !pocket.is_type( item_pocket::pocket_type::MAGAZINE ) ) {
            continue;
        }
        item *mag = pocket.magazine_current();
        if( mag != nullptr ) {
            return mag;
        }
    }
    return nullptr;
}

void item_contents::handle_liquid_or_spill( Character &guy )
{
    for( item_pocket pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::CONTAINER ) ) {
            pocket.handle_liquid_or_spill( guy );
        }
    }
}

void item_contents::casings_handle( const std::function<bool( item & )> &func )
{
    for( item_pocket &pocket : contents ) {
        pocket.casings_handle( func );
    }
}

void item_contents::clear_items()
{
    for( item_pocket &pocket : contents ) {
        pocket.clear_items();
    }
}

void item_contents::migrate_item( item &obj, const std::set<itype_id> &migrations )
{
    for( item_pocket pocket : contents ) {
        pocket.migrate_item( obj, migrations );
    }
}

bool item_contents::has_pocket_type( const item_pocket::pocket_type pk_type ) const
{
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( pk_type ) ) {
            return true;
        }
    }
    return false;
}

bool item_contents::has_any_with( const std::function<bool( const item &it )> &filter, item_pocket::pocket_type pk_type ) const
{
    for( const item_pocket pocket : contents ) {
        if( !pocket.is_type( pk_type ) ) {
            continue;
        }
        if( pocket.has_any_with( filter ) )
        {
            return true;
        }
    }
    return false;
}

bool item_contents::stacks_with( const item_contents &rhs ) const
{
    if( contents.size() != rhs.contents.size() ) {
        return false;
    }
    return std::equal( contents.begin(), contents.end(),
        rhs.contents.begin(),
        []( const item_pocket &a, const item_pocket &b ) {
        return a.stacks_with( b );
    } );
}

bool item_contents::is_funnel_container( units::volume &bigger_than ) const
{
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::CONTAINER ) ) {
            if( pocket.is_funnel_container( bigger_than ) ) {
                return true;
            }
        }
    }
    return false;
}

item *item_contents::get_item_with( const std::function<bool( const item &it )> &filter )
{
    for( item_pocket &pocket : contents ) {
        item *it = pocket.get_item_with( filter );
        if( it != nullptr ) {
            return it;
        }
    }
    return nullptr;
}

std::list<item *> item_contents::all_items_top( item_pocket::pocket_type pk_type )
{
    std::list<item *> all_items_internal;
    for( item_pocket &pocket : contents ) {
        if( pocket.is_type( pk_type ) ) {
            std::list<item *> contained_items = pocket.all_items_top();
            all_items_internal.insert( all_items_internal.end(), contained_items.begin(),
                contained_items.end() );
        }
    }
    return all_items_internal;
}

std::list<const item *> item_contents::all_items_top( item_pocket::pocket_type pk_type ) const
{
    std::list<const item *> all_items_internal;
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( pk_type ) ) {
            std::list<const item *> contained_items = pocket.all_items_top();
            all_items_internal.insert( all_items_internal.end(), contained_items.begin(),
                contained_items.end() );
        }
    }
    return all_items_internal;
}

std::list<item *> item_contents::all_items_top()
{
    std::list<item *> ret;
    for( const item_pocket::pocket_type pk_type : avail_types ) {
        std::list<item *> top{ all_items_top( pk_type ) };
        ret.insert( ret.end(), top.begin(), top.end() );
    }
    return ret;
}

std::list<const item *> item_contents::all_items_top() const
{
    std::list<const item *> ret;
    for( const item_pocket::pocket_type pk_type : avail_types ) {
        std::list<const item *> top{ all_items_top( pk_type ) };
        ret.insert( ret.end(), top.begin(), top.end() );
    }
    return ret;
}

std::list<item *> item_contents::all_items_ptr( item_pocket::pocket_type pk_type )
{
    std::list<item *> all_items_internal;
    for( item_pocket &pocket : contents ) {
        if( pocket.is_type( pk_type ) ) {
            std::list<item *> contained_items = pocket.all_items_ptr( pk_type );
            all_items_internal.insert( all_items_internal.end(), contained_items.begin(),
                contained_items.end() );
        }
    }
    return all_items_internal;
}

std::list<const item *> item_contents::all_items_ptr( item_pocket::pocket_type pk_type ) const
{
    std::list<const item *> all_items_internal;
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( pk_type ) ) {
            std::list<const item *> contained_items = pocket.all_items_ptr( pk_type );
            all_items_internal.insert( all_items_internal.end(), contained_items.begin(),
                contained_items.end() );
        }
    }
    return all_items_internal;
}

std::vector<item *> item_contents::gunmods()
{
    std::vector<item *> mods;
    for( item_pocket pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::MOD ) ) {
            std::vector<item *> internal_mods{ pocket.gunmods() };
            mods.insert( mods.end(), internal_mods.begin(), internal_mods.end() );
        }
    }
    return mods;
}

std::vector<const item *> item_contents::gunmods() const
{
    std::vector<const item *> mods;
    for( const item_pocket pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::MOD ) ) {
            std::vector<const item *> internal_mods{ pocket.gunmods() };
            mods.insert( mods.end(), internal_mods.begin(), internal_mods.end() );
        }
    }
    return mods;
}

units::volume item_contents::total_container_capacity() const
{
    units::volume total_vol = 0_ml;
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::CONTAINER ) ) {
            total_vol += pocket.volume_capacity();
        }
    }
    return total_vol;
}

units::volume item_contents::remaining_container_capacity() const
{
    units::volume total_vol = 0_ml;
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::CONTAINER ) ) {
            total_vol += pocket.remaining_volume();
        }
    }
    return total_vol;
}

void item_contents::remove_rotten( const tripoint &pnt )
{
    for( item_pocket &pocket : contents ) {
        // no reason to check mods, they won't rot
        if( !pocket.is_type( item_pocket::pocket_type::MOD ) ) {
            pocket.remove_rotten( pnt );
        }
    }
}

void item_contents::process( player *carrier, const tripoint &pos, bool activate, float insulation,
    temperature_flag flag, float spoil_multiplier )
{
    for( item_pocket &pocket : contents ) {
        // no reason to check mods, they won't rot
        if( !pocket.is_type( item_pocket::pocket_type::MOD ) ) {
            pocket.process( carrier, pos, activate, insulation, flag, spoil_multiplier );
        }
    }
}

int item_contents::remaining_capacity_for_liquid( const item &liquid ) const
{
    int charges_of_liquid = 0;
    for( const item_pocket &pocket : contents ) {
        if( !pocket.is_type( item_pocket::pocket_type::CONTAINER ) ) {
            continue;
        }
        if( pocket.can_contain( liquid ).success() ) {
            charges_of_liquid += liquid.charges_per_volume( pocket.remaining_volume() );
        }
    }
    return charges_of_liquid;
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

int item_contents::best_quality( const quality_id &id ) const
{
    int ret = 0;
    for( const item_pocket &pocket : contents ) {
        ret = std::max( pocket.best_quality( id ), ret );
    }
    return ret;
}

static void insert_separation_line( std::vector<iteminfo> &info )
{
    if( info.empty() || info.back().sName != "--" ) {
        info.push_back( iteminfo( "DESCRIPTION", "--" ) );
    }
}

void item_contents::info( std::vector<iteminfo> &info ) const
{
    int pocket_number = 1;
    std::vector<iteminfo> contents_info;
    std::vector<item_pocket> found_pockets;
    std::map<int, int> pocket_num; // index, amount
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::CONTAINER ) ) {
            bool found = false;
            int idx = 0;
            for( const item_pocket &found_pocket : found_pockets ) {
                if( found_pocket == pocket ) {
                    found = true;
                    pocket_num[idx]++;
                }
                idx++;
            }
            if( !found ) {
                found_pockets.push_back( pocket );
                pocket_num[idx]++;
            }
            pocket.contents_info( contents_info, pocket_number++, contents.size() != 1 );
        }
    }
    int idx = 0;
    for( const item_pocket &pocket : found_pockets ) {
        insert_separation_line( info );
        if( pocket_num[idx] > 1 ) {
            info.emplace_back( "DESCRIPTION", _( string_format( "<bold>Pockets (%d)</bold>",
                pocket_num[idx] ) ) );
        }
        idx++;
        pocket.general_info( info, idx, false );
    }
    info.insert( info.end(), contents_info.begin(), contents_info.end() );
}
