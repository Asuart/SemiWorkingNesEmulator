#pragma once
#include "Memory.h"
#include <bitset>

bool crossPage = false;

void GetOpcode() {
	opcode = mmc->ReadROM(PC);
	PC++;
}
void GetOperand() {
	op = mmc->ReadROM(PC);
	PC++;
}
void GetOperands() {
	_op[0] = mmc->ReadROM(PC);
	PC++;
	_op[1] = mmc->ReadROM(PC);
	PC++;
}

short RelAddr(short adr,char offset) {
	return adr + offset;
}
short GetIndirect(unsigned char zp) {
	unsigned short adr;
	char* p = (char*)&adr;
	if ((zp & 0xff) == 0xff) {
		p[0] = ZERO_PAGE[0xFF];
		p[1] = ZERO_PAGE[0x00];
	}
	else {
		p[0] = ZERO_PAGE[zp & 0xFF];
		p[1] = ZERO_PAGE[(zp & 0xff) + 1];
	}
	return adr;
}
void IMPL() {
	value = 0;
	address = 0;
}
void REL() {
	value = op & 0xff;
	address = RelAddr(PC, value);
	if ((PC & 0xff00) != (address & 0xff00)) cpuCycles++;
}
void IMM() {
	value = op & 0xff;
	address = 0;
}
void ZP() {
	address = op & 0xff;
	cell = &ZERO_PAGE[address];
	value = ZERO_PAGE[address];
}
void ZPX() {
	address = (op + X) & 0xff;
	cell = &ZERO_PAGE[address];
	value = ZERO_PAGE[address];
}
void ZPY() {
	address = (op + Y) & 0xff;
	cell = &ZERO_PAGE[address];
	value = ZERO_PAGE[address];
}
void ABS() {
	address = op;
	cell = mmc->GetROMCell(address);
	value = mmc->ReadROM(address);
}
void ABSX() {
	address = op + X;
	if ((op & 0xff00) != (address & 0xff00)) crossPage = true;
	cell = mmc->GetROMCell(address);
	value = mmc->ReadROM(address);
}
void ABSY() {
	address = op + Y;
	if ((op & 0xff00) != (address & 0xff00)) crossPage = true;
	cell = mmc->GetROMCell(address);
	value = mmc->ReadROM(address);
}
void INDX() {
	address = GetIndirect((op + X) & 0xff);
	cell = mmc->GetROMCell(address);
	value = mmc->ReadROM(address);
}
void INDY() {
	address = GetIndirect(op) + Y;
	if ((GetIndirect(op) & 0xff00) != (address & 0xff00)) crossPage = true;
	cell = mmc->GetROMCell(address);
	value = mmc->ReadROM(address);
}
void ACC() {
	value = AC;
	cell = &AC;
	address = 0;
}
void IND() {
	unsigned char* adr = (unsigned char*)&address;
	if ((op & 0xff) == 0xff) {
		adr[0] = mmc->ReadROM(op | 0xff);
		adr[1] = mmc->ReadROM(op & (~0xff));
	}
	else {
		adr[0] = mmc->ReadROM(op);
		adr[1] = mmc->ReadROM(op + 1);
	}
}
void ADDRESSING_ERROR() {
	cout << "Wrong addressing mode selected" << endl;
}

enum ADDRESSING_MODE {
	ADDR_IMPL = 0,
	ADDR_REL,
	ADDR_IMM,
	ADDR_ZP,
	ADDR_ZPX,
	ADDR_ZPY,
	ADDR_ABS,
	ADDR_ABSX,
	ADDR_ABSY,
	ADDR_INDX,
	ADDR_INDY,
	ADDR_ACC,
	ADDR_IND
};

struct Opcode {
	char* name;
	ADDRESSING_MODE addressing;
	char code;
	void(*func)();
	char cycles;
	char length;

	Opcode(char* _name,ADDRESSING_MODE _addressing, char _code, void(*_func)(),char _length, char _cycles = 2) {
		name = _name;
		addressing = _addressing;
		code = _code;
		func = _func;
		cycles = _cycles;
		length = _length;
	}
	Opcode(char _opcode, void(*_func)()) {
		length = 1;
		func = _func;
		opcode = _opcode;
		addressing = ADDR_IMPL;
		name = "NONE";
		cycles = 1;
	}
	Opcode() {
		length = 1;
		addressing = ADDR_IMM;
		name = "NONE";
	}

	void DecodeOperand() {
		switch (addressing) {
		case ADDR_IMPL:
			IMPL(); break;
		case ADDR_IMM:
			IMM(); break;
		case ADDR_ZP:
			ZP(); break;
		case ADDR_ZPX:
			ZPX(); break;
		case ADDR_ZPY:
			ZPY(); break;
		case ADDR_ABS:
			ABS(); break;
		case ADDR_ABSX:
			ABSX(); break;
		case ADDR_ABSY:
			ABSY(); break;
		case ADDR_INDX:
			INDX(); break;
		case ADDR_INDY:
			INDY(); break;
		case ADDR_ACC:
			ACC(); break;
		case ADDR_IND:
			IND(); break;
		case ADDR_REL:
			REL(); break;
		default:
			ADDRESSING_ERROR();
		}
	}
	void exec() {
		crossPage = false;
		switch (length) {
		case 2:
			GetOperand();
			break;
		case 3:
			GetOperands();
			break;
		}		
		DecodeOperand();
		func();
	}
};
Opcode opcodes[255];

enum STACK_ERROR {
	STACK_FULL,
	STACK_EMPTY
};
void StackError(STACK_ERROR err) {
	switch(err) {
	case STACK_FULL:
		cout << "Stack is full on push request!" << endl;
		break;
	case STACK_EMPTY:
		cout << "Stack is empty on pull request!" << endl;
		break;
	default:
		cout << "Unhandled STACK error" << endl;
		break;
	}
}

