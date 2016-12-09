#ifndef __EWA_MATH_DEF_H__
#define __EWA_MATH_DEF_H__

#include "ewa_base/math/tiny_cpx.h"
#include "ewa_base/math/tiny_vec.h"
#include "ewa_base/math/tiny_mat.h"
#include "ewa_base/math/tiny_box.h"

#define M_LIGHTSPEED 2.99792458e8

EW_ENTER


typedef tiny_vec<int32_t,3> vec3i;
typedef tiny_vec<double,3> vec3d;
typedef tiny_vec<double,2> vec2d;
typedef tiny_vec<float,3> vec3f;
typedef tiny_vec<float,2> vec2f;

typedef tiny_box<int32_t,3> box3i;
typedef tiny_box<double,3> box3d;
typedef tiny_box<float,3> box3f;

typedef type_mat<double,4,4> mat4d;


typedef tiny_storage<String,3> vec3s;
typedef tiny_storage<String,2> vec2s;

class box3s
{
public:
	vec3s lo,hi;
};

bool is_nan(double d);
bool is_nan(float d);

template<typename T>
class DLLIMPEXP_EWA_BASE numeric_trait_floating
{
public:
	typedef T type;
	static T min_value();
	static T max_value();
	static T inf_value();
	static T nan_value();
};

template<typename T>
class DLLIMPEXP_EWA_BASE numeric_trait_integer
{
public:
	typedef T type;
	static T min_value();
	static T max_value();
};

template<typename T>
class DLLIMPEXP_EWA_BASE numeric_trait : public tl::meta_if<tl::integer_type::id<T>::value!=-1, numeric_trait_integer<T>, numeric_trait_floating<T> >::type
{
public:

};

EW_LEAVE
#endif
