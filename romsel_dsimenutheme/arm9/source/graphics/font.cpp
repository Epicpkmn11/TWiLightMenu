#include "font.h"
#include <nds.h>
#include <stdio.h>
#include "common/tonccpy.h"
#include "graphics/ThemeTextures.h"

Font smallFont, mediumFont, largeFont;
uint16_t fontPalette[] = {0, 0xD6B5, 0xB9CE, 0xA108};

int Font::load(const std::string &path) {
	FILE *file = fopen(path.c_str(), "rb");

	if(!file)	return 1;

	// Get file size
	fseek(file, 0, SEEK_END);
	u32 fileSize = ftell(file);

	// Skip file info
	fseek(file, 0x14, SEEK_SET);
	fseek(file, fgetc(file)-1, SEEK_CUR);

	// Load glyph info
	u32 chunkSize;
	fread(&chunkSize, 4, 1, file);
	font.tileWidth = fgetc(file);
	font.tileHeight = fgetc(file);
	fread(&font.tileSize, 2, 1, file);

	// Load character glyphs
	int tileAmount = ((chunkSize-0x10)/font.tileSize);
	font.tiles = std::vector<char>(font.tileSize*tileAmount);
	fseek(file, 4, SEEK_CUR);
	fread(font.tiles.data(), font.tileSize, tileAmount, file);

	// Fix top row
	for(int i=0;i<tileAmount;i++) {
		font.tiles[i*font.tileSize] = 0;
		font.tiles[i*font.tileSize+1] = 0;
		font.tiles[i*font.tileSize+2] = 0;
	}

	// Load character widths
	fseek(file, 0x24, SEEK_SET);
	u32 locHDWC;
	fread(&locHDWC, 4, 1, file);
	fseek(file, locHDWC-4, SEEK_SET);
	fread(&chunkSize, 4, 1, file);
	fseek(file, 8, SEEK_CUR);
	font.widths = std::vector<char>(3*tileAmount);
	fread(font.widths.data(), 3, tileAmount, file);

	// Load character maps
	font.map = std::vector<short>(tileAmount);
	fseek(file, 0x28, SEEK_SET);
	u32 locPAMC, mapType;
	fread(&locPAMC, 4, 1, file);

	while(locPAMC < fileSize) {
		u16 firstChar, lastChar;
		fseek(file, locPAMC, SEEK_SET);
		fread(&firstChar, 2, 1, file);
		fread(&lastChar, 2, 1, file);
		fread(&mapType, 4, 1, file);
		fread(&locPAMC, 4, 1, file);

		switch(mapType) {
			case 0: {
				u16 firstTile;
				fread(&firstTile, 2, 1, file);
				for(int i=firstChar;i<=lastChar;i++) {
					font.map[firstTile+(i-firstChar)] = i;
				}
				break;
			} case 1: {
				for(int i=firstChar;i<=lastChar;i++) {
					u16 tile;
					fread(&tile, 2, 1, file);
					font.map[tile] = i;
				}
				break;
			} case 2: {
				u16 groupAmount;
				fread(&groupAmount, 2, 1, file);
				for(int i=0;i<groupAmount;i++) {
					u16 charNo, tileNo;
					fread(&charNo, 2, 1, file);
					fread(&tileNo, 2, 1, file);
					font.map[tileNo] = charNo;
				}
				break;
			}
		}
	}
	fclose(file);
	return 0;
}

// Get the index in the character maps where the letter appears
unsigned int Font::getCharIndex(const char16_t letter) {
	uint spriteIndex = 0;
	uint left = 0;
	uint right = font.map.size();
	uint mid = 0;

	while (left <= right)
	{
		mid = left + ((right - left) / 2);
		if (font.map[mid] == letter) {
			spriteIndex = mid;
			break;
		}

		if (font.map[mid] < letter) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}
	return spriteIndex;

	// If that doesn't find the char, do a linear search from the end
	// (since the only chars that should fail are at the end)
	for(int i=font.map.size()-1;i>=0;i--) {
		if(font.map[i] == letter)	return i;
	}
	return 0;
}

// Convert UTF-8 (up to 0xFFFF) to a single UTF-16 character
std::u16string utf8to16(const std::string &text) {
	std::u16string out;
	char16_t c;

	for(uint i=0;i<text.size();) {
		if(!(text[i] & 0x80)) {
			c = text[i++];
		} else if((text[i] & 0xE0) == 0xC0) {
			c  = (text[i++] & 0x1F) << 6;
			c |=  text[i++] & 0x3F;
		} else if((text[i] & 0xF0) == 0xE0) {
			c  = (text[i++] & 0x0F) << 12;
			c |= (text[i++] & 0x3F) << 6;
			c |=  text[i++] & 0x3F;
		} else {
			i++; // out of range or something (This only does up to 0xFFFF since it goes to a U16 anyways)
		}
		out += c;
	}
	return out;
}

