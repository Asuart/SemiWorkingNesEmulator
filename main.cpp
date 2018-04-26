//#define ASM
#include "PPU.h"
#include <Windows.h>
#include <string>

bool ENABLE_BREAK = false;

ofstream debug;

int scanline = 261;
char screenSpriteData[256 * 240 * 4];
bool toggleScroll = true;
bool EnableDisasm = false;
unsigned char curScrollX = 0;
unsigned char curScrollY = 0;


unsigned char GetKeyState(char key) {
	switch (key) {
	case 0:
		if (glfwGetKey(mainWindow, GLFW_KEY_C)) return 1;
		else return 0;
	case 1:
		if (glfwGetKey(mainWindow, GLFW_KEY_V)) return 1;
		else return 0;
	case 2:
		if (glfwGetKey(mainWindow, GLFW_KEY_Z)) return 1;
		else return 0;
	case 3:
		if (glfwGetKey(mainWindow, GLFW_KEY_X)) return 1;
		else return 0;
	case 4:
		if (glfwGetKey(mainWindow, GLFW_KEY_UP)) return 1;
		else return 0;
	case 5:
		if (glfwGetKey(mainWindow, GLFW_KEY_DOWN)) return 1;
		else return 0;
	case 6:
		if (glfwGetKey(mainWindow, GLFW_KEY_LEFT)) return 1;
		else return 0;
	case 7:
		if (glfwGetKey(mainWindow, GLFW_KEY_RIGHT)) return 1;
		else return 0;
	default:
		return 0;
	}
}

void HandlePPU() {
	if (address == 0x2002) {
		//ROM[0x2002] &= ~0x80; // wiki saysit must be cleard? but nestest fails then
		//scrollX = scrollY = vramPointer = 0;
	}
	else if (address == 0x4016) {
		static char readKeyNum = 0;
		if (writeOperation && !(ROM[0x4016] & 1)) {
			readKeyNum = 0;
			ROM[0x4016] = GetKeyState(readKeyNum);
			readKeyNum++;
		}
		else if (!writeOperation) {
			ROM[0x4016] = GetKeyState(readKeyNum);
			readKeyNum++;
		}
	}
	else if (address == 0x2006) {
		if (lowVramPointer) _vramPointer[1] = ROM[0x2006];
		else {
			_vramPointer[0] = ROM[0x2006];
			vramPointer = tempVramPointer;
		}

		lowVramPointer = !lowVramPointer;
	}
	else if (address == 0x2007) {
		if (vramPointer >= 0x4000) vramPointer = vramPointer % 0x4000;
		if (vramPointer < 0x3f00) {
			if (writeOperation) VROM[vramPointer] = ROM[0x2007];
			ROM[0x2007] = VROM[vramPointer];
			if (GetVRAMIncrement()) vramPointer += 32;
			else vramPointer++;
		}
		else {
			VROM[vramPointer] = ROM[0x2007];
			if (GetVRAMIncrement()) vramPointer += 32;
			else vramPointer++;
		}
	}
	else if (address == 0x4014) {
		char v = ROM[0x4014];
		short a = ((v << 8) & 0xff00);
		for (int i = 0; i < 0x100; i++, a++) {
			OAM[i] = ROM[a];
		}
	}
	else if (address == 0x2003) {
		OAMAddress = ROM[0x2003];
	}
	else if (address == 0x2004) {
		if (writeOperation) {
			OAM[OAMAddress] = ROM[0x2004];
		}
		if (!writeOperation && GetVBlank()) {

		}
		else {
			OAMAddress++;
		}
	}
	else if (address == 0x2005) {
		if (toggleScroll) scrollX = ROM[0x2005];
		else scrollY = ROM[0x2005];
		toggleScroll = !toggleScroll;
	}
	writeOperation = false;
	//ROM[0x4017] = 0;
}

void Run(int numCycles) {
	CyclesDown += numCycles;
	while (CyclesDown > 0) {
		cycle++;
		Step();
		HandlePPU();
	}
}



