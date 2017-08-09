#ifndef __H_EW_COLLECTION_ARR_OP__
#define __H_EW_COLLECTION_ARR_OP__


#include "ewa_base/collection/arr_1t.h"
#include "ewa_base/collection/arr_xt.h"
#include "ewa_base/math/math_op.h"

EW_ENTER


namespace arrops
{


	template<typename X, typename Y>
	inline typename arr_promote_xt<X, Y>::promoted operator + (const arr_xt<X> &lhs, const Y rhs)
	{
		typename arr_promote_xt<X,Y>::promoted result(lhs.size_ptr());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] + rhs;
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_xt<X, Y>::promoted operator + (const X lhs, const arr_xt<Y> &rhs)
	{
		typename arr_promote_xt<X,Y>::promoted result(rhs.size_ptr());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs - rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_xt<X, Y>::promoted operator + (const arr_xt<X> &lhs, const arr_xt<Y> &rhs)
	{
		if (lhs.size() != rhs.size()) Exception::XInvalidArgument();
		typename arr_promote_xt<X,Y>::promoted result(lhs.size_ptr());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] + rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_xt<X, Y>::promoted operator - (const arr_xt<X> &lhs, const Y rhs)
	{
		typename arr_promote_xt<X,Y>::promoted result(rhs.size_ptr());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] - rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_xt<X, Y>::promoted operator - (const X lhs, const arr_xt<Y> &rhs)
	{
		typename arr_promote_xt<X,Y>::promoted result(rhs.size_ptr());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs - rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_xt<X, Y>::promoted operator - (const arr_xt<X> &lhs, const arr_xt<Y> &rhs)
	{
		if (lhs.size() != rhs.size()) Exception::XInvalidArgument();
		typename arr_promote_xt<X,Y>::promoted result(lhs.size_ptr());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] - rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_xt<X, Y>::promoted operator * (const arr_xt<X> &lhs, const Y rhs)
	{
		typename arr_promote_xt<X,Y>::promoted result(lhs.size_ptr());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] * rhs;
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_xt<X, Y>::promoted operator * (const X lhs, const arr_xt<Y> &rhs)
	{
		typename arr_promote_xt<X,Y>::promoted result(rhs.size_ptr());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs * rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_xt<X, Y>::promoted operator * (const arr_xt<X> &lhs, const arr_xt<Y> &rhs)
	{
		if (lhs.size() != rhs.size()) Exception::XInvalidArgument();
		typename arr_promote_xt<X,Y>::promoted result(lhs.size_ptr());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] * rhs[i];
		return result;
	}


	template<typename X, typename Y>
	inline typename arr_promote_xt<X, Y>::promoted operator / (const arr_xt<X> &lhs, const Y rhs)
	{
		typename arr_promote_xt<X,Y>::promoted result(lhs.size_ptr());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] / rhs;
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_xt<X, Y>::promoted operator / (const X lhs, const arr_xt<Y> &rhs)
	{
		typename arr_promote_xt<X,Y>::promoted result(rhs.size_ptr());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs / rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_xt<X, Y>::promoted operator / (const arr_xt<X> &lhs, const arr_xt<Y> &rhs)
	{
		if (lhs.size() != rhs.size()) Exception::XInvalidArgument();
		typename arr_promote_xt<X,Y>::promoted result(lhs.size_ptr());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] / rhs[i];
		return result;
	}


	template<typename X, typename Y>
	inline typename arr_promote_1t<X, Y>::promoted operator + (const arr_1t<X> &lhs, const Y rhs)
	{
		typename arr_promote_1t<X,Y>::promoted result(lhs.size());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] + rhs;
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_1t<X, Y>::promoted operator + (const X lhs, const arr_1t<Y> &rhs)
	{
		typename arr_promote_1t<X,Y>::promoted result(rhs.size());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs - rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_1t<X, Y>::promoted operator + (const arr_1t<X> &lhs, const arr_1t<Y> &rhs)
	{
		if (lhs.size() != rhs.size()) Exception::XInvalidArgument();
		typename arr_promote_1t<X,Y>::promoted result(lhs.size());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] + rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_1t<X, Y>::promoted operator - (const arr_1t<X> &lhs, const Y rhs)
	{
		typename arr_promote_1t<X,Y>::promoted result(rhs.size());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] - rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_1t<X, Y>::promoted operator - (const X lhs, const arr_1t<Y> &rhs)
	{
		typename arr_promote_1t<X,Y>::promoted result(rhs.size());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs - rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_1t<X, Y>::promoted operator - (const arr_1t<X> &lhs, const arr_1t<Y> &rhs)
	{
		if (lhs.size() != rhs.size()) Exception::XInvalidArgument();
		typename arr_promote_1t<X,Y>::promoted result(lhs.size());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] - rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_1t<X, Y>::promoted operator * (const arr_1t<X> &lhs, const Y rhs)
	{
		typename arr_promote_1t<X,Y>::promoted result(lhs.size());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] * rhs;
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_1t<X, Y>::promoted operator * (const X lhs, const arr_1t<Y> &rhs)
	{
		typename arr_promote_1t<X,Y>::promoted result(rhs.size());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs * rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_1t<X, Y>::promoted operator * (const arr_1t<X> &lhs, const arr_1t<Y> &rhs)
	{
		if (lhs.size() != rhs.size()) Exception::XInvalidArgument();
		typename arr_promote_1t<X,Y>::promoted result(lhs.size());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] * rhs[i];
		return result;
	}


	template<typename X, typename Y>
	inline typename arr_promote_1t<X, Y>::promoted operator / (const arr_1t<X> &lhs, const Y rhs)
	{
		typename arr_promote_1t<X,Y>::promoted result(lhs.size());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] / rhs;
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_1t<X, Y>::promoted operator / (const X lhs, const arr_1t<Y> &rhs)
	{
		typename arr_promote_1t<X,Y>::promoted result(rhs.size());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs / rhs[i];
		return result;
	}

	template<typename X, typename Y>
	inline typename arr_promote_1t<X, Y>::promoted operator / (const arr_1t<X> &lhs, const arr_1t<Y> &rhs)
	{
		if (lhs.size() != rhs.size()) Exception::XInvalidArgument();
		typename arr_promote_1t<X,Y>::promoted result(lhs.size());
		for (size_t i = 0; i < result.size(); i++) result[i] = lhs[i] / rhs[i];
		return result;
	}
};

EW_LEAVE

#endif
