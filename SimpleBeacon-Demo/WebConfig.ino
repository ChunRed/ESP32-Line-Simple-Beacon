#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>

extern Preferences preferences;
extern String hwid_str;
extern int msg_val;

WebServer server(80);

void handleRoot() {
  String html = "<html><head><meta charset='utf-8'><meta name='viewport' "
                "content='width=device-width, initial-scale=1.0'></head><body "
                "style='font-family: Arial, sans-serif; margin: "
                "20px;'><h2>LINE Beacon 參數設定</h2>";
  html += "<form action='/save' method='POST'>";
  html += "HWID (10字元16進位): <br><input type='text' name='hwid' value='" + hwid_str + "' maxlength='10' style='padding: 5px; font-size: 16px; margin-top: "
                                                                                         "5px;'><br><br>";
  html += "MSG (數字): <br><input type='number' name='msg' value='" + String(msg_val) + "' style='padding: 5px; font-size: 16px; margin-top: 5px;'><br><br>";
  html += "<input type='submit' value='儲存並重啟' style='padding: 10px 20px; "
          "font-size: 16px; margin-top: 10px; cursor: pointer;'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  if (server.hasArg("hwid")) {
    preferences.putString("hwid", server.arg("hwid"));
  }
  if (server.hasArg("msg")) {
    preferences.putInt("msg", server.arg("msg").toInt());
  }
  server.send(200, "text/html",
              "<html><head><meta charset='utf-8'><meta name='viewport' "
              "content='width=device-width, initial-scale=1.0'></head><body "
              "style='font-family: Arial, sans-serif; margin: "
              "20px;'><h2>設定已儲存！</h2><p>設備正在重新啟動，請稍候...</p></"
              "body></html>");
  delay(1000);
  ESP.restart();
}

void setupWebServer() {
  WiFi.softAP("Beacon_Config");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();
}

void handleWebServer() {
  server.handleClient();
}
