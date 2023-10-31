

#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Arduino.h>

#define ALARM_Pin 12

const char* ssid     = "YOUR_WIFI_NAME";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "YOUR_WIFI_PASSWORD";     // The password of the Wi-Fi network

unsigned long checkAlarmDelay = 1000;
unsigned long lastCheckedAlarmTime = 0;

HTTPClient http;

String serverPath = "https://api.telegram.org/<YOUR_BOT_ID>/sendMessage";

// Root certificate
const char IRG_Root_X1 [] PROGMEM = R"CERT(
YOUR_ROOT_CERTIFICATE
)CERT";

X509List cert(IRG_Root_X1);

void setup() {
  pinMode(ALARM_Pin,INPUT);
  Serial.begin(9600);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
  
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); 
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer


  // Configure the time on the ESP8266, which is necessary to validate the certificate.
  // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void loop() { 
  WiFiClientSecure client;
  client.setTrustAnchors(&cert);

  if ((millis() - lastCheckedAlarmTime) > checkAlarmDelay) {
    int alarmState = digitalRead(ALARM_Pin);
    if (alarmState == HIGH) {
      if (WiFi.status() == WL_CONNECTED) {       

        // Your Domain name with URL path or IP address with path
        http.begin(client, serverPath.c_str());

        http.addHeader("content-type", "application/x-www-form-urlencoded");
        http.addHeader("accept", "*/*");

        int httpResponseCode = http.POST("chat_id=<YOUR_CHAT_ID>&text=Movement is detected!");
        
        if (httpResponseCode > 0) {
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
          String payload = http.getString();
      
          Serial.println(payload);
        } else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }
        // Free resources
        http.end();
      }
    }
    lastCheckedAlarmTime = millis();
  }
}