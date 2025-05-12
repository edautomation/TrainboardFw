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

#include "WifiProvisioning.h"

// Project
#include "FwConfig.h"
#include "Logging.h"

// Libraries
#include "WiFiManager.h"

static WiFiManager _wifi_manager;

std::optional<bool> WifiProv_Connect(uint32_t timeout_s)
{
    static bool was_connected = false;
    _wifi_manager.setTitle("Trainboard Configuration");
    _wifi_manager.setTimeout(timeout_s);
    if (was_connected && !WifiProv_IsConnectedToWifi())
    {
        _wifi_manager.disconnect();
    }

    constexpr uint32_t kSuffixLen = 6;
    constexpr uint32_t kMacAddrLen = 6;
    char combined_ap_name[strlen(kAccessPointName) + kSuffixLen];
    uint8_t mac[kMacAddrLen];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    const auto n_written_bytes =
        snprintf(combined_ap_name, sizeof(combined_ap_name), "%s_%02X%02X", kAccessPointName, mac[4], mac[5]);
    if (n_written_bytes != sizeof(combined_ap_name) - 1)
    {
        LOG_DEBUG("Error creating AP name");
    }

    was_connected = _wifi_manager.autoConnect(combined_ap_name);
    return was_connected;
}
bool WifiProv_IsConnectedToWifi()
{
    return (WiFi.status() == WL_CONNECTED);
}
void WifiProv_ResetCredentials()
{
    _wifi_manager.resetSettings();
}
void WifiProv_Disconnect()
{
    WiFi.disconnect(true);
}
bool WifiProv_HasCredentials()
{
    return _wifi_manager.getWiFiIsSaved();
}