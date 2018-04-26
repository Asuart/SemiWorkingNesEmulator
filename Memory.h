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
unsigned short BREAK_ADDR;

struct ROMinfo {
	bool fourPage;
	bool verticalMirroring;
	bool hasTrainer;
	bool PALmode;

	unsigned char CHRROMcount;
	unsigned char PRGROMcount;
	unsigned char PRGRAMcount;
	unsigned char mapper;
};
ROMinfo currentROM;

void LoadROM(char* path = "F:/mt.nes") {
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

	unsigned char mapper = 0;
	mapper = (ROMdata[6] >> 4) & 0xf;
	mapper |= ROMdata[7] & 0xF0;

	char CHRROMcount = ROMdata[5];
	char PRGROMcount = ROMdata[4];

	currentROM.fourPage = ROMdata[6] & 4;
	currentROM.verticalMirroring = (ROMdata[6] & 1);
	currentROM.PALmode = ROMdata[9] & 1;
	currentROM.hasTrainer = ROMdata[6] & 2;
	currentROM.PRGROMcount = ROMdata[4];
	currentROM.CHRROMcount = ROMdata[5];
	currentROM.PRGRAMcount = ROMdata[8];
	currentROM.mapper = mapper;


	std::cout << "ROM size: " << ROMsize << std::endl;
	std::cout << "PRG ROMs: " << (int)ROMdata[4] << std::endl;
	std::cout << "CHR ROMs: " << (int)ROMdata[5] << std::endl;
	std::cout << "PRG RAMs: " << (int)ROMdata[8] << std::endl;
	std::cout << "Mapper: " << (int)(mapper) << std::endl;
	std::cout << "Scrolling: " << ((ROMdata[6] & 4) ? "4 page" : (((int)ROMdata[6] & 1) ? "vertical" : "horizontal")) << std::endl;
	std::cout << "Cart has PRGRAM: " << ((int)ROMdata[6] & 2) << std::endl;
	std::cout << "Cart has Trainer: " << ((int)ROMdata[6] & 4) << std::endl;
	std::cout << "TV system: " << (((int)ROMdata[9] & 1)? "PAL" : "NTSC" )<< std::endl;
	if (ROMdata[9] & 1) {
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