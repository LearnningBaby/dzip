#pragma warning(disable : 4996)
#include <iostream>
#include "FileCompressHuffman.h"
#include "LZ77.h"
#include "DZzip.h"
void Menu() {
	std::cout << "*************************************************" << std::endl;
	std::cout << "********  张德忠的压缩系统 	      ***********" << std::endl;
	std::cout << "********  0: 退出		      ***********" << std::endl;
	std::cout << "********  1: huffman 压缩	      ***********" << std::endl;
	std::cout << "********  2: huffman 解压缩           ***********" << std::endl;
	std::cout << "********  3: LZ77      压缩           ***********" << std::endl;
	std::cout << "********  4: LZ77    解压缩           ***********" << std::endl;
	std::cout << "********  5: DZzip     压缩           ***********" << std::endl;
	std::cout << "********  6: DZzip   解压缩           ***********" << std::endl;
	std::cout << "*************************************************" << std::endl;

}


int main()
{
	FileCompressHuffman fcp;
	LZ77 lz;
	DZzip dz;
	int input = -1;
	std::string fileName;
	bool isQuit = false;

	while (true) {
		Menu();
		std::cout << "请输入操作: ";
		std::cin >> input;
		switch (input) {
		case 0: isQuit = true;
			std::cout << "bye!" << std::endl;
			break;
		case 1:
			std::cout << "请输入要压缩的文件全路径: ";
			std::cin >> fileName;
			fcp.CompressFile(fileName);
			std::cout << "压缩完成!" << std::endl;
			break;
		case 2:
			std::cout << "请输入要解压缩的文件全路径: ";
			std::cin >> fileName;
			fcp.UNCompressFile(fileName);
			std::cout << "解压缩完成!" << std::endl;
			break;
		case 3:
			std::cout << "请输入要压缩的文件全路径: ";
			std::cin >> fileName;
			lz.CompressFile(fileName);
			std::cout << "压缩完成!" << std::endl;
			break;
		case 4:
			std::cout << "请输入要解压缩的文件全路径: ";
			std::cin >> fileName;
			lz.UNCompressFile(fileName);
			std::cout << "解压缩完成!" << std::endl;
			break;
		case 5:
			std::cout << "请输入要压缩的文件全路径: ";
			std::cin >> fileName;
			dz.Deflate(fileName);
			std::cout << "压缩完成!" << std::endl;
			break;
		case 6:
			std::cout << "请输入要解压缩的文件全路径: ";
			std::cin >> fileName;
			dz.UnDeflate(fileName);
			std::cout << "解压缩完成!" << std::endl;
			break;
		default: 
			std::cout << "非法操作符!" << std::endl;
			break;
		}
		if (isQuit) break;
	}
	return 0;
}






#if 0 
void TestLZ77Compress() {
	LZ77 lz;
	lz.compressFile("zdz.txt");
}

void TestLZ77UNCompress() {
	LZ77 lz;
	lz.UNCompressFile("zdz.lzp");
}
int main() {
	TestLZ77Compress();
	TestLZ77UNCompress();
	return 0;
}
#endif 