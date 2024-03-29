/*
  Device: Arduino Wifi MKR 1010 Wifi
  File: dataserver
**********************************
  Description
  1) Connect to WiFi network and initiate Arduino as server
  2) Read data from sensors connected to Arduino
  3) Serialize sensors and information into JSON format
  4) Send JSON over WiFi
**********************************
*/

// External libraries and file imports
#include <ArduinoJson.h>
#include <LinkedList.h>
#include <NTPClient.h>
#include <SPI.h>
#include <WiFiNINA.h>

// Related header imports
#include "mkr_1010_wifi_functions.h"
#include "WiFiNINA_comm_functions.h"

// Global variables
boolean b_alreadyConnected = false; // Initialize to false in case the client was connected previously
int status = WL_IDLE_STATUS;  // Initialize status to idle. Will change after calling WiFi.begin()
WiFiServer server(7777);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");  // Shouldn't need offset, seems to return local time.
LinkedList<valveTask*> currentValveTasks = LinkedList<valveTask*>();  //Initialize current (empty) queue of watering tasks
LinkedList<valveTask*> *p_currentValveTasks = &currentValveTasks;  // Make a pointer for the task list

void setup() {
  // Initialize serial comm's
  Serial.begin(9600);
  while (!Serial) {
  }

  // Connect to WiFi and check version. Print out status once connected.
  checkWiFiConfig();
  status = connectWiFiNetwork(status);
  server.begin();
  printWifiStatus();

  timeClient.begin();  // Start the time client
}


void loop() {
  // If not connected to WiFi, continue trying to connect until available
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("lost WiFi connection... attempting to reconnect");
    connectWiFiNetwork(status);
  }

  WiFiClient WiFi_client = server.available();

  // If client is available, begin routine to exchange sensor/watering data
  if (WiFi_client) {
    dataserverReadClient(WiFi_client, p_currentValveTasks);
    Serial.println("Client Found!");

    // For first connection, clear buffer and send acknowledgement message
    if (!b_alreadyConnected) {
      WiFi_client.flush();
      b_alreadyConnected = true;
    }

    // 1) Send moisture data back to client. Client will compute which tasks are necessary based on this.
    // Assemble the Json object with sensor readings and timestamp
    timeClient.update();
    int timeNow = timeClient.getEpochTime();  // Current timestamp, seems to correct for local time OK
    dataPacket packet = getSensorReadings(timeNow);  // Returns structure containing moisture readings
    sendJson(packet, WiFi_client);  // Turn the data packet into Json format, and send to client. This is where the LL gets mixed up.
    WiFi_client.stop();  // Stop, so we can wait for the next request from client

    // 2) Receive tasks from client
    dataserverReadClient(WiFi_client, p_currentValveTasks);
  }
  performWateringTasks(p_currentValveTasks);  // We are passing pointer, so should empty the struct as all tasks will have finished
  delay(2000);
}
