#pragma once
#include <fstream>
#include <iostream>

using namespace std;

char ROM[0x10000];
char VROM[0x10000];
unsigned char OAM[0x100];

char* ZERO_PAGE = ROM;
char* PRG = &ROM[0x8000];
char* STACK = &ROM[0x100];
char* SPRPalette = &VROM[0x3f10];
char* BGPalette = &VROM[0x3f00];
char* CurPalette = BGPalette;

unsigned short NMI_ADDR;
unsigned short RESET_ADDR;

struct ROMinfo {
	bool fourPage;
	bool verticalMirroring;
};
ROMinfo currentROM;

void LoadROM(char* path = "F:/dk.nes") {
	char b0 = 0b1;
	char b1 = 0b10;
	char b2 = 0b100;
	char b3 = 0b1000;
	char b4 = 0b10000;
	char b5 = 0b100000;
	char b6 = 0b1000000;
	char b7 = 0b10000000;

	std::ifstream reader;
	reader.open(path, std::ifstream::binary);
	if (!reader) {
		std::cout << "Can open ROM" << std::endl;
		return;
	}
	reader.seekg(0, reader.end);
	int ROMsize = reader.tellg();
	reader.seekg(0, reader.beg);

	char* ROMdata = new char[ROMsize];
	reader.read(ROMdata, ROMsize);

	char mapper = 0;
	mapper = ROMdata[6] >> 4;
	mapper |= ROMdata[7] & 0xF0;

	char CHRROMcount = ROMdata[5];
	char PRGROMcount = ROMdata[4];

	currentROM.fourPage = ROMdata[6] & b3;
	currentROM.verticalMirroring = !(ROMdata[6] & b0);

	std::cout << "ROM size: " << ROMsize << std::endl;
	std::cout << "PRG ROMs: " << (int)ROMdata[4] << std::endl;
	std::cout << "CHR ROMs: " << (int)ROMdata[5] << std::endl;
	std::cout << "PRG RAMs: " << (int)ROMdata[8] << std::endl;
	std::cout << "Mapper: " << (int)mapper << std::endl;
	std::cout << "Scrolling: " << ((ROMdata[6] & b3) ? "4 page" : (((int)ROMdata[6] & b0) ? "vertical" : "horizontal")) << std::endl;
	std::cout << "Cart has PRGRAM: " << ((int)ROMdata[6] & b1) << std::endl;
	std::cout << "Cart has Trainer: " << ((int)ROMdata[6] & b2) << std::endl;
	std::cout << "TV system: " << (((int)ROMdata[9] & b0)? "PAL" : "NTSC" )<< std::endl;
	if (ROMdata[9] & b0) {
		cout << "PAL not supported" << endl;
		system("pause");
	}


	for (int i = 0; i < 0x10000; i++) ROM[i] = 0xff;
	//upload
	if (PRGROMcount == 2) {
		for (int i = 0; i < 16384; i++) {
			PRG[i] = ROMdata[16 + i];
		}
		for (int i = 0; i < 16384; i++) {
			PRG[i + 16384] = ROMdata[16 + 16384 + i];
		}
	}
	else if (PRGROMcount == 1) {
		for (int i = 0; i < 16384; i++) {
			PRG[i] = ROMdata[16 + i];
		}
		for (int i = 0; i < 16384; i++) {
			PRG[i + 16384] = ROMdata[16 + i];
		}
	}
	else {
		cout << "More then 2 PRGROM parts not supported" << endl;
		system("pause");
	}
	for (int i = 0; i < 8192; i++) {
		VROM[i] = ROMdata[16 + 16384 * PRGROMcount + i];
	}
}