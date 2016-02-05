#include "visitable.h"

#include "item.h"
#include "inventory.h"
#include "character.h"

template <typename T>
VisitResponse visitable<T>::visit_items( const std::function<VisitResponse(const item *, const item *)>& func ) const {
    return const_cast<visitable<T> *>( this )->visit_items( static_cast<const std::function<VisitResponse(item *, item *)>&>( func ) );
}

// Specialize visitable<T>::visit_items() for each class that will implement the visitable interface

static VisitResponse visit_internal( const std::function<VisitResponse(item *, item *)>& func, item *node, item *parent = nullptr ) {
    switch( func( node, parent ) ) {
        case VisitResponse::ABORT:
            return VisitResponse::ABORT;

        case VisitResponse::NEXT:
            for( auto& e : node->contents ) {
                if( visit_internal( func, &e, node ) == VisitResponse::ABORT ) {
                    return VisitResponse::ABORT;
                }
            }
        /* intentional fallthrough */

        case VisitResponse::SKIP:
            return VisitResponse::NEXT;
    }

    /* never reached but suppresses GCC warning */
    return VisitResponse::ABORT;
}

template <>
VisitResponse visitable<item>::visit_items( const std::function<VisitResponse( item *, item * )>& func )
{
    auto it = static_cast<item *>( this );
    return visit_internal( func, it );
}

template <>
VisitResponse visitable<inventory>::visit_items( const std::function<VisitResponse( item *, item * )>& func )
{
    auto inv = static_cast<inventory *>( this );
    for( auto& stack : inv->items ) {
        for( auto& it : stack ) {
            if( visit_internal( func, &it ) == VisitResponse::ABORT ) {
                return VisitResponse::ABORT;
            }
        }
    }
    return VisitResponse::NEXT;
}

template <>
VisitResponse visitable<Character>::visit_items( const std::function<VisitResponse( item *, item * )>& func )
{
    auto ch = static_cast<Character *>( this );

    if( !ch->weapon.is_null() && visit_internal( func, &ch->weapon, nullptr ) == VisitResponse::ABORT ) {
        return VisitResponse::ABORT;
    }

    for( auto& e : ch->worn ) {
        if( visit_internal( func, &e ) == VisitResponse::ABORT ) {
            return VisitResponse::ABORT;
        }
    }

    return ch->inv.visit_items( func );
}

// explicit template initialization for all classes implementing the visitable interface
template class visitable<item>;
template class visitable<inventory>;
template class visitable<Character>;
