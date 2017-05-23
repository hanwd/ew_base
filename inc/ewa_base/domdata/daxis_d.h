#ifndef __H_EW_DOMDATA_DAXIS_D__
#define __H_EW_DOMDATA_DAXIS_D__

#include "ewa_base/domdata/daxisunit.h"

EW_ENTER


class DLLIMPEXP_EWA_BASE DAxis : public FigGroupT<DAxisUnit, DObject>
{
public:

	typedef FigGroupT<DAxisUnit, DObject> basetype;

	enum
	{
		MODE_NONE,
		MODE_2D_BOX,
		MODE_2D_POLAR,
		MODE_3D_BOX,
		MODE_CYLINDER,
		MODE_3D_DIR,
		MODE_SPHERE,
	};

	DAxis(const String& name="axis_d");

	virtual bool DoGetChildren(DChildrenState& cs)
	{
		if(m_aItems.empty()) return false;
		return basetype::DoGetChildren(cs);
	}

	int m_nMode;

	void SetMode(int mode);

	DECLARE_OBJECT_INFO(DAxis, DObjectInfo);
};



EW_LEAVE

#endif