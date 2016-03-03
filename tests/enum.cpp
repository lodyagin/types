// -*-coding: mule-utf-8-unix; fill-column: 58; -*- *******

#include <iostream>
#include <string>
#include "types/enum.h"

using namespace types;

namespace rainbow {

struct red {};
struct orange {};
struct yellow {};
struct green {};
struct blue {};
struct indigo {};
struct violet {};

using colours = enumerate<
  int8_t, 
  red, orange, yellow, green, blue, indigo, violet
>;

} // rainbow

using namespace rainbow;

int main(int argc, char* argv[])
{
  colours colour;
  std::cout << colours::size() << std::endl;
  std::cout << colours::name(orange()) << std::endl;
  colour = yellow();
  std::cout << colour << std::endl;
  std::cout << colours(indigo()) << std::endl;
  std::cout << noenumalpha << colour << enumalpha 
    << colour << std::endl;
  std::cout << colours::parse<-3>(std::string("violet")) << std::endl;
  std::cout << colours::parse<-3>(std::string("VIOLET")) << std::endl;
}
