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

#include "ConnectionListener.h"

#include "Logging.h"
#include "ServerCommunication.h"
#include "Signals.h"
#include "TimerTicker.h"
#include "WifiProvisioning.h"

void ConnectionListener::Dispatch(const uint16_t event)
{
    if (TICK == event)
    {
        switch (state_)
        {
            case State::kWifiNok:
                WifiNokStateFunc();
                break;
            case State::kServerNok:
                ServerNokStateFunc();
                break;
            case State::kServerOk:
                ServerOkStateFunc();
                break;
        }
    }
}

void ConnectionListener::WifiNokStateFunc()
{
    if (WifiProv_IsConnectedToWifi())
    {
        LOG_INFO("Connection listener - Network up");
        state_ = State::kServerNok;
        event_queue_.push(NETWORK_UP);
    }
}

void ConnectionListener::ServerNokStateFunc()
{
    if (WifiProv_IsConnectedToWifi())
    {
        constexpr uint32_t kConnectionCheckIntervalInSeconds = 30;
        if (IsTimeElapsed(kConnectionCheckIntervalInSeconds))
        {
            if (Ping())
            {
                LOG_INFO("Connection listener - Connected");
                state_ = State::kServerOk;
                event_queue_.push(CONNECTED);
            }
            else
            {
                LOG_INFO("Connection listener - Still disconnected, trying again in 30 seconds");
            }
        }
    }
    else
    {
        HandleNetworkDown();
    }
}

void ConnectionListener::ServerOkStateFunc()
{
    if (WifiProv_IsConnectedToWifi())
    {
        constexpr uint32_t kDisconnectionCheckIntervalInSeconds = 60;
        if (IsTimeElapsed(kDisconnectionCheckIntervalInSeconds))
        {
            if (!Ping())
            {
                LOG_INFO("Connection listener - Disconnected");
                state_ = State::kServerNok;
                event_queue_.push(DISCONNECTED);
            }
            else
            {
                event_queue_.push(CONNECTED);
                LOG_INFO("Connection listener - Still connected");
            }
        }
    }
    else
    {
        HandleNetworkDown();
    }
}

bool ConnectionListener::Ping()
{
    LOG_INFO("Connection listener - Pinging server...");
    auto is_server_reachable = false;
    const auto ping_result = ServerCom_Ping();
    if (ping_result.has_value())
    {
        is_server_reachable = ping_result.value();
        if (is_server_reachable)
        {
            LOG_INFO("Connection listener - Success pinging server.");
        }
        else
        {
            LOG_INFO("Connection listener - Failure pinging server.");
        }
    }
    return is_server_reachable;
}

bool ConnectionListener::IsTimeElapsed(uint32_t seconds)
{
    auto is_counter_elapsed = false;
    const auto milliseconds = 1000 * seconds;
    const auto end_count = milliseconds / kTickPeriodMilliSeconds;
    cnt_++;
    if (cnt_ > end_count)
    {
        cnt_ = 0;
        is_counter_elapsed = true;
    }
    return is_counter_elapsed;
}

void ConnectionListener::HandleNetworkDown()
{
    LOG_INFO("Connection listener - Network down");
    WifiProv_Disconnect();  // Otherwise auto-reconnects and crashes...
    state_ = State::kWifiNok;
    event_queue_.push(NETWORK_DOWN);
}