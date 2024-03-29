#ifndef WIFININA_COMM_FUNCTIONS_H
#define WIFININA_COMM_FUNCTIONS_H

// Declaration of classes
class valveTask {
  public:
    const char *valveTag;  // TODO: has to be const?
    int pin;
    int volume;
    boolean newTask = true;
    int taskStart;
};

// Declaration of structures. First for the sensor reading.
// The first struct has the sensor name and its measured value
struct sensorReading {
  const char *type;
  int value;
};


struct valvePosition {
  const char *tag;
  bool valvePosition;
};


// The second struct has metadata (time of measurement, and physical location). It also has the sensorReadingg struct as a nested array.
struct subPacket {
  char area[12];
  sensorReading sensors[2];  // number of sensors to include
  valvePosition valve;
};


struct dataPacket {
  int timestamp;
  subPacket areas[3];  // Number of different areas
  };


// Declaration of functions (see WiFiNINA_comm_functions.cpp)
void dataserverReadClient(WiFiClient client, LinkedList<valveTask*> *listOfTasks);
void sendJson(dataPacket data, WiFiClient client);
void performWateringTasks(LinkedList<valveTask*> *listOfTasks);
dataPacket getSensorReadings(int time_stamp);

#endif