void RenderBG() {
	drawSprite = false;
	CurPalette = BGPalette;

	unsigned short curScanline = scanline + curScrollY;
	int line = curScanline % 8;
	int numY = ((curScanline % 240) >> 3) + 1;

	char pg1 = GetCurPage();
	if (scanline + curScrollY > 0xff) 
		(pg1 & 0b10) ? pg1 &= ~0b10 : pg1 |= 0b10;
	char pg2;
	if (pg1 & 1) pg2 = 0;
	else pg2 = 1;

	char fullLine[32 * 8 * 4 * 2];
	Color clearColor = palette.GetColor(BGPalette[0]);
	for (int i = 0; i < 32 * 16; i++) {
		fullLine[i * 4] = clearColor.r;
		fullLine[i * 4 + 1] = clearColor.g;
		fullLine[i * 4 + 2] = clearColor.b;
		fullLine[i * 4 + 3] = 0;
	}

	for (int i = 0; i < 32; i++) {
		curColorSet = pages[pg1].params[(numY / 4) * 8 + i / 4];
		curColorSet = (curColorSet >> (((((i % 4)) / 2) | (((numY % 4) / 2) << 1)) * 2)) & 0b11;
		spriteList[GetBGPattern()].DrawLine(pages[pg1].data[numY * 32 + i], line, (fullLine + (i * 8 * 4)));
	}
	for (int i = 32; i < 64; i++) {
	curColorSet = pages[pg2].params[(numY / 4) * 8 + (i-32) / 4];
	curColorSet = (curColorSet >> (((((i % 4)) / 2) | (((numY % 4) / 2) << 1)) * 2)) & 0b11;
	spriteList[GetBGPattern()].DrawLine(pages[pg2].data[numY * 32 + i - 32], line, (fullLine + (i * 8 * 4)));
	}
	for (int i = curScrollX, j = 0; j < 256; i++, j++) {
		screenSpriteData[scanline * 256 * 4 + j * 4] = fullLine[i * 4];
		screenSpriteData[scanline * 256 * 4 + j * 4 + 1] = fullLine[i * 4 + 1];
		screenSpriteData[scanline * 256 * 4 + j * 4 + 2] = fullLine[i * 4 + 2];
	}
}
void EvaluateSPR() {
	unsigned char spritesCount = 0;
	for (int i = OAMAddress; i < 256; i += 4) {
		if (OAM[i] >= scanline && OAM[i] < scanline + 8) {
			if ((OAM[i + 3] < 8 && GetLeftLineSPR()) || (OAM[i + 3] > 7 && OAM[i + 3] < 255)) spritesCount++;
		}
	}
	if (spritesCount > 8) SetSpriteOverflow(1);
}
void RenderSPR() {
	drawSprite = true;
	CurPalette = SPRPalette;
	unsigned char line = 0;
	if (GetSPRSize()) {
		for (int i = OAMAddress; i < 256; i += 4) {
			unsigned char patternTable = OAM[i + 1] & 1;
			unsigned char sprCode = ((OAM[i + 1] >> 1) & ~0x80) * 2;
			unsigned char sprHeight = 16;
			curColorSet = (OAM[i + 2] & 0b11);
			TopSPR = (OAM[i + 2] & (1 << 5));
			//tile on line
			if (OAM[i] >= scanline - 8 && OAM[i] < scanline + 8) {
				// flip vertical
				if (OAM[i + 2] & 0x80) line = (OAM[i] - scanline);
				else line = 7 - (OAM[i] - scanline);
				if (line > 7) {
					sprCode++;
					line %= 8;
				}
				// mask pixels over screen
				if (OAM[i] + line < 240 && scanline * 1024 + OAM[i + 3] * 4 <= 1024 * 240 - 32) {
					// flip horizontal
					if (OAM[i + 2] & (1 << 6)) spriteList[patternTable].DrawLineMirrored(sprCode, line, screenSpriteData + scanline * 1024 + OAM[i + 3] * 4);
					else spriteList[patternTable].DrawLine(sprCode, line, screenSpriteData + scanline * 1024 + OAM[i + 3] * 4);
				}
			}
		}
	}
	else {
		for (int i = OAMAddress; i < 256; i += 4) {
			curColorSet = (OAM[i + 2] & 0b11);
			TopSPR = (OAM[i + 2] & (1 << 5));
			// tile on line
			if (OAM[i] >= scanline && OAM[i] < scanline + 8) {
				// flip vertical
				if (OAM[i + 2] & 0x80) line = (OAM[i] - scanline);
				else line = 7 - (OAM[i] - scanline);
				// mask pixels over screen
				if (OAM[i] + line < 240 && scanline * 1024 + OAM[i + 3] * 4 <= 1024 * 240 - 32) {
					// flip horizontal
					if (OAM[i + 2] & (1 << 6)) spriteList[GetSPRPattern()].DrawLineMirrored(OAM[i + 1], line, screenSpriteData + scanline * 1024 + OAM[i + 3] * 4);
					else spriteList[GetSPRPattern()].DrawLine(OAM[i + 1], line, screenSpriteData + scanline * 1024 + OAM[i + 3] * 4);
				}
			}
		}
	}
}

