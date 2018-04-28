#pragma once
#include <fstream>
#include <iostream>

typedef char s8;
typedef short s16;
typedef int s32;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

using namespace std;

struct ROMinfo {
	u32 size;

	bool fourPage;
	bool verticalMirroring;
	bool hasTrainer;
	bool hasPRGRAM;
	bool PALmode;

	u8 CHRROMcount;
	u8 PRGROMcount;
	u8 PRGRAMcount;
	u8 mapper;
};
ROMinfo currentROM;


class Mapper {
protected:
	u8 ROM[0x10000];
	u8 VRAM[0x10000];
	u8 OAM[0x100];
public:
	virtual void LoadFromROM(u8* data) = 0;
	u8* GetOAMPointer() {
		return OAM;
	}
	u8* GetROMCell(u16 address) {
		return &ROM[address];
	}
	u8 ReadROM(u16 address) {
		return ROM[address];
	}
	u8* GetVRAMCell(u16 address) {
		return &VRAM[address];
	}
	u8 ReadVRAM(u16 address) {
		return VRAM[address];
	}
	void WriteToROM(u16 address, u8 value) {
		ROM[address] = value;
	}
	void WriteToVRAM(u16 address, u8 value) {
		VRAM[address] = value;
	}
	// copy value from addressed ROM cell to addressed VRAM cell
	void CopyROMToVRAM(u16 ROMaddress, u16 VRAMaddress) {
		VRAM[VRAMaddress] = ROM[ROMaddress];
	}
};
Mapper* mmc;

class Mapper0 : public Mapper {
protected:
	u8* PRG0;
	u8* PRG1;

public:
	Mapper0() {
		PRG0 = &ROM[0xc000];
		PRG1 = &ROM[0x8000];

		for (int i = 0; i < 0x10000; i++) {
			ROM[i] = 0;
			VRAM[i] = 0;
		}
		for (int i = 0; i < 0x100; i++) OAM[i] = 0;
	}
	void LoadFromROM(u8* data) {
	//	data = &data[0x10]; //cut off header

		// load PRG data
		if (currentROM.PRGROMcount == 1) for (int i = 0; i < 16384; i++) PRG0[i] = PRG1[i] = data[16 + i];
		else for (int i = 0; i < 16384*2; i++) PRG1[i] = data[16 + i];
		// load CHR data
		for (int i = 16384*currentROM.PRGROMcount + 16, j = 0; i < 16384*currentROM.PRGROMcount + 8192 + 16; i++, j++) VRAM[j] = data[i];
	}
};

// fast access
u8* ZERO_PAGE;
u8* STACK;
u8* OAM;
u8* BGPalette;
u8* SPRPalette;
u8* CurPalette;

void InitPointers() {
	ZERO_PAGE = mmc->GetROMCell(0x00);
	STACK = mmc->GetROMCell(0x100);
	OAM = mmc->GetOAMPointer();
	BGPalette = mmc->GetVRAMCell(0x3f00);
	SPRPalette = mmc->GetVRAMCell(0x3f10);
	CurPalette = BGPalette;
}

u16 NMI_ADDR;
u16 RESET_ADDR;
u16 BREAK_ADDR;

enum MAPPER {
	MAPPER0 = 0,
	MMC1,
	ERROR = 255
};

Mapper* CreateMapper(u8 mapperNumber) {
	Mapper* newMapper = new Mapper0();

	return newMapper;
}

void WriteHeaderInfo() {
	std::cout << "ROM size: " << currentROM.size << std::endl;
	std::cout << "PRG ROMs: " << (int)currentROM.PRGROMcount << std::endl;
	std::cout << "CHR ROMs: " << (int)currentROM.CHRROMcount << std::endl;
	std::cout << "PRG RAMs: " << (int)currentROM.PRGRAMcount << std::endl;
	std::cout << "Mapper: " << (int)currentROM.mapper << std::endl;
	std::cout << "Scrolling: " << (currentROM.fourPage ? "4 page" : (currentROM.verticalMirroring ? "vertical" : "horizontal")) << std::endl;
	std::cout << "Has PRGRAM: " << (currentROM.hasPRGRAM ? "true" : "false") << std::endl;
	std::cout << "Has Trainer: " << (currentROM.hasTrainer ? "true" : "false") << std::endl;
	std::cout << "TV system: " << (currentROM.PALmode ? "PAL" : "NTSC") << std::endl;
}

bool LoadROM(char* path = "F:/nestest.nes") {
	std::ifstream reader;
	reader.open(path, std::ifstream::binary);
	if (!reader) {
		std::cout << "Can't open ROM file." << std::endl;
		return 1;
	}

	reader.seekg(0, reader.end);
	currentROM.size = reader.tellg();
	reader.seekg(0, reader.beg);
	char* ROMdata = new char[currentROM.size];
	reader.read(ROMdata, currentROM.size);
	reader.close();

	u8 mapper = 0;
	mapper = (ROMdata[6] >> 4) & 0xf;
	mapper |= ROMdata[7] & 0xF0;

	char CHRROMcount = ROMdata[5];
	char PRGROMcount = ROMdata[4];

	currentROM.fourPage = ROMdata[6] & 4;
	currentROM.verticalMirroring = (ROMdata[6] & 1);
	currentROM.PALmode = ROMdata[9] & 1;
	currentROM.hasPRGRAM = ROMdata[6] & 2;
	currentROM.hasTrainer = ROMdata[6] & 4;
	currentROM.PRGROMcount = ROMdata[4];
	currentROM.CHRROMcount = ROMdata[5];
	currentROM.PRGRAMcount = ROMdata[8];
	currentROM.mapper = mapper;

	WriteHeaderInfo();

	if (currentROM.PALmode) {
		cout << "PAL is not supported" << endl;
		return 2;
	}

	mmc = CreateMapper(currentROM.mapper);
	mmc->LoadFromROM((u8*)ROMdata);
	InitPointers();

	delete[] ROMdata;
}