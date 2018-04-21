#pragma once

#include "Memory.h"
#include "Registers.h"
#include <bitset>
//#include <Windows.h>

void GetOpcode() {
	opcode = ROM[PC];
	PC++;
}
void GetOperand() {
	op = (unsigned char)ROM[PC];
	PC++;
}
void GetOperands() {
	_op[0] = (unsigned char)ROM[PC];
	PC++;
	_op[1] = (unsigned char)ROM[PC];
	PC++;
}

bool writeOperation = false;

short RelAddr(short adr,char offset) {
	return adr + offset;
}
short GetIndirect(short zp) {
	short adr;
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
void REL() {
	value = op;
	address = RelAddr(PC, value);
}
void IMM() {
	value = op & 0xff;
	address = 0;
}
void ZP() {
	address = op & 0xff;
	cell = (unsigned char*)&ZERO_PAGE[address];
	value = ZERO_PAGE[address];
}
void ZPX() {
	address = (op + (unsigned char)X) & 0xff;
	cell = (unsigned char*)&ZERO_PAGE[address];
	value = ZERO_PAGE[address];
}
void ZPY() {
	address = (op + (unsigned char)Y) & 0xff;
	cell = (unsigned char*)&ZERO_PAGE[address];
	value = ZERO_PAGE[address];
}
void ABS() {
	address = op;
	cell = (unsigned char*)&ROM[address];
	value = ROM[op];
}
void ABSX() {
	address = op + (unsigned char)X;
	cell = (unsigned char*)&ROM[address];
	value = ROM[address];
}
void ABSY() {
	address = op + (unsigned char)Y;
	cell = (unsigned char*)&ROM[address];
	value = ROM[address];
}
void INDX() {
	address = GetIndirect(op + (unsigned char)X);
	cell = (unsigned char*)&ROM[address];
	value = ROM[address];
}
void INDY() {
	address = GetIndirect(op) + (unsigned char)Y;
	cell = (unsigned char*)&ROM[address];
	value = ROM[address];
}
void ACC() {
	value = AC;
	cell = (unsigned char*)&AC;
	address = 0;
}
void IND() {
	char* adr = (char*)&address;
	if ((op & 0xff) == 0xff) {
		adr[0] = ROM[op | 0xff];
		adr[1] = ROM[op & (~0xff)];
	}
	else {
		adr[0] = ROM[op];
		adr[1] = ROM[op + 1];
	}
}
void CheckMirrors() {
	if (op >= 0x2000 && op < 0x4000) {
		op = ((op-0x2000) % 8) + 0x2000;
	}
}
struct Opcode {
	char* name;
	char* addressing;
	char code;
	void(*func)();
	char cycles;
	char length;
	Opcode(char* _name,char* _addressing, char _code, void(*_func)(),char _length, char _cycles = 1) {
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
		addressing = "IMPL";
		name = "NONE";
		cycles = 1;
	}
	Opcode() {
		length = 1;
		addressing = "IMM";
		name = "NONE";
	}
	void DecodeOperand() {
		if (addressing == "IMPL") {
			value = 0;
			address = 0;
		}
		else if (addressing == "IMM") {
			IMM();
		}
		else if (addressing == "ZP") {
			ZP();
		}
		else if (addressing == "ZPX") {
			ZPX();
		}
		else if (addressing == "ZPY") {
			ZPY();
		}
		else if (addressing == "ABS") {
			ABS();
		}
		else if (addressing == "ABSX") {
			ABSX();
		}
		else if (addressing == "ABSY") {
			ABSY();
		}
		else if (addressing == "INDX") {
			INDX();
		}
		else if (addressing == "INDY") {
			INDY();
		}
		else if (addressing == "ACC") {
			ACC();
		}
		else if (addressing == "IND") {
			IND();
		}
		else if (addressing == "REL") {
			REL();
		}
	}
	void exec() {
		// Get operand based on size
		switch (length) {
		case 2:
			GetOperand();
			break;
		case 3:
			GetOperands();
			break;
		}		
		CheckMirrors();
		DecodeOperand();

		func();
		//cout << "Value: " << (int)value << " Address: " << address << endl;
	}
};
Opcode opcodes[255];

void PUSH16(short val) {
	if (SP <= 1) {
		cout << "Stack is full on push request!" << endl;
		system("pause");
	}
	char* v = (char*)&val;
	STACK[SP] = v[1];
	SP--;
	STACK[SP] = v[0];
	SP--;
}
void PUSH8(char val) {
	if (SP <= 0) {
		cout << "Stack is full on push request!" << endl;
		system("pause");
	}
	STACK[SP] = val;
	SP--;
}
short PULL16() {
	if (SP >= 0xff) {
		cout << "Stack is empty on pull request!" << endl;
		system("Pause");
		return 0xbfff;
	}
	short val;
	char* v = (char*)&val;
	SP++;
	v[0] = STACK[SP];
	SP++;
	v[1] = STACK[SP];
	return val;
}
char PULL8() {
	if (SP >= 0xff) {
		cout << "Stack is empty on pull request!" << endl;
		system("pause");
	}
	SP++;
	char val = STACK[SP];
	return val;
}

bool ENABLE_DISASM = false;

void Disasm() {
	cout << hex << PC << " : " << opcodes[opcode].name << "_" << opcodes[opcode].addressing << "(" << (int)opcode << ")" << " A:" << (int)(AC & 0xff) << " X:" << (int)(X & 0xff) << " Y:" << (int)(Y & 0xff) << " S:" << (int)(SP) << " P:" << hex << bitset<8>(F & 0xff) << endl;
	cout << address << ":" << (int)value << endl;
}

void Step() {
	GetOpcode();
#ifdef ASM
	Disasm();
#endif
	opcodes[opcode].exec();
	CyclesDown -= opcodes[opcode].cycles;
}

void NONE() {
	if (opcode == 0x0c) {
		
	}
	else {
		
	}
}

void ADC() {
	unsigned short temp = value + (unsigned char)AC + (GetCarry() ? 1 : 0);
	SetCarry(temp > 0xff);
	SetZero(temp & 0xff);
	SetSign(temp & F_SIGN);
	SetOverflow(!((AC ^ value) & 0x80) && ((AC ^ temp) & 0x80));
	AC = temp;
}
void AND() {
	AC &= value;
	SetSign(AC);
	SetZero(AC);
}
void ASL() {
	SetCarry(value & 0x80);
	value <<= 1;
	value &= 0xff;
	SetSign(value);
	SetZero(value);
	*cell = value;
	writeOperation = true;
}
void BCC() {
	if (!GetCarry()) {
		//clk += ((PC & 0xFF00) != (RelAddr(PC, op) & 0xFF00) ? 2 : 1);
		PC = address;
	}
}
void BCS() {
	// jump when carry isnt set TODO
	if (GetCarry()) {
		//clk += ((PC & 0xFF00) != (RelAddr(PC, op) & 0xFF00) ? 2 : 1);
		PC = address;
	}
}
void BEQ() {
	if (GetZero()) {
		//clk += ((PC & 0xFF00) != (RelAddr(PC, op) & 0xFF00) ? 2 : 1);
		PC = address;
	}
}
void BIT() {
	unsigned char temp = value & AC;
	SetSign(value & F_SIGN);
	SetOverflow(value & F_TRANS);
	SetZero(temp);
}
void BMI() {
	if (GetSign()) {
		//clk += ((PC & 0xFF00) != (RelAddr(PC, op) & 0xFF00) ? 2 : 1);
		PC = address;
	}
}
void BNE() {
	if (!GetZero()) {
		//clk += ((PC & 0xFF00) != (RelAddr(PC, op) & 0xFF00) ? 2 : 1);
		PC = address;
	}
}
void BPL() {
	if (!GetSign()) {
		//clk += ((PC & 0xFF00) != (RelAddr(PC, op) & 0xFF00) ? 2 : 1);
		PC = address;
	}
}
void BRK() {
	PC++;
	PUSH16(PC);
	SetBreak(1);             /* Set BFlag before pushing */
	PUSH8(F);
	SetInterrupt(1);
	PC = (ROM[0xFFFE] | (ROM[0xFFFF] << 8));
}
void BVC() {
	if (!GetOverflow()) {
		//clk += ((PC & 0xFF00) != (RelAddr(PC, op) & 0xFF00) ? 2 : 1);
		PC = address;
	}
}
void BVS() {
	if (GetOverflow()) {
		//clk += ((PC & 0xFF00) != (RelAddr(PC, op) & 0xFF00) ? 2 : 1);
		PC = address;
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
	unsigned int temp = AC - value;
	SetCarry(AC >= value);
	SetSign(temp);
	SetZero(temp & 0xff);
}
void CPX() {
	unsigned int temp = X - value;
	SetCarry(X >= value);
	SetSign(temp);
	SetZero(temp & 0xff);
}
void CPY() {
	unsigned int temp = Y - value;
	SetCarry(Y >= value);
	SetSign(temp);
	SetZero(temp & 0xff);
}
void DEC() {
	value = (value - 1) & 0xff;
	SetSign(value);
	SetZero(value);
	*cell = value;
	writeOperation = true;
}
void DEX() {
	unsigned src = X;
	src = (src - 1) & 0xff;
	SetSign(src);
	SetZero(src);
	X = (src);
}
void DEY() {
	unsigned src = Y;
	src = (src - 1) & 0xff;
	SetSign(src);
	SetZero(src);
	Y = (src);
}
void EOR() {
	AC ^= value;
	SetSign(AC);
	SetZero(AC);
}
void INC() {
	value = (value + 1) & 0xff;
	SetSign(value);
	SetZero(value);
	*cell = value;
	writeOperation = true;
}
void INX() {
	unsigned src = X;
	src = (src + 1) & 0xff;
	SetSign(src);
	SetZero(src);
	X = (src);
}
void INY() {
	unsigned src = Y;
	src = (src + 1) & 0xff;
	SetSign(src);
	SetZero(src);
	Y = (src);
}
void JMP() {
	PC = address;
}
void JSR() {
	PUSH16(PC - 1);
	PC = address;
}
void LDA() {
	SetSign(value & 0xFF);
	SetZero(value & 0xFF);
	AC = value;
}
void LDX() {
	X = value;
	SetSign(value);
	SetZero(value);
}
void LDY() {
	Y = value;
	SetSign(value);
	SetZero(value);
}
void LSR() {
	SetCarry(value & 1);
	value = (value >> 1) & 0x7F;
	SetSign(0);
	SetZero(value);
	*cell = value;
	writeOperation = true;
}
void NOP() {

}
void ORA() {
	AC |= value;
	SetSign(AC);
	SetZero(AC);
}
void PHA() {
	PUSH8(AC);
}
void PHP() {
	PUSH8(F);
}
void PLA() {
	AC = PULL8();
	SetSign(AC & 0xff);
	SetZero(AC & 0xff);
}
void PLP() {
	F = PULL8();
}
void ROL() {
	bool carry = value & 0x80;
	value <<= 1;
	if (GetCarry()) value |= 0b1;
	SetCarry(carry);
	SetSign(value);
	SetZero(value);
	*cell = value;
	writeOperation = true;
}
void ROR() {
	bool carry = value & 0x01;
	value >>= 1;
	if (GetCarry()) value |= 0b10000000;
	SetCarry(carry);
	SetSign(value);
	SetZero(value);
	*cell = value;
	writeOperation = true;
}
void RTI() {
	F = PULL8();
	PC = PULL16();
}
void RTS() {
	address = PULL16();
	PC = address + 1;
}
void SBC() {
	unsigned short temp = (unsigned char)AC - (unsigned char)value - (GetCarry() ? 0 : 1);
	SetSign(temp);
	SetZero(temp & 0xff);	/* Sign and Zero are invalid in decimal mode */
	SetOverflow(((AC ^ temp) & 0x80) && ((AC ^ value) & 0x80));
	SetCarry(temp < 0x100);
	AC = (temp & 0xff);
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
	writeOperation = true;
	*cell = AC;
}
void STX() {
	writeOperation = true;
	*cell = X;
}
void STY() {
	writeOperation = true;
	*cell = Y;
}
void TAX() {
	SetSign(AC & 0xff);
	SetZero(AC & 0xff);
	X = AC&0xff;
}
void TAY() {
	SetSign(AC & 0xFF);
	SetZero(AC & 0xff);
	Y = AC&0xff;
}
void TSX() {
	X = SP & 0xff;
	SetSign(X & 0xff);
	SetZero(X & 0xff);
}
void TXA() {
	AC = X&0xff;
	SetSign(AC & 0xff);
	SetZero(AC & 0xff);
}
void TXS() {
	SP = X & 0xff;
}
void TYA() {
	AC = Y & 0xff;
	SetSign(AC & 0xff);
	SetZero(AC & 0xff);
}


#define UNOFFICIAL
#ifdef UNOFFICIAL
// Unofficial opcodes
void DOP() {
	// ok
}
void ANC() {
	value &= AC;
	SetZero(value);
	SetSign(value);
	SetCarry(GetSign());
}
void SAX() {
	*cell = (AC & X);
}
void ARR() {
	// store? ok
	AC &= value;
	bool carry = AC & 0x1;
	AC >>= 1;
	if (GetCarry()) AC |= 0b10000000;
	SetCarry(carry);
	SetSign(AC);
	SetZero(AC);
}
void ALR() {
	// store? ok
	AC &= value;
	bool carry = AC & 0x80;
	AC <<= 1;
	if (GetCarry()) AC |= 0b1;
	SetCarry(carry);
	SetSign(AC);
	SetZero(AC);
}
void ATX() {
	// ok
	AC &= X;
	X = AC;
	SetSign(AC);
	SetZero(AC);
}
void AXA() {
	system("pause");
	*cell = (AC & X & 7);
}
void AXS() {
	unsigned short temp = X & AC;
	temp -= value;
	X = temp;
	SetSign(X);
	SetZero(X);
	SetCarry(temp > 0xff);
}
void DCP() {
	value = (value - 1) & 0xff;
	*cell = value;
	unsigned short temp = (unsigned char)AC - (unsigned char)value;
	SetCarry(temp < 0x100);
	SetSign(temp);
	SetZero(temp & 0xff);
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
	system("pause");


}
void SYA() {
	system("pause");

}
void TOP() {
	// ok
}
void XAA() {
	system("pause");
	TXA();
	AND();
}
void XAS() {
	system("pause");
}
#endif

void LoadOpcodesTable() {
	for (int i = 0; i < 256; i++) {
		opcodes[i] = Opcode(i, NONE);
	}
	// unofficial
#ifdef UNOFFICIAL
	opcodes[0x04] = Opcode("DOP", "ZP", 0x04, DOP, 2);
	opcodes[0x14] = Opcode("DOP", "ZPX", 0x14, DOP, 2);
	opcodes[0x34] = Opcode("DOP", "ZPX", 0x34, DOP, 2);
	opcodes[0x44] = Opcode("DOP", "ZP", 0x44, DOP, 2);
	opcodes[0x54] = Opcode("DOP", "ZPX", 0x54, DOP, 2);
	opcodes[0x64] = Opcode("DOP", "ZP", 0x64, DOP, 2);
	opcodes[0x74] = Opcode("DOP", "ZPX", 0x74, DOP, 2);
	opcodes[0x80] = Opcode("DOP", "IMM", 0x80, DOP, 2);
	opcodes[0x82] = Opcode("DOP", "IMM", 0x82, DOP, 2);
	opcodes[0x89] = Opcode("DOP", "IMM", 0x89, DOP, 2);
	opcodes[0xc2] = Opcode("DOP", "IMM", 0xc2, DOP, 2);
	opcodes[0xd4] = Opcode("DOP", "ZPX", 0xd4, DOP, 2);
	opcodes[0xe2] = Opcode("DOP", "IMM", 0xe2, DOP, 2);
	opcodes[0xf4] = Opcode("DOP", "ZPX", 0xf4, DOP, 2);
	opcodes[0x0b] = Opcode("ANC", "IMM", 0x0b, ANC, 2);
	opcodes[0x2b] = Opcode("ANC", "IMM", 0x2b, ANC, 2);
	opcodes[0x87] = Opcode("SAX", "ZP", 0x87, SAX, 2);
	opcodes[0x97] = Opcode("SAX", "ZPY", 0x97, SAX, 2);
	opcodes[0x83] = Opcode("SAX", "INDX", 0x83, SAX, 2);
	opcodes[0x8f] = Opcode("SAX", "ABS", 0x8f, SAX, 3);
	opcodes[0x6b] = Opcode("ARR", "IMM", 0x6b, ARR, 2);
	opcodes[0x4b] = Opcode("ALR", "IMM", 0x4b, ALR, 2);
	opcodes[0xab] = Opcode("ATX", "IMM", 0xab, ATX, 2);
	opcodes[0x9f] = Opcode("AXA", "ABSY", 0x9f, AXA, 3);
	opcodes[0x93] = Opcode("AXA", "INDY", 0x93, AXA, 2);
	opcodes[0xcb] = Opcode("AXS", "IMM", 0xcb, AXS, 2);
	opcodes[0xc7] = Opcode("DCP", "ZP", 0xc7, DCP, 2);
	opcodes[0xd7] = Opcode("DCP", "ZPX", 0xd7, DCP, 2);
	opcodes[0xcf] = Opcode("DCP", "ABS", 0xcf, DCP, 3);
	opcodes[0xdf] = Opcode("DCP", "ABSX", 0xdf, DCP, 3);
	opcodes[0xdb] = Opcode("DCP", "ABSY", 0xdb, DCP, 3);
	opcodes[0xc3] = Opcode("DCP", "INDX", 0xc3, DCP, 2);
	opcodes[0xd3] = Opcode("DCP", "INDY", 0xd3, DCP, 2);
	opcodes[0xe7] = Opcode("ISC", "ZP", 0xe7, ISC, 2);
	opcodes[0xf7] = Opcode("ISC", "ZPX", 0xf7, ISC, 2);
	opcodes[0xef] = Opcode("ISC", "ABS", 0xef, ISC, 3);
	opcodes[0xff] = Opcode("ISC", "ABSX", 0xff, ISC, 3);
	opcodes[0xfb] = Opcode("ISC", "ABSY", 0xfb, ISC, 3);
	opcodes[0xe3] = Opcode("ISC", "INDX", 0xe3, ISC, 2);
	opcodes[0xf3] = Opcode("ISC", "INDY", 0xf3, ISC, 2);
	opcodes[0x02] = Opcode("KIL", "IMPL", 0x02, KIL, 1);
	opcodes[0x12] = Opcode("KIL", "IMPL", 0x12, KIL, 1);
	opcodes[0x22] = Opcode("KIL", "IMPL", 0x22, KIL, 1);
	opcodes[0x32] = Opcode("KIL", "IMPL", 0x32, KIL, 1);
	opcodes[0x42] = Opcode("KIL", "IMPL", 0x42, KIL, 1);
	opcodes[0x52] = Opcode("KIL", "IMPL", 0x52, KIL, 1);
	opcodes[0x62] = Opcode("KIL", "IMPL", 0x62, KIL, 1);
	opcodes[0x72] = Opcode("KIL", "IMPL", 0x72, KIL, 1);
	opcodes[0x92] = Opcode("KIL", "IMPL", 0x92, KIL, 1);
	opcodes[0xb2] = Opcode("KIL", "IMPL", 0xb2, KIL, 1);
	opcodes[0xd2] = Opcode("KIL", "IMPL", 0xd2, KIL, 1);
	opcodes[0xf2] = Opcode("KIL", "IMPL", 0xf2, KIL, 1);
	opcodes[0xbb] = Opcode("LAR", "ABSY", 0xbb, LAR, 3);
	opcodes[0xa7] = Opcode("LAX", "ZP", 0xa7, LAX, 2);
	opcodes[0xb7] = Opcode("LAX", "ZPY", 0xb7, LAX, 2);
	opcodes[0xaf] = Opcode("LAX", "ABS", 0xaf, LAX, 3);
	opcodes[0xbf] = Opcode("LAX", "ABSY", 0xbf, LAX, 3);
	opcodes[0xa3] = Opcode("LAX", "INDX", 0xa3, LAX, 2);
	opcodes[0xb3] = Opcode("LAX", "INDY", 0xb3, LAX, 2);
	opcodes[0x1a] = Opcode("NOP", "IMPL", 0x1a, NOP, 1);
	opcodes[0x3a] = Opcode("NOP", "IMPL", 0x3a, NOP, 1);
	opcodes[0x5a] = Opcode("NOP", "IMPL", 0x5a, NOP, 1);
	opcodes[0x7a] = Opcode("NOP", "IMPL", 0x7a, NOP, 1);
	opcodes[0xda] = Opcode("NOP", "IMPL", 0xda, NOP, 1);
	opcodes[0xfa] = Opcode("NOP", "IMPL", 0xfa, NOP, 1);
	opcodes[0x27] = Opcode("RLA", "ZP", 0x27, RLA, 2);
	opcodes[0x37] = Opcode("RLA", "ZPX", 0x37, RLA, 2);
	opcodes[0x2f] = Opcode("RLA", "ABS", 0x2f, RLA, 3);
	opcodes[0x3f] = Opcode("RLA", "ABSX", 0x3f, RLA, 3);
	opcodes[0x3b] = Opcode("RLA", "ABSY", 0x3b, RLA, 3);
	opcodes[0x23] = Opcode("RLA", "INDX", 0x23, RLA, 2);
	opcodes[0x33] = Opcode("RLA", "INDY", 0x33, RLA, 2);
	opcodes[0x67] = Opcode("RRA", "ZP", 0x67, RRA, 2);
	opcodes[0x77] = Opcode("RRA", "ZPX", 0x77, RRA, 2);
	opcodes[0x6f] = Opcode("RRA", "ABS", 0x6f, RRA, 3);
	opcodes[0x7f] = Opcode("RRA", "ABSX", 0x7f, RRA, 3);
	opcodes[0x7b] = Opcode("RRA", "ABSY", 0x7b, RRA, 3);
	opcodes[0x63] = Opcode("RRA", "INDX", 0x63, RRA, 2);
	opcodes[0x73] = Opcode("RRA", "INDY", 0x73, RRA, 2);
	opcodes[0xeb] = Opcode("SBC", "IMM", 0xeb, SBC, 2);
	opcodes[0x07] = Opcode("SLO", "ZP", 0x07, SLO, 2);
	opcodes[0x17] = Opcode("SLO", "ZPX", 0x17, SLO, 2);
	opcodes[0x0f] = Opcode("SLO", "ABS", 0x0f, SLO, 3);
	opcodes[0x1f] = Opcode("SLO", "ABSX", 0x1f, SLO, 3);
	opcodes[0x1b] = Opcode("SLO", "ABSY", 0x1b, SLO, 3);
	opcodes[0x03] = Opcode("SLO", "INDX", 0x03, SLO, 2);
	opcodes[0x13] = Opcode("SLO", "INDY", 0x13, SLO, 2);
	opcodes[0x47] = Opcode("SRE", "ZP", 0x47, SRE, 2);
	opcodes[0x57] = Opcode("SRE", "ZPX", 0x57, SRE, 2);
	opcodes[0x4f] = Opcode("SRE", "ABS", 0x4f, SRE, 3);
	opcodes[0x5f] = Opcode("SRE", "ABSX", 0x5f, SRE, 3);
	opcodes[0x5b] = Opcode("SRE", "ABSY", 0x5b, SRE, 3);
	opcodes[0x43] = Opcode("SRE", "INDX", 0x43, SRE, 2);
	opcodes[0x53] = Opcode("SRE", "INDY", 0x53, SRE, 2);
	opcodes[0x9e] = Opcode("SXA", "ABSY", 0x9e, SXA, 3);
	opcodes[0x9c] = Opcode("SYA", "ABSX", 0x9c, SYA, 3);
	opcodes[0x0c] = Opcode("TOP", "ABS", 0x0c, TOP, 3);
	opcodes[0x1c] = Opcode("TOP", "ABSX", 0x1c, TOP, 3);
	opcodes[0x3c] = Opcode("TOP", "ABSX", 0x3c, TOP, 3);
	opcodes[0x5c] = Opcode("TOP", "ABSX", 0x4c, TOP, 3);
	opcodes[0x7c] = Opcode("TOP", "ABSX", 0x7c, TOP, 3);
	opcodes[0xdc] = Opcode("TOP", "ABSX", 0xdc, TOP, 3);
	opcodes[0xfc] = Opcode("TOP", "ABSX", 0xfc, TOP, 3);
	opcodes[0x8b] = Opcode("XAA", "IMM", 0x8b, XAA, 2);
	opcodes[0x9b] = Opcode("XAS", "ABSY", 0x9b, XAS, 3);
#endif


	// official
	opcodes[0x69] = Opcode("ADC", "IMM", 0x69, ADC, 2, 2);
	opcodes[0x65] = Opcode("ADC", "ZP", 0x65, ADC, 2, 3);
	opcodes[0x75] = Opcode("ADC", "ZPX", 0x75, ADC, 2, 4);
	opcodes[0x6d] = Opcode("ADC", "ABS", 0x6d, ADC, 3, 4);
	opcodes[0x7d] = Opcode("ADC", "ABSX", 0x7d, ADC, 3, 4);
	opcodes[0x79] = Opcode("ADC", "ABSY", 0x79, ADC, 3, 4);
	opcodes[0x61] = Opcode("ADC", "INDX", 0x61, ADC, 2, 6);
	opcodes[0x71] = Opcode("ADC", "INDY", 0x71, ADC, 2, 5);
	opcodes[0x29] = Opcode("AND", "IMM", 0x29, AND, 2, 2);
	opcodes[0x25] = Opcode("AND", "ZP", 0x25, AND, 2, 3);
	opcodes[0x35] = Opcode("AND", "ZPX", 0x35, AND, 2, 4);
	opcodes[0x2d] = Opcode("AND", "ABS", 0x2d, AND, 3, 4);
	opcodes[0x3d] = Opcode("AND", "ABSX", 0x3d, AND, 3, 4);
	opcodes[0x39] = Opcode("AND", "ABSY", 0x39, AND, 3, 4);
	opcodes[0x21] = Opcode("AND", "INDX", 0x21, AND, 2, 6);
	opcodes[0x31] = Opcode("AND", "INDY", 0x31, AND, 2, 5);
	opcodes[0x0a] = Opcode("ASL", "ACC", 0x0a, ASL, 1, 2);
	opcodes[0x06] = Opcode("ASL", "ZP", 0x06, ASL, 2, 5);
	opcodes[0x16] = Opcode("ASL", "ZPX", 0x16, ASL, 2, 6);
	opcodes[0x0e] = Opcode("ASL", "ABS", 0x0e, ASL, 3, 6);
	opcodes[0x1e] = Opcode("ASL", "ABSX", 0x1e, ASL, 3, 7);
	opcodes[0x90] = Opcode("BCC", "REL", 0x90, BCC, 2, 2);
	opcodes[0xb0] = Opcode("BCS", "REL", 0xb0, BCS, 2, 2);
	opcodes[0xf0] = Opcode("BEQ", "REL", 0xf0, BEQ, 2, 2);
	opcodes[0x24] = Opcode("BIT", "ZP", 0x24, BIT, 2, 3);
	opcodes[0x2c] = Opcode("BIT", "ABS", 0x2c, BIT, 3, 4);
	opcodes[0x30] = Opcode("BMI", "REL", 0x30, BMI, 2, 2);
	opcodes[0xd0] = Opcode("BNE", "REL", 0xd0, BNE, 2, 2);
	opcodes[0x10] = Opcode("BPL", "REL", 0x10, BPL, 2, 2);
	opcodes[0x00] = Opcode("BRK", "IMPL", 0x00, BRK, 2, 7);
	opcodes[0x50] = Opcode("BVC", "REL", 0x50, BVC, 2, 2);
	opcodes[0x70] = Opcode("BVS", "REL", 0x70, BVS, 2, 2);
	opcodes[0x18] = Opcode("CLC", "IMPL", 0x18, CLC, 1, 2);
	opcodes[0xd8] = Opcode("CLD", "IMPL", 0xd8, CLD, 1, 2);
	opcodes[0x58] = Opcode("CLI", "IMPL", 0x58, CLI, 1, 2);
	opcodes[0xb8] = Opcode("CLV", "IMPL", 0xb8, CLV, 1, 2);
	opcodes[0xc9] = Opcode("CMP", "IMM", 0xc9, CMP, 2, 2);
	opcodes[0xc5] = Opcode("CMP", "ZP", 0xc5, CMP, 2, 3);
	opcodes[0xd5] = Opcode("CMP", "ZPX", 0xd5, CMP, 2, 4);
	opcodes[0xcd] = Opcode("CMP", "ABS", 0xcd, CMP, 3, 4);
	opcodes[0xdd] = Opcode("CMP", "ABSX", 0xdd, CMP, 3, 4);
	opcodes[0xd9] = Opcode("CMP", "ABSY", 0xd9, CMP, 3, 4);
	opcodes[0xc1] = Opcode("CMP", "INDX", 0xc1, CMP, 2, 6);
	opcodes[0xd1] = Opcode("CMP", "INDY", 0xd1, CMP, 2, 5);
	opcodes[0xe0] = Opcode("CPX", "IMM", 0xe0, CPX, 2, 2);
	opcodes[0xe4] = Opcode("CPX", "ZP", 0xe4, CPX, 2, 3);
	opcodes[0xec] = Opcode("CPX", "ABS", 0xec, CPX, 3, 4);
	opcodes[0xc0] = Opcode("CPY", "IMM", 0xc0, CPY, 2, 2);
	opcodes[0xc4] = Opcode("CPY", "ZP", 0xc4, CPY, 2, 3);
	opcodes[0xcc] = Opcode("CPY", "ABS", 0xcc, CPY, 3, 4);
	opcodes[0xc6] = Opcode("DEC", "ZP", 0xc6, DEC, 2, 5);
	opcodes[0xd6] = Opcode("DEC", "ZPX", 0xd6, DEC, 2, 6);
	opcodes[0xce] = Opcode("DEC", "ABS", 0xce, DEC, 3, 6);
	opcodes[0xde] = Opcode("DEC", "ABSX", 0xde, DEC, 3, 7);
	opcodes[0xca] = Opcode("DEX", "IMPL", 0xca, DEX, 1, 2);
	opcodes[0x88] = Opcode("DEY", "IMPL", 0x88, DEY, 1, 2);
	opcodes[0x49] = Opcode("EOR", "IMM", 0x49, EOR, 2, 2);
	opcodes[0x45] = Opcode("EOR", "ZP", 0x45, EOR, 2, 3);
	opcodes[0x55] = Opcode("EOR", "ZPX", 0x55, EOR, 2, 4);
	opcodes[0x4d] = Opcode("EOR", "ABS", 0x4d, EOR, 3, 4);
	opcodes[0x5d] = Opcode("EOR", "ABSX", 0x5d, EOR, 3, 4);
	opcodes[0x59] = Opcode("EOR", "ABSY", 0x59, EOR, 3, 4);
	opcodes[0x41] = Opcode("EOR", "INDX", 0x41, EOR, 2, 6);
	opcodes[0x51] = Opcode("EOR", "INDY", 0x51, EOR, 2, 5);
	opcodes[0xe6] = Opcode("INC", "ZP", 0xe6, INC, 2, 5);
	opcodes[0xf6] = Opcode("INC", "ZPX", 0xf6, INC, 2, 6);
	opcodes[0xee] = Opcode("INC", "ABS", 0xee, INC, 3, 6);
	opcodes[0xfe] = Opcode("INC", "ABSX", 0xfe, INC, 3, 7);
	opcodes[0xe8] = Opcode("INX", "IMPL", 0xe8, INX, 1, 2);
	opcodes[0xc8] = Opcode("INY", "IMPL", 0xc8, INY, 1, 2);
	opcodes[0x4c] = Opcode("JMP", "ABS", 0x4c, JMP, 3, 3);
	opcodes[0x6c] = Opcode("JMP", "IND", 0x6c, JMP, 3, 5);
	opcodes[0x20] = Opcode("JSR", "ABS", 0x20, JSR, 3, 6);
	opcodes[0xa9] = Opcode("LDA", "IMM", 0xa9, LDA, 2, 2);
	opcodes[0xa5] = Opcode("LDA", "ZP", 0xa5, LDA, 2, 3);
	opcodes[0xb5] = Opcode("LDA", "ZPX", 0xb5, LDA, 2, 4);
	opcodes[0xad] = Opcode("LDA", "ABS", 0xad, LDA, 3, 4);
	opcodes[0xbd] = Opcode("LDA", "ABSX", 0xbd, LDA, 3, 4);
	opcodes[0xb9] = Opcode("LDA", "ABSY", 0xb9, LDA, 3, 4);
	opcodes[0xa1] = Opcode("LDA", "INDX", 0xa1, LDA, 2, 6);
	opcodes[0xb1] = Opcode("LDA", "INDY", 0xb1, LDA, 2, 5);
	opcodes[0xa2] = Opcode("LDX", "IMM", 0xa2, LDX, 2, 2);
	opcodes[0xa6] = Opcode("LDX", "ZP", 0xa6, LDX, 2, 3);
	opcodes[0xb6] = Opcode("LDX", "ZPY", 0xb6, LDX, 2, 4);
	opcodes[0xae] = Opcode("LDX", "ABS", 0xae, LDX, 3, 4);
	opcodes[0xbe] = Opcode("LDX", "ABSY", 0xbe, LDX, 3, 4);
	opcodes[0xa0] = Opcode("LDY", "IMM", 0xa0, LDY, 2, 2);
	opcodes[0xa4] = Opcode("LDY", "ZP", 0xa4, LDY, 2, 3);
	opcodes[0xb4] = Opcode("LDY", "ZPX", 0xb4, LDY, 2, 4);
	opcodes[0xac] = Opcode("LDY", "ABS", 0xac, LDY, 3, 4);
	opcodes[0xbc] = Opcode("LDY", "ABSX", 0xbc, LDY, 3, 4);
	opcodes[0x4a] = Opcode("LSR", "ACC", 0x4a, LSR, 1, 2);
	opcodes[0x46] = Opcode("LSR", "ZP", 0x46, LSR, 2, 5);
	opcodes[0x56] = Opcode("LSR", "ZPX", 0x56, LSR, 2, 6);
	opcodes[0x4e] = Opcode("LSR", "ABS", 0x4e, LSR, 3, 6);
	opcodes[0x5e] = Opcode("LSR", "ABSX", 0x5e, LSR, 3, 7);
	opcodes[0xea] = Opcode("NOP", "IMPL", 0xea, NOP, 1, 2);
	opcodes[0x09] = Opcode("ORA", "IMM", 0x09, ORA, 2, 2);
	opcodes[0x05] = Opcode("ORA", "ZP", 0x05, ORA, 2, 3);
	opcodes[0x15] = Opcode("ORA", "ZPX", 0x15, ORA, 2, 4);
	opcodes[0x0d] = Opcode("ORA", "ABS", 0x0d, ORA, 3, 4);
	opcodes[0x1d] = Opcode("ORA", "ABSX", 0x1d, ORA, 3, 4);
	opcodes[0x19] = Opcode("ORA", "ABSY", 0x19, ORA, 3, 4);
	opcodes[0x01] = Opcode("ORA", "INDX", 0x01, ORA, 2, 6);
	opcodes[0x11] = Opcode("ORA", "INDY", 0x11, ORA, 2, 5);
	opcodes[0x48] = Opcode("PHA", "IMPL", 0x48, PHA, 1, 3);
	opcodes[0x08] = Opcode("PHP", "IMPL", 0x08, PHP, 1, 3);
	opcodes[0x68] = Opcode("PLA", "IMPL", 0x68, PLA, 1, 4);
	opcodes[0x28] = Opcode("PLP", "IMPL", 0x28, PLP, 1, 4);
	opcodes[0x2a] = Opcode("ROL", "ACC", 0x2a, ROL, 1, 2);
	opcodes[0x26] = Opcode("ROL", "ZP", 0x26, ROL, 2, 5);
	opcodes[0x36] = Opcode("ROL", "ZPX", 0x36, ROL, 2, 6);
	opcodes[0x2e] = Opcode("ROL", "ABS", 0x2e, ROL, 3, 6);
	opcodes[0x3e] = Opcode("ROL", "ABSX", 0x3e, ROL, 3, 7);
	opcodes[0x6a] = Opcode("ROR", "ACC", 0x6a, ROR, 1, 2);
	opcodes[0x66] = Opcode("ROR", "ZP", 0x66, ROR, 2, 5);
	opcodes[0x76] = Opcode("ROR", "ZPX", 0x76, ROR, 2, 6);
	opcodes[0x6e] = Opcode("ROR", "ABS", 0x6e, ROR, 3, 6);
	opcodes[0x7e] = Opcode("ROR", "ABSX", 0x7e, ROR, 3, 7);
	opcodes[0x40] = Opcode("RTI", "IMPL", 0x40, RTI, 1, 6);
	opcodes[0x60] = Opcode("RTS", "IMPL", 0x60, RTS, 1, 6);
	opcodes[0xe9] = Opcode("SBC", "IMM", 0xe9, SBC, 2, 2);
	opcodes[0xe5] = Opcode("SBC", "ZP", 0xe5, SBC, 2, 3);
	opcodes[0xf5] = Opcode("SBC", "ZPX", 0xf5, SBC, 2, 4);
	opcodes[0xed] = Opcode("SBC", "ABS", 0xed, SBC, 3, 4);
	opcodes[0xfd] = Opcode("SBC", "ABSX", 0xfd, SBC, 3, 4);
	opcodes[0xf9] = Opcode("SBC", "ABSY", 0xf9, SBC, 3, 4);
	opcodes[0xe1] = Opcode("SBC", "INDX", 0xe1, SBC, 2, 6);
	opcodes[0xf1] = Opcode("SBC", "INDY", 0xf1, SBC, 2, 5);
	opcodes[0x38] = Opcode("SEC", "IMPL", 0x38, SEC, 1, 2);
	opcodes[0xf8] = Opcode("SED", "IMPL", 0xf8, SED, 1, 2);
	opcodes[0x78] = Opcode("SEI", "IMPL", 0x78, SEI, 1, 2);
	opcodes[0x85] = Opcode("STA", "ZP", 0x85, STA, 2, 3);
	opcodes[0x95] = Opcode("STA", "ZPX", 0x95, STA, 2, 4);
	opcodes[0x8d] = Opcode("STA", "ABS", 0x8d, STA, 3, 4);
	opcodes[0x9d] = Opcode("STA", "ABSX", 0x9d, STA, 3, 5);
	opcodes[0x99] = Opcode("STA", "ABSY", 0x99, STA, 3, 5);
	opcodes[0x81] = Opcode("STA", "INDX", 0x81, STA, 2, 6);
	opcodes[0x91] = Opcode("STA", "INDY", 0x91, STA, 2, 6);
	opcodes[0x86] = Opcode("STX", "ZP", 0x86, STX, 2, 3);
	opcodes[0x96] = Opcode("STX", "ZPY", 0x96, STX, 2, 4);
	opcodes[0x8e] = Opcode("STX", "ABS", 0x8e, STX, 3, 4);
	opcodes[0x84] = Opcode("STY", "ZP", 0x84, STY, 2, 3);
	opcodes[0x94] = Opcode("STY", "ZPX", 0x94, STY, 2, 4);
	opcodes[0x8c] = Opcode("STY", "ABS", 0x8c, STY, 3, 4);
	opcodes[0xaa] = Opcode("TAX", "IMPL", 0xaa, TAX, 1, 2);
	opcodes[0xa8] = Opcode("TAY", "IMPL", 0xa8, TAY, 1, 2);
	opcodes[0xba] = Opcode("TSX", "IMPL", 0xba, TSX, 1, 2);
	opcodes[0x8a] = Opcode("TXA", "IMPL", 0x8a, TXA, 1, 2);
	opcodes[0x9a] = Opcode("TXS", "IMPL", 0x9a, TXS, 1, 2);
	opcodes[0x98] = Opcode("TYA", "IMPL", 0x98, TYA, 1, 2);
}