#include <ESP8266WiFiMulti.h>

void setupAP() {
    WiFi.softAP("InfiniteMatrix", "12345678");
    Serial.print("Starting AP, IP: ");
    Serial.println(WiFi.softAPIP());
}


void setupSTA() {
    WiFi.mode(WIFI_STA);

    ESP8266WiFiMulti wifiMulti;
    wifiMulti.addAP("Example SSID 1", "Example password");
    wifiMulti.addAP("Example SSID 2", "Example password");
    if (wifiMulti.run() != WL_CONNECTED) {
        Serial.println("failed to connect");
        setupAP();
        return;
    }

    Serial.print("Connected, IP: ");
    Serial.println(WiFi.localIP());
}