// Stack is cycled, and even with errors, it passes test
void PUSH16(short val) {
	char* v = (char*)&val;
	STACK[SP] = v[1];
	SP--;
	STACK[SP] = v[0];
	SP--;
}
void PUSH8(char val) {
	STACK[SP] = val;
	SP--;
}
short PULL16() {
	short val;
	char* v = (char*)&val;
	SP++;
	v[0] = STACK[SP];
	SP++;
	v[1] = STACK[SP];
	return val;
}
char PULL8() {
	SP++;
	return STACK[SP];
}

void Disasm() {
	cout << hex << PC - opcodes[opcode].length << " : " << opcodes[opcode].name << "_" << opcodes[opcode].addressing << "(" << (int)opcode << ")" << " A:" << (int)(AC & 0xff) << " X:" << (int)(X & 0xff) << " Y:" << (int)(Y & 0xff) << " S:" << (int)(SP) << " P:" << hex << bitset<8>(F & 0xff) << endl;
	cout << "Address and value: " << address << ":" << (int)value << endl;
}

void NONE() {
	cout << "Used unregistered opcode: " << hex << opcode << endl;
}

void ADC() {
	unsigned short temp = value + (unsigned char)AC + (GetCarry() ? 1 : 0);
	SetOverflow(!((AC ^ value) & 0x80) && ((AC ^ temp) & 0x80));
	AC = temp & 0xff;
	SetCarry(temp > 0xff);
	SetNZ(AC);
	if (crossPage && (opcodes[opcode].addressing == ADDR_ABSX || opcodes[opcode].addressing == ADDR_ABSY || opcodes[opcode].addressing == ADDR_INDY)) cpuCycles++;
}
void AND() {
	AC &= value;
	SetNZ(AC);
	if (crossPage && (opcodes[opcode].addressing == ADDR_ABSX || opcodes[opcode].addressing == ADDR_ABSY || opcodes[opcode].addressing == ADDR_INDY)) cpuCycles++;
}
void ASL() {
	SetCarry(value & 0x80);
	value = (value << 1); // & ~1
	SetNZ(value);
	*cell = value;
	writeOperation = true;
}
void BCC() {
	if (!GetCarry()) {
		PC = address;
		cpuCycles++;
	}
}
void BCS() {
	if (GetCarry()) {
		PC = address;
		cpuCycles++;
	}
}
void BEQ() {
	if (GetZero()) {
		PC = address;
		cpuCycles++;
	}
}
void BIT() {
	SetSign(value);
	SetOverflow(value & 0x40);
	SetZero(AC & value);
}
void BMI() {
	if (GetSign()) {
		PC = address;
		cpuCycles++;
	}
}
void BNE() {
	if (!GetZero()) {
		PC = address;
		cpuCycles++;
	}
}
void BPL() {
	if (!GetSign()) {
		PC = address;
		cpuCycles++;
	}
}
void BRK() {
	PUSH16(PC);
	SetBreak(1);
	SET_1();
	PUSH8(F);
	SetInterrupt(1);
	PC = BREAK_ADDR;
	
}
void BVC() {
	if (!GetOverflow()) {
		PC = address;
		cpuCycles++;
	}
}
void BVS() {
	if (GetOverflow()) {
		PC = address;
		cpuCycles++;
	}
}
void CLC() {
	SetCarry(0);
}
void CLD() {
	SetDecimal(0);
}
void CLI() {
	SetInterrupt(0);
}
void CLV() {
	SetOverflow(0);
}
void CMP() {
	unsigned short temp = AC - value;
	SetCarry(temp < 0x100);
	SetNZ(temp & 0xff);
	if (crossPage && (opcodes[opcode].addressing == ADDR_ABSX || opcodes[opcode].addressing == ADDR_ABSY || opcodes[opcode].addressing == ADDR_INDY)) cpuCycles++;
}
void CPX() {
	unsigned short temp = X - value;
	SetCarry(temp < 0x100);
	SetNZ(temp&0xff);
}
void CPY() {
	unsigned short temp = Y - value;
	SetCarry(temp < 0x100);
	SetNZ(temp&0xff);
}
void DEC() {
	value--;
	SetNZ(value);
	*cell = value;
	writeOperation = true;
}
void DEX() {
	X--;
	SetNZ(X);
}
void DEY() {
	Y--;
	SetNZ(Y);
}
void EOR() {
	AC ^= value;
	SetNZ(AC);
	if (crossPage && (opcodes[opcode].addressing == ADDR_ABSX || opcodes[opcode].addressing == ADDR_ABSY || opcodes[opcode].addressing == ADDR_INDY)) cpuCycles++;
}
void INC() {
	value++;
	SetNZ(value);
	*cell = value;
	writeOperation = true;
}
void INX() {
	X++;
	SetNZ(X);
}
void INY() {
	Y++;
	SetNZ(Y);
}
void JMP() {
	PC = address;
}
void JSR() {
	PUSH16(PC - 1);
	PC = address;
}
void LDA() {
	AC = value;
	SetNZ(AC);
	if (crossPage && (opcodes[opcode].addressing == ADDR_ABSX || opcodes[opcode].addressing == ADDR_ABSY || opcodes[opcode].addressing == ADDR_INDY)) cpuCycles++;
}
void LDX() {
	X = value;
	SetNZ(X);
	if (crossPage && opcodes[opcode].addressing == ADDR_ABSY) cpuCycles++;
}
void LDY() {
	Y = value;
	SetNZ(Y);
	if (crossPage && opcodes[opcode].addressing == ADDR_ABSX) cpuCycles++;
}
void LSR() {
	SetCarry(value & 1);
	value = (value >> 1); // & ~0x80
	SetNZ(value);
	*cell = value;
	writeOperation = true;
}
void NOP() {
	// Nothing
}
void ORA() {
	AC |= value;
	SetNZ(AC);
	if (crossPage && (opcodes[opcode].addressing == ADDR_ABSX || opcodes[opcode].addressing == ADDR_ABSY || opcodes[opcode].addressing == ADDR_INDY)) cpuCycles++;
}
void PHA() {
	PUSH8(AC);
}
void PHP() {
	SetBreak(1);
	SET_1();
	PUSH8(F);
}
void PLA() {
	AC = PULL8();
	SetNZ(AC);
}
void PLP() {
	F = PULL8();
	F |= 32; // from "other emulator"
}
void ROL() {
	bool carry = value & 0x80;
	value <<= 1;
	if (GetCarry()) value |= 0b1;
	else value &= ~0b1;
	SetCarry(carry);
	SetNZ(value);
	*cell = value;
	writeOperation = true;
}
void ROR() {
	bool carry = value & 1;
	value >>= 1;
	if (GetCarry()) value |= 0b10000000;
	else value &= ~0b10000000;
	SetCarry(carry);
	SetNZ(value);
	*cell = value;
	writeOperation = true;
}
void RTI() {
	F = PULL8();
	F |= 32;
	PC = PULL16();
}
void RTS() {
	PC = PULL16() + 1;
}
void SBC() {
	unsigned short temp = AC - value - (GetCarry() ? 0 : 1);
	SetOverflow(((AC ^ temp) & 0x80) && ((AC ^ value) & 0x80));
	SetCarry(temp < 0x100);
	AC = (temp & 0xff);
	SetNZ(AC);
	if (crossPage && (opcodes[opcode].addressing == ADDR_ABSX || opcodes[opcode].addressing == ADDR_ABSY || opcodes[opcode].addressing == ADDR_INDY)) cpuCycles++;
}
void SEC() {
	SetCarry(1);
}
void SED() {
	SetDecimal(1);
}
void SEI() {
	SetInterrupt(1);
}
void STA() {
	value = AC;
	*cell = AC;
	writeOperation = true;
}
void STX() {
	value = X;
	*cell = X;
	writeOperation = true;
}
void STY() {
	value = Y;
	*cell = Y;
	writeOperation = true;
}
void TAX() {
	X = AC;
	SetNZ(X);
}
void TAY() {
	Y = AC;
	SetNZ(Y);
}
void TSX() {
	X = SP;
	SetNZ(X);
}
void TXA() {
	AC = X;
	SetNZ(AC);
}
void TXS() {
	SP = X;
}
void TYA() {
	AC = Y;
	SetNZ(AC);
}


