
#ifndef __H_EW_COLLECTION_RBT_TREE_POLICY__
#define __H_EW_COLLECTION_RBT_TREE_POLICY__

#include "ewa_base/collection/detail/bst_tree_node.h"

EW_ENTER

typedef char rbt_color_type;

enum
{
	RBT_COLOR_RED,		// red is default
	RBT_COLOR_BLACK,
};

template<typename K,typename V,typename C>
class rbt_trait : public bst_trait<K,V,C,rbt_color_type>
{
public:

	typedef bst_node<K,V,rbt_color_type> node_type;

	static bool is_red(node_type* node)
	{
		return node&&node->extra==RBT_COLOR_RED;
	}

	static bool is_black(node_type* node)
	{
		return !node||node->extra==RBT_COLOR_BLACK;
	}

	static void mark_red(node_type* node)
	{
		node->extra=RBT_COLOR_RED;
	}

	static void mark_black(node_type* node)
	{
		if(node) node->extra=RBT_COLOR_BLACK;
	}

	static void mark(node_type* node,node_type* pref)
	{
		node->extra=pref->extra;
	}

	static void mark_inv(node_type* node)
	{
		node->extra=node->extra==RBT_COLOR_RED?RBT_COLOR_BLACK:RBT_COLOR_RED;
	}

	template<typename T>
	static void adjust_insert(T& t,node_type* n);

	template<typename T>
	static void delete_one_child(T& t,node_type* n);

	template<typename T>
	static bool bst_validate(T& t);
};

template<typename T>
class rbt_tree : public T
{
public:

	typedef typename T::bst_policy rbt_policy;
	typedef typename T::node_type node_type;


	bool bst_validate()
	{
		return rbt_depth()>=0;
	}

	intptr_t rbt_depth()
	{
		return rbt_depth_real(this->m_pRoot);
	}


	intptr_t rbt_depth_real(node_type* p)
	{
		if(p==NULL) return 0;
		intptr_t p1=rbt_depth_real(p->child1);
		intptr_t p2=rbt_depth_real(p->child2);
		if(p1<0||p2<0||p1!=p2)
		{
			return -1;
		}
		return p1+rbt_policy::is_black(p)?1:0;
	}

	void adjust_insert_case1(node_type* n)
	{
		node_type* p=n->parent;
		if(!p)
		{
			this->m_pRoot=n;
			rbt_policy::mark_black(n);
			return;
		}
		adjust_insert_case2(p,n);
	}

	void adjust_insert_case2(node_type* p,node_type* n)
	{
		if(rbt_policy::is_black(p))
		{
			return;
		}
		adjust_insert_case3(p,n);
	}


	void adjust_insert_case3(node_type* p,node_type* n)
	{
		node_type* g=p->parent;
		if( rbt_policy::is_red(g->child1) && rbt_policy::is_red(g->child2))
		{
			rbt_policy::mark_black(g->child1);
			rbt_policy::mark_black(g->child2);
			rbt_policy::mark_red(g);
			adjust_insert_case1(g);
		}
		else
		{
			adjust_insert_case4(p,n);
		}
	}


	void adjust_insert_case4(node_type* p,node_type* n)
	{
		node_type* g=p->parent;
		if(n==p->child2 && p==g->child1)
		{
			this->rotate_left(p);
			n=n->child1;
		}
		else if(n==p->child1 && p==g->child2)
		{
			this->rotate_right(p);
			n=n->child2;
		}
		adjust_insert_case5(n);
	}


	void adjust_insert_case5(node_type* n)
	{
		node_type* p=n->parent;
		node_type* g=p->parent;
		rbt_policy::mark_black(p);
		rbt_policy::mark_red(g);

		if(n==p->child1 && p==g->child1)
		{
			this->rotate_right(g);
		}
		else if(n==p->child2 && p==g->child2)
		{
			this->rotate_left(g);
		}
		else
		{
			System::LogWarning("bst_tree<...>::adjust_insert_case5 failed!");
		}
	}

