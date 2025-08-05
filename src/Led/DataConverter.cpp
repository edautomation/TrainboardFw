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

#include "DataConverter.h"

#include "FwConfig.h"
#include "Led.h"
#include "Logging.h"
#include "etl/string.h"
#include "etl/to_string.h"

static inline Led ExtractLed(const uint8_t* const data_in, uint32_t pos);
static inline bool IsInputValid(const uint8_t* const data_in, const uint32_t data_length, const Led* const leds_out, const uint32_t max_leds_out);
static inline void WriteLedsToArray(const uint8_t* const data_in, Led* const leds_out, uint32_t nr_of_leds);
static inline bool IsDataNullEmptyOrTooLong(const uint8_t* const data, uint32_t data_length, uint32_t max_length);

bool DataConv_IsDataValid(const uint8_t* const data, uint32_t data_length)
{
    bool is_data_valid = true;

    if (IsDataNullEmptyOrTooLong(data, data_length, kBufferSizeInBytes))
    {
        is_data_valid = false;
    }
    else
    {
        const auto number_of_leds = (data[0] << 8) | data[1];
        const auto number_of_leds_bytes_expected = kBytesPerLed * number_of_leds;
        const auto expected_buffer_length = kBytesInHeader + number_of_leds_bytes_expected;
        is_data_valid = (data_length == expected_buffer_length);

        if (!is_data_valid)
        {
            etl::string<64> msg("DataConverter - Wrong data length: ");
            etl::to_string(data_length, msg, true);
            msg.append(" != ");
            etl::to_string(expected_buffer_length, msg, true);
            LOG_ERROR(msg.c_str());
        }
    }

    return is_data_valid;
}

bool DataConv_IsHistoryDataValid(const uint8_t* const data, uint32_t data_length)
{
    bool is_data_valid = true;

    if (IsDataNullEmptyOrTooLong(data, data_length, kNumberOfHistoryFrames * kBufferSizeInBytes))
    {
        LOG_DEBUG("DataConv - Data null, empty or too long");
        is_data_valid = false;
    }
    else
    {
        size_t frame_start_index = 0U;
        size_t frame_cnt = 0U;
        while ((frame_cnt < kNumberOfHistoryFrames) && (frame_start_index + kBytesInHeader < data_length) && is_data_valid)
        {
            const auto second_byte_in_frame = frame_start_index + 1;
            const auto n_leds_in_frame = (data[frame_start_index] << 8) + data[second_byte_in_frame];
            const auto frame_length = kBytesInHeader + n_leds_in_frame * kBytesPerLed;
            is_data_valid = DataConv_IsDataValid(&data[frame_start_index], frame_length);
            if (!is_data_valid)
            {
                etl::string<64> msg("DataConv - Invalid frame at index: ");
                etl::to_string(frame_start_index, msg, true);
                LOG_ERROR(msg.c_str());
            }
            frame_cnt++;
            frame_start_index += frame_length;
        }

        if (frame_start_index != data_length)
        {
            etl::string<64> msg("DataConv - Frame start index does not match data length: ");
            etl::to_string(frame_start_index, msg, true);
            LOG_ERROR(msg.c_str());
        }
        is_data_valid = is_data_valid && (frame_start_index == data_length);
    }

    return is_data_valid;
}

std::optional<uint32_t> DataConv_DataToLeds(const uint8_t* const data_in,
                                            const uint32_t data_length,
                                            Led* const leds_out,
                                            const uint32_t max_leds_out)
{
    std::optional<uint32_t> result;

    if (!IsInputValid(data_in, data_length, leds_out, max_leds_out))
    {
        LOG_DEBUG("DataConv - Data invalid");
        result = std::nullopt;
    }
    else
    {
        const auto nr_of_leds = (data_in[0] << 8) | data_in[1];
        const auto expected_data_size = kBytesInHeader + kBytesPerLed * nr_of_leds;

        if ((expected_data_size != data_length) || (nr_of_leds > max_leds_out))
        {
            LOG_DEBUG("DataConv - Invalid data size or number of leds");
            result = 0U;
        }
        else
        {
            result = nr_of_leds;
            WriteLedsToArray(data_in, leds_out, nr_of_leds);
        }
    }

    return result;
}

static inline Led ExtractLed(const uint8_t* const data_in, uint32_t pos)
{
    ASSERT(nullptr != data_in);

    const auto strip_id = data_in[pos];
    const auto led_id = data_in[pos + 1U];

    const auto red = data_in[pos + 2U];
    const auto green = data_in[pos + 3U];
    const auto blue = data_in[pos + 4U];

    const uint32_t color = (red << 16U) + (green << 8U) + blue;
    const uint16_t id = ((strip_id << 8U) | led_id);

    return Led{id, color};
}

static inline bool IsInputValid(const uint8_t* const data_in, const uint32_t data_length, const Led* const leds_out, const uint32_t max_leds_out)
{
    return (nullptr != data_in) && (0U != data_length) && (nullptr != leds_out) && (0U != max_leds_out);
}

static inline void WriteLedsToArray(const uint8_t* const data_in, Led* const leds_out, uint32_t nr_of_leds)
{
    ASSERT(nullptr != data_in);
    ASSERT(nullptr != leds_out);

    for (auto i = 0U; i < nr_of_leds; i++)
    {
        auto pos = (i * kBytesPerLed) + kBytesInHeader;
        leds_out[i] = ExtractLed(data_in, pos);
    }
}

static inline bool IsDataNullEmptyOrTooLong(const uint8_t* const data, uint32_t data_length, uint32_t max_length)
{
    const bool is_data_null = (data == nullptr);
    const bool is_data_empty = (data_length == 0U);
    const bool is_data_too_long = (data_length > max_length);

    if (is_data_null)
    {
        LOG_ERROR("DataConverter - Buffer is nullptr!");
    }

    if (is_data_empty)
    {
        LOG_ERROR("DataConverter - Data empty!");
    }

    if (is_data_too_long)
    {
        etl::string<64> msg("DataConverter - Data too long: ");
        etl::to_string(data_length, msg, true);
        msg.append(" > ");
        etl::to_string(max_length, msg, true);
        LOG_ERROR(msg.c_str());
    }

    return (is_data_null || is_data_empty || is_data_too_long);
}
