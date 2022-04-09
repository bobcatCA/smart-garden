// WiFiNina_comm_functions.cpp

// Main communications-related functions for the garden server.

// External librairies and headers to import
#include <ArduinoJson.h>
#include <LinkedList.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include "WiFiNINA_comm_functions.h"



// Classes
class wateringValve {
    // Watering valves that can stay open for a specified time (or volume)
    int valvePin;

  public:
    wateringValve(int pin) {
      valvePin = pin;
      pinMode(valvePin, OUTPUT);
    };

    // Open or close
    void valveOpen() {
      digitalWrite(valvePin, HIGH);
      Serial.println("valve open");
    }

    void valveClose() {
      digitalWrite(valvePin, LOW);
      Serial.println("valve closed");
    }
};


void dataserverReadClient(WiFiClient client, LinkedList<valveTask*> *listOfTasks) {

  // Given a WifiNina client object, it reads a packet from the client and deserializes
  // them from Json format. It then builds a dataPacket structure, which is eventually passed
  // to the watering function, which parses and performs the requested tasks.

  // WiFiClient client: WifiNina WiFiClient object, a client that has sent a request and is connected to the server
  // Linked List listOfTasks: Pointer to a linked list. The list nodes are of type valveTask, which contain information
  // about which valves are requested to open and for how long.

  // Initiate empty char, and store all incoming characters in it.
  char result[127];
  int charCount = 0;
  while (client.available()) {
    // Read the next character from the client, and store it in the ith element of the result array.
    char thisChar = client.read();
    result[charCount] = thisChar;
    charCount += 1;
  }

  Serial.println(result);  // TODO: delete this debug line

  // Initialize empty Json document, and verify that deserialization process worked.
  StaticJsonDocument<1028> json_doc;
  DeserializationError err = deserializeJson(json_doc, result);

  if (err) {
    Serial.println("Json parsing failed");
    return;
  }


  // Now, assemble the Json information into the data packet structure
  // Metadata (units, time, IDs)
  const char* task_name = json_doc[0]["name"];
  long num_tasks = json_doc[0]["task_count"];
  const char* units = json_doc[0]["units"];

  // Loop through the second part of the json document, where the actual tasks are
  for  (int i = 0; i < num_tasks; i++) {
    const char* valve_tag = json_doc[1]["tags"][i];
    //    int pin = json_doc[1]["pins"][i];
    long water_volume = json_doc[1]["volumes"][i];

    // Use the valve tag and volume to build new tasks...
    valveTask *task = new valveTask();
    task->valveTag = valve_tag;
    //    task->pin = pin;
    task->volume = water_volume;

    // ...and add it to the queue
    listOfTasks->add(task);
  }
}


void sendJson(dataPacket data, WiFiClient client)  {

  // Given a specific data array and a wifinina client object,
  // the function will reformat the data into Json format, serialize it, and send it via the client object.

  // dataPacket data: array of numbers and characters, designed to be carrying information about the sensors and their current values
  // WiFiClient client: WifiNina WiFiClient object. basically a client that has sent a request and is connected to the server

  // Initialize Json document of size 255, assign metadata from dataPacket
  StaticJsonDocument<255> json_doc;  // TODO: optimize size, or use dynamic?
  json_doc["area"] = data.area;
  json_doc["timestamp"] = data.timestamp;

  // Add nested arrays to hold sensor readings and metadata
  JsonArray tags = json_doc.createNestedArray("tags");
  JsonArray values = json_doc.createNestedArray("values");

  // Store sensor tags and readings in the Json document
  int numSensors = 3;  // TODO: update to non hard-coded
  for (int i = 0; i < numSensors; i++) {
    values.add(data.data[i].value);
    tags.add(data.data[i].tag);
  }
  serializeJson(json_doc, client);  // Serialize the data and send to client
  serializeJson(json_doc, Serial);  // TODO: delete debugging line
}


void performWateringTasks(LinkedList<valveTask*> *listOfTasks) {
  // Given a list of tasks, this function loops through the list, and performs each task.
  // The tasks are removed from the queue after completion

  int numTasks = listOfTasks->size();
  if (numTasks == 0) {
    Serial.println("no tasks");
    return;
  }

  // Initialize a task that is the first one in the queue.
  valveTask *task;
  task = listOfTasks->get(0);
  wateringValve row(6);  // TODO: pass the pin # in from json (not working yet)
  //  wateringValve row(task->pin);

  // Go into if statement only if the task has not yet been started
  if (task->newTask == true) {
    task->taskStart = millis();  // Create a starting reference, will compare to this initial value to water correct volume.
    task->newTask = false;  // Set to "in-progress", so will only enter this condition when the task once at start.

    // Make a new valve, corresponding to the task, and open it.
    row.valveOpen();  // Open the valve
  }

  // If the water volume has exceeded the request, close the valve
  if ((millis() - task->taskStart) > task->volume) {
    row.valveClose();  // close valve
    listOfTasks->shift();  // delete task
  }
}


dataPacket getSensorReadings(int time_stamp) {
  // Compiles the current moisture readings into a data structure, and returns that data structure

  // int time_stamp: time stamp to mark when the data was recorded.

  // Get individual sensor readings, and put them into a dataPacket
  struct sensorReading reading1 = {"Row_1", 13};
  struct sensorReading reading2 = {"Row_2", 19};
  struct sensorReading reading3 = {"Row_2", 19};
  struct dataPacket packet = {"greenhouse", time_stamp, reading1, reading2, reading3};  // Compile data into larger struct
  return packet;
}
