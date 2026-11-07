#include <WiFi.h>
#include "esp_wifi.h"

// Pin LED bawaan pada board ESP32 DevKit V1
#define LED_PIN 2

// Channel WiFi yang digunakan (1 - 11)
const uint8_t WIFI_CHANNEL = 1;

// Daftar 5 nama WiFi (SSID) yang ingin dideploy (maksimal 32 karakter per nama)
const char* ssids[5] = {
  "ESP32_WiFi_01",
  "ESP32_WiFi_02",
  "ESP32_WiFi_03",
  "ESP32_WiFi_04",
  "ESP32_WiFi_05"
};

const int TOTAL_SSID = 5;

void sendBeacon(const char* ssid, uint8_t index) {
  uint8_t packet[128] = {
    // 0-1: Frame Control (0x80 = Beacon Frame)
    0x80, 0x00,
    // 2-3: Duration
    0x00, 0x00,
    // 4-9: Destination Address (Broadcast)
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    // 10-15: Source Address (Locally Administered MAC)
    0x02, 0x1a, 0x2b, 0x3c, 0x4d, 0x00,
    // 16-21: BSSID (Disamakan dengan Source Address)
    0x02, 0x1a, 0x2b, 0x3c, 0x4d, 0x00,
    // 22-23: Sequence Number (Diatur otomatis oleh hardware)
    0x00, 0x00,
    // 24-31: Timestamp (8 byte)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // 32-33: Beacon Interval (100 TU = ~102.4 ms)
    0x64, 0x00,
    // 34-35: Capability Info (0x0401 = Open Network, ESS, Short Slot Time)
    0x01, 0x04
  };

  // Ubah byte terakhir MAC/BSSID agar HP mendeteksinya sebagai 5 titik akses berbeda
  packet[15] = index + 1;
  packet[21] = index + 1;

  // Batasi panjang SSID maksimal 32 karakter sesuai standar 802.11
  size_t ssidLen = strlen(ssid);
  if (ssidLen > 32) ssidLen = 32;

  // Tag 0: SSID Parameter Set
  packet[36] = 0x00;            // Element ID: SSID
  packet[37] = (uint8_t)ssidLen; // Panjang SSID
  memcpy(&packet[38], ssid, ssidLen);

  size_t pos = 38 + ssidLen;

  // Tag 1: Supported Rates (Wajib ada agar terbaca oleh pemindai WiFi HP/Laptop)
  const uint8_t supportedRates[] = {
    0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c
  };
  memcpy(&packet[pos], supportedRates, sizeof(supportedRates));
  pos += sizeof(supportedRates);

  // Tag 3: DS Parameter Set (Channel)
  packet[pos++] = 0x03;         // Element ID: DS Parameter
  packet[pos++] = 0x01;         // Panjang: 1 byte
  packet[pos++] = WIFI_CHANNEL; // Channel aktif

  // Pancarkan paket mentah melalui antarmuka Access Point
  esp_wifi_80211_tx(WIFI_IF_AP, packet, pos, true);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Inisialisasi mode Access Point dengan SSID bawaan disembunyikan (hidden = 1)
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32_Hidden_Init", "12345678", WIFI_CHANNEL, 1);
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  Serial.println("\n=== Tes Deploy 5 SSID Dimulai ===");
  for (int i = 0; i < TOTAL_SSID; i++) {
    Serial.printf("[%d] %s\n", i + 1, ssids[i]);
  }
}

void loop() {
  // Pancarkan ke-5 SSID secara berurutan
  for (int i = 0; i < TOTAL_SSID; i++) {
    sendBeacon(ssids[i], i);
    delay(20); // Total siklus 5 x 20ms = 100ms (sesuai interval beacon standar)
  }

  // Kedipkan LED biru bawaan setiap ~1 detik sebagai indikator aktif
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 1000) {
    lastBlink = millis();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

// Compiled via Termux at 2026-11-07T07:45:50Z
