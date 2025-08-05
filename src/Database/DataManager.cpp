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

#include "DataManager.h"
#include "DataManagerTypes.h"

#include "FakeData.h"
#include "FwConfig.h"
#include "Logging.h"
#include "WifiProvisioning.h"

#include "etl/array.h"
#include "etl/circular_buffer.h"

// WARNING: NOTHING IS THREAD-SAFE HERE!

// Anonymous namespace as alternative to static
// https://stackoverflow.com/questions/4977252/why-an-unnamed-namespace-is-a-superior-alternative-to-static
namespace
{

class LiveDataStoreReader : public DataReader
{
  public:
    explicit LiveDataStoreReader(const RealDataCircularBuffer& circular_buffer) : buffer_(circular_buffer) {}

    uint32_t ReadData(uint8_t* const data, const uint32_t max_length) override
    {
        if ((nullptr == data) || (0U == max_length))
        {
            return 0U;
        }

        uint32_t read_frame_length = 0U;
        const Frame& newest_frame = buffer_.back();
        const auto frame_length = newest_frame.length;
        if (frame_length > max_length)
        {
            read_frame_length = 0U;
        }
        else
        {
            for (auto i = 0U; i < frame_length; i++)
            {
                data[i] = newest_frame.data[i];
            }
            read_frame_length = frame_length;
        }

        return read_frame_length;
    }

  private:
    const RealDataCircularBuffer& buffer_;
};

class LiveDataStoreWriter : public DataWriter
{
  public:
    explicit LiveDataStoreWriter(RealDataCircularBuffer& circular_buffer) : buffer_(circular_buffer) {}
    bool SaveData(const uint8_t* const data, const uint32_t data_length) override
    {
        if ((nullptr == data) || (0U == data_length) || kBufferSizeInBytes < data_length)
        {
            return false;
        }

        Frame new_frame{data_length, {}};
        for (auto i = 0U; i < data_length; i++)
        {
            new_frame.data[i] = data[i];
        }
        buffer_.push(new_frame);

        return true;
    }

  private:
    RealDataCircularBuffer& buffer_;
};

class HistoryDataStoreReader : public DataReader
{
  public:
    explicit HistoryDataStoreReader(const RealDataCircularBuffer& circular_buffer) : buffer_(circular_buffer) {}

    uint32_t ReadData(uint8_t* const data, const uint32_t max_length) override
    {
        const Frame& frame = buffer_[frame_index_];

        if ((nullptr == data) || (0U == max_length) || frame.length > max_length)
        {
            return 0U;
        }

        for (auto i = 0U; i < max_length; i++)
        {
            data[i] = frame.data[i];
        }

        IncrementFrameIndex();

        return frame.length;
    }
    void Reset()
    {
        frame_index_ = 0;
    }

  protected:
    uint8_t GetFrameIndex() const { return frame_index_; }
    void SetFrameIndex(uint8_t index) { frame_index_ = index; }
    virtual void IncrementFrameIndex()
    {
        frame_index_++;
        if (frame_index_ >= buffer_.size())
        {
            frame_index_ = 0U;
        }
    }

  private:
    const RealDataCircularBuffer& buffer_;
    uint8_t frame_index_{0};
};

class FakeDataStoreReader : public HistoryDataStoreReader
{
  public:
    using HistoryDataStoreReader::HistoryDataStoreReader;

  protected:
    void IncrementFrameIndex() override
    {
        auto frame_index = GetFrameIndex();
        frame_index++;
        if (frame_index >= kNumberOfFakeFrames)
        {
            frame_index = 0U;
        }
        SetFrameIndex(frame_index);
    }
};

class HistoryDataStoreWriter : public DataWriter
{
  public:
    explicit HistoryDataStoreWriter(RealDataCircularBuffer& circular_buffer) : buffer_(circular_buffer) {}
    bool SaveData(const uint8_t* const data, const uint32_t data_length) override
    {
        if ((nullptr == data) || (0U == data_length) || (kBufferSizeInBytes * kNumberOfHistoryFrames < data_length))
        {
            return false;
        }

        size_t frame_start_index = 0U;
        auto is_frame_length_info_in_valid_memory = [&]() { return frame_start_index + kBytesInHeader < data_length; };

        buffer_.clear();  // Discard the old data
        while (!buffer_.full() && is_frame_length_info_in_valid_memory())
        {
            const auto second_byte_in_frame = frame_start_index + 1;
            const auto frame_length = kBytesInHeader + (data[second_byte_in_frame] * kBytesPerLed);
            if (frame_start_index + frame_length > data_length)
            {
                // We would read further than the buffer size
                break;
            }
            Frame frame{frame_length, {}};
            for (auto i = 0U; i < frame_length; i++)
            {
                frame.data[i] = data[frame_start_index + i];
            }
            frame_start_index += frame_length;
            buffer_.push(frame);
        }

        return true;
    }

  private:
    RealDataCircularBuffer& buffer_;
};

Frame _real_data_buffer[kNumberOfHistoryFrames + 1]{};
RealDataCircularBuffer _real_data{static_cast<void*>(&_real_data_buffer[0]), kNumberOfHistoryFrames};

DataReaderMode _data_reader_mode{DataReaderMode::kLive};
DataWriterMode _data_writer_mode{DataWriterMode::kMultiple};
LiveDataStoreWriter _live_data_store_writer{_real_data};
LiveDataStoreReader _live_data_store_reader{_real_data};
HistoryDataStoreWriter _history_data_store_writer{_real_data};
HistoryDataStoreReader _history_data_store_reader{_real_data};
FakeDataStoreReader _fake_data_store_reader{g_fake_data};

}  // namespace

void DataMgr_Reset()
{
    _data_reader_mode = DataReaderMode::kLive;
    _data_writer_mode = DataWriterMode::kMultiple;
    _real_data.clear();
    _history_data_store_reader.Reset();
}

void DataMgr_SetReaderMode(DataReaderMode mode)
{
    _data_reader_mode = mode;
}

DataReaderMode DataMgr_GetReaderMode()
{
    return _data_reader_mode;
}

void DataMgr_SetWriterMode(DataWriterMode mode)
{
    _data_writer_mode = mode;
}

DataWriterMode DataMgr_GetWriterMode()
{
    return _data_writer_mode;
}

DataWriter* DataMgr_GetWriter()
{
    DataWriter* writer = nullptr;
    if (DataWriterMode::kSingle == _data_writer_mode)
    {
        writer = &_live_data_store_writer;
    }
    else if (DataWriterMode::kMultiple == _data_writer_mode)
    {
        writer = &_history_data_store_writer;
    }
    else
    {
        // Not allowed to write
    }
    return writer;
}

DataReader* DataMgr_GetReader()
{
    DataReader* reader = nullptr;
    switch (_data_reader_mode)
    {
        case DataReaderMode::kLive:
            reader = &_live_data_store_reader;
            break;
        case DataReaderMode::kHistory:
            reader = &_history_data_store_reader;
            break;
        case DataReaderMode::kOffline:
            reader = &_fake_data_store_reader;
            break;
    }
    return reader;
}
