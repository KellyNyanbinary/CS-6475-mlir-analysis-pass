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

enum class Kind { Bottom, Zero, NonZero, Top };

inline const char *name(Kind kind) {
  switch (kind) {
  case Kind::Bottom:
    return "bottom";
  case Kind::Zero:
    return "zero";
  case Kind::NonZero:
    return "nonzero";
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

  /// Least upper bound.  Two disagreeing facts lose all information.
  static ZeroState join(const ZeroState &lhs, const ZeroState &rhs) {
    if (lhs.kind == Kind::Bottom)
      return rhs;
    if (rhs.kind == Kind::Bottom)
      return lhs;
    if (lhs.kind == rhs.kind)
      return lhs;
    return top();
  }

  bool operator==(const ZeroState &other) const { return kind == other.kind; }
  bool operator!=(const ZeroState &other) const { return kind != other.kind; }

  void print(llvm::raw_ostream &os) const { os << name(kind); }
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const ExtExtSignState &state) {
  state.print(os);
  return os;
}

} // namespace extextsign

#endif
