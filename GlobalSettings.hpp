#ifndef GLOBALSETTINGS_HPP
#define GLOBALSETTINGS_HPP

#include "GlobalResources.hpp"

static bool GOD_CAMERA_ENABLED = false;

//static const ScreenSize SCREEN_RESOLUTION = ScreenResolution::LOW_480p;
static const ScreenSize SCREEN_RESOLUTION = ScreenResolution::HD_720p;
//static const ScreenSize SCREEN_RESOLUTION = ScreenResolution::FHD_1080p;
//static const ScreenSize SCREEN_RESOLUTION = ScreenResolution::QHD_1440p;
//static const ScreenSize SCREEN_RESOLUTION = ScreenResolution::UHD_1;
//static const ScreenSize SCREEN_RESOLUTION = ScreenResolution::FHD_1920x1000;

//static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::LOW;
//static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::MEDIUM;
static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::HIGH;
//static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::EXTREME;
//static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::ULTRA;

static const BOOL WINDOWED_MODE_SETTING = TRUE;

static const int NR_OF_QUADREE_LEVELS = 8;

#endif // !GLOBALSETTINGS_HPP
