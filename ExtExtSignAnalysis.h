//===- ExtExtSignAnalysis.h - Sparse forward analysis over ExtExtSignState
//-===//

#ifndef EXTEXTSIGN_ANALYSIS_H
#define EXTEXTSIGN_ANALYSIS_H

#include "ExtExtSignDomain.h"
#include "mlir/Analysis/DataFlow/SparseAnalysis.h"

namespace extextsign {

using ExtExtSignLattice = mlir::dataflow::Lattice<ExtExtSignState>;

class ExtExtSignAnalysis
    : public mlir::dataflow::SparseForwardDataFlowAnalysis<ExtExtSignLattice> {
public:
  using SparseForwardDataFlowAnalysis::SparseForwardDataFlowAnalysis;

  /// Transfer function: given the states of `op`'s operands, set the states of
  /// its results.  Must be monotone in the operand states.
  mlir::LogicalResult
  visitOperation(mlir::Operation *op,
                 llvm::ArrayRef<const ExtExtSignLattice *> operands,
                 llvm::ArrayRef<ExtExtSignLattice *> results) override;

  /// The state of anything entering the analysis from outside: function
  /// arguments, and results the transfer function declines to reason about.
  void setToEntryState(ExtExtSignLattice *lattice) override;
};

} // namespace extextsign

#endif
