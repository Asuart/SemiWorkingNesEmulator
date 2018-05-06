#pragma once
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include "ShaderLib.h"
#include "Opcodes.h"

struct Vertex {
	vec2 pos;
	vec2 tex;
	Vertex() {
		pos = vec2(0, 0);
		tex = vec2(0, 0);
	}
	Vertex(float x, float y, float u, float v) {
		pos = vec2(x, y);
		tex = vec2(u, v);
	}
};
struct Plane {
	Vertex vertices[6];
	Plane() {}
	Plane(vec2 topLeft, vec2 bottomRight) {
		vertices[0] = Vertex(topLeft.x, topLeft.y, 0, 0);
		vertices[1] = Vertex(topLeft.x, bottomRight.y, 0, 1);
		vertices[2] = Vertex(bottomRight.x, bottomRight.y, 1, 1);
		vertices[3] = Vertex(topLeft.x, topLeft.y, 0, 0);
		vertices[4] = Vertex(bottomRight.x, bottomRight.y, 1, 1);
		vertices[5] = Vertex(bottomRight.x, topLeft.y, 1, 0);
	}
};
Plane screenPlane;
GLuint screenVAO;
GLuint screenPlaneObject;
GLuint screenSprite;
GLuint shader;

const u16 SCREEN_SCALE = 3;
const u16 SCREEN_WIDTH = 256;
const u16 SCREEN_HEIGHT = 240;
const u16 NTSC_VIEW_HEIGHT = 224;
const u16 SPRITE_SIZE = 8;
const u16 SPRITE_LIST_SIZE = 16;

const u16 PAGE_CHAR_SIZE = 960;
const u16 PAGE_PARAMS_SIZE = 64;

GLFWwindow* mainWindow;
const int SCREEN_SPRITE_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT * 4;
u8 screenSpriteData[SCREEN_SPRITE_SIZE];

u8 GetCurPage() {
	return mmc->ReadROM(0x2000) & 0b11;
}
bool GetVRAMIncrement() {
	return mmc->ReadROM(0x2000) & (1 << 2);
}
bool GetSPRPattern() {
	return mmc->ReadROM(0x2000) & (1 << 3);
}
bool GetBGPattern() {
	return mmc->ReadROM(0x2000) & (1 << 4);
}
bool GetSPRSize() {
	return mmc->ReadROM(0x2000) & (1 << 5);
}
bool GetNMIEnabled() {
	return mmc->ReadROM(0x2000) & (1 << 7);
}
bool GetClipBG() {
	return !(mmc->ReadROM(0x2001) & (1 << 1));
}
bool GetClipSPR() {
	return !(mmc->ReadROM(0x2001) & (1 << 2));
}
bool GetShowBG() {
	return mmc->ReadROM(0x2001) & (1 << 3);
}
bool GetShowSPR() {
	return mmc->ReadROM(0x2001) & (1 << 4);
}
bool GetVBlank() {
	return mmc->ReadROM(0x2002) & (1 << 7);
}

void SetSpriteOverflow(bool flag) {
	if (flag) *mmc->GetROMCell(0x2002) |= (1 << 5);
	else *mmc->GetROMCell(0x2002) &= ~(1 << 5);
}
void SetSpriteHit(bool flag) {
	if (flag) *mmc->GetROMCell(0x2002) |= (1 << 6);
	else *mmc->GetROMCell(0x2002) &= ~(1 << 6);
}
void SetVBlank(bool flag) {
	if (flag) *mmc->GetROMCell(0x2002) |= (1 << 7);
	else *mmc->GetROMCell(0x2002) &= ~(1 << 7);
}
void SetWriteLock(bool flag) {
	if (flag) *mmc->GetROMCell(0x2002) |= (1 << 4);
	else *mmc->GetROMCell(0x2002) &= ~(1 << 4);
}

struct Color {
	unsigned char r, g, b;
	Color(unsigned char _r = 0, unsigned char _g = 0, unsigned char _b = 0) {
		r = _r;
		g = _g;
		b = _b;
	}
};
char curColorSet = 0;

struct Palette {
	Color colorTable[0x100];

