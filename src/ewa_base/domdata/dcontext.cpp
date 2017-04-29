
#include "ewa_base/domdata/dcontext.h"
#include "ewa_base/domdata/dobject.h"
#include "ewa_base/domdata/dunit.h"
#include "ewa_base/basic/scanner_helper.h"

EW_ENTER

void DCtxStateMaterial::init(DObject* p,DObject* b)
{
	arr.clear();
	stk.clear();

	stk.push_back(p);
	ref = val = arr.find2(p);


	arr.insert(b);
}

void DCtxStateUnit::init(DUnit* p)
{
	stk.clear();
	ref = val = p->ku;
	val /= ref;
}

void DCtxStateUnit::push(DUnit* p)
{
	if (p == NULL)
	{
		basetype::push();
	}
	else
	{
		KUnit ku = p->ku;
		ku /= ref;
		basetype::push(ku);
	}
}



void DCtxStateMaterial::pop()
{
	stk.pop_back();
}

void DCtxStateMaterial::push(DObject* p)
{
	stk.push_back(p ? p : stk.back());
	val = arr.find2(stk.back()->DecorateWithM4(m4));
}

bool read_vertext(const char* &ptr, vec3d &pos)
{
	while (1)
	{
		for (; *ptr != 'v'; ptr++)
		{
			if (!*ptr) return false;
		}

		if (memcmp(ptr, "vertex", 6) != 0) continue;
		ptr += 6;

		if (!ScannerHelper<const char*>::read_number(ptr, pos[0]))
		{
			return false;
		}
		if (!ScannerHelper<const char*>::read_number(ptr, pos[1]))
		{
			return false;
		}
		if (!ScannerHelper<const char*>::read_number(ptr, pos[2]))
		{
			return false;
		}
		return true;
	}
}

bool LoadStlText(StringBuffer<char>& sb, DStlModel& stl)
{

	const char* ptr = sb.c_str();
	while (*ptr && *ptr != '\n') ptr++;

	int nd = 0;

	DataPtrT<DStlDomainStl> pdomain(new DStlDomainStl);
	arr_1t<vec3i>& triangles(pdomain->triangles);
	indexer_set<vec3d> vertex_indexer;
	vec3d pos;
	vec3i tri;


	while (read_vertext(ptr, pos))
	{
		tri[nd++] = vertex_indexer.find2(pos);
		if (nd == 3)
		{
			triangles.push_back(tri);
			nd = 0;
		}
	}

	if (*ptr != 0)
	{
		return false;
	}

	if (nd != 0)
	{
		return false;
	}

	vertex_indexer.swap_array(pdomain->vertexs);
	stl.children.clear();
	stl.children.push_back(pdomain);

	return true;
}

bool LoadStlBin(StringBuffer<char>& sb, DStlModel& stl)
{
	static const int HEAD_SIZE = 80;
	static const int NUM_SIZE = 4;
	static const int SIZE_OF_FACET = 50;
	static const int STL_MIN_SIZE = 284;

	if (sb.size()<HEAD_SIZE) return false;
	size_t sz = sb.size();
	size_t bs = sz - HEAD_SIZE - NUM_SIZE;
	if ((bs%SIZE_OF_FACET) != 0 || sz<STL_MIN_SIZE)
	{
		return false;
	}
	int num_faces = bs / SIZE_OF_FACET;

	const char* p = sb.c_str() + HEAD_SIZE;
	if (num_faces != *(int*)(p))
	{
		return false;
	}
	p += NUM_SIZE;

	typedef tiny_vec<float, 3> vec3f;

	DataPtrT<DStlDomainStl> pdomain(new DStlDomainStl);
	arr_1t<vec3i>& triangles(pdomain->triangles);
	indexer_set<vec3f> vertex_indexer;
	vec3f pos;
	vec3i tri;

	triangles.resize(num_faces);

	typedef vec3f tri4[4];

	for (int i = 0; i<num_faces; i++)
	{
		vec3i& tri(triangles[i]);
		tri4& facet_triangles(*(tri4*)p);

		tri[0] = vertex_indexer.find2(facet_triangles[1]);
		tri[1] = vertex_indexer.find2(facet_triangles[2]);
		tri[2] = vertex_indexer.find2(facet_triangles[3]);

		p = p + 50;
	}

	pdomain->vertexs.assign(vertex_indexer.begin(), vertex_indexer.end());

	stl.children.clear();
	stl.children.push_back(pdomain);
	return true;
}

bool DStlModel::LoadStl(const String& file)
{
	StringBuffer<char> sb;
	if (!sb.load(file, FILE_TYPE_BINARY))
	{
		return false;
	}
	if (sb.empty()) return false;

	if (LoadStlBin(sb, *this))
	{
		return true;
	}
	return LoadStlText(sb, *this);
}