void HandleNMI() {
	PUSH16(PC);
	PUSH8(F);
	PC = NMI_ADDR;
}

void LimitFPS() {
	Sleep(1000.0 / 80.0);
}

void PresentFrame() {
	glClear(GL_COLOR_BUFFER_BIT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, NTSC_VIEW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)screenSpriteData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glfwSwapBuffers(mainWindow);

	LimitFPS();
}
void PreRender() {
	SetVBlank(0);
	SetSpriteOverflow(0);
	SetSpriteHit(0);
	SetWriteLock(0);

	OAMAddress = 0;
	curScrollX = scrollX;
	curScrollY = scrollY;
}
void RenderScanline() {
	if (GetShowBG()) {
		RenderBG();
		curScrollX = scrollX;
	}
	if (GetShowSPR()) {
		EvaluateSPR();
		RenderSPR();
	}
	OAMAddress = 0;
}
void PostRender() {
	// wiki says it is safe and timings a bit better
	SetVBlank(1);
	SetWriteLock(1);
	PresentFrame();
	if (GetNMIEnabled()) {
		HandleNMI();
	}
}
void DuringVBlank() {
	// nothing
}

void HBlank() {
	if (scanline >= 0 && scanline < 240) {
		RenderScanline();
	}
	else if (scanline == 240) {
		PostRender(); // start VBlank
	}
	else if (scanline > 240 && scanline <= 260) {
		DuringVBlank(); // idle
	}
	else if (scanline == 261) {
		PreRender();
		scanline = -1;
	}
	scanline++;
}

void NextLine() {
	Run(113);
	HBlank();
}

void CheckPause() {
	if (glfwGetKey(mainWindow, GLFW_KEY_SPACE)) {
		cout << "pause" << endl;
		ENABLE_BREAK = true;

		for (int i = 0; i < 16; i++) {
			cout <<hex<<(int)BGPalette[i] << endl;
		}

		while (!glfwGetKey(mainWindow, GLFW_KEY_Q)) {
			if (glfwGetKey(mainWindow, GLFW_KEY_S)) {
			}
			glfwPollEvents();
		}
		cout << "unpause" << endl;
	}
}

int main() {
	InitWindow();
	InitGL();
	LoadROM();

	spriteList[0] = SpriteList(VROM);
	spriteList[1] = SpriteList(VROM + 0x1000);
	palette.LoadColorTable();
	LoadOpcodesTable();

	// setup mirroring
	if (currentROM.fourPage) {
		cout << "Four pages not supported" << endl;
		system("pause");
	}
	else if (currentROM.verticalMirroring) {
		pages[2].data = pages[0].data;
		pages[2].params = pages[0].params;
		pages[3].data = pages[1].data;
		pages[3].params = pages[1].params;
	}
	else {
		pages[1].data = pages[0].data;
		pages[1].params = pages[0].params;
		pages[3].data = pages[2].data;
		pages[3].params = pages[2].params;
	}

	// pressets
	F = 0x34;
	AC = X = Y = 0;
	SP = 0xfd;
	ROM[0x4017] = 0;
	ROM[0x4015] = 0;
	for (int i = 0; i < 0x10; i++) ROM[0x4000 + i] = 0;
	cycle = 0;
	vramPointer = 0x2000;
	scanline = 261;
	for (int i = 0; i < 0x800; i++) ROM[i] = 0x0;
	for (int i = 0; i < 256; i++) OAM[i] = 0x0;

	// get nmi address
	char* p = (char*)&NMI_ADDR;
	p[0] = ROM[0xfffa];
	p[1] = ROM[0xfffb];
	// get reset address
	p = (char*)&RESET_ADDR;
	p[0] = ROM[0xfffc];
	p[1] = ROM[0xfffd];
	// get break address
	p = (char*)&BREAK_ADDR;
	p[0] = ROM[0xfffe];
	p[1] = ROM[0xffff];
	PC = RESET_ADDR;

	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenPlaneObject);
	glBindTexture(GL_TEXTURE_2D, screenSprite);

	while (true) {
		NextLine();
		CheckPause();
		glfwPollEvents();
	}
	return 0;
}