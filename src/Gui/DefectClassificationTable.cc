#include "Gui/DefectClassificationTable.hh"


AbstractToken* AbstractToken::tokenByNumber(unsigned int N,const std::vector<std::vector<RangeScanLine>>& spec, QPoint beginIdx, QPoint endIdx)
{
	switch(N){
	case 0:
		return new MinimumToken(spec, beginIdx, endIdx);
	case 1:
		return new MaximumToken(spec, beginIdx, endIdx);
	case 2:
		return new PositiveHalfSumToken(spec, beginIdx, endIdx);
	case 3:
		return new NegativeHalfSumToken(spec, beginIdx, endIdx);
	case 4:
		return new PositiveRelativeResidualToken(spec, beginIdx, endIdx);
	default:
		return new NegativeRelativeResidualToken(spec, beginIdx, endIdx);
	}
}