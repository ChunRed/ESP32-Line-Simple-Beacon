# ESP32 LINE Simple Beacon

這是一個基於 ESP32 的 LINE Simple Beacon 測試專案，支援動態變更 Beacon 的 `HWID` 和 `MSG` 參數，不須每次修改參數都重新編譯上傳程式碼。

## 功能特色
- 發送 LINE Simple Beacon 和 iBeacon 訊號。
- 透過硬體按鈕 (預設為 IO0 / BOOT 鍵) 進入「設定模式」。
- 設定模式會建立一個 Wi-Fi 熱點 (Access Point)，並啟動網頁伺服器，可直接透過瀏覽器修改 `HWID` 與 `MSG`。
- 設定的數值會儲存在 ESP32 的 Flash 記憶體中 (Preferences)，斷電重開機後亦不消失。

## 首次編譯與上傳注意事項

由於同時使用了藍芽 (BLE) 與 Wi-Fi 網頁伺服器 (WebServer) 的函式庫，程式容量會超過 ESP32 預設的 1.2MB 限制。
在上傳程式碼之前，請務必於 Arduino IDE 進行以下設定：

1. 開啟 **工具 (Tools)** 選單。
2. 尋找 **Partition Scheme** (或 分區表)。
3. 將其更改為: **Huge APP (3MB No OTA/1MB SPIFFS)** _(或任何容量超過 1.5MB 給 APP 使用的選項)_。
4. 選擇你的 ESP32 開發板與連接埠，然後點擊「上傳」。

## 操作說明

### 1. 正常發射模式
ESP32 上電後，會自動從記憶體讀取上一次儲存的 `HWID` 與 `MSG`，並開始交替發射 LINE Simple Beacon 及 iBeacon 藍芽訊號。
*如果從未設定過，預設值為: HWID = `"018e3a1ff0"` (金草蘭)，MSG = `3`。*

### 2. 進入「設定模式」
如果需要更改 `HWID` 或 `MSG`：
1. ESP32 運作時，找到開發板上的 **BOOT 按鈕 (IO0)**。
2. 按下該按鈕一下 (不用長按，點擊即可切換模式)。
3. 此時序列埠 (Baud rate 115200) 會顯示 `Enter Config Mode. Starting AP...`。ESP32 將會停止藍芽廣播，改為建立一個 Wi-Fi 熱點。

### 3. 使用網頁變更參數
1. 進入設定模式後，使用你的手機或電腦，打開 Wi-Fi 設定。
2. 尋找並連線至名稱為 **`Beacon_Config`** 的 Wi-Fi 網路 (此為開放網路，無需密碼)。
3. 開啟網頁瀏覽器，在網址列輸入進入: **`http://192.168.4.1`**。
4. 網頁將顯示「LINE Beacon 參數設定」的畫面：
   - **HWID**: 輸入專屬於該 Beacon 的 10 字元 16 進位字串 (例如 `018e3a1ff0`)。
   - **MSG**: 輸入代表該 Beacon 的編號 / 數字。
5. 按下 **「儲存並重啟」** 鍵。
6. 看見網頁顯示「設定已儲存！」，此時 ESP32 會自動重新開機，套用剛剛的新數值並變回「正常發射模式」。

## 進階設定
如果您想要幫 `Beacon_Config` 熱點加上連線密碼，可以開啟 `SimpleBeacon-Demo.ino`，找到這行程式碼：
```cpp
WiFi.softAP("Beacon_Config");
```
並將其修改為：
```cpp
WiFi.softAP("Beacon_Config", "你的密碼"); // 密碼需至少 8 個字元
```
修改後重新上傳即可。