DataPtrT<DStlDomainStl> DStlModel::GetUnitObject(const String& s, int d)
{
	if (s == "cube")
	{
		static DataPtrT<DStlDomainStl> pStl;
		if (!pStl)
		{
			pStl.reset(new DStlDomainStl);
			arr_1t<vec3d>& vertexs(pStl->vertexs);
			arr_1t<vec3i>& triangles(pStl->triangles);
			vertexs.push_back(vec3d(-0.5, -0.5, -0.5));
			vertexs.push_back(vec3d(+0.5, -0.5, -0.5));
			vertexs.push_back(vec3d(+0.5, +0.5, -0.5));
			vertexs.push_back(vec3d(-0.5, +0.5, -0.5));
			vertexs.push_back(vec3d(-0.5, -0.5, +0.5));
			vertexs.push_back(vec3d(+0.5, -0.5, +0.5));
			vertexs.push_back(vec3d(+0.5, +0.5, +0.5));
			vertexs.push_back(vec3d(-0.5, +0.5, +0.5));
			triangles.push_back(vec3i(0, 2, 1));
			triangles.push_back(vec3i(0, 3, 2));
			triangles.push_back(vec3i(4, 5, 6));
			triangles.push_back(vec3i(4, 6, 7));
			triangles.push_back(vec3i(0, 1, 4));
			triangles.push_back(vec3i(1, 5, 4));
			triangles.push_back(vec3i(1, 2, 5));
			triangles.push_back(vec3i(2, 6, 5));
			triangles.push_back(vec3i(2, 3, 6));
			triangles.push_back(vec3i(3, 7, 6));
			triangles.push_back(vec3i(3, 0, 7));
			triangles.push_back(vec3i(0, 4, 7));
		}
		return pStl;
	}
	else if (s == "cylinder")
	{
		return NULL;
	}
	else
	{
		return NULL;
	}
}

DCtxLocker2::DCtxLocker2(DContext& c, const mat4d& m, DUnit* p) :dc(c)
{
	dc.cu.push(p);

	mat4d m4 = dc.cx.val;

	double s = dc.cu.val.ul / dc.cu.stk.back().ul;
	if (s != 1.0)
	{
		m4.Scale(s);
	}

	m4.MultMatrix(m);

	dc.cx.push(m4);

}

DCtxLocker2::~DCtxLocker2()
{
	dc.cx.pop();
	dc.cu.pop();
}

DContext::DContext() :cm(cx.val)
{
	m_tmpType = NX_NOOP;
	LoadIdentity();
}


void DContext::RenderObject(DObject* p)
{
	if (!p) return;
	p->DoRender(*this);
}

void DContext::LoadIdentity()
{
	cx.val.LoadIdentity();
}

void DContext::Translate(const vec3d& v)
{
	cx.val.Translate(v);
}

void DContext::Scale(const vec3d& v)
{
	cx.val.Scale(v);
}

void DContext::Scale(double v)
{
	cx.val.Scale(v);
}

void DContext::Rotate(double d, const vec3d& v)
{
	cx.val.Rotate(d, v);
}


void DContext::B3Box_Points(const box3d& b3, int t)
{
	DContext& dc(*this);
	dc.Begin(t | DContext::NX_POINTS);
	dc.Vertex(vec3d(b3.lo[0], b3.lo[1], b3.lo[2]));
	dc.Vertex(vec3d(b3.lo[0], b3.lo[1], b3.hi[2]));
	dc.Vertex(vec3d(b3.lo[0], b3.hi[1], b3.lo[2]));
	dc.Vertex(vec3d(b3.lo[0], b3.hi[1], b3.hi[2]));
	dc.Vertex(vec3d(b3.hi[0], b3.lo[1], b3.lo[2]));
	dc.Vertex(vec3d(b3.hi[0], b3.lo[1], b3.hi[2]));
	dc.Vertex(vec3d(b3.hi[0], b3.hi[1], b3.lo[2]));
	dc.Vertex(vec3d(b3.hi[0], b3.hi[1], b3.hi[2]));
	dc.End();
}


void DContext::Begin(int t)
{
	if (m_tmpType != NX_NOOP)
	{
		System::LogError("DContex::Begin m_tmpType=%d", m_tmpType);
	}

	m_tmpVertexs.clear();
	m_tmpType = t;
}

void DContext::End()
{
	m_tmpType = NX_NOOP;
	m_tmpVertexs.clear();
}

void DContext::Commit()
{

}

void DContext::EnterDocument()
{

}

void DContext::LeaveDocument()
{

}

void DContext::VertexArray(const vec3d* p, size_t n)
{
	for (size_t i = 0; i<n; i++)
	{
		m_tmpVertexs.push_back(cx.val*p[i]);
	}
}

void DContext::VertexArray(int* p, size_t n, const vec3d* v)
{
	for (size_t i = 0; i<n; i++)
	{
		m_tmpVertexs.push_back(cx.val*v[p[i]]);
	}
}

void DContext::StlDomain(DataPtrT<DStlDomain> pdomain)
{
	if (!pdomain) return;

	if (DStlDomainStl* p = dynamic_cast<DStlDomainStl*>(pdomain.get()))
	{
		const vec3d* pv = p->vertexs.data();
		const vec3i* pt = p->triangles.data();
		size_t nt = p->triangles.size();

		Begin(NX_TRIANGLES);
		VertexArray((int*)pt, nt * 3, pv);
		End();
	}
	else if (DStlDomainGrp* p = dynamic_cast<DStlDomainGrp*>(pdomain.get()))
	{
		for (size_t i = 0; i<p->children.size(); i++)
		{
			StlDomain(p->children[i]);
		}
	}

}


EW_LEAVE
