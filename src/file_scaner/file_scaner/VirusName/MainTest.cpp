#include "VirusFileAPI.h"
#include "ImportData.h"
#include "cmdParser.h"

/*
构造索引文件:
-m	读取 -src 指定原始 txt，索引文件存于 -out

例：
-m -src D:\Work\storage\Virus\FTName.Dat.txt -out D:\Work\storage\Virus\indexFile

获取病毒名:
-r  读取 -src 索引文件 根据 -n 指定 keyNo 获取病毒名

升级:
-u	原索引 -src -add 指定升级 txt 合成新索引文件 -out

导出:
-d	读取 -src 索引文件 存于 -out
*/

int main(int argc, char* argv[])
{
	char* srcPath = NULL;  // 读取路径
	char* outPath = NULL;  // 储存路径
	char* addPath = NULL;  // 升级文件路径

	int keyNo = 0; // 索引编号

	if (cmdOptionExists(argv, argv + argc, "-src"))
	{
		srcPath = getCmdOption(argv, argv + argc, "-src");
	}
	if (cmdOptionExists(argv, argv + argc, "-out"))
	{
		outPath = getCmdOption(argv, argv + argc, "-out");
	}
	if (cmdOptionExists(argv, argv + argc, "-add"))
	{
		addPath = getCmdOption(argv, argv + argc, "-add");
	}
	if (cmdOptionExists(argv, argv + argc, "-n"))
	{
		keyNo = atoi(getCmdOption(argv, argv + argc, "-n"));
	}

	if (argc >= 2)
	{
		//----------------------------------------------------------------------
		// 传入数据，保存文件
		if (cmdOptionExists(argv, argv + argc, "-m"))
		{
			int base = GetBaseNo(srcPath);

			if (base == -1)
				return -1;

			clock_t start = clock();
			NameDatBuilder* nameDatBuilder = new NameDatBuilder(base);
			ImportData importData;
			// 导入内容
			if (importData.Import(nameDatBuilder, srcPath) == -1)
			{
				delete nameDatBuilder;
				return -1;
			}
			clock_t end = clock();
			printf(" Import:   %fs\n", (double)(end - start) / CLOCKS_PER_SEC);
		
			if (nameDatBuilder->SaveToFile(outPath) != 0)
				cout << "SaveToFile err" << endl;
				//return -1;

			end = clock();
			printf(" SaveToFile:   %fs\n", (double)(end - start) / CLOCKS_PER_SEC);

			delete nameDatBuilder;

			return 0;
		}

		//----------------------------------------------------------------------
		// 读取所有元素，保存文件
		if (cmdOptionExists(argv, argv + argc, "-d"))
		{
			clock_t start = clock();

			NameDatReader* nameDatReader = new NameDatReader;

			if (nameDatReader->Init(srcPath) == -1)
				return -1;

			int base = (int)nameDatReader->headerInfo->baseNo;

			Decode_CompressedFile(nameDatReader, outPath);

			clock_t end = clock();
			printf(" Decode_CompressedFile:      %fs\n", (double)(end - start) / CLOCKS_PER_SEC);

			delete nameDatReader;

			return 0;
		}

		//----------------------------------------------------------------------
		// 查询单个病毒名

		if (cmdOptionExists(argv, argv + argc, "-r"))
		{
			NameDatReader* nameDatReader = new NameDatReader;

			if (nameDatReader->Init(srcPath) == -1)
				return -1;

			char strBuf[MAX_VIR_NAME] = { 0 };

			nameDatReader->GetName(keyNo, strBuf);

			if (strlen(strBuf) != 0)
				printf("\n %s\n\n", strBuf);
			else
				printf("\n empty\n\n");

			delete nameDatReader;
		}
	}
	//----------------------------------------------------------------------
	// 读取 update，合成新索引文件
	if (cmdOptionExists(argv, argv + argc, "-u"))
	{
		clock_t start = clock();

		cout << "outPath : " << outPath << endl;
		cout << "srcPath : " << srcPath << endl;
		cout << "addPath : " << addPath << endl;


		if (UpdateNameData(outPath, srcPath, addPath) == -1)
			return -1;

		clock_t end = clock();
		printf("\n UpdateNameDate:   %fs\n", (double)(end - start) / CLOCKS_PER_SEC);
		return 0;
	}
	
	return 0;
}