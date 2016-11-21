#ifndef CONFIG_H
#define CONFIG_H

#include <cstdlib>

/*
 * Global variables go here.
 */

const float kCylinderRadius = 0.25;
const int kMaxBones = 128;
/*
 * Extra credit: what would happen if you set kNear to 1e-5? How to solve it?
 */
const float kNear = 0.1f;
const float kFar = 5000.0f;
const float kFov = 45.0f;

// Floor info.
const float kFloorEps = 0.5 * (0.025 + 0.0175);
const float kFloorWidth = 2048.0f;
const float kFloorXMin = -2048.0f;
const float kFloorXMax = 2048.0f;
const float kFloorZMin = -2048.0f;
const float kFloorZMax = 2048.0f;
const float kFloorY = -100.0f;
const float kFloorHeight = 400.0f;

const int kFloorSize = 7;	// 1024x1024
const int kFloorDepth = 6;
const int kFloorSeed = std::rand();

const float kSkySize = 2.0f;


#endif
