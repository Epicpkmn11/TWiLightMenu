#include "ThemeConfig.h"
#include "ThemeTextures.h"
#include "themefilenames.h"
#include "common/dsimenusettings.h"
#include "common/singleton.h"
#include "common/inifile.h"

#include <nds.h>
#include <string>

ThemeConfig::ThemeConfig()
	: ThemeConfig(false) {}

// Magic numbers derived from default dark theme
ThemeConfig::ThemeConfig(bool _3dsDefaults)
	: _startBorderRenderY(81), _startBorderSpriteW(32), _startBorderSpriteH(80), _startTextRenderY(143), _titleboxRenderY(85),
	  _bubbleTipRenderY(80), _bubbleTipRenderX(122), _bubbleTipSpriteH(8), _bubbleTipSpriteW(11),
	  _rotatingCubesRenderY(78), _shoulderLRenderY(172), _shoulderLRenderX(0), _shoulderRRenderY(172), _shoulderRRenderX(178),
	  _volumeRenderY(4), _volumeRenderX(16), _batteryRenderY(5), _batteryRenderX(235),
	  _dateRenderY(7), _dateRenderX(200), _timeRenderY(7), _timeRenderX(200),
	  // _photoRenderY(24), _photoRenderX(179),
	  _startTextUserPalette(true), _startBorderUserPalette(true), _buttonArrowUserPalette(true),
	  _movingArrowUserPalette(true), _launchDotsUserPalette(true), _dialogBoxUserPalette(true),
	  _renderPhoto(true), _renderDropDown(true), _renderName(true),
	  _playStartupJingle(false), _startupJingleDelayAdjust(0),
	  _allowBoxArt(false), _allowRVid(false),
{
	// hack to reassign 3ds defaults
	if (_3dsDefaults) { 
		_startBorderRenderY = 92;
		_startBorderSpriteH = 64;
		_titleboxRenderY = 96;
		_bubbleTipRenderX = 125;
		_bubbleTipRenderY = 98;
		_bubbleTipSpriteH = 7;
		_bubbleTipSpriteW = 7;
		_renderPhoto = false;
	}
	
	if (ms().theme == 4 || ms().theme == 5) {
		_renderPhoto = false;
	}
}


void ThemeConfig::loadConfig()
{
	printf("tc().loadConfig()\n");
	CIniFile themeConfig(TFN_THEME_SETTINGS);
	_startBorderRenderY = themeConfig.GetInt("THEME", "StartBorderRenderY", _startBorderRenderY);
	_startBorderSpriteW = themeConfig.GetInt("THEME", "StartBorderSpriteW", _startBorderSpriteW);
	_startBorderSpriteH = themeConfig.GetInt("THEME", "StartBorderSpriteH", _startBorderSpriteH);
	_startTextRenderY = themeConfig.GetInt("THEME", "StartTextRenderY", _startTextRenderY);

	_bubbleTipRenderY = themeConfig.GetInt("THEME", "BubbleTipRenderY", _bubbleTipRenderY);
	_bubbleTipRenderX = themeConfig.GetInt("THEME", "BubbleTipRenderX", _bubbleTipRenderX);
	_bubbleTipSpriteW = themeConfig.GetInt("THEME", "BubbleTipSpriteW", _bubbleTipSpriteW);
	_bubbleTipSpriteH = themeConfig.GetInt("THEME", "BubbleTipSpriteH", _bubbleTipSpriteH);
	_titleboxRenderY = themeConfig.GetInt("THEME", "TitleboxRenderY", _titleboxRenderY);
	_volumeRenderX = themeConfig.GetInt("THEME", "VolumeRenderX", _volumeRenderX);
	_volumeRenderY = themeConfig.GetInt("THEME", "VolumeRenderY", _volumeRenderY);
	// _photoRenderX = themeConfig.GetInt("THEME", "PhotoRenderX", _photoRenderX);
	// _photoRenderY = themeConfig.GetInt("THEME", "PhotoRenderY", _photoRenderY);
	_shoulderLRenderY = themeConfig.GetInt("THEME", "ShoulderLRenderY", _shoulderLRenderY);
	_shoulderLRenderX = themeConfig.GetInt("THEME", "ShoulderLRenderX", _shoulderLRenderX);
	_shoulderRRenderY = themeConfig.GetInt("THEME", "ShoulderRRenderY", _shoulderRRenderY);
	_shoulderRRenderX = themeConfig.GetInt("THEME", "ShoulderRRenderX", _shoulderRRenderX);
	_batteryRenderY = themeConfig.GetInt("THEME", "BatteryRenderY", _batteryRenderY);
	_batteryRenderX = themeConfig.GetInt("THEME", "BatteryRenderX", _batteryRenderX);
	
	_dateRenderY = themeConfig.GetInt("THEME", "DateRenderY", _dateRenderY);
	_dateRenderX = themeConfig.GetInt("THEME", "DateRenderX", _dateRenderX);
	_timeRenderY = themeConfig.GetInt("THEME", "TimeRenderY", _timeRenderY);
	_timeRenderX = themeConfig.GetInt("THEME", "TimeRenderX", _timeRenderX);

	_startTextUserPalette = themeConfig.GetInt("THEME", "StartTextUserPalette", _startTextUserPalette);
	_startBorderUserPalette = themeConfig.GetInt("THEME", "StartBorderUserPalette", _startBorderUserPalette);
	_buttonArrowUserPalette = themeConfig.GetInt("THEME", "ButtonArrowUserPalette", _buttonArrowUserPalette);
	_movingArrowUserPalette = themeConfig.GetInt("THEME", "MovingArrowUserPalette", _movingArrowUserPalette);
	_launchDotsUserPalette = themeConfig.GetInt("THEME", "LaunchDotsUserPalette", _launchDotsUserPalette);
	_dialogBoxUserPalette = themeConfig.GetInt("THEME", "DialogBoxUserPalette", _dialogBoxUserPalette);
	_rotatingCubesRenderY = themeConfig.GetInt("THEME", "RotatingCubesRenderY", _rotatingCubesRenderY);
	_renderPhoto = themeConfig.GetInt("THEME", "RenderPhoto", _renderPhoto);
	_renderDropDown = themeConfig.GetInt("THEME", "RenderDropDown", _renderDropDown);
	_renderName = themeConfig.GetInt("THEME", "RenderName", _renderName);

	_allowBoxArt = themeConfig.GetInt("THEME", "AllowBoxArt", _allowBoxArt);
	_allowRVid = themeConfig.GetInt("THEME", "AllowRVid", _allowRVid);

	_playStartupJingle = themeConfig.GetInt("THEME", "PlayStartupJingle", _playStartupJingle);
	_startupJingleDelayAdjust = themeConfig.GetInt("THEME", "StartupJingleDelayAdjust", _startupJingleDelayAdjust);
}
