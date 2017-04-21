#ifndef __FIGURE_FIGAXIS_D_H__
#define __FIGURE_FIGAXIS_D_H__

#include "ewa_base/figure/figaxis.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE FigAxisD : public FigGroupT<AxisUnit, FigAxis>
{
public:

	typedef FigGroupT<AxisUnit, FigAxis> basetype;

	FigAxisD(const String& name="axis_d");

	DECLARE_OBJECT_INFO(FigAxisD, ObjectSymbolInfo);
};


EW_LEAVE

#endif