# Python program to implement client side of chat room.

# Library imports
import json
import socket
import time
import sqlite3

def receive_data(socket_object):
    """
    :param socket_object:
    :return: buffer: string of data received from server
    """
    buffer = b''  # binary notation at start of string

    # Attempt to receive data from the socket object (server connection)
    try:
        while True:
            data = socket_object.recv(56)  # TODO: optimize memory allocation
            if not data:
                break
            buffer += data

    # If unsuccessful, print exception
    except Exception as loopException:
        print("Exception occurred in loop, exiting...", loopException)

    # In any case, close the connection and return the buffered characters
    finally:
        socket_object.close()
    return buffer


def connect_to_server(address):
    """
    :param address: IP address and port
    :return: None
    """
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:

        try:
            # Try connecting to the server address....
            s.connect(address)

            # If connection successful, get watering tasks and make into JSON stirng...
            watering_tasks = get_watering_tasks()
            json_string = json.dumps(watering_tasks, separators=(',', ':'))
            print(json_string)
            json_bytes = json_string.encode('utf-8')
            s.sendall(json_bytes)  # ...and send the entire string (watering instructions) to the server

            # Now, receive the sensor data from the server
            data = receive_data(s)
            data_decoded = json.loads(data)

        except Exception as connectException:
            # If unsuccessful (usually in connecting to server), write data returned to NONE and exit functoin
            data_decoded = None
            print("Exception occured while connecting to server...", connectException)

    return data_decoded


def get_watering_tasks():
    """
    :return: tasks: a structure containing all the tasks with tags and volumes requested
    """

    # tasks = []

    tasks = [{
        # First part of structure with Metadata (name, ID, units, etc.) to be sent as first part of json structure
        "name": "watering_tasks",
        "task_count": 3,
        "units": "litres"
    }, {
        # Second part of structure with rows, valve control pins, and volumes requested.
        "tags": ["row_1", "row_2", "row_3"],
        "pins": [6, 7, 8],
        "volumes": [3000, 6000, 9000]
    }]
    return tasks


def save_to_database(database_name, json_decoded):
    """
    :param database_name: string: filename of the database file (SQLite database)
    :param json_decoded: dictionary: decoded JSON object, containing dictionary with sensor and metadata
    :return: None

    Given a database file and a dictionary with sensor data, this function connects to the database, and
    inserts the data into the specified table.
    """
    print(json_decoded)
    conn = sqlite3.connect(database_name)

    timestamp = json_decoded['timestamp']
    # Loop through all the sensors, and commit insertions for the database table
    for idx, sensor in enumerate(json_decoded['sensors']):
        tag = sensor
        value = json_decoded['readings'][idx]
        string_exe = "INSERT INTO tbl_analog VALUES ({}, '{}', {})".format(timestamp, tag, value)
        conn.execute(string_exe)
        pass

    # Commit insertions and close the connection to the database
    conn.commit()
    conn.close()
    return


# Find Connect to IP and exchange information
ip = "192.168.0.17"
port = 7777
address = (ip, port)
database = 'garden_data.db'
json_sensors = connect_to_server(address)

if json_sensors:
    save_to_database(database, json_sensors)
else:
    pass
