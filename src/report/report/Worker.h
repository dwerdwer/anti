
#pragma once
#include <iostream>
#include <string.h>
#include "Rep.h"
#include "Mutex.h"
#include "NetUtils.h"
#include "Tools.h"
#include "KVMessage.h"
using namespace std;

#define DATA_INFO 1
#define DATA_FILE 0
#define MAXSTRLEN 4096
#define DatabasePath "./data.db"
#define DatabaseTable "tabData"

#define TABSCRIPT "CREATE TABLE tabData (\
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\
    type INT NOT NULL,\
    addr CHAR (0, 128) NOT NULL,\
    data CHAR (0, 4096) NOT NULL,\
    time INTEGER NOT NULL,\
	flag INT NOT NULL,\
	other CHAR (0, 1024) );"

#define IDXSCRIPT "CREATE INDEX i_time ON tabData (\
	time COLLATE BINARY COLLATE BINARY DESC);"

struct ReqData{
	int nError;
	char mbsBuf[MAXSTRLEN];
};
struct StoreNode{
	int id;
	int type;
	char addr[128];
	char data[4096];
	long time;
	int flag;
	char other[1024];
};

class Worker{
	public:
		Worker(){
			mFlag = 1;
		}
		~Worker(){
			endWork();
		}
		static void ReqProc(int nFlag,char * pBuf,int nLength,int nTarget,void * pUsrData){

			cout<<"\r\n##########  Recv data ##########"<<endl;
			cout<<"falg:"<<nFlag<<endl;
			cout<<"length:"<<nLength<<endl;
			cout<<"target"<<nTarget<<endl;
			cout<<"data:"<<pBuf<<endl;
			cout<<"\r\n##########  Recv data ##########"<<endl;
            int msgSize = 0;
            IResponseMessage* msgContent = NewInstance_IResponseMessage((const unsigned char*)pBuf, (int)nLength, &msgSize);
            if (msgContent != NULL)
            {
                // msgContent->Dump();
                //int errCode = msgContent->Get_Error();
                delete msgContent;
                //return errCode;
            }
			ReqData * pReqData = (ReqData *)pUsrData;
			pReqData->nError = nFlag;
		}

		int doInsert(const int nType,const char * pAddr,const char * pData,const long nTime,const int nFlag,const char * pOther){
			int nSuc = 0;
			if ((strlen(pAddr) + strlen(pData) + strlen(pOther)) < 4000){
				char * pSqlBuf = new char[4096];
				if (pSqlBuf){
					sprintf(pSqlBuf,"insert into %s (type,addr,data,time,flag,other) values (%d,'%s','%s',%ld,%d,'%s');",DatabaseTable,nType,pAddr,pData,nTime,nFlag,pOther);
					if (DB::getInstance()->doExcute(DatabasePath,pSqlBuf)){
						nSuc = 1;
					}
					delete [] pSqlBuf;
					pSqlBuf = NULL;
				}
			}
			return nSuc;
		}

