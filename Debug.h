#pragma once
#include <fstream>
#include <iostream>


unsigned short tPC;
char tX, tY, tA, tP;
unsigned tS;
unsigned char top;
char pnt;
char line[128] = "";
std::ifstream reader;

void OpenTest() {
	char* path = "F:/nestest.txt";
	reader.open(path, std::ios::in);
	if (!reader) {
		std::cout << "Can open ROM" << std::endl;
		return;
	}
}

char cast() {
	char c = line[pnt];
	pnt++;
	switch (c) {
	case '0':
		return 0x0;
	case '1':
		return 0x1;
	case '2':
		return 0x2;
	case '3':
		return 0x3;
	case '4':
		return 0x4;
	case '5':
		return 0x5;
	case '6':
		return 0x6;
	case '7':
		return 0x7;
	case '8':
		return 0x8;
	case '9':
		return 0x9;
	case 'A':
		return 0xa;
	case 'B':
		return 0xb;
	case 'C':
		return 0xc;
	case 'D':
		return 0xd;
	case 'E':
		return 0xe;
	case 'F':
		return 0xf;
	}
}

void compare() {
	static int n = 0;
	n++;
	pnt = 0;
	reader.read(line, 17);
	tPC = 0;
	tPC |= cast() << 12;
	tPC |= cast() << 8;
	tPC |= cast() << 4;
	tPC |= cast();
	top = 0;
	top |= cast() << 4;
	top |= cast();
	tA = 0;
	tA |= cast() << 4;
	tA |= cast();
	tX = 0;
	tX |= cast() << 4;
	tX |= cast();
	tY = 0;
	tY |= cast() << 4;
	tY |= cast();
	tP = 0;
	tP |= cast() << 4;
	tP |= cast();
	tS = 0;
	tS |= cast() << 4;
	tS |= cast();

	cout << hex << tPC << " : " << opcodes[top].name << "_" << opcodes[opcode].addressing <<"("<< ((short)top & 0xff) <<")" << " A:" << ((short)tA & 0xff) << " X:" << ((short)tX & 0xff) << " Y:" << ((short)tY & 0xff) << " S:" << ((short)(tS) & 0xff) <<" P:" << ((short)tP & 0xff) <<  endl;

}