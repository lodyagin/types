// -*-coding: mule-utf-8-unix; fill-column: 58; -*- *******

#include <iostream>
#include <string>
#include "types/enum.h"
#include "types/typeinfo.h"

namespace types {

namespace enum_name_ {

//! get a name of enum value
template<class EnumVal>
std::string get_name()
{
  auto type_name = type<EnumVal>::name();
  // select only type name, discard namespace
  auto pos = type_name.find_last_of(':');
  if (pos != decltype(type_name)::npos)
    return type_name.substr(pos + 1);
  else
    return type_name;
};

//! static storage for an enum value name
template<class Val>
struct name_cacher
{
  //! If the name was found before return the cached
  //! value, found it in other case
  static void init_name_cache()
  {
    if (name_cache.empty())
      name_cache = get_name<Val>();
  }

  static std::string name_cache;
};

template<class Val>
std::string name_cacher<Val>::name_cache;

} // enum_name_

template<class... Vals>
class enumerate;

template<>
class enumerate<> : protected enum_name_::name_cacher<void>
{
public:
  static constexpr std::size_t n = 0;

  static const std::string& name() 
  { 
    return enum_name_::name_cacher<void>::name_cache;
    // empty name
  }
};

template<class Val, class... Vals>
class enumerate<Val, Vals...> 
  : public enumerate<Vals...>,
    protected enum_name_::name_cacher<Val>
{
  using base = enumerate<Vals...>;
  using cacher = enum_name_::name_cacher<Val>;

public:
  static constexpr std::size_t n = base::n + 1;

  using base::name;

  static const std::string& name(Val) 
  {
    cacher::init_name_cache();
    return cacher::name_cache;
  }
};

} // types

namespace rainbow {

struct red {};
struct orange {};
struct yellow {};
struct green {};
struct blue {};
struct indigo {};
struct violet {};

using colours = types::enumerate
  <red, orange, yellow, green, blue, indigo, violet>;

} // rainbow

using namespace rainbow;

int main(int argc, char* argv[])
{
  std::cout << colours::n << std::endl;
  std::cout << colours::name(orange()) << std::endl;
}
