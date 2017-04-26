
#ifndef __FIGURE_FIGCOORD_H__
#define __FIGURE_FIGCOORD_H__

#include "ewa_base/figure/figitem.h"
#include "ewa_base/figure/figaxis_d.h"
#include "ewa_base/figure/figdata.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE FigCoord : public FigGroupT<FigItem, FigItem>
{
public:

	typedef FigGroupT<FigItem, FigItem> basetype;

	FigCoord();

	DataPtrT<FigDataManager> m_pDataManager;
	DataPtrT<FigAxis> m_pAxis;

	DECLARE_OBJECT_INFO(FigCoord, ObjectSymbolInfo);
};

class DLLIMPEXP_EWA_BASE FigCoord2D : public FigCoord
{
public:

	FigCoord2D();

	DECLARE_OBJECT_INFO(FigCoord2D, ObjectSymbolInfo);
};

class DLLIMPEXP_EWA_BASE FigCoord3D : public FigCoord
{
public:

	FigCoord3D();

	DECLARE_OBJECT_INFO(FigCoord3D, ObjectSymbolInfo);
};


EW_LEAVE
#endif