		int useData(StoreNode * pNode,vector<RecNode> & recVec){
			int nFlag = 0;
			if (pNode && recVec.size() == 1){
				RecNode nodeRec = recVec.at(0);
				ValNode nodeVal;
				for (size_t index=0;index<nodeRec.mVecData.size();index++){
					nodeVal = nodeRec.mVecData.at(index);
					if (nodeVal.mName.compare("id") == 0){
						pNode->id = atoi(nodeVal.mValue.c_str());
					}else if (nodeVal.mName.compare("type") == 0){
						pNode->type = atoi(nodeVal.mValue.c_str());
					}else if (nodeVal.mName.compare("addr") == 0){
						strcpy(pNode->addr,nodeVal.mValue.c_str());
					}else if (nodeVal.mName.compare("data") == 0){
						strcpy(pNode->data,nodeVal.mValue.c_str());
					}else if (nodeVal.mName.compare("time") == 0){
						pNode->time = atol(nodeVal.mValue.c_str());
					}else if (nodeVal.mName.compare("flag") == 0){
						pNode->flag = atoi(nodeVal.mValue.c_str());
					}else if (nodeVal.mName.compare("other") == 0){
						strcpy(pNode->other,nodeVal.mValue.c_str());
					}
				}
				nFlag = 1;
			}
			return nFlag;
		}
		int beginWrok(){
			printf("\r\n > beginWrok......");
			DB::getInstance()->doExcute(DatabasePath,TABSCRIPT);
			DB::getInstance()->doExcute(DatabasePath,IDXSCRIPT);
				printf("\r\n > Create database and index success ......");
				StoreNode tmpNode;
				vector<RecNode> vecRec;
				char sqlBuf[128];
				memset(sqlBuf,0,sizeof(sqlBuf));
				sprintf(sqlBuf,"select * from %s limit 1;",DatabaseTable);
				char sqlDel[128];
				while (mFlag){
					//printf("\r\n > get data from database ......");
					memset(&tmpNode,0,sizeof(StoreNode));
					vecRec.clear();
					if (DB::getInstance()->doExcute(DatabasePath,sqlBuf,vecRec) && useData(&tmpNode,vecRec))
					{
						printf("\r\n > upload data id:%d type:%d host:%s data:%s \r\n",tmpNode.id,tmpNode.type,tmpNode.addr,tmpNode.data);
						//从数据库中取出一条数据
						if (tmpNode.type == DATA_INFO){
                            //要发送的数据为普通数据
							ReqData reqData;
							memset(&reqData,0,sizeof(ReqData));
							printf("\r\n > write data to net");
                            //
                            string datas = Tools::Decode(tmpNode.data,strlen(tmpNode.data));
                            doReq(tmpNode.addr,NULL,datas.c_str(),datas.size(),tmpNode.time,ReqProc,&reqData);  //modify by tpfei 2018/1/31
							if (reqData.nError){
								//发送成功,从数据库中删除这些数据
								printf("\r\n >Send to server ok type = DATA_INFO data=%s",tmpNode.data);
								memset(sqlDel,0,sizeof(sqlDel));
								sprintf(sqlDel,"delete from %s where id = %d;",DatabaseTable,tmpNode.id);
								DB::getInstance()->doExcute(DatabasePath,sqlDel);
							}else{
								//发送失败
							}
						}else if (tmpNode.type == DATA_FILE){
							//要发送的数据是一个文件
							if (access(tmpNode.data,0) != -1){
								//要发送的文件存在，应该发送出去
								ReqData reqData;
								memset(&reqData,0,sizeof(ReqData));
								doReq(tmpNode.addr,
                                    NULL,
									tmpNode.data,
									strlen(tmpNode.data),
									tmpNode.time,
									ReqProc,&reqData);
								if (reqData.nError){
									//发送成功,从数据库中删除这些数据
									printf("\r\n >Send to server ok type = DATA_FILE data=%s",tmpNode.data);
									memset(sqlDel,0,sizeof(sqlDel));
									sprintf(sqlDel,"delete from %s where id = %d;",DatabaseTable,tmpNode.id);
									DB::getInstance()->doExcute(DatabasePath,sqlDel);
								}else{
								//发送失败
								}
							}else{
								//文件已不存在了，删除数据库是的记录
								printf("\r\n >Delete data file is net exist data=%s",tmpNode.data);
								memset(sqlDel,0,sizeof(sqlDel));
								sprintf(sqlDel,"delete from %s where id = %d;",DatabaseTable,tmpNode.id);
								DB::getInstance()->doExcute(DatabasePath,sqlDel);
							}
						}
					}
					sleep(1);
			}
			return 0;
		}
		int endWork(){
			mFlag = 0;
			return mFlag;
		}
		int getFlag(){
			return mFlag;
		}
	private:
		int mFlag;
};

