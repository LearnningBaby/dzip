#pragma warning(disable : 4996)
#include <iostream>
#include "FileCompressHuffman.h"

void TestCompress() {
	std::string filePath = "zdz.txt";
	FileCompressHuffman fio;
	fio.CompressFile(filePath);
}

int main()
{	
	TestCompress();
	return 0;
}

