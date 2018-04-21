#pragma once

struct Palette {
	unsigned char GetR(unsigned char color) {
		return 50 * color;
	}
	unsigned char GetG(unsigned char color) {
		return 0;
	}
	unsigned char GetB(unsigned char color) {
		return 0;
	}
};