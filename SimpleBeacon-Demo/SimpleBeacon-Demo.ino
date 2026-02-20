

// #define HWID "018df4be74" //LEAVE
// #define MSG 1

// #define HWID "018e39bdd8" //赤榕
// #define MSG 2

// #define HWID "018e3a1ff0" // 金草蘭
// #define MSG 3

// #define HWID "018e52b1fe" //穀穗
// #define MSG 4

// #define HWID "018e5457a3" //構樹
// #define MSG 5

// #define HWID "018e58b93e" //月桃
// #define MSG 6

// #define HWID "018e5c7d23" //青剛櫟
// #define MSG 7

// #define HWID "018e618f8f" //小葉桑
// #define MSG 8

// #define HWID "018e62d8ea" //五節芒
// #define MSG 9

#include "Arduino.h"
#include "BLEAdvertising.h"
#include "BLEBeacon.h"
#include "BLEDevice.h"
#include <Preferences.h>

Preferences preferences;
String hwid_str;
int msg_val;
bool configMode = false;

// 網頁相關函式 (實作於 WebConfig.ino)
void setupWebServer();
void handleWebServer();

/**
 * Bluetooth TX power level(index), it's just a index corresponding to
 * power(dbm).
 * * ESP_PWR_LVL_N12 (-12 dbm)
 * * ESP_PWR_LVL_N9  (-9 dbm)
 * * ESP_PWR_LVL_N6  (-6 dbm)
 * * ESP_PWR_LVL_N3  (-3 dbm)
 * * ESP_PWR_LVL_N0  ( 0 dbm)
 * * ESP_PWR_LVL_P3  (+3 dbm)
 * * ESP_PWR_LVL_P6  (+6 dbm)
 * * ESP_PWR_LVL_P9  (+9 dbm)
 */
#define POWER_LEVEL ESP_PWR_LVL_N12

bool isAdDataLine = true;
BLEAdvertising *pAdvertising;
BLEAdvertisementData adDataLine, adDataIbeacon,
  adDataEmpty = BLEAdvertisementData();

byte htoi(byte c) {
  if ('0' <= c && c <= '9')
    return c - '0';
  if ('A' <= c && c <= 'F')
    return c - 'A' + 10;
  if ('a' <= c && c <= 'f')
    return c - 'a' + 10;
  return 0;
}

void hexDecode(byte *out, const char *in, size_t inlen) {
  for (size_t i = 0; i < inlen; i++) {
    out[i] = (htoi(in[i << 1]) << 4) + htoi(in[(i << 1) + 1]);
  }
}

void hexEncode(char *out, const byte *in, size_t inlen) {
  const char *hexMap = "0123456789abcdef";
  for (size_t i = 0; i < inlen; i++) {
    out[i << 1] = hexMap[(in[i] >> 4) & 0x0F];
    out[(i << 1) + 1] = hexMap[in[i] & 0x0F];
  }
  out[inlen << 1] = '\0';
}

void debugByte(const byte *in, size_t inlen) {
  for (size_t i = 0; i < inlen; i++)
    Serial.printf(" %02x", in[i]);
}

BLEAdvertisementData genAdDataLine(const byte *msg, size_t msglen) {
  BLEAdvertisementData adData;
  BLEUUID line_uuid("FE6F");

  // flag
  // LE General Discoverable Mode (2)
  // BR/EDR Not Supported (4)
  adData.setFlags(0x06);

  // LINE Corp UUID
  adData.setCompleteServices(line_uuid);

  // Service Data
  byte payload[14];
  size_t pos = 0;

  payload[pos++] = byte(0x02);  // Frame Type of the LINE Simple Beacon Frame

  hexDecode(&payload[pos], hwid_str.c_str(), 5);  // HWID of LINE Beacon
  pos += 5;

  payload[pos++] =
    byte(0x7F);  // Measured TxPower of the LINE Simple Beacon Frame

  if (msglen > 13)
    msglen = 13;
  memcpy(&payload[pos], msg,
         msglen);  // Device message of LINE Simple Beacon Frame
  pos += msglen;

  std::string strServiceData((const char *)payload, pos);
  adData.setServiceData(line_uuid, strServiceData);

  Serial.print("genAdDataLine =");
  debugByte(payload, pos);
  Serial.println("");

  return adData;
}

BLEAdvertisementData genAdDataIbeacon() {
  byte uuid[16];
  hexDecode(uuid, "d0d2ce249efc11e582c41c6a7a17ef38",
            16);  // iBeacon UUID of LINE

  BLEBeacon beacon = BLEBeacon();
  beacon.setManufacturerId(
    0x4C00);  // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  beacon.setProximityUUID(BLEUUID(uuid, 16, false));
  beacon.setMajor(0x4C49);
  beacon.setMinor(0x4e45);
  BLEAdvertisementData adData = BLEAdvertisementData();

  adData.setFlags(0x04);  // BR_EDR_NOT_SUPPORTED 0x04

  std::string strServiceData = "";

  strServiceData += (char)26;    // Len
  strServiceData += (char)0xFF;  // Type
  strServiceData += beacon.getData();
  adData.addData(strServiceData);
  return adData;
}

void setup() {
  // init
  Serial.begin(115200);

  preferences.begin("beacon", false);
  hwid_str = preferences.getString("hwid", "018e3a1ff0");  // Default: 金草蘭
  msg_val = preferences.getInt("msg", 3);                  // Default: 3

  pinMode(0, INPUT_PULLUP);

  // initial BLE

  BLEDevice::init("B1a");
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, POWER_LEVEL);
  pAdvertising = BLEDevice::getAdvertising();

  // print device info
  Serial.printf("%s, Power Level = %c, HWID = %s\n",
                BLEDevice::toString().c_str(), char(POWER_LEVEL + '0'),
                hwid_str.c_str());

  // set adDataIbeacon and adDataLine
  adDataIbeacon = genAdDataIbeacon();
  // adDataLine = genAdDataLine((const byte
  // *)BLEDevice::getAddress().getNative(), 6);
  byte myNumber = msg_val;
  adDataLine = genAdDataLine(&myNumber, 1);

  BLEAdvertisementData scanResponseData;
  scanResponseData.setName("B1a");

  // broadcast adDataLine first
  isAdDataLine = true;
  pAdvertising->setAdvertisementData(adDataLine);
  pAdvertising->setScanResponseData(scanResponseData);
  pAdvertising->start();
  delay(1000);
}

void loop() {
  if (!configMode) {
    if (digitalRead(0) == LOW) {
      delay(50);  // debounce
      if (digitalRead(0) == LOW) {
        configMode = true;
        Serial.println("Enter Config Mode. Starting AP...");

        pAdvertising->stop();

        // 啟動網頁伺服器
        setupWebServer();
        return;
      }
    }
  }

  if (configMode) {
    handleWebServer();
  } else {
    isAdDataLine = !isAdDataLine;
    pAdvertising->setAdvertisementData(isAdDataLine ? adDataLine
                                                    : adDataIbeacon);
    pAdvertising->start();
    delay(1000);
  }
}