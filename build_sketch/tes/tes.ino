#include <WiFi.h>
#include "esp_wifi.h"

// Pin LED bawaan board DOIT ESP32 DevKit V1 (GPIO 2)
#define LED_PIN 2

// Daftar 5 Nama Wi-Fi (SSID) untuk tes hasil compile & flash
const int TOTAL_WIFI = 5;
const char* daftar_ssid[TOTAL_WIFI] = {
  "01_ESP32_Test_WiFi_A",
  "02_ESP32_Test_WiFi_B",
  "03_ESP32_Test_WiFi_C",
  "04_ESP32_Test_WiFi_D",
  "05_ESP32_Test_WiFi_E"
};

// Template paket IEEE 802.11 Beacon Frame standar agar ke-5 SSID tampil bersamaan di daftar Wi-Fi HP
uint8_t beaconPacket[128] = {
  0x80, 0x00,                         // Frame Control: Beacon (0x80)
  0x00, 0x00,                         // Duration
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination: Broadcast
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source MAC (akan dibedakan tiap SSID)
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // BSSID MAC (akan dibedakan tiap SSID)
  0x00, 0x00,                         // Sequence number
  0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
  0x64, 0x00,                         // Beacon Interval (100 TU = ~102.4 ms)
  0x01, 0x04,                         // Capability Info (ESS, Open Network / Tanpa Password)
  0x00                                // Tag 0: SSID Parameter Set (diisi dinamis di bawah)
};

unsigned long lastBlink = 0;
bool ledState = false;

void kirimBeaconSSID(int index) {
  const char* ssid = daftar_ssid[index];
  int ssidLen = strlen(ssid);
  if (ssidLen > 32) ssidLen = 32;

  // Bedakan byte terakhir MAC Address untuk masing-masing dari 5 SSID
  uint8_t customMac[6] = { 0x36, 0x33, 0x33, 0x33, 0x33, (uint8_t)(0x10 + index) };
  memcpy(&beaconPacket[10], customMac, 6);
  memcpy(&beaconPacket[16], customMac, 6);

  // Set panjang SSID dan salin nama SSID ke dalam frame
  beaconPacket[37] = ssidLen;
  memcpy(&beaconPacket[38], ssid, ssidLen);

  int pos = 38 + ssidLen;

  // Tag 1: Supported Rates (1, 2, 5.5, 11, 6, 9, 12, 18 Mbps)
  uint8_t ratesAndChannel[] = {
    0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24,
    0x03, 0x01, 0x01 // Tag 3: DS Parameter Set (Channel 1)
  };

  memcpy(&beaconPacket[pos], ratesAndChannel, sizeof(ratesAndChannel));
  int packetSize = pos + sizeof(ratesAndChannel);

  // Pancarkan paket Beacon melalui interface AP ESP32
  esp_wifi_80211_tx(WIFI_IF_AP, beaconPacket, packetSize, false);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.println("\n==========================================");
  Serial.println("Memulai Tes Deploy 5 Nama Wi-Fi ESP32...");
  Serial.println("==========================================");

  // Inisialisasi mode Access Point pada Channel 1
  WiFi.mode(WIFI_AP);
  WiFi.softAP(daftar_ssid[0], NULL, 1, 0, 4);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  for (int i = 0; i < TOTAL_WIFI; i++) {
    Serial.printf("[%d] Mengaktifkan SSID: %s\n", i + 1, daftar_ssid[i]);
  }
  Serial.println("Silakan cek daftar Wi-Fi di HP kamu!");
}

void loop() {
  // Pancarkan ke-5 SSID secara simultan setiap ~20ms per siklus
  for (int i = 0; i < TOTAL_WIFI; i++) {
    kirimBeaconSSID(i);
    delay(4);
  }

  // Kedipkan LED biru (GPIO 2) setiap 500ms sebagai tanda ESP32 berjalan normal
  if (millis() - lastBlink >= 500) {
    lastBlink = millis();
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  }
}

// Compiled via Termux at 2026-11-07T07:22:26Z