#define UNOFFICIAL
#ifdef UNOFFICIAL
// Unofficial opcodes
void DOP() {
	// ok
}
void ANC() {
	AC &= value;
	SetNZ(AC);
	SetCarry(AC&F_SIGN);
}
void SAX() {
	*cell = (AC & X);
}
void ARR() {
	AC &= value;
	bool carry = AC & 1;
	AC >>= 1;
	if (GetCarry()) AC |= 0b10000000;
	else AC &= ~0b10000000;
	switch ((AC & 0b1100000)>>5) {
		case 0:
			UNSET_CARRY();
			UNSET_TRANS();
			break;
		case 1:
			SET_TRANS();
			UNSET_CARRY();
			break;
		case 2:
			SET_TRANS();
			SET_CARRY();
			break;
		case 3:
			SET_CARRY();
			UNSET_TRANS();
			break;
	}
	SetNZ(AC);
}
void ALR() {
	AC &= value;
	SetCarry(AC & 1);
	AC = (AC >> 1); // & ~0x80
	SetNZ(AC);
}
void ATX() {
	ORA();
	AND();
	TAX();
}
void AXA() {
	cout << "AXA opcode is unofficial "<<hex<<PC << endl;
	system("pause");
	*cell = (AC & X & 7);
}
void AXS() {
	u16 temp = AC & X;
	temp -= value;
	SetCarry(temp < 0x100);
	X = temp & 0xff;
	SetNZ(X);
}
void DCP() {
	value = (value - 1) & 0xff;
	*cell = value;
	unsigned short temp = (unsigned char)AC - (unsigned char)value;
	SetCarry(temp < 0x100);
	SetNZ(temp & 0xff);
}
void ISC() {
	value = (value + 1) & 0xff;
	*cell = value;
	unsigned short temp = (unsigned char)AC - (unsigned char)value - (GetCarry() ? 0 : 1);
	SetSign(temp);
	SetZero(temp & 0xff);	/* Sign and Zero are invalid in decimal mode */
	SetOverflow(((AC ^ temp) & 0x80) && ((AC ^ value) & 0x80));
	SetCarry(temp < 0x100);
	AC = (temp & 0xff);
}
void KIL() {
	cout << "KIL command is unofficial" << endl;
	system("pause");
}
void LAR() {
	AC = value & SP;
	X = AC;
	SP = AC;
	SetSign(AC);
	SetZero(AC);
}
void LAX() {
	SetSign(value & 0xFF);
	SetZero(value & 0xFF);
	AC = value;
	X = value;
}
void RLA() {
	bool carry = value & 0x80;
	value <<= 1;
	if (GetCarry()) value |= 0b1;
	*cell = value;
	AC &= value;
	SetSign(AC);
	SetZero(AC);
	SetCarry(carry);
}
void RRA() {
	bool carry = value & 0x01;
	value >>= 1;
	if (GetCarry()) value |= 0b10000000;
	SetCarry(carry);
	*cell = value;
	unsigned short temp = value + (unsigned char)AC + (GetCarry() ? 1 : 0);
	SetCarry(temp > 0xff);
	SetZero(temp & 0xff);
	SetSign(temp & F_SIGN);
	SetOverflow(!((AC ^ value) & 0x80) && ((AC ^ temp) & 0x80));
	AC = temp;
}
void SLO() {
	SetCarry(value & 0x80);
	value <<= 1;
	value &= 0xff;
	*cell = value;
	AC |= value;
	SetSign(AC);
	SetZero(AC);
}
void SRE() {
	SetCarry(value & 1);
	value = (value >> 1) & 0x7F;
	*cell = value;
	AC ^= value;
	SetSign(AC);
	SetZero(AC);
}
void SXA() {
	u8 temp = (X & ((address >> 8) + 1)) & 0xff;
	u8 val = (address - Y) & 0xFF;
	if ((Y + val) <= 0xff) {
		*cell = temp;
	}
}
void SYA() {
	u8 temp = (Y & ((address >> 8) + 1)) & 0xff;
	u8 val = (address - X) & 0xFF;
	if ((X + val) <= 0xff) {
		*cell = temp;
	}
}
void TOP() {
	// ok
}
void XAA() {
	cout << "XAA command is unofficial" << endl;

	system("pause");
	TXA();
	AND();
}
void XAS() {
	cout << "XAS command is unofficial" << endl;

	system("pause");
}
#endif

