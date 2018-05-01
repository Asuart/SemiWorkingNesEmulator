#pragma once

using namespace std;
using namespace glm;

u8 AC, F, X, Y; // CPU register.
u8 SP; // Stack pointer
u16 PC; // Programm counter

u8 opcode; // Currently executing opcode
u16 op; // operand after instruction
u8* _op = (u8*)&op;

u16 lPC;// todo: delete (for debug purpose)

// vram address and toggle to swap writing byte in it
u16 vramPointer;
u16 tempVramPointer;
u8* _vramPointer = (u8*)&tempVramPointer;
bool lowVramPointer = true;

// decode addressing mode to those values
u16 address;
u8 value;
u8* cell; // to write value back

// to read input and some other things
bool writeOperation = false;

bool toggleScroll = true;
u8 curScrollX = 0;
u8 curScrollY = 0;
u8 scrollX, scrollY;

u8 OAMAddress;

u32 cycle = 0;
s32 CyclesDown = 0;

u16 scanline = 261;

bool BotSPR = false; // Hide sprite under BG
bool drawSprite = false; // Now drawing sprite

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
const u8 F_CARRY = 0b1; // carry flag
const u8 F_ZERO = 0b10; // zero flag
const u8 F_INT = 0b100; // interrupt flag
const u8 F_DECIMAL = 0b1000; // decimal flag not used in dendy
const u8 F_BREAK = 0b10000; // break flag
const u8 F_1 = 0b100000; // 1 flag
const u8 F_TRANS = 0b1000000; // transfer to sign bit flag
const u8 F_SIGN = 0b10000000; // sign flag

// get flags
bool GetFlag(u8 flagName) {
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
// get value (not flags state)
void SetSign(u8 b) { 
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

// set sign and zero flags. Get value.
void SetNZ(u8 value) {
	SetSign(value);
	SetZero(value);
}