void Font::print(bool top, int xPos, int yPos, const std::string &text) {
	print(top, xPos, yPos, utf8to16(text));
}

void Font::print(bool top, int xPos, int yPos, const std::u16string &text) {
	int x=xPos;
	for(uint c=0;c<text.size();c++) {
		if(text[c] == '\n') {
			x = xPos;
			yPos += font.tileHeight;
			continue;
		}

		int t = getCharIndex(text[c]);
		u8 bitmap[font.tileWidth*font.tileHeight];
		for(int i=0;i<font.tileSize;i++) {
			bitmap[(i*4)]   = font.tiles[i+(t*font.tileSize)]>>6 & 3;
			bitmap[(i*4)+1] = font.tiles[i+(t*font.tileSize)]>>4 & 3;
			bitmap[(i*4)+2] = font.tiles[i+(t*font.tileSize)]>>2 & 3;
			bitmap[(i*4)+3] = font.tiles[i+(t*font.tileSize)]    & 3;
		}

		if(x > 256) {
			x = xPos+font.widths[t*3];
			yPos += font.tileHeight;
		}

		if(top) {
			// for(int py=0;py<font.tileHeight;py++) {
			// 	for(int px=0;px<font.widths[(t*3)+1];px++) {
			// 		if(fontPalette[bitmap[(py*font.tileWidth)+px]]>>15 != 0) {
			// 			_bgSubBuffer[((yPos+py)*256)+x+px] = fontPalette[bitmap[(py*font.tileWidth)+px]];
			// 		}
			// 	}
			// }
			for(int i=0;i<font.tileHeight;i++) {
				tonccpy(bg2Sub+((yPos+i)*256)+x+font.widths[t*3], bitmap+(i*font.tileWidth), font.widths[(t*3)+1]);
			}
		} else {
			for(int i=0;i<font.tileHeight;i++) {
				tonccpy(bg2Main+((yPos+i)*256)+x+font.widths[t*3], bitmap+(i*font.tileWidth), font.widths[(t*3)+1]);
			}
		}
		x += font.widths[(t*3)+2];
	}
}

int Font::calcWidth(const std::string &message) {
	std::u16string text = utf8to16(message);
	int width = 0;
	for(uint c=0;c<text.size();c++) {
		int t = getCharIndex(text[c]);
		width += font.widths[(t*3)+2];
	}
	return width;
}

void Font::print(bool top, int x, int y, int value) {
	print(top, x, y, std::to_string(value));
}

int Font::getCenteredX(const std::string &text) {
	return (SCREEN_WIDTH - calcWidth(text)) / 2;
}

void Font::printCentered(bool top, int y, const std::string &text) {
	print(top, getCenteredX(text), y, text);
}

void Font::printCentered(bool top, int y, int value) {
	printCentered(top, y, std::to_string(value));
}

int calcSmallFontWidth(const std::string &text) {
	return mediumFont.calcWidth(text);
}
int calcLargeFontWidth(const std::string &text) {
	return largeFont.calcWidth(text);
}

void printSmall(bool top, int x, int y, const std::string &message){
	mediumFont.print(top, x, y, message);
}
void printSmall(bool top, int x, int y, const std::u16string &message){
	mediumFont.print(top, x, y, message);
}
void printSmallCentered(bool top, int y, const std::string &message){
	mediumFont.printCentered(top, y, message);
}
void printLarge(bool top, int x, int y, const std::string &message){
	largeFont.print(top, x, y, message);
}
void printLargeCentered(bool top, int y, const std::string &message){
	largeFont.printCentered(top, y, message);
}

void clearText(bool top) {
	if(top) {
		dmaFillHalfWords(0, bg2Sub, 256*128);
	} else {
		dmaFillHalfWords(0, bg2Main, 256*192);
	}
}
void clearText(){
	clearText(true);
	clearText(false);
}

void fontInit(void){
	largeFont.load("nitro:/graphics/font/font_l.nftr");
	mediumFont.load("nitro:/graphics/font/font_m.nftr");
	smallFont.load("nitro:/graphics/font/font_s.nftr");
}