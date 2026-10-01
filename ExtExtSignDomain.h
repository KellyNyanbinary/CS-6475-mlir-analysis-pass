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
  Bottom,
  NegOne,
  PosOne,
  Zero,
  Neg,
  Pos,
  NegZero, // <= 0, non-positive
  PosZero, // >= 0, non-negative
  Top
};

inline const char *name(Kind kind) {
  switch (kind) {
  case Kind::Bottom:
    return "bottom";
  case Kind::NegOne:
    return "negone";
  case Kind::PosOne:
    return "posone";
  case Kind::Zero:
    return "zero";
  case Kind::Neg:
    return "neg";
  case Kind::Pos:
    return "pos";
  case Kind::NegZero:
    return "negzero";
  case Kind::PosZero:
    return "poszero";
  case Kind::Top:
    return "top";
  }
  return "top";
}

struct ExtExtSignState {
  Kind kind = Kind::Bottom;

  ExtExtSignState() = default;
  /* implicit */ ExtExtSignState(Kind kind) : kind(kind) {}

  static ExtExtSignState bottom() { return Kind::Bottom; }
  static ExtExtSignState top() { return Kind::Top; }

  bool isBottom() const { return kind == Kind::Bottom; }

  /// Least upper bound.
  static ExtExtSignState join(const ExtExtSignState &lhs,
                              const ExtExtSignState &rhs) {
    // Axis order:
    // [0] = Bottom,  [1] = NegOne,  [2] = PosOne,
    // [3] = Zero,    [4] = Neg,     [5] = Pos,
    // [6] = NegZero, [7] = PosZero, [8] = Top
    constexpr Kind join_table[9][9] = {
        {Kind::Bottom, Kind::NegOne, Kind::PosOne, Kind::Zero, Kind::Neg,
         Kind::Pos, Kind::NegZero, Kind::PosZero, Kind::Top},
        {Kind::NegOne, Kind::NegOne, Kind::Top, Kind::NegZero, Kind::Neg,
         Kind::Top, Kind::NegZero, Kind::Top, Kind::Top},
        {Kind::PosOne, Kind::Top, Kind::PosOne, Kind::PosZero, Kind::Top,
         Kind::Pos, Kind::Top, Kind::PosZero, Kind::Top},
        {Kind::Zero, Kind::NegZero, Kind::PosZero, Kind::Zero, Kind::NegZero,
         Kind::PosZero, Kind::NegZero, Kind::PosZero, Kind::Top},
        {Kind::Neg, Kind::Neg, Kind::Top, Kind::NegZero, Kind::Neg, Kind::Top,
         Kind::NegZero, Kind::Top, Kind::Top},
        {Kind::Pos, Kind::Top, Kind::Pos, Kind::PosZero, Kind::Top, Kind::Pos,
         Kind::Top, Kind::PosZero, Kind::Top},
        {Kind::NegZero, Kind::NegZero, Kind::Top, Kind::NegZero, Kind::NegZero,
         Kind::Top, Kind::NegZero, Kind::Top, Kind::Top},
        {Kind::PosZero, Kind::Top, Kind::PosZero, Kind::PosZero, Kind::Top,
         Kind::PosZero, Kind::Top, Kind::PosZero, Kind::Top},
        {Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top,
         Kind::Top, Kind::Top, Kind::Top},
    };

    int lhs_index = static_cast<int>(lhs.kind);
    int rhs_index = static_cast<int>(rhs.kind);
    return join_table[lhs_index][rhs_index];
  };

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
