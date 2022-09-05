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
    }

    void valveClose() {
      digitalWrite(valvePin, LOW);
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
  char result[255];
  int charCount = 0;
  while (client.available()) {
    // Read the next character from the client, and store it in the ith element of the result array.
    char thisChar = client.read();
    result[charCount] = thisChar;
    charCount += 1;
  }

  Serial.println(result);  // TODO: delete this debug line

  // Initialize empty Json document, and verify that deserialization process worked.
  StaticJsonDocument<270> json_doc;
  DeserializationError err = deserializeJson(json_doc, result);

  if (err) {
    //    Serial.println("Json parsing failed");
    //    Serial.println(err.f_str());
    return;
  }


  // Now, assemble the Json information into the data packet structure
  // Metadata (units, time, IDs)
  const char* task_name = json_doc[0]["name"];
  long num_tasks = json_doc[0]["task_count"];
  const char* units = json_doc[0]["units"];

  if (num_tasks == 0) {
    //    Serial.println("no tasks!");
    return;
  }

  // Loop through the second part of the json document, where the actual tasks are
  for  (int i = 0; i < num_tasks; i++) {
    const char* valve_tag = json_doc[1]["tags"][i];
    int pin = json_doc[1]["pins"][i];
    long water_volume = json_doc[1]["volumes"][i];

    // Use the valve tag and volume to build new tasks...
    valveTask *task = new valveTask();
    task->valveTag = valve_tag;
    task->pin = pin;
    task->volume = water_volume * 1000;  // Convert seconds to milliseconds

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
  StaticJsonDocument<511> json_doc;  // TODO: optimize size, or use dynamic?
  json_doc["timestamp"] = data.timestamp;

  // Add nested arrays to hold sensor readings, valve positions and metadata
  JsonArray entry = json_doc.createNestedArray("entry");
  int numAreas = 3;  // TODO: update to non hard-coded
  int numSensors = 2;  // TODO: update to non hard-coded
  for (int i = 0; i < numAreas; i++) {
    JsonObject location = entry.createNestedObject();
    location["area"] = data.areas[i].area;
    JsonArray sensorData = location.createNestedArray("data");
    for (int j = 0; j < numSensors; j++) {
      sensorData.add(data.areas[i].sensors[j].type);
      sensorData.add(data.areas[i].sensors[j].value);
      }
    }
    // Final Format:
    // {"timestamp": int;
    //    "entry" {
    //      location: string;
    //      sensorData{
    //        str: sensorType
    //        value: int
    //          }
    //        }
    //     "entry" ....etc
    //    }

  serializeJson(json_doc, client);  // Serialize the data and send to client
}


void performWateringTasks(LinkedList<valveTask*> *listOfTasks) {
  // Given a list of tasks, this function loops through the list, and performs each task.
  // The tasks are removed from the queue after completion

  int numTasks = listOfTasks->size();
  if (numTasks == 0) {
    return;
  }

  // Initialize a task that is the first one in the queue.
  valveTask *task;
  task = listOfTasks->get(0);
  wateringValve row(task->pin);

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
  // Get individual sensor readings, and put them into a dataPacket
  // Row 1
  struct sensorReading moisture1 = {"moisture", 13};
  struct sensorReading temp1 = {"temp", 35};
  struct valvePosition valve1 = {"valve", true};

  // Area 2
  struct sensorReading moisture2 = {"moisture", 25};
  struct sensorReading temp2 = {"temp", 32};
  struct valvePosition valve2 = {"valve", true};

  // Area 3
  struct sensorReading moisture3 = {"moisture", 48};
  struct sensorReading temp3 = {"temp", 41};
  struct valvePosition valve3 = {"valve", true};

  // Water volume
  // struct sensorReading waterVolume = {"water_volume", 999};

  // Compile all sensor values into a packet, and return
  struct subPacket packet1 = {"back", moisture1, temp1};
  struct subPacket packet2 = {"front", moisture2, temp2};
  struct subPacket packet3 = {"greenhouse", moisture3, temp3};
  
  struct dataPacket totalPacket = {time_stamp, packet1, packet2, packet3};
  return totalPacket;
}
