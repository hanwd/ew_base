#ifndef __H_EW_JSON__
#define __H_EW_JSON__

#include "ewa_base/scripting/variant.h"

EW_ENTER

DLLIMPEXP_EWA_BASE Variant parse_json(const String& json,bool unescape_value=false);
DLLIMPEXP_EWA_BASE void to_json(const Variant& json,StringBuffer<char>& sb);

class DLLIMPEXP_EWA_BASE JsonWriter
{
public:
	JsonWriter(StringBuffer<char>&);

	void WriteName(const String& name);

	void WriteValue(const Variant& value);
	void WriteValue(const String& value);
	void WriteValue(double value);
	void WriteValue(int64_t value);
	void WriteValue(bool value);
	void WriteValue(const VariantTable& value);
	void WriteValue(const dcomplex& value);

	template<typename T>
	void WriteArray(T* p,size_t n)
	{
		sb<<"["<<"\r\n";
		{
			LockState<String> lock(tb,tb+"\t");
			for(size_t i=0;i<n;i++)
			{
				sb<<tb;
				WriteValue(p[i]);
				if(i+1<n) sb<<",";
				sb<<"\r\n";
			}		
		}
		sb<<tb<<"]";
	}


	template<typename T>
	void WriteValue(const arr_xt<T>& value)
	{
		WriteArray(value.data(),value.size());
	}

	template<typename T>
	void WriteValue(const arr_1t<T>& value)
	{
		WriteArray(value.data(),value.size());
	}

	template<typename T>
	void Write(const String& name,const T& value,bool prop_end=false)
	{
		WriteName(name);
		WriteValue(value);
		if(prop_end) sb<<",";
		sb<<"\r\n";
	}


	StringBuffer<char>& sb;
	String tb;
};

EW_LEAVE
#endif
