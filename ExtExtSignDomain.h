//===- ExtExtSignDomain.h - The abstract domain --------------------------===//
//
// A nine-point lattice recording the sign of an integer value, with some
// additional values.
//
//           Top
//          /   \
//         0-   0+
//        /  \ /  \
//       -    0    +
//        \   |   /
//        -1  |  1
//          \ | /
//           Bot
//
// MLIR's dataflow framework asks only three things of a lattice value:
//
//   * a default constructor, which must produce the bottom element, because the
//     solver starts every value optimistically and lowers it as facts arrive;
//   * a static join(), which must be commutative, associative, idempotent, and
//     monotone -- assertions in Lattice<> check monotonicity in debug builds;
//   * operator== and print().
//
//===----------------------------------------------------------------------===//

#ifndef EXTEXTSIGN_DOMAIN_H
#define EXTEXTSIGN_DOMAIN_H

#include "llvm/Support/raw_ostream.h"

namespace extextsign {

enum class Kind {
  Bot,
  NOn, // negative one
  POn, // positive one
  Zer, // zero
  Neg, // negative
  Pos, // positive
  NPs, // <= 0, non-positive
  NNg, // >= 0, non-negative
  Top
};

inline const char *name(Kind kind) {
  switch (kind) {
  case Kind::Bot:
    return "bottom";
  case Kind::NOn:
    return "negative one";
  case Kind::POn:
    return "positive one";
  case Kind::Zer:
    return "zero";
  case Kind::Neg:
    return "negative";
  case Kind::Pos:
    return "positive";
  case Kind::NPs:
    return "non-positive";
  case Kind::NNg:
    return "non-negative";
  case Kind::Top:
    return "top";
  }
  return "top";
}

struct ExtExtSignState {
  Kind kind = Kind::Bot;

  ExtExtSignState() = default;
  /* implicit */ ExtExtSignState(Kind kind) : kind(kind) {}

  static ExtExtSignState bottom() { return Kind::Bot; }
  static ExtExtSignState top() { return Kind::Top; }

  bool isBottom() const { return kind == Kind::Bot; }

  /// Least upper bound.
  static ExtExtSignState join(const ExtExtSignState &lhs,
                              const ExtExtSignState &rhs) {
    // clang-format off
    // Axis order:
    // [0] = Bottom, [1] = NegOne, [2] = PosOne,
    // [3] = Zero,   [4] = Neg,    [5] = Pos,
    // [6] = NonPos, [7] = NonNeg, [8] = Top
    constexpr Kind join_table[9][9] = {
        {Kind::Bot, Kind::NOn, Kind::POn, Kind::Zer, Kind::Neg, Kind::Pos, Kind::NPs, Kind::NNg, Kind::Top},
        {Kind::NOn, Kind::NOn, Kind::Top, Kind::NPs, Kind::Neg, Kind::Top, Kind::NPs, Kind::Top, Kind::Top},
        {Kind::POn, Kind::Top, Kind::POn, Kind::NNg, Kind::Top, Kind::Pos, Kind::Top, Kind::NNg, Kind::Top},
        {Kind::Zer, Kind::NPs, Kind::NNg, Kind::Zer, Kind::NPs, Kind::NNg, Kind::NPs, Kind::NNg, Kind::Top},
        {Kind::Neg, Kind::Neg, Kind::Top, Kind::NPs, Kind::Neg, Kind::Top, Kind::NPs, Kind::Top, Kind::Top},
        {Kind::Pos, Kind::Top, Kind::Pos, Kind::NNg, Kind::Top, Kind::Pos, Kind::Top, Kind::NNg, Kind::Top},
        {Kind::NPs, Kind::NPs, Kind::Top, Kind::NPs, Kind::NPs, Kind::Top, Kind::NPs, Kind::Top, Kind::Top},
        {Kind::NNg, Kind::Top, Kind::NNg, Kind::NNg, Kind::Top, Kind::NNg, Kind::Top, Kind::NNg, Kind::Top},
        {Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top}
    };
    // clang-format on

    int lhs_index = static_cast<int>(lhs.kind);
    int rhs_index = static_cast<int>(rhs.kind);
    return join_table[lhs_index][rhs_index];
  }

  bool operator==(const ExtExtSignState &other) const {
    return kind == other.kind;
  }
  bool operator!=(const ExtExtSignState &other) const {
    return kind != other.kind;
  }

  void print(llvm::raw_ostream &os) const { os << name(kind); }
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const ExtExtSignState &state) {
  state.print(os);
  return os;
}

} // namespace extextsign

#endif
