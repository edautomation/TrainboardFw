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

#include "gtest/gtest.h"

#include "DataConverter.h"

#include "FwConfig.h"
#include "blob_HistoryData.hpp"

TEST(IsHistoryDataValidTest, NullPtrBuffer_ReturnsFalse)
{
    EXPECT_FALSE(DataConv_IsHistoryDataValid(nullptr, 42U));
}

TEST(IsHistoryDataValidTest, ZeroLength_ReturnsFalse)
{
    uint8_t buffer[42U];
    EXPECT_FALSE(DataConv_IsHistoryDataValid(buffer, 0U));
}

TEST(IsHistoryDataValidTest, LengthGreaterThanMaxBufferSize_ReturnsFalse)
{
    uint8_t buffer[42U];
    EXPECT_FALSE(DataConv_IsHistoryDataValid(buffer, (kNumberOfHistoryFrames * kBufferSizeInBytes) + 1U));
}
TEST(IsHistoryDataValidTest, LessThanMaxFrames_ReturnsTrue)
{
    constexpr size_t kNLedsInFrame = 3U;
    constexpr size_t kFrameLength = kBytesInHeader + kNLedsInFrame * kBytesPerLed;
    constexpr size_t kNFrames = 3U;
    constexpr size_t kBufferLength = kNFrames * kFrameLength;
    uint8_t buffer[kBufferLength] = {};
    for (auto i = 0U; i < kNFrames; i++)
    {
        buffer[i * kFrameLength + 1] = static_cast<uint8_t>(kNLedsInFrame & 0xFF);
        buffer[i * kFrameLength + 2] = static_cast<uint8_t>((kNLedsInFrame >> 8) & 0xFF);
    }
    EXPECT_TRUE(DataConv_IsHistoryDataValid(buffer, kBufferLength));
}

TEST(IsHistoryDataValidTest, WrongFrame_ReturnFalse)
{
    std::array<uint8_t, HISTORY_BUFFER_LENGTH> buffer(history_buffer);
    buffer.at(1) = 42U;  // wrong nr of leds
    EXPECT_FALSE(DataConv_IsHistoryDataValid(buffer.data(), HISTORY_BUFFER_LENGTH));
}

TEST(IsHistoryDataValidTest, ValidNumberOfFramesAndValidFrames_ReturnsTrue)
{
    constexpr size_t kNLedsInFrame = 3U;
    constexpr size_t kFrameLength = kBytesInHeader + kNLedsInFrame * kBytesPerLed;
    constexpr size_t kNFrames = kNumberOfHistoryFrames;
    constexpr size_t kBufferLength = kNFrames * kFrameLength;
    uint8_t buffer[kBufferLength] = {};
    for (auto i = 0U; i < kNFrames; i++)
    {
        buffer[i * kFrameLength + 1] = static_cast<uint8_t>(kNLedsInFrame & 0xFF);
        buffer[i * kFrameLength + 2] = static_cast<uint8_t>((kNLedsInFrame >> 8) & 0xFF);
    }
    EXPECT_TRUE(DataConv_IsHistoryDataValid(buffer, kBufferLength));
}

TEST(IsHistoryDataValidTest, RealDataFromServer_ReturnsTrue)
{
    EXPECT_TRUE(DataConv_IsHistoryDataValid(history_buffer.data(), HISTORY_BUFFER_LENGTH));
}
