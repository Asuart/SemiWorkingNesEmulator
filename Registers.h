#pragma once
unsigned char AC, F, X, Y;
unsigned char SP;
unsigned short PC;
unsigned char opcode;

unsigned short op; // operand after instruction
unsigned char* _op = (unsigned char*)&op;

// vram address and toggle to swap writing byte in it
unsigned short vramPointer;
unsigned short tempVramPointer;
char* _vramPointer = (char*)&tempVramPointer;
bool lowVramPointer = true;

// decode addressing mode to those values
unsigned short address;
unsigned char value;
unsigned char* cell; // to write value back

// to read input and some other sings
bool writeOperation = false;

unsigned char scrollX, scrollY;
unsigned char OAMAddress;

unsigned int cycle = 0;
int CyclesDown = 0;

#define SET_CARRY() F |= F_CARRY
#define UNSET_CARRY() F &= ~F_CARRY
#define SET_ZERO() F |= F_ZERO
#define UNSET_ZERO() F &= ~F_ZERO
#define SET_INT() F |= F_INT
#define UNSET_INT() F &= ~F_INT
#define SET_DECIMAL() F |= F_DECIMAL
#define UNSET_DECIMAL() F &= ~F_DECIMAL
#define SET_BREAK() F |= F_BREAK
#define UNSET_BREAK() F &= ~F_BREAK
#define SET_1() F |= F_1
#define UNSET_1() F &= ~F_1
#define SET_TRANS() F |= F_TRANS
#define UNSET_TRANS() F &= ~F_TRANS
#define SET_SIGN() F |= F_SIGN
#define UNSET_SIGN() F &= ~F_SIGN

// register flags
const unsigned char F_CARRY = 0b1; // carry flag
const unsigned char F_ZERO = 0b10; // zero flag
const unsigned char F_INT = 0b100; // interrupt flag
const unsigned char F_DECIMAL = 0b1000; // decimal flag not used in dendy
const unsigned char F_BREAK = 0b10000; // break flag
const unsigned char F_1 = 0b100000; // 1 flag
const unsigned char F_TRANS = 0b1000000; // transfer to sign bit flag
const unsigned char F_SIGN = 0b10000000; // sign flag

// get flags
bool GetFlag(char flagName) {
	return F & flagName;
}
bool GetCarry() {
	return F & F_CARRY;
}
bool GetZero() {
	return F & F_ZERO;
}
bool GetInterrupt() {
	return F & F_INT;
}
bool GetDecimal() {
	return F & F_DECIMAL;
}
bool GetBreak() {
	return F & F_BREAK;
}
bool Get1() {
	return F & F_1;
}
bool GetOverflow() {
	return F & F_TRANS;
}
bool GetSign() {
	return F & F_SIGN;
}

//set flags if get true, else unset
void SetCarry(bool b) {
	if (b) SET_CARRY();
	else UNSET_CARRY();
}
void SetZero(bool b) {
	if (b) UNSET_ZERO();
	else SET_ZERO();
}
void SetSign(char b) {
	if (b & F_SIGN) SET_SIGN();
	else UNSET_SIGN();
}
void SetOverflow(bool b) {
	if (b) SET_TRANS();
	else UNSET_TRANS();
}
void SetInterrupt(bool b) {
	if (b) SET_INT();
	else UNSET_INT();
}
void SetBreak(bool b) {
	if (b) SET_BREAK();
	else UNSET_BREAK();
}
void SetDecimal(bool b) {
	if (b)SET_DECIMAL();
	else UNSET_DECIMAL();
}
void Set1(bool b) {
	if (b)SET_1();
	else UNSET_1();
}
