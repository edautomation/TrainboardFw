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
#include "DataManagerTypes.h"
#include "FakeData.h"
#include "FwConfig.h"

#include <array>

class DataMgrFakeStoreTest : public ::testing::Test
{
  protected:
    DataReader* reader_;
    void SetUp() override
    {
        DataMgr_Reset();
        DataMgr_SetReaderMode(DataReaderMode::kOffline);
        reader_ = DataMgr_GetReader();
        ASSERT_NE(reader_, nullptr);
    }
};

TEST_F(DataMgrFakeStoreTest, Reader_DataNullptr_ReturnZero)
{
    EXPECT_EQ(reader_->ReadData(nullptr, 42U), 0U);
}

TEST_F(DataMgrFakeStoreTest, Reader_LengthZero_ReturnZero)
{
    std::array<uint8_t, 42U> buffer;
    EXPECT_EQ(reader_->ReadData(buffer.data(), 0U), 0U);
}

TEST_F(DataMgrFakeStoreTest, CircularReadThroughFakeData_EnsureCircularBehaviour)
{
    for (auto i = 0; i < kNumberOfFakeFrames + 5U; i++)
    {
        Frame read_frame{};
        const auto read_length = reader_->ReadData(read_frame.data, kBufferSizeInBytes);

        EXPECT_EQ(read_length, g_fake_data[i % kNumberOfHistoryFrames].length);
        for (auto j = 0; j < read_length; j++)
        {
            ASSERT_EQ(read_frame.data[j], g_fake_data[i % kNumberOfHistoryFrames].data[j]);
        }
    }
}
