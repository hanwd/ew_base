#ifndef __H_EW_COLLECTION_AVL_TREE_POLICY__
#define __H_EW_COLLECTION_AVL_TREE_POLICY__

#include "ewa_base/collection/detail/bst_tree_node.h"

EW_ENTER

typedef int avl_factor_type;


template<typename K,typename V,typename C>
class avl_trait : public bst_trait<K,V,C,avl_factor_type>
{
public:

	typedef bst_node<K,V,avl_factor_type> node_type;

	template<typename T>
	static void adjust_insert(T& t,node_type* n);

	template<typename T>
	static void delete_one_child(T& t,node_type* n);

	template<typename T>
	static bool bst_validate(T& t);
};


template<typename T>
class avl_tree : public T
{
public:

	typedef T basetype;

	typedef typename T::bst_policy avl_policy;
	typedef typename T::node_type node_type;

	using basetype::rotate_left;
	using basetype::rotate_right;



	bool bst_validate()
	{
		return avl_depth()>=0;
	}

	intptr_t avl_depth()
	{
		return avl_depth_real(this->m_pRoot);
	}


	intptr_t avl_depth_real(node_type* p)
	{
		if(p==NULL) return 0;
		intptr_t p1=avl_depth_real(p->child1);
		intptr_t p2=avl_depth_real(p->child2);

		if(p1<0||p2<0||p1-p2>+1||p1-p2<-1)
		{
			return -1;
		}

		return std::max(p1,p2)+1;
	}

	// node n depth decreased by 1, reblance n's parent
	void blance_parent(node_type* n)
	{
		node_type* p=n->parent;
		if(!p) return;
		if(n==p->child1)
		{
			blance_child1(p);
		}
		else
		{
			blance_child2(p);
		}
	}

	// case avl_factor is +2
	template<bool d>
	void handle_unblance_gt(node_type* p)
	{
		EW_ASSERT(p->extra==2);
		node_type* n=p->child1;
		avl_factor_type f1=n->extra;

		if(f1>=0)
		{
			this->rotate_right(p);
			avl_factor_type fx=n->extra;
			n->extra=fx-1;
			p->extra=-n->extra;

			if(d && f1==1)
			{
				blance_parent(n);
			}

		}
		else
		{
			node_type* x=n->child2;
			this->rotate_left(n);
			this->rotate_right(p);

			int fx=x->extra;
			if(fx>=0)
			{
				n->extra=0;
				p->extra=-fx;
				x->extra=0;
			}
			else
			{
				n->extra=-fx;
				p->extra=0;
				x->extra=0;
			}

			if(d) blance_parent(x);
		}
	}

	// case avl_factor is -2
	template<bool d>
	void handle_unblance_lt(node_type* p)
	{
		EW_ASSERT(p->extra==-2);

		node_type* n=p->child2;
		avl_factor_type f1=n->extra;
		if(f1<=0)
		{
			this->rotate_left(p);
			avl_factor_type fx=n->extra;
			n->extra=fx+1;
			p->extra=-n->extra;

			if(d && f1==-1)
			{
				this->blance_parent(n);
			}
		}
		else
		{
			node_type* x=n->child1;
			int fx=x->extra;

			this->rotate_right(n);
			this->rotate_left(p);

			if(fx<=0)
			{
				n->extra=0;
				p->extra=-fx;
				x->extra=0;
			}
			else
			{
				n->extra=-fx;
				p->extra=0;
				x->extra=0;
			}

			if(d) blance_parent(x);
		}
	}

	void adjust_insert(node_type* n)
	{

		node_type* p=n->parent;
		if(!p)
		{
			this->m_pRoot=n;
			return;
		}

		while(1)
		{
			if(n==p->child1)
			{
				avl_factor_type f1=++p->extra;
				if(f1==0)
				{
					return;
				}
				else if(f1==2)
				{
					handle_unblance_gt<false>(p);
					return;
				}
				EW_ASSERT(f1==+1);
			}
			else
			{
				avl_factor_type f1=--p->extra;
				if(f1==0)
				{
					return;
				}
				else if(f1==-2)
				{
					handle_unblance_lt<false>(p);
					return;
				}
				EW_ASSERT(f1==-1);
			}

			n=p;p=p->parent;
			if(!p)
			{
				return;
			}
		}
	}

	// depth of n's child1 descreased, handle it
	void blance_child1(node_type *n)
	{

		avl_factor_type f1=--n->extra;
		if(f1==-1)
		{
			return;
		}
		if(f1==-2)
		{
			handle_unblance_lt<true>(n);
			return;
		}
		EW_ASSERT(f1==0);
		blance_parent(n);
	}

	// depth of n's child2 descreased, handle it
	void blance_child2(node_type *n)
	{
		avl_factor_type f1=++n->extra;

		if(f1==+1)
		{
			return;
		}
		if(f1==+2)
		{
			handle_unblance_gt<true>(n);
			return;
		}

		EW_ASSERT(f1==0);
		blance_parent(n);

	}


	void delete_one_child(node_type *n)
	{

		node_type *child=n->child1!=NULL?n->child1:n->child2;
		node_type* p=n->parent;

		if(!p)
		{
			this->m_pRoot=child;
			if(child) child->parent=NULL;
		}
		else
		{
			if(n==p->child1)
			{
				basetype::set_child1(p,child);
				blance_child1(p);
			}
			else
			{
				basetype::set_child2(p,child);
				blance_child2(p);
			}
		}

		this->bst_destroy(n);
	}

};

template<typename K,typename V,typename C>
template<typename T>
void avl_trait<K,V,C>::adjust_insert(T& t,node_type* n)
{
	n->extra=0;
	((avl_tree<T>&)t).adjust_insert(n);
}

template<typename K,typename V,typename C>
template<typename T>
void avl_trait<K,V,C>::delete_one_child(T& t,node_type* n)
{
	((avl_tree<T>&)t).delete_one_child(n);
}


template<typename K,typename V,typename C>
template<typename T>
bool avl_trait<K,V,C>::bst_validate(T& t)
{
	return ((avl_tree<T>&)t).bst_validate();
}

EW_LEAVE

#endif
