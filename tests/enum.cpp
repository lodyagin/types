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

} // enum_name_

template<class... Vals>
class enumerate;

template<>
class enumerate<>
{
public:
  static constexpr std::size_t n = 0;

  static const std::string& name() 
  { 
    static std::string empty_name;
    return empty_name;
  }
};

template<class Val, class... Vals>
class enumerate<Val, Vals...> : public enumerate<Vals...>
{
  using base = enumerate<Vals...>;

public:
  static constexpr std::size_t n = base::n + 1;

  using base::name;

  static const std::string& name(Val) 
  {
    static std::string the_name = 
      enum_name_::get_name<Val>();
    return the_name;
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
