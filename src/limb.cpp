#include "limb.h"

#include "generic_factory.h"
#include "json.h"
#include "type_id.h"

// LOADING
// spell_type

namespace
{
    generic_factory<limb_type> spell_factory( "limb" );
} // namespace

template<>
const limb_type &string_id<limb_type>::obj() const
{
    return spell_factory.obj( *this );
}

template<>
bool string_id<limb_type>::is_valid() const
{
    return spell_factory.is_valid( *this );
}

void limb_type::load_limb( JsonObject &jo, const std::string &src )
{
    spell_factory.load( jo, src );
}
