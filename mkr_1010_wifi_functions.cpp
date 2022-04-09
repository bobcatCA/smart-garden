// mkr_1010_wifi_functions.cpp
// Generic functions library for accessing and operating with WiFiNINA

// External librairies and headers
#include <WiFiNINA.h>
#include "mkr_1010_wifi_functions.h"
#include "arduino_secrets.h"

// Network logon details
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

void checkWiFiConfig() {
  // Checks for WiFi connectivity, flags if firmware is out of date.

  // Check for no communication, don't proceed until connected.
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Unable to communicate with WiFi module");
    while (true);
  }

  // Check WiFi firmware version, warn if out of date
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }
}


int connectWiFiNetwork(int status) {
  // Connect to WiFi network, given the secred SSID and password

  // int status: status of wifi network (status is WifiNina reserved name)
  
  // Return status: WiFiNina object with status of WiFi connectivity (not connected, connected, attempting to connect, etc.)
  while (status != WL_CONNECTED) {
    // If not already connected, attempt to connect to network.
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    delay(10000);  // Wait 10 seconds while WiFi connects
  }
  return status;  // Status should be Connected now.
}


void printWifiStatus() {
  // Print details of the current WiFi network connection:
  // Network Name, Arduino IP address, Signal Strength

  // print the WiFi network SSID:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the Arduino board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
