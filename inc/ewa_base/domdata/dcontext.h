#ifndef __H_EW_DOMDATA_DCONTEXT__
#define __H_EW_DOMDATA_DCONTEXT__



#include "ewa_base/domdata/dunit.h"



EW_ENTER

class DContext;

class DStlDomain : public ObjectData
{
public:
	DStlDomain(int t) :type(t){}
	const int type;
	BitFlags flags;

};

class DStlDomainStl : public DStlDomain
{
public:
	DStlDomainStl() :DStlDomain(0){}

	arr_1t<vec3d> vertexs;
	arr_1t<vec3d> normals;

	arr_1t<vec3i> triangles;
	arr_1t<int> fnormal;
	arr_1t<int> vnormal;


};

class DStlDomainGrp : public DStlDomain
{
public:
	DStlDomainGrp() :DStlDomain(1){}
	arr_1t<DataPtrT<DStlDomain> > children;
};

class DStlModel : public DStlDomainGrp
{
public:
	bool LoadStl(const String& file);
	static DataPtrT<DStlDomainStl> GetUnitObject(const String& s, int d = -1);
};

template<typename T>
class DCtxState
{
public:

	typedef T type;

	type val;
	type ref;
	arr_1t<type> stk;

	void push()
	{
		stk.push_back(val);
	}
	void push(const type& v)
	{
		stk.push_back(val);
		val = v;
	}
	void pop()
	{
		stk.pop_back(val);
	}
};

class DCtxStateMatrix : public DCtxState<mat4d>
{
public:
	typedef DCtxState<mat4d> basetype;
	using basetype::push;

	void push(const mat4d& v)
	{
		basetype::push();
		val = val*v;
	}
};

class DCtxStateMaterial
{
public:
	typedef DObject* type;

	DCtxStateMaterial(mat4d& m) :m4(m){}

	mat4d& m4;


	template<typename T>
	void push(NamedReferenceT<T>& ref)
	{
		push(ref.get());
	}

	void init(DObject* p,DObject* b=NULL);

	void pop();

	void push(DObject* p);

	int ref;
	int val;

	arr_1t<DObject*> stk;
	indexer_set<DataPtrT<DObject> > arr;
};


class DCtxStateUnit : public DCtxState<KUnit>
{
public:

	typedef DUnit* type;
	typedef DCtxState<KUnit> basetype;
	using basetype::push;

	void init(DUnit* p);
	void push(DUnit* p);

};

template<typename T>
class DCtxLocker
{
public:
	DCtxLocker(T& t) :target(t){ target.push(); }
	DCtxLocker(T& t, const typename T::type& v) :target(t){ target.push(v); }
	~DCtxLocker(){ target.pop(); }
	T& target;
};


class DCtxLocker2
{
public:

	DCtxLocker2(DContext& c, const mat4d& m, DUnit* p);
	~DCtxLocker2();

	DContext& dc;
};

class DContext : public Object
{
public:

	enum
	{
		NX_NOOP,
		NX_TRIANGLES,
		NX_TRIANGLE_STRIP,
		NX_QUADS,
		NX_QUAD_STRIP,
		NX_POINTS,
		NX_LINES,
		NX_LINE_LOOP,

		NX_MASK		= (1 << 8) - 1,

		NX_CBOX		= 1 << 8,
		NX_SHOW		= 1 << 9,
		FLAG_SHP	= 1 << 0,
		FLAG_RCD	= 1 << 1,
		FLAG_SRC	= 1 << 2,
		FLAG_SHOW	= 1 << 3,
		FLAG_POST	= 1 << 4,
		FLAG_BBOX	= 1 << 5,

	};


	DContext();

	void Vertex(const vec3d& v){ m_tmpVertexs.push_back(cx.val*v); }
	void VertexArray(const vec3d* p, size_t n);
	void VertexArray(int* p, size_t n, const vec3d* v);

	virtual void Begin(int t);
	virtual void End();
	virtual void Commit();

	void Translate(const vec3d& v);
	void Scale(const vec3d& v);
	void Scale(double v);
	void Rotate(double d, const vec3d& v);

	void LoadIdentity();
	void PushMatrix(){ cx.push(); }
	void PopMatrix(){ cx.pop(); }
	void LoadMatrix(const mat4d& m){ cx.val = m; }
	void MultMatrix(const mat4d& m){ cx.val = cx.val*m; }
	void MultMatrix(const mat4d& m, double u){ cx.val.MultMatrix(m, u); }

	const mat4d& GetMatrix(){ return cx.val; }

	void StlDomain(DataPtrT<DStlDomain> model);


	virtual void B3Box_Points(const box3d& b3, int t = 0);

	virtual void EnterDocument();
	virtual void LeaveDocument();

	virtual void RenderObject(DObject* p);

	template<typename T>
	void RenderObject(DataPtrT<T>& p){ RenderObject(p.get()); }

	BitFlags flags;

	DCtxStateUnit cu;
	DCtxStateMatrix cx;
	DCtxStateMaterial cm;

protected:
	arr_1t<vec3d> m_tmpVertexs;
	int m_tmpType;
};

template<typename P>
class DContextTriangle : public DContext
{
public:

	void End()
	{

		P dc(*this);

		dc.Enter();

		if (m_tmpType == NX_TRIANGLES)
		{
			size_t n = m_tmpVertexs.size() / 3;
			for (size_t i = 0; i<n; i++)
			{
				dc.Handle(&m_tmpVertexs[i * 3 + 0]);
			}
		}
		else if (m_tmpType == NX_TRIANGLE_STRIP || m_tmpType == NX_QUAD_STRIP)
		{
			size_t n = m_tmpVertexs.size() - 2;
			for (size_t i = 0; i<n; i++)
			{
				if (i % 2 == 0)
				{
					dc.Handle(&m_tmpVertexs[i + 0]);
				}
				else
				{
					dc.HandleInverse(&m_tmpVertexs[i + 0]);
				}
			}
		}
		else if (m_tmpType == NX_QUADS)
		{
			size_t n = m_tmpVertexs.size() / 4;
			for (size_t i = 0; i<n; i++)
			{
				dc.Handle(&m_tmpVertexs[i * 4 + 0]);
				dc.HandleTriangle(m_tmpVertexs[i * 4 + 0], m_tmpVertexs[i * 4 + 3], m_tmpVertexs[i * 4 + 2]);
			}
		}

		dc.Leave();

		DContext::End();
	}
};

EW_LEAVE

#endif

