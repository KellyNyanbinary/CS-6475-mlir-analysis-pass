//===- ExtExtSignAnalysis.cpp - Transfer functions ------------------------------===//
//
// The transfer function: given what is known about an operation's operands,
// state what is known about its results.  This file and ExtExtSignDomain.h
// are the two to replace when building a different analysis; the rest of the
// project is scaffolding.
//
// There are deliberately only two rules here, one of each kind an analysis
// needs: one that introduces facts out of nothing (constants), and one that
// propagates facts it was given (`and`).  Everything else is unknown.  Adding
// a third rule should be a matter of adding a third `if`.
//
//===----------------------------------------------------------------------===//

#include "ExtExtSignAnalysis.h"

#include "ExtExtSignDomain.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Matchers.h"
#include <iostream>

using namespace mlir;

namespace extextsign {

void ExtExtSignAnalysis::setToEntryState(ExtExtSignLattice *lattice) {
  propagateIfChanged(lattice, lattice->join(ExtExtSignState::top()));
}

LogicalResult
ExtExtSignAnalysis::visitOperation(Operation *op,
                                   ArrayRef<const ExtExtSignLattice *> operands,
                                   ArrayRef<ExtExtSignLattice *> results) {
  // Raising a result to top says "this operation could produce anything",
  // which is always a sound answer and is what every unhandled case does.
  auto unknown = [&] {
    setAllToEntryStates(results);
    return success();
  };

  // Only single-result integer operations are interesting here.  Calls, loads,
  // floats, and vectors all land in `unknown`.
  if (op->getNumResults() != 1 || !op->getResult(0).getType().isIntOrIndex())
    return unknown();
  ExtExtSignLattice *result = results[0];

  // Basic value matching.
  IntegerAttr value;
  if (matchPattern(op, m_Constant(&value))) {
    std::cout << "Performing assignment\n";

    ExtExtSignState state;

    if (value.getValue().isZero())
      state = ExtExtSignState(Kind::Zer);
    else if (value.getValue().isAllOnes())
      state = Kind::NOn;
    else if (value.getValue().isOne())
      state = Kind::POn;
    else if (value.getValue().isNegative())
      state = Kind::Neg;
    else if (value.getValue().isStrictlyPositive())
      state = Kind::Pos;
    else if (value.getValue().isNonPositive())
      state = Kind::NPs;
    else if (value.getValue().isNonNegative())
      state = Kind::NNg;
    else
      state = Kind::Top;

    propagateIfChanged(result, result->join(state));
    return success();
  }

  // Abstract x + y
  // Where we are going, we don't care about overflows.
  if (isa<LLVM::AddOp>(op)) {
    std::cout << "Performing abstract +\n";

    // clang-format off
    // Axis order:
    // [0] = Bottom, [1] = NegOne, [2] = PosOne,
    // [3] = Zero,   [4] = Neg,    [5] = Pos,
    // [6] = NonPos, [7] = NonNeg, [8] = Top
    constexpr Kind add_table[9][9] = {
        {Kind::Bot, Kind::Bot, Kind::Bot, Kind::Bot, Kind::Bot, Kind::Bot, Kind::Bot, Kind::Bot, Kind::Bot},
        {Kind::Bot, Kind::Neg, Kind::Zer, Kind::NOn, Kind::Neg, Kind::NNg, Kind::Neg, Kind::Top, Kind::Top},
        {Kind::Bot, Kind::Zer, Kind::Pos, Kind::POn, Kind::NPs, Kind::Pos, Kind::Top, Kind::Pos, Kind::Top},
        {Kind::Bot, Kind::NOn, Kind::POn, Kind::Zer, Kind::Neg, Kind::Pos, Kind::NPs, Kind::NNg, Kind::Top},
        {Kind::Bot, Kind::Neg, Kind::NPs, Kind::Neg, Kind::Neg, Kind::Top, Kind::Neg, Kind::Top, Kind::Top},
        {Kind::Bot, Kind::NNg, Kind::Pos, Kind::Pos, Kind::Top, Kind::Pos, Kind::Top, Kind::Pos, Kind::Top},
        {Kind::Bot, Kind::Neg, Kind::Top, Kind::NPs, Kind::Neg, Kind::Top, Kind::NPs, Kind::Top, Kind::Top},
        {Kind::Bot, Kind::Top, Kind::Pos, Kind::NNg, Kind::Top, Kind::Pos, Kind::Top, Kind::NNg, Kind::Top},
        {Kind::Bot, Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top},
    };
    // clang-format on

    ExtExtSignState lhs = operands[0]->getValue();
    ExtExtSignState rhs = operands[1]->getValue();
    int lhs_index = static_cast<int>(lhs.kind);
    int rhs_index = static_cast<int>(rhs.kind);
    ExtExtSignState state = add_table[lhs_index][rhs_index];
    propagateIfChanged(result, result->join(state));
    return success();
  }

  // Abstract x - y
  // Where we are going, we don't care about underflows.
  if (isa<LLVM::SubOp>(op)) {
    std::cout << "Performing abstract -\n";

    // clang-format off
    // Axis order:
    // [0] = Bottom, [1] = NegOne, [2] = PosOne,
    // [3] = Zero,   [4] = Neg,    [5] = Pos,
    // [6] = NonPos, [7] = NonNeg, [8] = Top
    constexpr Kind sub_table[9][9] = {
        {Kind::Bot, Kind::Bot, Kind::Bot, Kind::Bot, Kind::Bot, Kind::Bot, Kind::Bot, Kind::Bot, Kind::Bot},
        {Kind::Bot, Kind::Zer, Kind::Neg, Kind::NOn, Kind::NNg, Kind::Neg, Kind::Top, Kind::Neg, Kind::Top},
        {Kind::Bot, Kind::Pos, Kind::Zer, Kind::POn, Kind::Pos, Kind::NPs, Kind::Pos, Kind::Top, Kind::Top},
        {Kind::Bot, Kind::POn, Kind::NOn, Kind::Zer, Kind::Pos, Kind::Neg, Kind::NNg, Kind::NPs, Kind::Top},
        {Kind::Bot, Kind::NPs, Kind::Neg, Kind::Neg, Kind::Top, Kind::Neg, Kind::Top, Kind::Neg, Kind::Top},
        {Kind::Bot, Kind::Pos, Kind::NNg, Kind::Pos, Kind::Pos, Kind::Top, Kind::Pos, Kind::Top, Kind::Top},
        {Kind::Bot, Kind::Top, Kind::Neg, Kind::NPs, Kind::Top, Kind::Neg, Kind::Top, Kind::NPs, Kind::Top},
        {Kind::Bot, Kind::Pos, Kind::Top, Kind::NNg, Kind::Pos, Kind::Top, Kind::NNg, Kind::Top, Kind::Top},
        {Kind::Bot, Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top, Kind::Top},
    };
    // clang-format on

    ExtExtSignState lhs = operands[0]->getValue();
    ExtExtSignState rhs = operands[1]->getValue();
    int lhs_index = static_cast<int>(lhs.kind);
    int rhs_index = static_cast<int>(rhs.kind);
    ExtExtSignState state = sub_table[lhs_index][rhs_index];
    propagateIfChanged(result, result->join(state));
    return success();
  }

  // TODO: other signs

  return unknown();
}

} // namespace extextsign
