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

#include "DataManager.h"
#include "FwConfig.h"

#include <array>

class DataMgrHistoryStoreTest : public ::testing::Test
{
  protected:
    static constexpr uint32_t kFrameLength = 7U;
    static constexpr uint32_t kHistDataBufferLength = kFrameLength * kNumberOfHistoryFrames;
    std::array<uint8_t, kHistDataBufferLength> hist_buffer = {};
    DataReader* reader_;
    DataWriter* writer_;
    void SetUp() override
    {
        DataMgr_Reset();
        DataMgr_SetReaderMode(DataReaderMode::kHistory);
        DataMgr_SetWriterMode(DataWriterMode::kMultiple);
        reader_ = DataMgr_GetReader();
        writer_ = DataMgr_GetWriter();
        ASSERT_NE(reader_, nullptr);
        ASSERT_NE(writer_, nullptr);

        // Fake history buffer
        size_t buffer_index = 0;
        for (uint8_t i = 0U; i < kNumberOfHistoryFrames; i++, buffer_index += kFrameLength)
        {
            hist_buffer[buffer_index + 1] = 1U;  // Number of LEDs
            hist_buffer[buffer_index + 2] = 1 * i;
            hist_buffer[buffer_index + 3] = 1 * i + 1;
            hist_buffer[buffer_index + 4] = 1 * i + 2;
            hist_buffer[buffer_index + 5] = 1 * i + 3;
            hist_buffer[buffer_index + 6] = 1 * i + 4;
        }
    }
};

TEST_F(DataMgrHistoryStoreTest, Reader_DataNullptr_ReturnZero)
{
    EXPECT_EQ(reader_->ReadData(nullptr, 42U), 0U);
}

TEST_F(DataMgrHistoryStoreTest, Reader_LengthZero_ReturnZero)
{
    uint8_t buffer[42U];
    EXPECT_EQ(reader_->ReadData(buffer, 0U), 0U);
}

TEST_F(DataMgrHistoryStoreTest, Writer_DataNullptr_ReturnFalse)
{
    EXPECT_FALSE(writer_->SaveData(nullptr, 42U));
}

TEST_F(DataMgrHistoryStoreTest, Writer_LengthZero_ReturnFalse)
{
    uint8_t buffer[42U] = {};
    EXPECT_FALSE(writer_->SaveData(buffer, 0U));
}

TEST_F(DataMgrHistoryStoreTest, Writer_LengthTooBig_ReturnFalse)
{
    constexpr auto kHistoryDataMaxSize = kBufferSizeInBytes * kNumberOfHistoryFrames;
    uint8_t buffer[kHistoryDataMaxSize + 1] = {};
    EXPECT_FALSE(writer_->SaveData(buffer, kHistoryDataMaxSize + 1));
}

TEST_F(DataMgrHistoryStoreTest, WriterWithValidData_ReadData_GetFirstFrame)
{
    EXPECT_TRUE(writer_->SaveData(hist_buffer.data(), hist_buffer.size()));

    std::array<uint8_t, kFrameLength> read_buffer{};
    std::array<uint8_t, kFrameLength> expected_frame{0, 1, 0, 1, 2, 3, 4};
    EXPECT_EQ(kFrameLength, reader_->ReadData(read_buffer.data(), read_buffer.size()));
    EXPECT_EQ(read_buffer, expected_frame);
}

TEST_F(DataMgrHistoryStoreTest, WriterWithValidData_ReadDataWithBufferTooSmall_ReturnZeroLength)
{
    EXPECT_TRUE(writer_->SaveData(hist_buffer.data(), hist_buffer.size()));

    std::array<uint8_t, kFrameLength> read_buffer{};
    EXPECT_EQ(0U, reader_->ReadData(read_buffer.data(), read_buffer.size() - 1));
}

TEST_F(DataMgrHistoryStoreTest, WriterWithValidData_ReadDataTwice_GetSecondFrame)
{
    EXPECT_TRUE(writer_->SaveData(hist_buffer.data(), hist_buffer.size()));

    std::array<uint8_t, kFrameLength> read_buffer{};
    std::array<uint8_t, kFrameLength> expected_frame{0, 1, 1, 2, 3, 4, 5};
    EXPECT_EQ(kFrameLength, reader_->ReadData(read_buffer.data(), read_buffer.size()));
    EXPECT_EQ(kFrameLength, reader_->ReadData(read_buffer.data(), read_buffer.size()));
    EXPECT_EQ(read_buffer, expected_frame);
}
TEST_F(DataMgrHistoryStoreTest, Writer_Not70Frames_ReturnTrue)
{
    EXPECT_TRUE(writer_->SaveData(hist_buffer.data(), hist_buffer.size() - 1));
}

