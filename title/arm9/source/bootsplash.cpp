#include "bootsplash.h"
#include <nds.h>
// #include <fat.h>
// #include <stdio.h>
#include <maxmod9.h>

// #include "common/lzss.h"
// #include "common/tonccpy.h"
#include "common/dsimenusettings.h"
#include "common/systemdetails.h"
#include "graphics/gif.hpp"

#include "soundbank.h"
//#include "soundbank_bin.h"

extern bool useTwlCfg;

extern u16 bmpImageBuffer[256*192];
// extern u16 videoImageBuffer[39][256*144];

extern u16 convertToDsBmp(u16 val);

// extern void* dsiSplashLocation;

extern bool fadeType;
extern bool controlTopBright;
extern bool controlBottomBright;
extern int screenBrightness;

bool cartInserted;

// static char videoFrameFilename[256];

static FILE* videoFrameFile;

// extern bool rocketVideo_playVideo;
// extern bool rocketVideo_playBackwards;
// extern bool rocketVideo_screen;
// extern int rocketVideo_videoYpos;
// extern int rocketVideo_videoYsize;
// extern int rocketVideo_videoFrames;
// extern int rocketVideo_videoFps;
// extern int rocketVideo_currentFrame;

// #define CONSOLE_SCREEN_WIDTH 32
// #define CONSOLE_SCREEN_HEIGHT 24

mm_sound_effect dsiboot;
mm_sound_effect proceed;

void splashSoundInit() {
	mmInitDefaultMem((mm_addr)0x02FA0000);

	mmLoadEffect( SFX_DSIBOOT );
	mmLoadEffect( SFX_SELECT );

	dsiboot = {
		{ SFX_DSIBOOT } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};

	proceed = {
		{ SFX_SELECT } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
}

void drawNintendoLogoToVram(void) {
	if (!cartInserted) return;

	// Draw first half of Nintendo logo
	int x = 66;
	int y = 155;
	for (int i=122*14; i<122*28; i++) {
		if (x >= 66+122) {
			x = 66;
			y--;
		}
		if (BG_GFX[(256*192)+i] != 0xFFFF) {
			BG_GFX[y*256+x] = BG_GFX[(256*192)+i];
		}
		x++;
	}
}

void BootSplashDSi(void) {
	u16 whiteCol = 0xFFFF;
	whiteCol = ((whiteCol>>10)&0x1f) | ((whiteCol)&((31-3*ms().blfLevel)<<5)) | (whiteCol&(31-6*ms().blfLevel))<<10 | BIT(15);
	for (int i = 0; i < 256*256; i++) {
		BG_GFX[i] = whiteCol;
		BG_GFX_SUB[i] = whiteCol;
	}

	cartInserted = (REG_SCFG_MC != 0x11);

	int language = ms().guiLanguage;
	if (ms().guiLanguage == -1) {
		language = (useTwlCfg ? *(u8*)0x02000406 : PersonalData->language);
	}

	char currentDate[16];
	time_t Raw;
	time(&Raw);
	const struct tm *Time = localtime(&Raw);

	strftime(currentDate, sizeof(currentDate), "%m/%d", Time);

	bool virtualPain = (strcmp(currentDate, "04/01") == 0);

	if (virtualPain) {
		videoFrameFile = fopen("nitro:/graphics/VirtualPain.bmp", "rb");

		if (videoFrameFile) {
			// Start loading
			fseek(videoFrameFile, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
			fseek(videoFrameFile, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 1, 0x18000, videoFrameFile);
			u16* src = bmpImageBuffer;
			int x = 0;
			int y = 191;
			for (int i=0; i<256*192; i++) {
				if (x >= 256) {
					x = 0;
					y--;
				}
				u16 val = *(src++);
				BG_GFX[y*256+x] = convertToDsBmp(val);
				x++;
			}
		}
		fclose(videoFrameFile);

		if (cartInserted) {
			videoFrameFile = fopen("nitro:/graphics/nintendoPain.bmp", "rb");

			if (videoFrameFile) {
				// Start loading
				fseek(videoFrameFile, 0xe, SEEK_SET);
				u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
				fseek(videoFrameFile, pixelStart, SEEK_SET);
				fread(bmpImageBuffer, 1, 0x1D00, videoFrameFile);
				u16* src = bmpImageBuffer;
				int x = 67;
				int y = 159;
				for (int i=0; i<122*30; i++) {
					if (x >= 67+122) {
						x = 67;
						y--;
					}
					u16 val = *(src++);
					BG_GFX[y*256+x] = convertToDsBmp(val);
					x++;
				}
			}
			fclose(videoFrameFile);
		}

		controlTopBright = true;
		controlBottomBright = true;
	} else {
		controlBottomBright = false;
	}

	Gif splash("nitro:/video/dsisplash.gif", true, true);
	Gif healthSafety;
	if (ms().dsiSplash == 2) {
		// Load H&S image
		// Get the language for the splash screen
		char path[256];
		sprintf(path, (virtualPain ? "nitro:/video/VirtualPain_bot.gif" : "nitro:/video/tttstc_%i.gif"), language);
		healthSafety.load(path, false, true);
	}

	timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(100), Gif::timerHandler);

	if (cartInserted) {
		// Gif nintendo("nitro:/graphics/nintendo.gif", false, false);
		// FILE *file = fopen("nitro:/graphics/nintendo.bmp", "rb");

		// if (file) {
		// 	// Start loading

		// 	fseek(videoFrameFile, 0xE, SEEK_SET);
		// 	u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
		// 	fseek(videoFrameFile, pixelStart, SEEK_SET);
		// 	fread(bmpImageBuffer, 1, 0x1B00, videoFrameFile);
		// 	u16* src = bmpImageBuffer;
		// 	for (int i=0; i<122*28; i++) {
		// 		u16 val = *(src++);
		// 		if (val != 0x7C1F) {
		// 			BG_GFX[(256*192)+i] = convertToDsBmp(val);
		// 		}
		// 	}
		// }
		// fclose(videoFrameFile);
	}

	controlTopBright = true;
	controlBottomBright = true;
	fadeType = true;

	while (!splash.finished()) {
		if (splash.waitingForInput()) {
			scanKeys();
			if(keysDown()) {
				splash.resume();
				mmEffectEx(&proceed);
			}
		}
		swiWaitForVBlank();
	}

	// Fade out
	controlTopBright = true;
	controlBottomBright = true;
	fadeType = false;
	for (int i = 0; i < 30; i++) { swiWaitForVBlank(); }

	timerStop(0);
}

void BootSplashInit(void) {
	videoSetMode(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	vramSetBankD(VRAM_D_MAIN_BG_0x06000000);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	
	// bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	// bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);

	splashSoundInit();
	BootSplashDSi();
}
