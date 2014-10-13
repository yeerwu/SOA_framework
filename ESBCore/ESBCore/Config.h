#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <windows.h>
#include <string>
#include <map>

using namespace std;
/**
 * @class ESBConfig
 *
 * @brief read configuration items from ini files
 *
 * @note Before using this class you must call load method.
 *
 */
template <class T>
class ESBConfig
{
public:
	ESBConfig()
	{
		reader_ = new T;
	}

	~ESBConfig()
	{
		if (reader_)
			delete reader_;
	}

	int load(string _file)
	{
		if (reader_)
			return reader->load(_file);
	}

	string readItem(string _name)
	{
		if (reader_)
			return reader->read(_name_);
	}

protected:
	T* reader_;
};

class IniReader
{
public:
	IniReader() {}

	int load(string _file);
	string read(string _name);

protected:
	map<string, string> items_;
	string filename_;
};

typedef ESBConfig<IniReader> IniConfig;

#endif