	void delete_one_child(node_type *n)
	{

		node_type *child=n->child1!=NULL?n->child1:n->child2;
		node_type* p=n->parent;

		if (p == NULL)
		{
			if(child!=NULL)
			{
				this->m_pRoot=child;
				this->m_pRoot->parent=NULL;
				rbt_policy::mark_black(this->m_pRoot);
			}
			else
			{
				this->m_pRoot =NULL;
			}
		}
		else
		{
			if (p->child1 == n)
			{
				p->child1 = child;
			}
			else
			{
				p->child2 = child;
			}

			if(child!=NULL)
			{
				child->parent=p;

				if (rbt_policy::is_black(n))
				{
					if(rbt_policy::is_red(child))
					{
						rbt_policy::mark_black(child);
					}
					else
					{
						adjust_delete_case1(child);
					}
				}
			}
			else if(rbt_policy::is_black(n))
			{
				node_type* s=p->child1;
				if(s==NULL) s=p->child2;
				adjust_delete_case2(p,s);
			}
		}

		this->bst_destroy(n);
	}



	void adjust_delete_case1(node_type *n)
	{
		node_type* p=n->parent;
		if(!p)
		{
			EW_ASSERT(rbt_policy::is_black(n));
			return;
		}
		adjust_delete_case2(p,p->child1==n?p->child2:p->child1);
	}


	void adjust_delete_case2(node_type* p,node_type *s)
	{
		if(rbt_policy::is_red(s))
		{
			rbt_policy::mark_black(s);
			rbt_policy::mark_red(p);

			if(s==p->child2)
			{
				this->rotate_left(p);
				s=p->child2;
			}
			else
			{
				this->rotate_right(p);
				s=p->child1;
			}
		}

		adjust_delete_case3(p,s);
	}


	void adjust_delete_case3(node_type* p,node_type *s) // case3 and case4
	{
		EW_ASSERT(rbt_policy::is_black(s));

		if(rbt_policy::is_black(s->child1) && rbt_policy::is_black(s->child2))
		{
			if(rbt_policy::is_black(p))
			{
				rbt_policy::mark_red(s);
				adjust_delete_case1(p);
			}
			else
			{
				rbt_policy::mark_red(s);
				rbt_policy::mark_black(p);
			}
		}
		else
		{
			adjust_delete_case5(p,s);
		}

	}


	void adjust_delete_case5(node_type* p,node_type *s)
	{
		EW_ASSERT(rbt_policy::is_black(s));
		if(s==p->child2 && rbt_policy::is_black(s->child2) && rbt_policy::is_red(s->child1))
		{
			rbt_policy::mark_red(s);
			rbt_policy::mark_black(s->child1);
			this->rotate_right(s);
			s=p->child2;
		}
		else if(s==p->child1 && rbt_policy::is_black(s->child1) && rbt_policy::is_red(s->child2))
		{
			rbt_policy::mark_red(s);
			rbt_policy::mark_black(s->child2);
			this->rotate_left(s);
			s=p->child1;
		}

		adjust_delete_case6(p,s);

	}

	void adjust_delete_case6(node_type* p,node_type *s)
	{
		rbt_policy::mark(s,p);
		rbt_policy::mark_black(p);
		if(s==p->child2)
		{
			rbt_policy::mark_black(s->child2);
			this->rotate_left(p);
		}
		else
		{
			rbt_policy::mark_black(s->child1);
			this->rotate_right(p);
		}
	}


};

template<typename K,typename V,typename C>
template<typename T>
void rbt_trait<K,V,C>::adjust_insert(T& t,node_type* n)
{
	mark_red(n);
	((rbt_tree<T>&)t).adjust_insert_case1(n);
}

template<typename K,typename V,typename C>
template<typename T>
void rbt_trait<K,V,C>::delete_one_child(T& t,node_type* n)
{
	((rbt_tree<T>&)t).delete_one_child(n);
}

template<typename K,typename V,typename C>
template<typename T>
bool rbt_trait<K,V,C>::bst_validate(T& t)
{
	return ((rbt_tree<T>&)t).bst_validate();
}

EW_LEAVE

#endif
