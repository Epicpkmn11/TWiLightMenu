#pragma once

#include <string>
#include <vector>

class Font {
private:
	unsigned int getCharIndex(const char16_t letter);
	struct Info {
		std::vector<char> tiles, widths;
		std::vector<short> map;
		short tileSize, tileWidth, tileHeight;
	} font;

public:
	Font() { };
	int load(const std::string &path);
	void print(bool top, int x, int y, const std::string &text);
	void print(bool top, int x, int y, const std::u16string &text);
	int calcWidth(const std::string &text);
	void print(bool top, int x, int y, int value);
	int getCenteredX(const std::string &text);
	void printCentered(bool top, int y, const std::string &text);
	void printCentered(bool top, int y, int value);
};

void processPrints(void);

void printSmall(bool top, int x, int y, const std::string &message);
void printSmall(bool top, int x, int y, const std::u16string &message);
void printSmallCentered(bool top, int y, const std::string &message);
void printLarge(bool top, int x, int y, const std::string &message);
void printLargeCentered(bool top, int y, const std::string &message);

int calcSmallFontWidth(const std::string &text);
int calcLargeFontWidth(const std::string &text);

void clearText(bool top);
void clearText();

void fontInit(void);

extern Font smallFont, mediumFont, largeFont;