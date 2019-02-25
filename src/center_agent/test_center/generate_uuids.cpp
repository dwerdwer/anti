#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include "uuid.h"
#include <fcntl.h>
#include <errno.h>
#define UUIDS_FILE "./uuids"
char *gen_uuid(char* buf)
{
	uuid_t uuid;
	uuid_generate_time_safe(uuid);
	uuid_unparse(uuid, buf);
	return buf;
}

int main(int argc, char *argv[])
{
	uint32_t uuid_count;
	int opt;
    std::string filename;
	while((opt = getopt(argc, argv, "c:f:")) != -1)
	{
		switch(opt){
			case 'c':
				uuid_count = atoi(optarg);
				printf("option: %c uuid_count: %d\n", opt, uuid_count);
				break;
            case 'f':
                filename = optarg;
                printf("FileName is: %s\n", optarg);
                break;
			case '?':
				printf("Unknown option: %c\n", (char)(optopt));
				return -1;
				break;
		}
	}

	FILE *fp;
	//fp = fopen(UUIDS_FILE, "a+");
	fp = fopen(filename.c_str(), "a+");
	if (fp == NULL)
		printf("open file error %d\n", errno);

	for (uint32_t i = 0; i < uuid_count; i++)
	{
		char buf_uuid[1024];
		std::string uuid = gen_uuid(buf_uuid);
		int count = fwrite(uuid.c_str(), uuid.length(), 1,fp);
		fwrite("\n", strlen("\n"), 1, fp);
		//printf("write count is %d\n",count);

	}
	fclose(fp);


	//读文件
	FILE *in;
	in = fopen(UUIDS_FILE, "r");
	if (in == NULL)
	{
		printf("error %d\n", errno);
		return -1;
	}
	char line[1024];
	while (fgets(line, 1023, in)) {
		printf("%s\n",line);
	}
	fclose(in);

}
