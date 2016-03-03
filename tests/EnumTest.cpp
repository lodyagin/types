#include "enum.h"
#include "gtest/gtest.h"

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

} // namespace rainbow

TEST(Enum, rainbow)
{
  using namespace rainbow;

  colours colour;
  EXPECT_EQ(7U, colours::size());
  EXPECT_EQ("orange", colours::name(orange()));
  colour = yellow();
  EXPECT_EQ(yellow(), colour);
  EXPECT_EQ(colours(yellow()), colour);
  {
      std::ostringstream ss;
      ss << colours(indigo());
      EXPECT_EQ("indigo", ss.str());
  }

  {
      std::ostringstream ss;
      ss << noenumalpha << colour << enumalpha << colour;
      EXPECT_EQ("2yellow", ss.str());
  }
}

namespace ext_rainbow {

struct colour
{
    colour() = default;
    colour(const colour&) = delete;
    colour& operator=(const colour&) = delete;

    virtual int c() const = 0;
};

struct red    : colour { int c() const { return 1; } };
struct orange : colour { int c() const { return 2; } };
struct yellow : colour { int c() const { return 3; } };
struct green  : colour { int c() const { return 4; } };
struct blue   : colour { int c() const { return 5; } };
struct indigo : colour { int c() const { return 6; } };
struct violet : colour { int c() const { return 7; } };

using colours = enumerate<
  int8_t, 
  red, orange, yellow, green, blue, indigo, violet
>;

} // namespace ext_rainbow

TEST(Enum, ext_rainbow)
{
  using namespace ext_rainbow;

  colours colour;
  EXPECT_EQ(7U, colours::size());
  EXPECT_EQ("orange", colours::name(orange()));
  colour = yellow();
  EXPECT_EQ(yellow(), colour);
  EXPECT_EQ(colours(yellow()), colour);
  {
      std::ostringstream ss;
      ss << colours(indigo());
      EXPECT_EQ("indigo", ss.str());
  }

  {
      std::ostringstream ss;
      ss << noenumalpha << colour << enumalpha << colour;
      EXPECT_EQ("2yellow", ss.str());
  }
}

