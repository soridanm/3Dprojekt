/**
* Course: DV1542 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: GlobalSettings.hpp
*
* File summary:
*	A file to make it easy to switch between different screen resolutions, 
*	shadow map resolutions, quadtree depths, and windowed/non-windowed mode. 
*/

#ifndef GLOBALSETTINGS_HPP
#define GLOBALSETTINGS_HPP

#include "GlobalResources.hpp"

static bool GOD_CAMERA_ENABLED = !false;

//static const ScreenSize SCREEN_RESOLUTION = ScreenResolutionPresets::LOW_480p;
//static const ScreenSize SCREEN_RESOLUTION = ScreenResolutionPresets::HD_720p;
static const ScreenSize SCREEN_RESOLUTION = ScreenResolutionPresets::FHD_1080p;
//static const ScreenSize SCREEN_RESOLUTION = ScreenResolutionPresets::QHD_1440p;
//static const ScreenSize SCREEN_RESOLUTION = ScreenResolutionPresets::UHD_1;
//static const ScreenSize SCREEN_RESOLUTION = ScreenResolutionPresets::FHD_1920x1000;

//static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::LOW;
//static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::MEDIUM;
static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::HIGH;
//static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::EXTREME;
//static const ShadowQuality SHADOW_QUALITY = ShadowMapPresets::ULTRA;

static const BOOL WINDOWED_MODE_SETTING = TRUE;

static const int NR_OF_QUADREE_LEVELS = 5;

#endif // !GLOBALSETTINGS_HPP
