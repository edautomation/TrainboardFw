// Trainboard.ch
// Copyright (C) 2024 Emile Décosterd
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef FW_CONFIG_H_
#define FW_CONFIG_H_

#include <cstdint>

// Version info
#define FW_VERSION "0.11.0"
constexpr uint32_t kHwVersionPin = 8;

// Scheduling
constexpr uint32_t kEventQueueSize = 16U;
constexpr uint32_t kTickPeriodMilliSeconds = 20;

// Data
constexpr uint32_t kMaxLedsOn = 312U;
constexpr uint32_t kBytesPerLed = 5U;
constexpr uint32_t kBytesInHeader = 2U;
constexpr uint32_t kBufferSizeInBytes = (kMaxLedsOn * kBytesPerLed) + kBytesInHeader;
constexpr uint32_t kNumberOfHistoryFrames = 45U;
constexpr uint32_t kNumberOfFakeFrames = kNumberOfHistoryFrames;

// Provisioning configuration
constexpr uint32_t kConnectTimeout = 20;           // s
constexpr uint32_t kConnectRetries = 3;            // Number of retries from the library
constexpr uint32_t kConfigPortalTimeout = 5 * 60;  // s
constexpr const char* kAccessPointName = "Trainboard_AP";

// Button
constexpr uint32_t kDebounceDurationMilliSeconds = 50U;
constexpr uint32_t kShortPushMinDurationMilliSeconds = 100U;

// Light sensor
constexpr uint8_t kDefaultBrightness = 15;
constexpr uint8_t kMinimumBrightness = 7;
constexpr uint32_t kBrigthnessUpdateRateMilliSec = 100;

// Pins
constexpr uint8_t kButtonPinNr = 7;
constexpr uint8_t kLedLevelShiftEnPin = 9;
constexpr uint8_t kLightSensorSdaPin = 4;
constexpr uint8_t kLightSensorSclPin = 6;

// Led
constexpr uint32_t kNumberOfStrips = 4U;
constexpr uint32_t kTransitionDurationInTicks = 2000 / kTickPeriodMilliSeconds;

constexpr uint32_t kV1Strip1NLeds = 145;
constexpr uint32_t kV1Strip2NLeds = 54;
constexpr uint32_t kV1Strip3NLeds = 69;
constexpr uint32_t kV1Strip4NLeds = 35;
constexpr auto kV1NLeds = (kV1Strip1NLeds + kV1Strip2NLeds + kV1Strip3NLeds + kV1Strip4NLeds);

constexpr uint32_t kV11Strip1NLeds = 84;
constexpr uint32_t kV11Strip2NLeds = 65;
constexpr uint32_t kV11Strip3NLeds = 76;
constexpr uint32_t kV11Strip4NLeds = 87;
constexpr auto kV11NLeds = (kV11Strip1NLeds + kV11Strip2NLeds + kV11Strip3NLeds + kV11Strip4NLeds);

constexpr uint32_t kV12Strip1NLeds = 84;
constexpr uint32_t kV12Strip2NLeds = 65;
constexpr uint32_t kV12Strip3NLeds = 75;
constexpr uint32_t kV12Strip4NLeds = 86;
constexpr auto kV12NLeds = (kV12Strip1NLeds + kV12Strip2NLeds + kV12Strip3NLeds + kV12Strip4NLeds);

#endif  // FW_CONFIG_H_
