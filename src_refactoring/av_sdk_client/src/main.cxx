
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <string.h>
#include "antivirus_interface.h"

std::vector<std::string> Split(std::string str)
{
    std::vector<std::string> data;
    if (str.empty() == false || str.length() >= 10240)
    {
        char array[10240] = {0};
        char *buffer = (char *)str.c_str();
        int index_word = 0;
        for (int index = 0; index < strlen(buffer); index++)
        {
            if (buffer[index] >= 33 && buffer[index] <= 126)
            {
                array[index_word] = buffer[index];
                index_word = index_word + 1;
            }
            else
            {
                if (strlen(array) > 0)
                {
                    data.push_back(array);
                    memset(array, 0, sizeof(array));
                    index_word = 0;
                }
            }
            if (index == (strlen(buffer) - 1))
            {
                if (strlen(array) > 0)
                {
                    data.push_back(array);
                    memset(array, 0, sizeof(array));
                    index_word = 0;
                }
            }
        }
    }
    return data;
}


uint32_t ResultNotify(const char * file,uint32_t flag,uint64_t record,const char * description, void *ptr_param){
    printf("Message from sdk:%s\n",file);
    return 1;
}


uint32_t DataNotifyProc(const char * data,uint32_t length,int32_t flag,void * ptr_param){

    printf("Message from sdk:%s\n",data);
    return 1;
}


int main(){
    void * ptr_sdk = av_sdk_init();
    if (ptr_sdk == nullptr)
        return 0;
    printf("please input\n");
    char buffer[1024] = {0};
    while(true){
        if (gets(buffer) != NULL && strlen(buffer) > 0){

            std::vector<std::string> args = Split(buffer);
            if (args[0].compare("quit") == 0){
                break;
            }else if (args[0].compare("-scan") == 0){
                std::string path = args[1];
                printf("#-scan [%s]\n",path.c_str());
                uint32_t option = Backup | Unzip;
                uint32_t flag = av_scan_target(ptr_sdk,option,path.c_str(),DataNotifyProc,NULL);  
            }else if (args[0].compare("-get") == 0){
                av_get_path(ptr_sdk,DataNotifyProc,NULL);
            }
        }else{
            printf("please input your data\n");
        }
    }
    av_sdk_uninit(ptr_sdk);
    return 0;
}

