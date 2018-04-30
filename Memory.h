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

extern u8 value;
extern u16 address;
extern bool writeOperation;

class Mapper {
protected:
	u8 ROM[0x10000];
	u8 VRAM[0x10000];
	u8 OAM[0x100];
public:
	virtual void LoadFromROM(u8* data) = 0;
	virtual void UpdateState() = 0;
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
	void UpdateState() {
		// nothing
	}
};
class MMC1 : public Mapper {
	// currently not working
	enum CHUNK_MODE { KB4 = 0, KB8, KB16, KB32 };

	static const u16 PRGBankSize = 16384;

	void ResetSR() {
		SR = (1 << 4);
	}
	void SwitchCHRROM(u8 bank) {
		static const u16 bankSize = 8192;
		u8* data = &CHRROMdata[bank*bankSize];
		for (int i = 0; i < bankSize; i++) CHR0[i] = data[i];
	}
	void SwitchCHR0(u8 bank) {
		static const u16 bankSize = 4096;
		u8* data = &CHRROMdata[bank*bankSize];
		for (int i = 0; i < bankSize; i++) CHR0[i] = data[i];
	}
	void SwitchCHR1(u8 bank) {
		static const u16 bankSize = 4096;
		u8* data = &CHRROMdata[bank*bankSize];
		for (int i = 0; i < bankSize; i++) CHR1[i] = data[i];
	}
	void SwitchPRGROM(u8 bank) {
		bank &= 0b1110; // ignore upper and lower bits
		u8* data = &PRGROMdata[bank*PRGBankSize];
		for (int i = 0; i < PRGBankSize * 2; i++) PRG0[i] = data[i];
	}
	void SwitchPRG0(u8 bank) {
		bank &= 0b1111; // ignore upper bit
		u8* data = &PRGROMdata[bank*PRGBankSize];
		for (int i = 0; i < PRGBankSize; i++) PRG0[i] = data[i];
	}
	void SwitchPRG1(u8 bank) {
		bank &= 0b1111; // ignore upper bit
		u8* data = &PRGROMdata[bank*PRGBankSize];
		for (int i = 0; i < PRGBankSize; i++) PRG1[i] = data[i];
	}
protected:
	static const u32 PRGROMcapacity = 524288; // Maximum PRG data size in ROM.
	static const u32 CHRROMcapacity = 131072; // Maximum CHR data size in ROM.

	// banks
	u8* PRGRAM; // 8KB fixed on all, but SOROM and SXXROM.
	u8* PRG0; // 16KB fixed to first or switchable at 0x8000.
	u8* PRG1; // 16KB fixed to last or switchable at 0xc000.
	u8* CHR0; // 4KB switchable at 0x0000 PPU.
	u8* CHR1; // 4KB switchable at 0x1000 PPU.

	// Registers
	u8 SR; // Shift Register.
	u8 CTRL; // Control Register.

	// data containers
	u8 PRGROMdata[PRGROMcapacity]; // All PRG containers.
	u8 CHRROMdata[CHRROMcapacity]; // ALL CHR containers.

public:
	MMC1() {
		for (int i = 0; i < PRGROMcapacity; i++) PRGROMdata[i] = 0;
		for (int i = 0; i < CHRROMcapacity; i++) CHRROMdata[i] = 0;
		for (int i = 0; i < 0x10000; i++) {
			ROM[i] = 0;
			VRAM[i] = 0;
		}
		for (int i = 0; i < 0x100; i++) OAM[i] = 0;
		ResetSR();
		CTRL = 0;

		PRGRAM = &ROM[0x6000];
		PRG0 = &ROM[0x8000];
		PRG1 = &ROM[0xc000];
		CHR0 = &VRAM[0x0000];
		CHR1 = &VRAM[0x1000];
	}
	void LoadFromROM(u8* data) {
		// cut header
		data = &data[0x10];
		for (int i = 0; i < 16384 * currentROM.PRGROMcount; i++) PRGROMdata[i] = data[i];
		for (int i = 16384 * currentROM.PRGROMcount, j = 0; j < 8192 * currentROM.CHRROMcount; i++, j++) CHRROMdata[j] = data[i];
		// load start data
		for (int i = 0; i < 16384; i++)PRG0[i] = PRGROMdata[i];
		for (int i = 0; i < 16384; i++)PRG1[i] = PRGROMdata[7*16384 + i];
		for (int i = 0; i < 8096; i++)CHR0[i] = CHRROMdata[i];
	}
	void UpdateState() {
		if (writeOperation && address > 0x7fff) {
			if (value & 0x80) ResetSR();
			else {
				if (!(SR & 1)) SR = ((SR >> 1) | ((value & 1) << 4));
				else {
					SR = ((SR >> 1) | ((value & 1) << 4)) & 0b11111; // Shift register is now full
					// Depend on 5th write address choose operation
					if (address < 0xa000) { // Copy to Control register
						CTRL = SR;
					}
					else if (address >= 0xe000) { // Switch CHR0
						SR &= 0b1111;
						switch ((CTRL >> 2) & 0b11) {
						case 0:
						case 1:
							SwitchPRGROM(SR); break;
						case 2:
							SwitchPRG1(SR); break;
						case 3:
							SwitchPRG0(SR); break;
						}
					}
					ResetSR();

				}

			}
		}
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
	MAP_MAPPER0 = 0,
	MAP_MMC1,
	MAP_ERROR = 255
};

Mapper* CreateMapper(u8 mapperNumber) {
	Mapper* newMapper;
	switch (mapperNumber) {
	case MAP_MAPPER0:
		newMapper = new Mapper0();
		break;
	case MAP_MMC1:
		newMapper = new MMC1();
		break;
	default:
		cout << "Unknown mapper, used 0mapper instead." << endl;
		newMapper = new Mapper0();
	}
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

bool LoadROM(char* path = "F:/ninja.nes") {
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