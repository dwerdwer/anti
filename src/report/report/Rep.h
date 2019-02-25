#include <iostream>
#include <unistd.h>
#include <vector>
#include <map>
#include <stdio.h>
#include "Mutex.h"
#include <stdlib.h>
#include "sqlite3.h"
using namespace std;

#define TYPE_NUM 0
#define TYPE_STR 1

struct ValNode
{
	string mName;
	string mValue;
	int mType;
	ValNode() {
	}
	ValNode(string nameString, string valueString, int valueType) {
		mName = nameString;
		mValue = valueString;
		mType = valueType;
	}
};

struct RecNode
{
	vector<ValNode> mVecData;
};

class DB
{
private:
	//Mutex mMutex;
	std::map<std::string,void *> mDatabaseSet;
	sqlite3 * getDatabase(string dbString){
		return (sqlite3 *)mDatabaseSet[dbString];
	}
	sqlite3 * putDatabase(string dbString){
		if (dbString.length() > 0){
			sqlite3 *pDB;
			if (!sqlite3_open(dbString.c_str(), &pDB)) {
				mDatabaseSet[dbString] = (void *)pDB;
				return (sqlite3 *)mDatabaseSet[dbString];
			}else{
				printf("Can't open database: %s\n", sqlite3_errmsg(pDB));
			}
		}
		return NULL;
	}
public:

	DB() {
	}

	~DB() {
		if (mDatabaseSet.size() > 0){
			sqlite3 *ptrDB;
			std::map<std::string, void *>::iterator it = mDatabaseSet.begin();
			while (it != mDatabaseSet.end()) {
				ptrDB = (sqlite3 *)(it->second);
				sqlite3_close(ptrDB);
				++it;
			}
		}
	}

	static DB * getInstance(){
		static DB db;
		return &db;
	}

	static int ExecProc(void *pData, int argc, char **argv, char **pColName) {
		if (pData) {
			vector<RecNode> * ptrRec = (vector<RecNode> *)pData;
			RecNode recNode;
			for (int index = 0; index<argc; index++) {
				recNode.mVecData.push_back(ValNode(pColName[index], argv[index], -1));
			}
			ptrRec->push_back(recNode);
		}
		return 0;
	}

	int doExcute(string dbString, string sqlString) {
		int nFlag = 0;
		if (dbString.length() > 0 && sqlString.length() > 0) {
			//printf("\nSql:: %s\n", sqlString.c_str());
			//mMutex.enter();
			sqlite3 *pDB = getDatabase(dbString);
			if (pDB){
				char * pError = 0;
				if (sqlite3_exec(pDB, sqlString.c_str(), NULL, NULL, &pError) != SQLITE_OK) {
					printf("SQL error: %s\n", pError);
					sqlite3_free(pError);
				}
				else {
					nFlag = 1;
				}
			}else{
				pDB = putDatabase(dbString);
				if (pDB){
					char * pError = 0;
					if (sqlite3_exec(pDB, sqlString.c_str(), NULL, NULL, &pError) != SQLITE_OK) {
						printf("SQL error: %s\n", pError);
						sqlite3_free(pError);
					}
					else {
						nFlag = 1;
					}
				}
			}
			//mMutex.leave();
		}
		return nFlag;
	}

	int doExcute(string dbString, string sqlString,vector<RecNode> & vecRec) {
		int nFlag = 0;
		if (dbString.length() > 0 && sqlString.length() > 0) {
			//printf("\nSql:: %s\n", sqlString.c_str());
			//mMutex.enter();
			sqlite3 *pDB = getDatabase(dbString);
			if (pDB){
				char * pError = 0;
				if (sqlite3_exec(pDB, sqlString.c_str(), DB::ExecProc, (void *)&vecRec, &pError) != SQLITE_OK) {
					printf("SQL error: %s\n", pError);
					sqlite3_free(pError);
				}
				else {
					nFlag = 1;
				}
			}else{
				pDB = putDatabase(dbString);
				if (pDB){
					char * pError = 0;
					if (sqlite3_exec(pDB, sqlString.c_str(), DB::ExecProc, (void *)&vecRec, &pError) != SQLITE_OK) {
						printf("SQL error: %s\n", pError);
						sqlite3_free(pError);
					}
					else {
						nFlag = 1;
					}
				}
			}
			//mMutex.leave();
		}
		return nFlag;
	}
};
/*
class DB
{
private:
	//Mutex mMutex;
public:

	DB() {
	}

	~DB() {
	}

	static DB * getInstance(){
		static DB db;
		return &db;
	}

	static int ExecProc(void *pData, int argc, char **argv, char **pColName) {
		if (pData) {
			vector<RecNode> * ptrRec = (vector<RecNode> *)pData;
			RecNode recNode;
			for (int index = 0; index<argc; index++) {
				recNode.mVecData.push_back(ValNode(pColName[index], argv[index], -1));
			}
			ptrRec->push_back(recNode);
		}
		return 0;
	}

	int doExcute(string dbString, string sqlString) {
		int nFlag = 0;
		if (dbString.length() > 0 && sqlString.length() > 0) {
			//printf("\nSql:: %s\n", sqlString.c_str());
			//mMutex.enter();
			sqlite3 *pDB;
			if (sqlite3_open(dbString.c_str(), &pDB)) {
				printf("Can't open database: %s\n", sqlite3_errmsg(pDB));
			}
			else {
				char * pError = 0;
				if (sqlite3_exec(pDB, sqlString.c_str(), NULL, NULL, &pError) != SQLITE_OK) {
					printf("SQL error: %s\n", pError);
					sqlite3_free(pError);
				}
				else {
					nFlag = 1;
				}
				sqlite3_close(pDB);
			}
			//mMutex.leave();
		}
		return nFlag;
	}

	int doExcute(string dbString, string sqlString,vector<RecNode> & vecRec) {
		int nFlag = 0;
		if (dbString.length() > 0 && sqlString.length() > 0) {
			//printf("\nSql:: %s\n", sqlString.c_str());
			//mMutex.enter();
			sqlite3 *pDB;
			if (sqlite3_open(dbString.c_str(), &pDB)) {
				printf("Can't open database: %s\n", sqlite3_errmsg(pDB));
			}
			else {
				char * pError = 0;
				if (sqlite3_exec(pDB, sqlString.c_str(), DB::ExecProc, (void *)&vecRec, &pError) != SQLITE_OK) {
					printf("SQL error: %s\n", pError);
					sqlite3_free(pError);
				}
				else {
					nFlag = 1;
				}
				sqlite3_close(pDB);
			}
			//mMutex.leave();
		}
		return nFlag;
	}
};
*/
