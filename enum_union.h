#ifndef TYPES_ENUM_UNION_H
#define TYPES_ENUM_UNION_H

#include "types/enum.h"
#include "types/safe_union.h"

namespace types {

template<class Int, class... Vals>
class enum_union : public safe_union<Vals...>, protected enumerate<Int, Vals...>
{
};

} // namespace types

#endif 
