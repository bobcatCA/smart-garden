# Python program to implement client side of chat room.

# Library imports
import socket
import json


def receive_data(socket_object):
    """
    :param socket_object:
    :return: buffer: string of data received from server
    """
    buffer = b''  # binary
    try:
        while True:
            data = socket_object.recv(56)  # TODO: optimize memory allocation
            if not data:
                break
            buffer += data
    except Exception as loopException:
        print("Exception occurred in loop, exiting...", loopException)
    finally:
        socket_object.close()  # Close connection
    return buffer


def connect_to_server(address):
    """
    :param address: IP address and port
    :return: None
    """
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect(address)
        
        watering_tasks = get_watering_tasks()
        if watering_tasks:
            json_string = json.dumps(watering_tasks, separators=(',', ':'))
            print(json_string)
            json_bytes = json_string.encode('utf-8')
            s.sendall(json_bytes)
            
        else:
            s.sendall(b'data_query')
            pass
        
        data = receive_data(s)
        print(f"Received {data!r}")
    return

def get_watering_tasks():
    """
    :return: tasks: list of tasks with tags and volumes requested
    """
    tasks = []

    # Metadata (name, ID, units, etc.) to be sent as first part of json structure
    task_preamble = {'name': 'watering_tasks',
                     'task_count': 3,
                     'units': 'litres'
                     }

    # Compile list of tasks (Beta)
    # TODO: Smarten the compile process, link to
    tasks_values = {}
    tasks_values['tags'] = ['row_1', 'row_2', 'row_3']
    # tasks_values['pins'] = [6, 7, 8]  # TODO: pins passing in not working
    tasks_values['volumes'] = [2000, 4000, 6000]
    tasks.append(task_preamble)
    tasks.append(tasks_values)
    return tasks

# Find Connect to IP and exchange information
ip = "192.168.0.13"
port = 7777
address = (ip, port)
connect_to_server(address)

    