TEST_F(DataMgrHistoryStoreTest, LiveModeOverwriteHistory_SecondOldestFrameIsTakeWhenReadingAndNewFrameReadLast)
{
    // Write history
    EXPECT_TRUE(writer_->SaveData(hist_buffer.data(), hist_buffer.size()));

    // Change to live mode, write new data
    DataMgr_SetReaderMode(DataReaderMode::kLive);
    DataMgr_SetWriterMode(DataWriterMode::kSingle);
    auto live_writer = DataMgr_GetWriter();
    ASSERT_NE(nullptr, live_writer);
    const std::array<uint8_t, kFrameLength + kBytesPerLed> live_data{0, 2, 0, 1, 255, 255, 255, 1, 2, 42, 42, 42};
    EXPECT_TRUE(live_writer->SaveData(live_data.data(), live_data.size()));

    // Change to history again
    DataMgr_SetReaderMode(DataReaderMode::kHistory);
    DataMgr_SetWriterMode(DataWriterMode::kMultiple);

    // Read from history: 0,1,2,3,4 was overwritten
    std::array<uint8_t, kFrameLength> read_buffer{};
    std::array<uint8_t, kFrameLength> expected_frame{0, 1, 1, 2, 3, 4, 5};
    EXPECT_EQ(kFrameLength, reader_->ReadData(read_buffer.data(), read_buffer.size()));
    EXPECT_EQ(read_buffer, expected_frame);

    // Read all but last frame from history
    constexpr auto kNumberOfFramesBetweenFirstAndLast = kNumberOfHistoryFrames - 2;
    for (auto i = 0; i < kNumberOfFramesBetweenFirstAndLast; i++)
    {
        EXPECT_EQ(kFrameLength, reader_->ReadData(read_buffer.data(), read_buffer.size()));
    }

    // Read last frame, should be the one we just wrote in live mode
    std::array<uint8_t, kFrameLength + kBytesPerLed> read_last_hist{};
    EXPECT_EQ(kFrameLength + kBytesPerLed, reader_->ReadData(read_last_hist.data(), read_last_hist.size()));
    EXPECT_EQ(read_last_hist, live_data);
}

TEST_F(DataMgrHistoryStoreTest, HistoryModeReadOneFrame_GetReaderAgain_ReadNextFrame)
{
    // Write history
    ASSERT_TRUE(writer_->SaveData(hist_buffer.data(), hist_buffer.size()));

    // Read one frame
    std::array<uint8_t, kFrameLength> read_buffer{};
    std::array<uint8_t, kFrameLength> expected_frame_1{0, 1, 0, 1, 2, 3, 4};
    EXPECT_EQ(kFrameLength, reader_->ReadData(read_buffer.data(), read_buffer.size()));
    EXPECT_EQ(read_buffer, expected_frame_1);

    // Get reader again (use case: get the reader in a polling function)
    reader_ = DataMgr_GetReader();

    // Read the next frame
    std::array<uint8_t, kFrameLength> expected_frame_2{0, 1, 1, 2, 3, 4, 5};
    EXPECT_EQ(kFrameLength, reader_->ReadData(read_buffer.data(), read_buffer.size()));
    EXPECT_EQ(read_buffer, expected_frame_2);
}

TEST_F(DataMgrHistoryStoreTest, HistoryMode_RewriteHistory_HistoryRewritten)
{
    // Write history
    ASSERT_TRUE(writer_->SaveData(hist_buffer.data(), hist_buffer.size()));

    // Rewrite history
    size_t buffer_index = 0;
    for (uint8_t i = 0U; i < kNumberOfHistoryFrames; i++, buffer_index += kFrameLength)
    {
        hist_buffer[buffer_index + 1] = 1U;  // Number of LEDs
        hist_buffer[buffer_index + 2] = 2 * i + 1;
        hist_buffer[buffer_index + 3] = 2 * i + 2;
        hist_buffer[buffer_index + 4] = 2 * i + 3;
        hist_buffer[buffer_index + 5] = 2 * i + 4;
        hist_buffer[buffer_index + 6] = 2 * i + 5;
    }
    ASSERT_TRUE(writer_->SaveData(hist_buffer.data(), hist_buffer.size()));

    // Check if history was rewritten
    std::array<uint8_t, kFrameLength> read_buffer{};
    std::array<uint8_t, kFrameLength> new_second_frame{0, 1, 3, 4, 5, 6, 7};
    EXPECT_EQ(kFrameLength, reader_->ReadData(read_buffer.data(), read_buffer.size()));
    EXPECT_EQ(kFrameLength, reader_->ReadData(read_buffer.data(), read_buffer.size()));
    EXPECT_EQ(read_buffer, new_second_frame);
}

// TODO: write less than max history size, then read max history size: last frame is the same as the first one

TEST_F(DataMgrHistoryStoreTest, WriterWithValidData_ReadDataMaxSize_GetLastFrame)
{
    EXPECT_TRUE(writer_->SaveData(hist_buffer.data(), hist_buffer.size()));

    static_assert(kNumberOfHistoryFrames < UINT8_MAX);
    constexpr auto k = static_cast<uint8_t>(kNumberOfHistoryFrames) - 1;
    std::array<uint8_t, kFrameLength> read_buffer{};
    std::array<uint8_t, kFrameLength> expected_frame{0, 1, k, k + 1, k + 2, k + 3, k + 4};
    for (auto i = 0; i < kHistDataBufferLength; i++)
    {
        ASSERT_EQ(kFrameLength, reader_->ReadData(read_buffer.data(), read_buffer.size()));
    }
    EXPECT_EQ(read_buffer, expected_frame);
}

TEST_F(DataMgrHistoryStoreTest, WriterWithValidDataLessThanMax_ReadDataMaxSize_GetFirstFrame)
{
    EXPECT_TRUE(writer_->SaveData(hist_buffer.data(), hist_buffer.size() - 1));

    std::array<uint8_t, kFrameLength> read_buffer{};
    std::array<uint8_t, kFrameLength> expected_frame{0, 1, 0, 1, 2, 3, 4};
    for (auto i = 0; i < kNumberOfHistoryFrames; i++)
    {
        ASSERT_EQ(kFrameLength, reader_->ReadData(read_buffer.data(), read_buffer.size()));
    }
    EXPECT_EQ(read_buffer, expected_frame);
}
