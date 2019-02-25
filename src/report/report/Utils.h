
#ifndef _UTILS_
#define _UTILS_
#include <iostream>
#include <string.h>
using namespace std;

class Utils
{
public:
    //从给定路径中取出文件名
	static string getFileFromPath(string path) {
		string nameString = "";
		if (path.length() > 0) {
			size_t index = path.find_last_of("/");
			if (index != string::npos) {
				nameString = path.substr(index +1);
			}
		}
		return nameString;
	}
    
};

#endif
