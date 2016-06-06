#ifndef __H_EW_BASIC_LANGUAGE__
#define __H_EW_BASIC_LANGUAGE__

#include "ewa_base/config.h"
#include "ewa_base/basic/pointer.h"
#include "ewa_base/basic/object.h"
#include "ewa_base/collection/indexer_map.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE String;

class DLLIMPEXP_EWA_BASE Language
{
public:
	Language();
	Language& operator=(const Language& o);

	// Init with a file, po file or files that saved by Language::Save
	bool AddCatalog(const String& file);

	bool AddCatalog(const indexer_map<String,String>& mp);

	bool GetCatalog(indexer_map<String,String>& mp);

	bool Save(const String& file);

	const String& Translate(const String& msg) const;

	const String& Translate(const String& msg,const String& def) const;

	void Clear();

	bool SetLanguage(const String& s);

	String GetLanguage();

	static Language& current();

	static arr_1t<String> GetLanguages();

private:
	DataPtrT<ObjectData> m_pimpl;
	void _language_updated();
};

EW_LEAVE
#endif