void LoadOpcodesTable() {
	for (int i = 0; i < 256; i++) {
		opcodes[i] = Opcode(i, NONE);
	}
	// unofficial
#ifdef UNOFFICIAL
	opcodes[0x04] = Opcode("DOP", ADDR_ZP, 0x04, DOP, 2);
	opcodes[0x14] = Opcode("DOP", ADDR_ZPX, 0x14, DOP, 2);
	opcodes[0x34] = Opcode("DOP", ADDR_ZPX, 0x34, DOP, 2);
	opcodes[0x44] = Opcode("DOP", ADDR_ZP, 0x44, DOP, 2);
	opcodes[0x54] = Opcode("DOP", ADDR_ZPX, 0x54, DOP, 2);
	opcodes[0x64] = Opcode("DOP", ADDR_ZP, 0x64, DOP, 2);
	opcodes[0x74] = Opcode("DOP", ADDR_ZPX, 0x74, DOP, 2);
	opcodes[0x80] = Opcode("DOP", ADDR_IMM, 0x80, DOP, 2);
	opcodes[0x82] = Opcode("DOP", ADDR_IMM, 0x82, DOP, 2);
	opcodes[0x89] = Opcode("DOP", ADDR_IMM, 0x89, DOP, 2);
	opcodes[0xc2] = Opcode("DOP", ADDR_IMM, 0xc2, DOP, 2);
	opcodes[0xd4] = Opcode("DOP", ADDR_ZPX, 0xd4, DOP, 2);
	opcodes[0xe2] = Opcode("DOP", ADDR_IMM, 0xe2, DOP, 2);
	opcodes[0xf4] = Opcode("DOP", ADDR_ZPX, 0xf4, DOP, 2);
	opcodes[0x0b] = Opcode("ANC", ADDR_IMM, 0x0b, ANC, 2);
	opcodes[0x2b] = Opcode("ANC", ADDR_IMM, 0x2b, ANC, 2);
	opcodes[0x87] = Opcode("SAX", ADDR_ZP, 0x87, SAX, 2);
	opcodes[0x97] = Opcode("SAX", ADDR_ZPY, 0x97, SAX, 2);
	opcodes[0x83] = Opcode("SAX", ADDR_INDX, 0x83, SAX, 2);
	opcodes[0x8f] = Opcode("SAX", ADDR_ABS, 0x8f, SAX, 3);
	opcodes[0x6b] = Opcode("ARR", ADDR_IMM, 0x6b, ARR, 2);
	opcodes[0x4b] = Opcode("ALR", ADDR_IMM, 0x4b, ALR, 2);
	opcodes[0xab] = Opcode("ATX", ADDR_IMM, 0xab, ATX, 2);
	opcodes[0x9f] = Opcode("AXA", ADDR_ABSY, 0x9f, AXA, 3);
	opcodes[0x93] = Opcode("AXA", ADDR_INDY, 0x93, AXA, 2);
	opcodes[0xcb] = Opcode("AXS", ADDR_IMM, 0xcb, AXS, 2);
	opcodes[0xc7] = Opcode("DCP", ADDR_ZP, 0xc7, DCP, 2);
	opcodes[0xd7] = Opcode("DCP", ADDR_ZPX, 0xd7, DCP, 2);
	opcodes[0xcf] = Opcode("DCP", ADDR_ABS, 0xcf, DCP, 3);
	opcodes[0xdf] = Opcode("DCP", ADDR_ABSX, 0xdf, DCP, 3);
	opcodes[0xdb] = Opcode("DCP", ADDR_ABSY, 0xdb, DCP, 3);
	opcodes[0xc3] = Opcode("DCP", ADDR_INDX, 0xc3, DCP, 2);
	opcodes[0xd3] = Opcode("DCP", ADDR_INDY, 0xd3, DCP, 2);
	opcodes[0xe7] = Opcode("ISC", ADDR_ZP, 0xe7, ISC, 2);
	opcodes[0xf7] = Opcode("ISC", ADDR_ZPX, 0xf7, ISC, 2);
	opcodes[0xef] = Opcode("ISC", ADDR_ABS, 0xef, ISC, 3);
	opcodes[0xff] = Opcode("ISC", ADDR_ABSX, 0xff, ISC, 3);
	opcodes[0xfb] = Opcode("ISC", ADDR_ABSY, 0xfb, ISC, 3);
	opcodes[0xe3] = Opcode("ISC", ADDR_INDX, 0xe3, ISC, 2);
	opcodes[0xf3] = Opcode("ISC", ADDR_INDY, 0xf3, ISC, 2);
	opcodes[0x02] = Opcode("KIL", ADDR_IMPL, 0x02, KIL, 1);
	opcodes[0x12] = Opcode("KIL", ADDR_IMPL, 0x12, KIL, 1);
	opcodes[0x22] = Opcode("KIL", ADDR_IMPL, 0x22, KIL, 1);
	opcodes[0x32] = Opcode("KIL", ADDR_IMPL, 0x32, KIL, 1);
	opcodes[0x42] = Opcode("KIL", ADDR_IMPL, 0x42, KIL, 1);
	opcodes[0x52] = Opcode("KIL", ADDR_IMPL, 0x52, KIL, 1);
	opcodes[0x62] = Opcode("KIL", ADDR_IMPL, 0x62, KIL, 1);
	opcodes[0x72] = Opcode("KIL", ADDR_IMPL, 0x72, KIL, 1);
	opcodes[0x92] = Opcode("KIL", ADDR_IMPL, 0x92, KIL, 1);
	opcodes[0xb2] = Opcode("KIL", ADDR_IMPL, 0xb2, KIL, 1);
	opcodes[0xd2] = Opcode("KIL", ADDR_IMPL, 0xd2, KIL, 1);
	opcodes[0xf2] = Opcode("KIL", ADDR_IMPL, 0xf2, KIL, 1);
	opcodes[0xbb] = Opcode("LAR", ADDR_ABSY, 0xbb, LAR, 3);
	opcodes[0xa7] = Opcode("LAX", ADDR_ZP, 0xa7, LAX, 2);
	opcodes[0xb7] = Opcode("LAX", ADDR_ZPY, 0xb7, LAX, 2);
	opcodes[0xaf] = Opcode("LAX", ADDR_ABS, 0xaf, LAX, 3);
	opcodes[0xbf] = Opcode("LAX", ADDR_ABSY, 0xbf, LAX, 3);
	opcodes[0xa3] = Opcode("LAX", ADDR_INDX, 0xa3, LAX, 2);
	opcodes[0xb3] = Opcode("LAX", ADDR_INDY, 0xb3, LAX, 2);
	opcodes[0x27] = Opcode("RLA", ADDR_ZP, 0x27, RLA, 2);
	opcodes[0x37] = Opcode("RLA", ADDR_ZPX, 0x37, RLA, 2);
	opcodes[0x2f] = Opcode("RLA", ADDR_ABS, 0x2f, RLA, 3);
	opcodes[0x3f] = Opcode("RLA", ADDR_ABSX, 0x3f, RLA, 3);
	opcodes[0x3b] = Opcode("RLA", ADDR_ABSY, 0x3b, RLA, 3);
	opcodes[0x23] = Opcode("RLA", ADDR_INDX, 0x23, RLA, 2);
	opcodes[0x33] = Opcode("RLA", ADDR_INDY, 0x33, RLA, 2);
	opcodes[0x67] = Opcode("RRA", ADDR_ZP, 0x67, RRA, 2);
	opcodes[0x77] = Opcode("RRA", ADDR_ZPX, 0x77, RRA, 2);
	opcodes[0x6f] = Opcode("RRA", ADDR_ABS, 0x6f, RRA, 3);
	opcodes[0x7f] = Opcode("RRA", ADDR_ABSX, 0x7f, RRA, 3);
	opcodes[0x7b] = Opcode("RRA", ADDR_ABSY, 0x7b, RRA, 3);
	opcodes[0x63] = Opcode("RRA", ADDR_INDX, 0x63, RRA, 2);
	opcodes[0x73] = Opcode("RRA", ADDR_INDY, 0x73, RRA, 2);
	opcodes[0xeb] = Opcode("SBC", ADDR_IMM, 0xeb, SBC, 2);
	opcodes[0x07] = Opcode("SLO", ADDR_ZP, 0x07, SLO, 2);
	opcodes[0x17] = Opcode("SLO", ADDR_ZPX, 0x17, SLO, 2);
	opcodes[0x0f] = Opcode("SLO", ADDR_ABS, 0x0f, SLO, 3);
	opcodes[0x1f] = Opcode("SLO", ADDR_ABSX, 0x1f, SLO, 3);
	opcodes[0x1b] = Opcode("SLO", ADDR_ABSY, 0x1b, SLO, 3);
	opcodes[0x03] = Opcode("SLO", ADDR_INDX, 0x03, SLO, 2);
	opcodes[0x13] = Opcode("SLO", ADDR_INDY, 0x13, SLO, 2);
	opcodes[0x47] = Opcode("SRE", ADDR_ZP, 0x47, SRE, 2);
	opcodes[0x57] = Opcode("SRE", ADDR_ZPX, 0x57, SRE, 2);
	opcodes[0x4f] = Opcode("SRE", ADDR_ABS, 0x4f, SRE, 3);
	opcodes[0x5f] = Opcode("SRE", ADDR_ABSX, 0x5f, SRE, 3);
	opcodes[0x5b] = Opcode("SRE", ADDR_ABSY, 0x5b, SRE, 3);
	opcodes[0x43] = Opcode("SRE", ADDR_INDX, 0x43, SRE, 2);
	opcodes[0x53] = Opcode("SRE", ADDR_INDY, 0x53, SRE, 2);
	opcodes[0x9e] = Opcode("SXA", ADDR_ABSY, 0x9e, SXA, 3);
	opcodes[0x9c] = Opcode("SYA", ADDR_ABSX, 0x9c, SYA, 3);
	opcodes[0x0c] = Opcode("TOP", ADDR_ABS, 0x0c, TOP, 3);
	opcodes[0x1c] = Opcode("TOP", ADDR_ABSX, 0x1c, TOP, 3);
	opcodes[0x3c] = Opcode("TOP", ADDR_ABSX, 0x3c, TOP, 3);
	opcodes[0x5c] = Opcode("TOP", ADDR_ABSX, 0x4c, TOP, 3);
	opcodes[0x7c] = Opcode("TOP", ADDR_ABSX, 0x7c, TOP, 3);
	opcodes[0xdc] = Opcode("TOP", ADDR_ABSX, 0xdc, TOP, 3);
	opcodes[0xfc] = Opcode("TOP", ADDR_ABSX, 0xfc, TOP, 3);
	opcodes[0x8b] = Opcode("XAA", ADDR_IMM, 0x8b, XAA, 2);
	opcodes[0x9b] = Opcode("XAS", ADDR_ABSY, 0x9b, XAS, 3);
	opcodes[0x1a] = Opcode("NOP", ADDR_IMPL, 0x1a, NOP, 1,2);
	opcodes[0x3a] = Opcode("NOP", ADDR_IMPL, 0x3a, NOP, 1,2);
	opcodes[0x5a] = Opcode("NOP", ADDR_IMPL, 0x5a, NOP, 1,2);
	opcodes[0x7a] = Opcode("NOP", ADDR_IMPL, 0x7a, NOP, 1,2);
	opcodes[0xda] = Opcode("NOP", ADDR_IMPL, 0xda, NOP, 1,2);
	opcodes[0xfa] = Opcode("NOP", ADDR_IMPL, 0xfa, NOP, 1,2);
#endif

	// official
	opcodes[0x69] = Opcode("ADC", ADDR_IMM, 0x69, ADC, 2, 2);
	opcodes[0x65] = Opcode("ADC", ADDR_ZP, 0x65, ADC, 2, 3);
	opcodes[0x75] = Opcode("ADC", ADDR_ZPX, 0x75, ADC, 2, 4);
	opcodes[0x6d] = Opcode("ADC", ADDR_ABS, 0x6d, ADC, 3, 4);
	opcodes[0x7d] = Opcode("ADC", ADDR_ABSX, 0x7d, ADC, 3, 4);
	opcodes[0x79] = Opcode("ADC", ADDR_ABSY, 0x79, ADC, 3, 4);
	opcodes[0x61] = Opcode("ADC", ADDR_INDX, 0x61, ADC, 2, 6);
	opcodes[0x71] = Opcode("ADC", ADDR_INDY, 0x71, ADC, 2, 5);
	opcodes[0x29] = Opcode("AND", ADDR_IMM, 0x29, AND, 2, 2);
	opcodes[0x25] = Opcode("AND", ADDR_ZP, 0x25, AND, 2, 3);
	opcodes[0x35] = Opcode("AND", ADDR_ZPX, 0x35, AND, 2, 4);
	opcodes[0x2d] = Opcode("AND", ADDR_ABS, 0x2d, AND, 3, 4);
	opcodes[0x3d] = Opcode("AND", ADDR_ABSX, 0x3d, AND, 3, 4);
	opcodes[0x39] = Opcode("AND", ADDR_ABSY, 0x39, AND, 3, 4);
	opcodes[0x21] = Opcode("AND", ADDR_INDX, 0x21, AND, 2, 6);
	opcodes[0x31] = Opcode("AND", ADDR_INDY, 0x31, AND, 2, 5);
	opcodes[0x0a] = Opcode("ASL", ADDR_ACC, 0x0a, ASL, 1, 2);
	opcodes[0x06] = Opcode("ASL", ADDR_ZP, 0x06, ASL, 2, 5);
	opcodes[0x16] = Opcode("ASL", ADDR_ZPX, 0x16, ASL, 2, 6);
	opcodes[0x0e] = Opcode("ASL", ADDR_ABS, 0x0e, ASL, 3, 6);
	opcodes[0x1e] = Opcode("ASL", ADDR_ABSX, 0x1e, ASL, 3, 7);
	opcodes[0x90] = Opcode("BCC", ADDR_REL, 0x90, BCC, 2, 2);
	opcodes[0xb0] = Opcode("BCS", ADDR_REL, 0xb0, BCS, 2, 2);
	opcodes[0xf0] = Opcode("BEQ", ADDR_REL, 0xf0, BEQ, 2, 2);
	opcodes[0x24] = Opcode("BIT", ADDR_ZP, 0x24, BIT, 2, 3);
	opcodes[0x2c] = Opcode("BIT", ADDR_ABS, 0x2c, BIT, 3, 4);
	opcodes[0x30] = Opcode("BMI", ADDR_REL, 0x30, BMI, 2, 2);
	opcodes[0xd0] = Opcode("BNE", ADDR_REL, 0xd0, BNE, 2, 2);
	opcodes[0x10] = Opcode("BPL", ADDR_REL, 0x10, BPL, 2, 2);
	opcodes[0x00] = Opcode("BRK", ADDR_IMM, 0x00, BRK, 2, 7);
	opcodes[0x50] = Opcode("BVC", ADDR_REL, 0x50, BVC, 2, 2);
	opcodes[0x70] = Opcode("BVS", ADDR_REL, 0x70, BVS, 2, 2);
	opcodes[0x18] = Opcode("CLC", ADDR_IMPL, 0x18, CLC, 1, 2);
	opcodes[0xd8] = Opcode("CLD", ADDR_IMPL, 0xd8, CLD, 1, 2);
	opcodes[0x58] = Opcode("CLI", ADDR_IMPL, 0x58, CLI, 1, 2);
	opcodes[0xb8] = Opcode("CLV", ADDR_IMPL, 0xb8, CLV, 1, 2);
	opcodes[0xc9] = Opcode("CMP", ADDR_IMM, 0xc9, CMP, 2, 2);
	opcodes[0xc5] = Opcode("CMP", ADDR_ZP, 0xc5, CMP, 2, 3);
	opcodes[0xd5] = Opcode("CMP", ADDR_ZPX, 0xd5, CMP, 2, 4);
	opcodes[0xcd] = Opcode("CMP", ADDR_ABS, 0xcd, CMP, 3, 4);
	opcodes[0xdd] = Opcode("CMP", ADDR_ABSX, 0xdd, CMP, 3, 4);
	opcodes[0xd9] = Opcode("CMP", ADDR_ABSY, 0xd9, CMP, 3, 4);
	opcodes[0xc1] = Opcode("CMP", ADDR_INDX, 0xc1, CMP, 2, 6);
	opcodes[0xd1] = Opcode("CMP", ADDR_INDY, 0xd1, CMP, 2, 5);
	opcodes[0xe0] = Opcode("CPX", ADDR_IMM, 0xe0, CPX, 2, 2);
	opcodes[0xe4] = Opcode("CPX", ADDR_ZP, 0xe4, CPX, 2, 3);
	opcodes[0xec] = Opcode("CPX", ADDR_ABS, 0xec, CPX, 3, 4);
	opcodes[0xc0] = Opcode("CPY", ADDR_IMM, 0xc0, CPY, 2, 2);
	opcodes[0xc4] = Opcode("CPY", ADDR_ZP, 0xc4, CPY, 2, 3);
	opcodes[0xcc] = Opcode("CPY", ADDR_ABS, 0xcc, CPY, 3, 4);
	opcodes[0xc6] = Opcode("DEC", ADDR_ZP, 0xc6, DEC, 2, 5);
	opcodes[0xd6] = Opcode("DEC", ADDR_ZPX, 0xd6, DEC, 2, 6);
	opcodes[0xce] = Opcode("DEC", ADDR_ABS, 0xce, DEC, 3, 6);
	opcodes[0xde] = Opcode("DEC", ADDR_ABSX, 0xde, DEC, 3, 7);
	opcodes[0xca] = Opcode("DEX", ADDR_IMPL, 0xca, DEX, 1, 2);
	opcodes[0x88] = Opcode("DEY", ADDR_IMPL, 0x88, DEY, 1, 2);
	opcodes[0x49] = Opcode("EOR", ADDR_IMM, 0x49, EOR, 2, 2);
	opcodes[0x45] = Opcode("EOR", ADDR_ZP, 0x45, EOR, 2, 3);
	opcodes[0x55] = Opcode("EOR", ADDR_ZPX, 0x55, EOR, 2, 4);
	opcodes[0x4d] = Opcode("EOR", ADDR_ABS, 0x4d, EOR, 3, 4);
	opcodes[0x5d] = Opcode("EOR", ADDR_ABSX, 0x5d, EOR, 3, 4);
	opcodes[0x59] = Opcode("EOR", ADDR_ABSY, 0x59, EOR, 3, 4);
	opcodes[0x41] = Opcode("EOR", ADDR_INDX, 0x41, EOR, 2, 6);
	opcodes[0x51] = Opcode("EOR", ADDR_INDY, 0x51, EOR, 2, 5);
	opcodes[0xe6] = Opcode("INC", ADDR_ZP, 0xe6, INC, 2, 5);
	opcodes[0xf6] = Opcode("INC", ADDR_ZPX, 0xf6, INC, 2, 6);
	opcodes[0xee] = Opcode("INC", ADDR_ABS, 0xee, INC, 3, 6);
	opcodes[0xfe] = Opcode("INC", ADDR_ABSX, 0xfe, INC, 3, 7);
	opcodes[0xe8] = Opcode("INX", ADDR_IMPL, 0xe8, INX, 1, 2);
	opcodes[0xc8] = Opcode("INY", ADDR_IMPL, 0xc8, INY, 1, 2);
	opcodes[0x4c] = Opcode("JMP", ADDR_ABS, 0x4c, JMP, 3, 3);
	opcodes[0x6c] = Opcode("JMP", ADDR_IND, 0x6c, JMP, 3, 5);
	opcodes[0x20] = Opcode("JSR", ADDR_ABS, 0x20, JSR, 3, 6);
	opcodes[0xa9] = Opcode("LDA", ADDR_IMM, 0xa9, LDA, 2, 2);
	opcodes[0xa5] = Opcode("LDA", ADDR_ZP, 0xa5, LDA, 2, 3);
	opcodes[0xb5] = Opcode("LDA", ADDR_ZPX, 0xb5, LDA, 2, 4);
	opcodes[0xad] = Opcode("LDA", ADDR_ABS, 0xad, LDA, 3, 4);
	opcodes[0xbd] = Opcode("LDA", ADDR_ABSX, 0xbd, LDA, 3, 4);
	opcodes[0xb9] = Opcode("LDA", ADDR_ABSY, 0xb9, LDA, 3, 4);
	opcodes[0xa1] = Opcode("LDA", ADDR_INDX, 0xa1, LDA, 2, 6);
	opcodes[0xb1] = Opcode("LDA", ADDR_INDY, 0xb1, LDA, 2, 5);
	opcodes[0xa2] = Opcode("LDX", ADDR_IMM, 0xa2, LDX, 2, 2);
	opcodes[0xa6] = Opcode("LDX", ADDR_ZP, 0xa6, LDX, 2, 3);
	opcodes[0xb6] = Opcode("LDX", ADDR_ZPY, 0xb6, LDX, 2, 4);
	opcodes[0xae] = Opcode("LDX", ADDR_ABS, 0xae, LDX, 3, 4);
	opcodes[0xbe] = Opcode("LDX", ADDR_ABSY, 0xbe, LDX, 3, 4);
	opcodes[0xa0] = Opcode("LDY", ADDR_IMM, 0xa0, LDY, 2, 2);
	opcodes[0xa4] = Opcode("LDY", ADDR_ZP, 0xa4, LDY, 2, 3);
	opcodes[0xb4] = Opcode("LDY", ADDR_ZPX, 0xb4, LDY, 2, 4);
	opcodes[0xac] = Opcode("LDY", ADDR_ABS, 0xac, LDY, 3, 4);
	opcodes[0xbc] = Opcode("LDY", ADDR_ABSX, 0xbc, LDY, 3, 4);
	opcodes[0x4a] = Opcode("LSR", ADDR_ACC, 0x4a, LSR, 1, 2);
	opcodes[0x46] = Opcode("LSR", ADDR_ZP, 0x46, LSR, 2, 5);
	opcodes[0x56] = Opcode("LSR", ADDR_ZPX, 0x56, LSR, 2, 6);
	opcodes[0x4e] = Opcode("LSR", ADDR_ABS, 0x4e, LSR, 3, 6);
	opcodes[0x5e] = Opcode("LSR", ADDR_ABSX, 0x5e, LSR, 3, 7);
	opcodes[0xea] = Opcode("NOP", ADDR_IMPL, 0xea, NOP, 1, 2);
	opcodes[0x09] = Opcode("ORA", ADDR_IMM, 0x09, ORA, 2, 2);
	opcodes[0x05] = Opcode("ORA", ADDR_ZP, 0x05, ORA, 2, 3);
	opcodes[0x15] = Opcode("ORA", ADDR_ZPX, 0x15, ORA, 2, 4);
	opcodes[0x0d] = Opcode("ORA", ADDR_ABS, 0x0d, ORA, 3, 4);
	opcodes[0x1d] = Opcode("ORA", ADDR_ABSX, 0x1d, ORA, 3, 4);
	opcodes[0x19] = Opcode("ORA", ADDR_ABSY, 0x19, ORA, 3, 4);
	opcodes[0x01] = Opcode("ORA", ADDR_INDX, 0x01, ORA, 2, 6);
	opcodes[0x11] = Opcode("ORA", ADDR_INDY, 0x11, ORA, 2, 5);
	opcodes[0x48] = Opcode("PHA", ADDR_IMPL, 0x48, PHA, 1, 3);
	opcodes[0x08] = Opcode("PHP", ADDR_IMPL, 0x08, PHP, 1, 3);
	opcodes[0x68] = Opcode("PLA", ADDR_IMPL, 0x68, PLA, 1, 4);
	opcodes[0x28] = Opcode("PLP", ADDR_IMPL, 0x28, PLP, 1, 4);
	opcodes[0x2a] = Opcode("ROL", ADDR_ACC, 0x2a, ROL, 1, 2);
	opcodes[0x26] = Opcode("ROL", ADDR_ZP, 0x26, ROL, 2, 5);
	opcodes[0x36] = Opcode("ROL", ADDR_ZPX, 0x36, ROL, 2, 6);
	opcodes[0x2e] = Opcode("ROL", ADDR_ABS, 0x2e, ROL, 3, 6);
	opcodes[0x3e] = Opcode("ROL", ADDR_ABSX, 0x3e, ROL, 3, 7);
	opcodes[0x6a] = Opcode("ROR", ADDR_ACC, 0x6a, ROR, 1, 2);
	opcodes[0x66] = Opcode("ROR", ADDR_ZP, 0x66, ROR, 2, 5);
	opcodes[0x76] = Opcode("ROR", ADDR_ZPX, 0x76, ROR, 2, 6);
	opcodes[0x6e] = Opcode("ROR", ADDR_ABS, 0x6e, ROR, 3, 6);
	opcodes[0x7e] = Opcode("ROR", ADDR_ABSX, 0x7e, ROR, 3, 7);
	opcodes[0x40] = Opcode("RTI", ADDR_IMPL, 0x40, RTI, 1, 6);
	opcodes[0x60] = Opcode("RTS", ADDR_IMPL, 0x60, RTS, 1, 6);
	opcodes[0xe9] = Opcode("SBC", ADDR_IMM, 0xe9, SBC, 2, 2);
	opcodes[0xe5] = Opcode("SBC", ADDR_ZP, 0xe5, SBC, 2, 3);
	opcodes[0xf5] = Opcode("SBC", ADDR_ZPX, 0xf5, SBC, 2, 4);
	opcodes[0xed] = Opcode("SBC", ADDR_ABS, 0xed, SBC, 3, 4);
	opcodes[0xfd] = Opcode("SBC", ADDR_ABSX, 0xfd, SBC, 3, 4);
	opcodes[0xf9] = Opcode("SBC", ADDR_ABSY, 0xf9, SBC, 3, 4);
	opcodes[0xe1] = Opcode("SBC", ADDR_INDX, 0xe1, SBC, 2, 6);
	opcodes[0xf1] = Opcode("SBC", ADDR_INDY, 0xf1, SBC, 2, 5);
	opcodes[0x38] = Opcode("SEC", ADDR_IMPL, 0x38, SEC, 1, 2);
	opcodes[0xf8] = Opcode("SED", ADDR_IMPL, 0xf8, SED, 1, 2);
	opcodes[0x78] = Opcode("SEI", ADDR_IMPL, 0x78, SEI, 1, 2);
	opcodes[0x85] = Opcode("STA", ADDR_ZP, 0x85, STA, 2, 3);
	opcodes[0x95] = Opcode("STA", ADDR_ZPX, 0x95, STA, 2, 4);
	opcodes[0x8d] = Opcode("STA", ADDR_ABS, 0x8d, STA, 3, 4);
	opcodes[0x9d] = Opcode("STA", ADDR_ABSX, 0x9d, STA, 3, 5);
	opcodes[0x99] = Opcode("STA", ADDR_ABSY, 0x99, STA, 3, 5);
	opcodes[0x81] = Opcode("STA", ADDR_INDX, 0x81, STA, 2, 6);
	opcodes[0x91] = Opcode("STA", ADDR_INDY, 0x91, STA, 2, 6);
	opcodes[0x86] = Opcode("STX", ADDR_ZP, 0x86, STX, 2, 3);
	opcodes[0x96] = Opcode("STX", ADDR_ZPY, 0x96, STX, 2, 4);
	opcodes[0x8e] = Opcode("STX", ADDR_ABS, 0x8e, STX, 3, 4);
	opcodes[0x84] = Opcode("STY", ADDR_ZP, 0x84, STY, 2, 3);
	opcodes[0x94] = Opcode("STY", ADDR_ZPX, 0x94, STY, 2, 4);
	opcodes[0x8c] = Opcode("STY", ADDR_ABS, 0x8c, STY, 3, 4);
	opcodes[0xaa] = Opcode("TAX", ADDR_IMPL, 0xaa, TAX, 1, 2);
	opcodes[0xa8] = Opcode("TAY", ADDR_IMPL, 0xa8, TAY, 1, 2);
	opcodes[0xba] = Opcode("TSX", ADDR_IMPL, 0xba, TSX, 1, 2);
	opcodes[0x8a] = Opcode("TXA", ADDR_IMPL, 0x8a, TXA, 1, 2);
	opcodes[0x9a] = Opcode("TXS", ADDR_IMPL, 0x9a, TXS, 1, 2);
	opcodes[0x98] = Opcode("TYA", ADDR_IMPL, 0x98, TYA, 1, 2);
}