#ifndef COHORS_TYPES_CONSTEXPR_INT_H
#define COHORS_TYPES_CONSTEXPR_INT_H

#include <cstdint>

namespace types {

namespace constexpr_ {

template<class word_t, size_t n>
struct multiword : multiword<word_t, n-1>
{
  word_t word;
  constexpr multiword() : word(0) {}
  constexpr multiword(word_t i) : word(i) {}
};

template<class word_t>
struct multiword<word_t, 0> 
{
};

}

template<
  size_t bits,
  class word_t = uintmax_t
>
class ulongint
{
public:
  static_assert(
    bits % sizeof(word_t) == 0,
    "types::ulongint: invalid size in bits"
  );

  static constexpr int n_words = bits / sizeof(word_t);

  constexpr ulongint(word_t i) : mw(i) {}

protected:
  constexpr_::multiword<word_t, n_words> mw;
};

} // types

#endif

