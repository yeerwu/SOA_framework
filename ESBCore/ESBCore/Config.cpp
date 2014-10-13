#include "Config.h"


int IniReader::load(string _file)
{
	filename_ = _file;
	return 0;
}

string IniReader::read(string _name)
{
	if (items_.find(_name) == items_.end())
	{
		char szBuf[128];
		GetPrivateProfileString("Config", _name.c_str(), "", szBuf, 128, filename_.c_str());

		items_[_name] = szBuf;
	}
	return items_[_name];
}