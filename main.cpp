//#define ASM
#include "PPU.h"
#include <Windows.h>
#include <string>

bool ENABLE_BREAK = false;

ofstream debug;

int scanline = -1;
char screenSpriteData[256 * 240 * 3];
bool toggleScroll = true;

bool EnableDisasm = false;

void SetVBlank(bool flag) {
	if (!flag) {
		ROM[0x2002] &= ~(1 << 7);
	}
	else {
		ROM[0x2002] |= (1 << 7);
	}
}
void SetSprite0(bool flag) {
	if (!flag) {
		ROM[0x2002] &= ~(1 << 6);
	}
	else {
		ROM[0x2002] |= (1 << 6);
	}
}
void SetWriteLock(bool flag) {
	if (!flag) {
		ROM[0x2002] &= ~(1 << 4);
	}
	else {
		ROM[0x2002] |= (1 << 4);
	}
}
bool GetWriteEnabled() {
	return (ROM[0x2002] & ~(1 << 4));
}
bool GetNMIEnabled() {
	return ROM[0x2000] & (1 << 7);
}
bool GetBGRender() {
	return ROM[0x2001] & (1 << 3);
}
bool GetSPRRender() {
	return ROM[0x2001] & (1 << 4);
}
bool GetBGSpriteList() {
	return ROM[0x2000] & 0b10000;
}
bool GetIncrementMode() {
	return ROM[0x2000] & 0b100;
}
bool GetSPRSpriteList() {
	return ROM[0x2000] & 0b1000;
}

bool GetKeyState(char key) {
	switch (key) {
	case 0:
		if (glfwGetKey(mainWindow, GLFW_KEY_C)) return true;
		else return false;
	case 1:
		if (glfwGetKey(mainWindow, GLFW_KEY_V)) return true;
		else return false;
	case 2:
		if (glfwGetKey(mainWindow, GLFW_KEY_Z)) return true;
		else return false;
	case 3:
		if (glfwGetKey(mainWindow, GLFW_KEY_X)) return true;
		else return false;
	case 4:
		if (glfwGetKey(mainWindow, GLFW_KEY_UP)) return true;
		else return false;
	case 5:
		if (glfwGetKey(mainWindow, GLFW_KEY_DOWN)) return true;
		else return false;
	case 6:
		if (glfwGetKey(mainWindow, GLFW_KEY_LEFT)) return true;
		else return false;
	case 7:
		if (glfwGetKey(mainWindow, GLFW_KEY_RIGHT)) return true;
		else return false;
	default:
		return false;
	}
}

void HandlePPU() {
	if (address == 0x2002) {
		ROM[0x2002] &= ~0x80;
		ROM[0x2002] &= ~0x40;
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
		else _vramPointer[0] = ROM[0x2006];
		lowVramPointer = !lowVramPointer;
	}
	else if (address == 0x2007) {
		if (vramPointer >= 0x4000) vramPointer = vramPointer % 0x4000;
		if(writeOperation) VROM[vramPointer] = ROM[0x2007];
		ROM[0x2007] = VROM[vramPointer];
		if (GetIncrementMode()) vramPointer += 32;
		else vramPointer++;
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
		cout << "2004 not implemented" << endl;
	}
	else if (address == 0x2005) {
		if (toggleScroll) scrollX = ROM[0x2005];
		else scrollY = ROM[0x2005];
		toggleScroll = !toggleScroll;
	}
	writeOperation = false;
	ROM[0x2003] = OAMAddress;
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
	CurPalette = BGPalette;
	int line = scanline % 8;
	int numY = (scanline >> 3) + 1;

	char pg1 = GetCurPage();
	char pg2;
	if (pg1) pg2 = 0;
	else pg2 = 1;

	char fullLine[32 * 8 * 3 * 2];
	Color clearColor = palette.GetColor(SPRPalette[0]);
	for (int i = 0; i < 32 * 8 * 2; i++) {
		fullLine[i * 3] = clearColor.r;
		fullLine[i * 3 + 1] = clearColor.g;
		fullLine[i * 3 + 2] = clearColor.b;
	}

	for (int i = 0; i < 32; i++) {
		curColorSet = pages[pg1].params[(numY / 4) * 8 + i / 4];
		curColorSet = (curColorSet >> (((((i % 4)) / 2) | (((numY % 4) / 2) << 1)) * 2)) & 0b11;
		spriteList[GetBGSpriteList()].DrawLine(pages[pg1].data[numY * 32 + i], line, (fullLine + (i * 8 * 3)));
	}
	for (int i = 32; i < 64; i++) {
	curColorSet = pages[pg2].params[(numY / 4) * 8 + (i-32) / 4];
	curColorSet = (curColorSet >> (((((i % 4)) / 2) | (((numY % 4) / 2) << 1)) * 2)) & 0b11;
	spriteList[GetBGSpriteList()].DrawLine(pages[pg2].data[numY * 32 + i - 32], line, (fullLine + (i * 8 * 3)));
	}
	for (int i = scrollX, j = 0; i < (scrollX + 256); i++, j++) {
		screenSpriteData[scanline * 256 * 3 + j * 3] = fullLine[i * 3];
		screenSpriteData[scanline * 256 * 3 + j * 3 + 1] = fullLine[i * 3 + 1];
		screenSpriteData[scanline * 256 * 3 + j * 3 + 2] = fullLine[i * 3 + 2];
	}
}
void EvaluateSPR() {
	// sprite evaluation not requiered
}

