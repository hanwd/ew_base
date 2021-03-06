#ifndef __H_EW_WND_GLCONTEXT__
#define __H_EW_WND_GLCONTEXT__

#include "ewc_base/config.h"
#include "ewa_base/domdata/dobject.h"

EW_ENTER

class DLLIMPEXP_EWC_BASE DataNode;
class DLLIMPEXP_EWC_BASE DataNodeSymbol;
class DLLIMPEXP_EWC_BASE DataModel;
class DLLIMPEXP_EWC_BASE GLDC;

class DLLIMPEXP_EWC_BASE GLContext : public NonCopyable
{
public:

	GLContext();
	~GLContext();

	virtual void SetCurrent(wxWindow* w);
	virtual void SwapBuffers();

	static void CheckError();

protected:

	void* m_hWnd;
	void* m_hDC;
	void* m_hGL;

	virtual void OnInitGL();

};


class DLLIMPEXP_EWC_BASE GLBuffer
{
public:

	GLBuffer();

	enum
	{
		FLAG_NEED_COLOR=1<<0,
		FLAG_NEED_DEPTH=1<<1,
		FLAG_NEED_ALPHA=1<<2,
	};

	vec2i vpos;
	vec2i size;

	arr_xt<float> depth;
	arr_xt<uint8_t> alpha;

	arr_xt<tiny_storage<uint8_t,3> > color;

	mat4d projmatrix;
	mat4d viewmatrix;

	BitFlags flags;

	void ReadPixels();

	void Blit(wxDC& dc);

protected:

	DataPtrT<ObjectData> m_pExtraData;

};


class DLLIMPEXP_EWC_BASE GLStatusLocker
{
public:
	GLStatusLocker(int cap_);
	GLStatusLocker(int cap_,bool enable_);
	~GLStatusLocker();
private:
	int cap;
	bool status;
};

class DLLIMPEXP_EWC_BASE GLBBoxInfo
{
public:
	box3d b3bbox;
	box3d b3axis;
	mat4d m4data;
};

class DLLIMPEXP_EWC_BASE GLClipInfo
{
public:
	vec4d plane[4];
	bool enable;

	void set(const box3d& b3bbox)
	{
		enable = true;
		plane[0].set3(+1, 0, 0); plane[0][3] = -b3bbox.lo[0];
		plane[1].set3(-1, 0, 0); plane[1][3] = +b3bbox.hi[0];
		plane[2].set3(0, +1, 0); plane[2][3] = -b3bbox.lo[1];
		plane[3].set3(0, -1, 0); plane[3][3] = +b3bbox.hi[1];
	}

	GLClipInfo() :enable(false){}
};

class DLLIMPEXP_EWC_BASE GLClipLocker
{
public:
	GLDC &dc;
	GLClipInfo ci0;
	GLClipLocker(GLDC& dc_, const GLClipInfo& ci_);
	~GLClipLocker();
};

class DLLIMPEXP_EWC_BASE GLMatrixLocker
{
public:
	GLDC& dc;
	GLMatrixLocker(GLDC& dc_);
	GLMatrixLocker(GLDC& dc_, const mat4d& m4_);
	~GLMatrixLocker();
};

class DLLIMPEXP_EWC_BASE GLGroupLocker
{
public:
	GLDC& dc;
	GLGroupLocker(GLDC& dc_);
	~GLGroupLocker();
};

class DLLIMPEXP_EWC_BASE DVec2i : public vec2i
{
public:
	DVec2i(const wxSize& sz);
	DVec2i(const wxPoint& pt);
	DVec2i(const vec2i& o);
};



typedef void (*GLDC_Color_Function)(const DColor& c);

class DLLIMPEXP_EWC_BASE GLDC : public GLContext
{
public:

	GLDC();

	enum
	{

		RENDER_CALC_MINSIZE,
		RENDER_SET_REALSIZE,
		RENDER_SOLID,
		RENDER_ALPHA,
		RENDER_LINE,
		RENDER_TEXT,

		RENDER_SELECT,
	};


	GLBBoxInfo bi;
	GLClipInfo ci;


	GLClipInfo ClipPlane(const GLClipInfo& ci_);


	void Light(DLight& light, bool f);

	void Reshape(const DVec2i& s_, const DVec2i& p_ = vec2i(0, 0));

	void SetCurrent(wxWindow* w);

	void Clear(const DColor& c);
	void Clear();

	void LoadIdentity();


	GLDC_Color_Function Color;

	void PushMatrix();

	void PopMatrix();

	void MultMatrix(const mat4d& m4);

	void Mode(int mode);

	int Mode();

	void RenderNode(DataNode* node);
	void RenderModel(DataModel* model);
	void RenderSelect(DataModel* model);

	void SaveBuffer(const String& id);
	void LoadBuffer(const String& id,wxDC& dc);

	void Enable(int id, bool f = true);


	DataNode* HitTest(unsigned x, unsigned y);

	void LineWidth(double w);
	void LineStyle(const DLineStyle& style);


	vec2i GetExtend(const String& text);

	void RenderImage(const arr_1t<double>& xpos, const arr_1t<double>& ypos, arr_xt<double>& value);
	void RenderImage(const arr_xt<double>& xpos, const arr_xt<double>& ypos, arr_xt<double>& value);
	void RenderImage(const arr_1t<float>& xpos, const arr_1t<float>& ypos, arr_xt<float>& value);
	void RenderImage(const arr_xt<float>& xpos, const arr_xt<float>& ypos, arr_xt<float>& value);

	void PrintText(const String& text, const vec3d& pos=vec3d(),const vec3d& shf=vec3d(), const vec3d& pxl=vec3d());

	void SetFont(const DFontStyle& font);


	void EnterGroup();
	void LeaveGroup();



	AtrributeUpdator attrupdator;


	void UpdateAttribute(DataNodeSymbol* node);

	void Translate(const vec3d& pos);

protected:

	template<typename T>
	void RenderImageReal(const T* px, const T* py, const arr_xt<T>& value);

	class TextData;

	vec2i DoGetExtend(const TextData& text);	
	void DoPrintText(const TextData& text, const vec3d& pos,const vec3d& shf, const vec3d& pxl);

	arr_1t<uint8_t> m_aBitmapCachedData;
	DataPtrT<ObjectData> m_pFontData;
	DFontStyle m_tFontStyle;


	static void _Color1(const DColor& color);
	static void _Color2(const DColor& color);


	int m_nMode;
	box3d m_b3BBox;
	DColor m_v4BgColor;
	GLClipInfo m_tClipInfo;
	arr_1t<int> m_aLights;

	indexer_set<DataNode*> m_aNodes;
	bst_map<String, GLBuffer> m_mapBuffers;
};



EW_LEAVE

#endif
