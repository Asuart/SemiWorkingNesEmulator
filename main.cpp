#include "PPU.h"
#include <Windows.h>
#include <string>

const u32 BGLineLength = 32 * 2 * 8 * 4; // tiles * 2 pages * pixels per tile * params per pixel;
const u32 SPRLineLength = 32 * 8 * 4; // tiles * pixels per tile * params per pixel;
const u16 scanlineCount = 262; // Scanlines per frame;
const u16 scanlineCycles = 341; // Pixel per scanline;

u8 currentBGLine[BGLineLength];
u8 currentSPRLine[SPRLineLength];

u16 currentPixel = 0;

void(*ppuPipeline[scanlineCount][scanlineCycles])();


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
void HandleControlWrite() {
	if (address == 0x2002) {
		*mmc->GetROMCell(0x2002) &= ~0x80;
	}
	else if (address == 0x4016) {
		static char readKeyNum = 0;
		if (writeOperation && !(mmc->ReadROM(0x4016) & 1)) {
			readKeyNum = 0;
			*mmc->GetROMCell(0x4016) = GetKeyState(readKeyNum);
			readKeyNum++;
		}
		else if (!writeOperation) {
			*mmc->GetROMCell(0x4016) = GetKeyState(readKeyNum);
			readKeyNum++;
		}
	}
	else if (address == 0x2006) {
		if (lowVramPointer) _vramPointer[1] = mmc->ReadROM(0x2006);
		else {
			_vramPointer[0] = mmc->ReadROM(0x2006);
			vramPointer = tempVramPointer;
		}
		lowVramPointer = !lowVramPointer;
	}
	else if (address == 0x2007) {
		if (vramPointer >= 0x4000) vramPointer = vramPointer % 0x4000;
		if (writeOperation) mmc->CopyROMToVRAM(0x2007, vramPointer);
		mmc->WriteToROM(0x2007, mmc->ReadVRAM(vramPointer));
		if (GetVRAMIncrement()) vramPointer += 32;
		else vramPointer++;
	}
	else if (address == 0x4014) {
		char v = mmc->ReadROM(0x4014);
		short a = ((v << 8) & 0xff00);
		for (int i = 0; i < 0x100; i++, a++) {
			OAM[i] = mmc->ReadROM(a);
		}
	}
	else if (address == 0x2003) {
		OAMAddress = mmc->ReadROM(0x2003);
	}
	else if (address == 0x2004) {
		if (writeOperation) {
			OAM[OAMAddress] = mmc->ReadROM(0x2004);
		}
		if (!writeOperation && GetVBlank()) {

		}
		else {
			OAMAddress++;
		}
	}
	else if (address == 0x2005) {
		if (toggleScroll) scrollX = mmc->ReadROM(0x2005);
		else scrollY = mmc->ReadROM(0x2005);
		toggleScroll = !toggleScroll;
	}
	writeOperation = false;
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

	for (int i = 0; i < 256 * 240 * 4; i++) screenSpriteData[i] = 0;
	LimitFPS();
}
void CheckInput() {
	if (glfwGetKey(mainWindow, GLFW_KEY_SPACE)) {
		cout << "pause" << endl;
		while (!glfwGetKey(mainWindow, GLFW_KEY_Q)) {
			if (glfwGetKey(mainWindow, GLFW_KEY_ESCAPE)) {

				break;
			}
			glfwPollEvents();
		}
		cout << "unpause" << endl;
	}
	else if (glfwGetKey(mainWindow, GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(mainWindow, true);
	}
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

void PrerenderBGLine() {
	CurPalette = BGPalette;
	u8 currentPattern = GetBGPattern();
	u16 curScanline = scanline + curScrollY;
	u8 line = curScanline % 8;
	u8 numY = ((curScanline % 240) >> 3) + 1;

	// Set pages
	u8 pg1 = GetCurPage();
	if (scanline + curScrollY > 0xff)
		(pg1 & 0b10) ? pg1 &= ~0b10 : pg1 |= 0b10;
	u8 pg2;
	if (pg1 & 1) pg2 = pg1 & 0b10;
	else pg2 = (pg1 & 0b10) | 1;

	memset(currentBGLine, 0, BGLineLength); // Clear BG line

	for (int i = 0; i < 32; i++) {
		curColorSet = pages[pg1].params[(numY / 4) * 8 + i / 4];
		curColorSet = (curColorSet >> (((((i % 4)) / 2) | (((numY % 4) / 2) << 1)) * 2)) & 0b11;
		spriteList[currentPattern].DrawLine(pages[pg1].data[numY * 32 + i], line, currentBGLine, i * 8);
	}
	for (int i = 32; i < 64; i++) {
		curColorSet = pages[pg2].params[(numY / 4) * 8 + (i - 32) / 4];
		curColorSet = (curColorSet >> (((((i % 4)) / 2) | (((numY % 4) / 2) << 1)) * 2)) & 0b11;
		spriteList[currentPattern].DrawLine(pages[pg2].data[(numY-1) * 32 + i], line, currentBGLine + 1024, i * 8);
	}
}
void PrerenderSPRLine() {
	CurPalette = SPRPalette;
	u8 line = 0;
	memset(currentSPRLine, 0, SPRLineLength); // Clear SPR Line

	for (int i = OAMAddress; i < 256; i += 4) {
		curColorSet = (OAM[i + 2] & 0b11);
		if (OAM[i] >= scanline && OAM[i] < scanline + 8) {
			// flip vertical
			if (OAM[i + 2] & 0x80) line = (OAM[i] - scanline);
			else line = 7 - (OAM[i] - scanline);
			// mask pixels over screen
			if (OAM[i] + line < 240 && scanline * 1024 + OAM[i + 3] * 4 <= 1024 * 240 - 32) {
				// flip horizontal
				if (OAM[i + 2] & (1 << 6)) spriteList[GetSPRPattern()].DrawLine(OAM[i + 1], line, currentSPRLine, OAM[i + 3], true);
				else spriteList[GetSPRPattern()].DrawLine(OAM[i + 1], line, currentSPRLine, OAM[i + 3], false);
			}
		}
	}
}

void Idle() {
}
void FetchPixel() {
	screenSpriteData[scanline * 1024 + currentPixel * 4] = currentBGLine[(currentPixel + curScrollX) * 4];
	screenSpriteData[scanline * 1024 + currentPixel * 4 + 1] = currentBGLine[(currentPixel + curScrollX) * 4 + 1];
	screenSpriteData[scanline * 1024 + currentPixel * 4 + 2] = currentBGLine[(currentPixel + curScrollX) * 4 + 2];
	if (currentSPRLine[currentPixel * 4 + 3]) {
		if(currentBGLine[(currentPixel + curScrollX) * 4 + 3] && currentSPRLine[currentPixel  * 4 + 3]) SetSpriteHit(1);

		screenSpriteData[scanline * 1024 + currentPixel * 4] = currentSPRLine[currentPixel * 4];
		screenSpriteData[scanline * 1024 + currentPixel * 4 + 1] = currentSPRLine[currentPixel * 4 + 1];
		screenSpriteData[scanline * 1024 + currentPixel * 4 + 2] = currentSPRLine[currentPixel * 4 + 2];
	}
}
void ReloadXScroll() {
	curScrollX = scrollX;
}
void ReloadYScroll() {
	curScrollY = scrollY;
}
void StartVBlank() {
	SetVBlank(1);
	SetWriteLock(1);
	PresentFrame();
	if (GetNMIEnabled()) HandleNMI();
}
void EndVBlank() {
	SetVBlank(0);
	SetSpriteHit(0);
	SetSpriteOverflow(0);
}
void StartLine() {
	PrerenderSPRLine();
	PrerenderBGLine();
	FetchPixel();
}

void InitPPUPipeline() {
	// Reset ALL to idle for safe
	for (int i = 0; i < scanlineCount; i++) {
		for (int j = 0; j < scanlineCycles; j++) {
			ppuPipeline[i][j] = Idle;
		}
	}

	// Set actual Pipeline
	for (int i = 0; i < 240; i++) {
		for (int j = 2; j < 257; j++) {
			ppuPipeline[i][j] = FetchPixel;
		}
		ppuPipeline[i][257] = ReloadXScroll;
		ppuPipeline[i][1] = StartLine;
	}
	ppuPipeline[261][280] = ReloadYScroll;
	ppuPipeline[241][1] = StartVBlank;
	ppuPipeline[261][1] = EndVBlank;
}

void CPUStep() {
	GetOpcode();
	opcodes[opcode].exec();
	cpuCycles = opcodes[opcode].cycles;
}

void PPUStep() {
	ppuPipeline[scanline][currentPixel]();
	currentPixel++;

	// cycles the ppu programm
	if (currentPixel == 341) {
		currentPixel = 0;
		scanline++;
		if (scanline == 262) scanline = 0;
	}
}
void Step() {
	CPUStep();
	HandleControlWrite();
	for (int i = 0; i < cpuCycles * 3; i++) PPUStep(); // 1 cpu cycle = 3 ppu cycles, and 1 ppu cycle = 1 ppu step.
}
void Run(int numCycles) {
	for (int i = 0; i < numCycles; i++) Step();
}

int main() {
	InitWindow();
	InitGL();
	LoadROM();

	palette.LoadColorTable();
	LoadOpcodesTable();
	InitPPUPipeline();

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

	while (!glfwWindowShouldClose(mainWindow)) {
		Run(200); // Run for 200 CPU cycles, then check for input
		CheckInput();
		glfwPollEvents();
	}
	return 0;
}