void RenderSPR() {
	SetSprite0(1);
	CurPalette = SPRPalette;
	for (int i = 0; i < 256; i += 4) {
		curColorSet = (OAM[i + 2] & 0b11);
		//vertical mirroring
		if (OAM[i + 2] & 0x80) {
			// if sprite is on scanline
			if (OAM[i] >= scanline && OAM[i] < scanline + 8) {
				int line = (OAM[i] - scanline);
				if (OAM[i] + line < 240 && OAM[i] + line > 0) {
					if (OAM[i + 2] & 0b1000000) {
						if (scanline * 256 * 3 + OAM[i + 3] * 3 <= 256 * 240 * 3 - 24) {
							spriteList[GetSPRSpriteList()].DrawLineMirrored(OAM[i + 1], line, screenSpriteData + scanline * 256 * 3 + OAM[i + 3] * 3);
						}
					}
					else {
						if (scanline * 256 * 3 + OAM[i + 3] * 3 <= 256 * 240 * 3 - 8 * 4) {
							spriteList[GetSPRSpriteList()].DrawLine(OAM[i + 1], line, screenSpriteData + scanline * 256 * 3 + OAM[i + 3] * 3);
						}
					}
				}
			}
		}
		else {
			// if sprite is on scanline
			if (OAM[i] >= scanline && OAM[i] < scanline+8) {
				int line = 7 -(OAM[i] - scanline);
				if (OAM[i] + line < 240 && OAM[i] + line > 0) {
					if (OAM[i + 2] & 0b1000000) {
						if (scanline * 256 * 3 + OAM[i + 3] * 3 <= 256 * 240 * 3 - 24) {
							spriteList[GetSPRSpriteList()].DrawLineMirrored(OAM[i + 1], line, screenSpriteData + scanline * 256 * 3 + OAM[i + 3] * 3);
						}
					}
					else {
						if (scanline * 256 * 3 + OAM[i + 3] * 3 <= 256 * 240 * 3 - 8 * 4) {
							spriteList[GetSPRSpriteList()].DrawLine(OAM[i + 1], line, screenSpriteData + scanline * 256 * 3 + OAM[i + 3] * 3);
						}
					}
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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, NTSC_VIEW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenSpriteData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glfwSwapBuffers(mainWindow);

	LimitFPS();
}
void PreRender() {
	SetWriteLock(0);
}
void RenderScanline() {
	if (GetBGRender()) {
		RenderBG();
	}
	if (GetSPRRender()) {
		EvaluateSPR();
		RenderSPR();
	}
}
void PostRender() {
	// none
}
void StartVBlank() {
	PresentFrame();
	SetVBlank(1);
	SetWriteLock(1); // enable
	if (GetNMIEnabled()) {
		HandleNMI();
	}
}
void DuringVBlank() {
	// nothing
}
void EndVBlank() {
	SetVBlank(0);
	SetSprite0(0);
	SetWriteLock(0); //disable
	SetOverflow(0);
}

void HBlank() {
	if (scanline == -1) {
		PreRender();
	}
	else if (scanline >= 0 && scanline < 240) {
		RenderScanline();
	}
	else if (scanline == 240) {
		PostRender();
		StartVBlank();
	}
	else if (scanline > 240 && scanline < 260) {
		DuringVBlank();
	}
	else if (scanline == 260) {
		EndVBlank();
	}
	else if (scanline == 261) {
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
		for (int i = 0; i < 16; i++) {
			cout << hex << (int)BGPalette[i] << endl;
		}

		ENABLE_BREAK = true;
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
		pages[1].data = pages[0].data;
		pages[1].params = pages[0].params;
		pages[3].data = pages[2].data;
		pages[3].params = pages[2].params;
	}
	else {
		pages[2].data = pages[0].data;
		pages[2].params = pages[0].params;
		pages[3].data = pages[1].data;
		pages[3].params = pages[1].params;
	}

	// pressets
	SP = 0xff;
	F = 0x34;
	cycle = 0;
	vramPointer = 0;
	for (int i = 0; i < 0x800; i++)
		ROM[i] = 0x0;
	for (int i = 0; i < 256; i++) {
		OAM[i] = 0x0;
	}

	// get nmi address
	char* p = (char*)&NMI_ADDR;
	p[0] = ROM[0xfffa];
	p[1] = ROM[0xfffb];
	//get reset address
	p = (char*)&RESET_ADDR;
	p[0] = ROM[0xfffc];
	p[1] = ROM[0xfffd];
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