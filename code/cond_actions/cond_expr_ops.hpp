// cond_expr_ops.hpp

#ifndef COND_EXPR_OPS_H
#define COND_EXPR_OPS_H


namespace epw {
namespace condaction {

	enum BinaryNumericOp {
		bnMultiply,
		bnDivide,
		bnAdd,
		bnSubtract,
	};

	enum UnaryNumericOp {
		unNegate,
	};

	enum BinaryComparisonOp {
		bcGreater,
		bcLess,
		bcGreaterEq,
		bcLessEq,
		bcEqual,
		bcNotEqual,
	};

	enum BinaryLogicalOp {
		blAnd,
		blOr,
	};

	enum UnaryLogicalOp {
		ulNot,
	};

}
}


#endif


