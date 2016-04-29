#ifndef __H_EW_BASIC_LANGUAGE__
#define __H_EW_BASIC_LANGUAGE__

#include "ewa_base/config.h"
#include "ewa_base/collection/indexer_map.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE String;

class DLLIMPEXP_EWA_BASE Language : public NonCopyable
{
public:
	Language();
	~Language();

	// Init with a file, po file or files that saved by Language::Save
	bool AddCatalog(const String& file);

	bool AddCatalog(const indexer_map<String,String>& mp);

	bool GetCatalog(indexer_map<String,String>& mp);

	bool Save(const String& file);

	// Translate string
	const String& Translate(const String& msg) const;

	const String& Translate(const String& msg,const String& def) const;

	void Clear();

	void Ref(Language& lang);

	static Language& current();

private:
	void* impl;
};

EW_LEAVE
#endif