	Palette() {
		LoadColorTable();
	}
	void LoadColorTable() {
		for (int i = 0; i < 256; i++) {
			colorTable[i] = Color(0, 0, 0);
		}
		// 0x0X
		colorTable[0x00] = Color(117, 117, 117);
		colorTable[0x01] = Color(39, 27, 143);
		colorTable[0x02] = Color(0, 0, 171);
		colorTable[0x03] = Color(71, 0, 159);
		colorTable[0x04] = Color(143, 0, 119);
		colorTable[0x05] = Color(171, 0, 19);
		colorTable[0x06] = Color(167, 0, 0);
		colorTable[0x07] = Color(127, 11, 0);
		colorTable[0x08] = Color(67, 47, 0);
		colorTable[0x09] = Color(0, 71, 0);
		colorTable[0x0a] = Color(0, 81, 0);
		colorTable[0x0b] = Color(0, 63, 23);
		colorTable[0x0c] = Color(27, 63, 95);
		// 0x1X
		colorTable[0x10] = Color(188, 188, 188);
		colorTable[0x11] = Color(0, 115, 239);
		colorTable[0x12] = Color(35, 59, 239);
		colorTable[0x13] = Color(131, 0, 243);
		colorTable[0x14] = Color(191, 0, 191);
		colorTable[0x15] = Color(231, 0, 91);
		colorTable[0x16] = Color(219, 43, 0);
		colorTable[0x17] = Color(203, 79, 15);
		colorTable[0x18] = Color(139, 115, 0);
		colorTable[0x19] = Color(0, 151, 0);
		colorTable[0x1a] = Color(0, 171, 0);
		colorTable[0x1b] = Color(0, 147, 59);
		colorTable[0x1c] = Color(0, 131, 139);
		// 0x2X
		colorTable[0x20] = Color(255, 255, 255);
		colorTable[0x21] = Color(63, 191, 255);
		colorTable[0x22] = Color(95, 151, 255);
		colorTable[0x23] = Color(167, 139, 253);
		colorTable[0x24] = Color(247, 123, 255);
		colorTable[0x25] = Color(255, 119, 183);
		colorTable[0x26] = Color(255, 119, 99);
		colorTable[0x27] = Color(255, 155, 59);
		colorTable[0x28] = Color(243, 191, 63);
		colorTable[0x29] = Color(131, 211, 19);
		colorTable[0x2a] = Color(79, 223, 75);
		colorTable[0x2b] = Color(88, 248, 152);
		colorTable[0x2c] = Color(0, 235, 219);
		// 0x3X
		colorTable[0x30] = Color(255, 255, 255);
		colorTable[0x31] = Color(171, 231, 255);
		colorTable[0x32] = Color(199, 215, 255);
		colorTable[0x33] = Color(215, 203, 255);
		colorTable[0x34] = Color(255, 199, 255);
		colorTable[0x35] = Color(255, 199, 219);
		colorTable[0x36] = Color(255, 191, 179);
		colorTable[0x37] = Color(255, 219, 171);
		colorTable[0x38] = Color(255, 231, 163);
		colorTable[0x39] = Color(227, 255, 163);
		colorTable[0x3a] = Color(171, 243, 191);
		colorTable[0x3b] = Color(179, 255, 207);
		colorTable[0x3c] = Color(159, 255, 243);
	}
	Color GetColor(char code) {
		return colorTable[code];
	}
};
Palette palette;

struct SpriteList {
	u8* patternTable;

	SpriteList() {
		patternTable = mmc->GetROMCell(0x6000); // most common CHRRAM address
	}
	SpriteList(u8* patternTableAddress) {
		patternTable = patternTableAddress;
	}

	void DrawLine(u8 tile, u8 line, u8* sprite, u8 start, bool mirrored = false) {
		u8 l1, l2, pixel;
		l1 = patternTable[tile * 16 + line];
		l2 = patternTable[tile * 16 + line + 8];
		for (int i = start; i < start + 8 && i < 256; i++) {
			if (!mirrored) {
				pixel = (((l1 >> 7) & 0b1) | ((l2 >> 6) & 0b10)) & 0b11;
				l1 <<= 1;
				l2 <<= 1;
			}
			else {
				pixel = ((l1 & 0b1) | ((l2 << 1) & 0b10)) & 0b11;
				l1 >>= 1;
				l2 >>= 1;
			}
			if (pixel) {
				Color col = palette.GetColor(CurPalette[(curColorSet << 2) | pixel]);
				sprite[i * 4] = col.r;
				sprite[i * 4 + 1] = col.g;
				sprite[i * 4 + 2] = col.b;
				sprite[i * 4 + 3] = 1;
			}
		}
	}
};
SpriteList spriteList[2];

void Reset() {
	spriteList[0] = SpriteList(mmc->GetVRAMCell(0x0));
	spriteList[1] = SpriteList(mmc->GetVRAMCell(0x1000));

	if (currentROM.fourPage)
		SetMirroringFourPage();
	else if (currentROM.verticalMirroring)
		SetMirroringVertical();
	else
		SetMirroringHorizontal();

	// pressets
	PC = RESET_ADDR;
	F = 0x34;
	AC = X = Y = 0;
	SP = 0xfd;
	cycle = 0;
	vramPointer = 0x2000;
	scanline = 261;
}

void Load(const char* path) {
	LoadROM(path);
	Reset();
}

void drop_callback(GLFWwindow* window, int count, const char** paths){
	Load(paths[0]);
}

bool InitWindow() {
	if (!glfwInit()) {
		cout << "Init glfw failed" << endl;
		return false;
	}
	mainWindow = glfwCreateWindow(SCREEN_WIDTH*SCREEN_SCALE, SCREEN_HEIGHT*SCREEN_SCALE, "NES-MULATOR", NULL, NULL);
	if (!mainWindow) {
		cout << "Init window failed" << endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(mainWindow);
	glfwSetDropCallback(mainWindow, drop_callback);
	return true;
}
bool InitGL() {
	glewExperimental = GL_TRUE;
	if (glewInit()) {
		cout << "Glew init failed!" << endl;
		return true;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_DOUBLEBUFFER);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glGenTextures(1, &screenSprite);
	glGenVertexArrays(1, &screenVAO);
	glGenBuffers(1, &screenPlaneObject);
	shader = ShaderLibrary.GetShader("base");


	glBindTexture(GL_TEXTURE_2D, screenSprite);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	screenPlane = Plane(vec2(-1, 1), vec2(1, -1));


	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenPlaneObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 24, screenPlane.vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0); // position
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat))); // texcoords
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glUseProgram(shader);
	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenPlaneObject);
	glBindTexture(GL_TEXTURE_2D, screenSprite);

	return false;
}



