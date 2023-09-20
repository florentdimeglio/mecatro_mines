import threading
from datetime import datetime
import time
from pathlib import Path 
from collections import OrderedDict

import struct
import serial
import serial.tools.list_ports
from .utils import *

CHUNK_SIZE = 10000

class TelemetryListener:
    def __init__(self, port_name="/dev/cu.usbserial-220"):
        '''
        Object listening to the serial port, and creating the logs
        '''
        self.port_name = port_name
        self.baudrate = 230400
        self.targer_folder = ""

        self.connected = False
        self.should_connect = False
        self.should_disconnect = False

        self.time = [0] * CHUNK_SIZE
        self.log_data = {}
        self.line_idx = -1
        self.status_message = ["Waiting"]
        self.should_terminate = False
        self.data_to_send = ""

    def set_port_name(self, port_name):
        self.port_name = port_name 
    
    def set_baudrate(self, baudrate):
        self.baudrate = baudrate 

    def set_folder(self, folder):
        self.targer_folder = folder 

    def attempt_connection(self):
        self.should_connect = True  

    def disconnect(self):
        self.should_disconnect = True   

    def terminate(self):
        self.should_disconnect = True   
        self.should_terminate = True   
    
    def is_connected(self):
        return self.connected

    def get_data(self):
        return self.time, self.log_data, self.line_idx 
        
    def get_status(self):
        status = self.status_message
        self.status_message = []
        return status
    
    def send(self, data):
        '''
        Send data to the Arduino.
        '''
        self.data_to_send += data
        
    def listener_thread(self):

        while not(self.should_terminate):
            self.time = []
            self.log_data = OrderedDict()
            self.line_idx = -1
            # Wait for connection order
            while not(self.should_connect) and not(self.should_terminate):
                time.sleep(0.050)
            if self.should_terminate:
                return
            self.should_connect = False
            self.should_disconnect = False
            # Try to connect
            start_time = None

            try:
                with serial.Serial(self.port_name, self.baudrate, timeout=0.1) as ser:
                    self.status_message.append("Connected, waiting for Arduino to boot")
                    self.connected = True
                    # Wait for arduino to boot
                    TIMEOUT = 5.0
                    start_time  = time.time()
                    desired_update_period_us = -1
                    while not(self.should_disconnect) and not(self.should_terminate) and time.time() - start_time < TIMEOUT:
                        line = str(ser.readline())
                        if line.startswith("b'mecatro@"):
                            header = line.split("\\")[0].split("b'mecatro@")[1].split("@")
                            desired_update_period_us = int(header[0])
                            self.log_data = OrderedDict()
                            for h in header[1:]:
                                self.log_data[h.strip()] = []
                            break
                        time.sleep(0.01)
                    if self.should_terminate:
                        return
                    if desired_update_period_us < 0:
                        self.status_message.append(f"Timeout on Arduino boot. Is the correct baudrate selected ?")
                        self.connected = False
                        break 
                    self.status_message.append(f"Arduino code started. Target period: {desired_update_period_us}us")

                    max_time = 1.05 * desired_update_period_us / 1e6

                    self.data_to_send = ""
                    while not(self.should_disconnect):
                        # Send data.
                        ser.write(self.data_to_send.encode('ascii'))
                        self.data_to_send = ""
                        # Read data byte by byte until an @ is found
                        if ser.read(1) == b'@':
                            data = ser.read(4 * (1 + len(self.log_data.keys()))) # Number of byts expected: n variables + 1 for time.
                            
                            self.line_idx += 1
                            # Resize buffers as needed
                            if self.line_idx >= len(self.time):
                                self.time += [0] * CHUNK_SIZE
                                for k in self.log_data:
                                    self.log_data[k] += [0] * CHUNK_SIZE

                            # Decode
                            self.time[self.line_idx] = struct.unpack('<I', data[:4])[0] / 1.0e6
                            for i, h in enumerate(self.log_data):
                                self.log_data[h][self.line_idx] = struct.unpack('f', data[4*(i+1):4*(i+2)])[0]

                            # Check arduino speed
                            dt = self.time[self.line_idx] - self.time[self.line_idx - 1]
                            if dt > max_time:
                                self.status_message.append(f"Warning: lag detected on the Arduino: iteration duration {int(1e6 * dt)}us > {desired_update_period_us}us.")

            except serial.serialutil.SerialException as e:
                if "Access is denied" in str(e):
                    self.status_message.append("ERROR: Access to serial port denied. Is Arduino SerialMonitor running ?")
                elif "The device does not recognize the command" in str(e):
                    self.status_message.append("ERROR: Communication with Arduino lost.")
                else:
                    self.status_message.append(f"ERROR: serial communication error: {str(e)}")
                    
            self.status_message.append("Arduino disconnected")

            self.connected = False
            if self.line_idx > 0:
                log_path = Path(self.targer_folder) / ("log_" + datetime.now().strftime("%d%m%Y_%H%M%S") + ".csv")
                # Crop log
                self.time = self.time[:self.line_idx]
                for d in self.log_data:
                    self.log_data[d] = self.log_data[d][:self.line_idx]

                with open(log_path, 'w') as f:
                    f.write("Time," + ",".join(self.log_data.keys()) + "\n")
                    for i in range(self.line_idx):
                        f.write(f"{self.time[i]:.5f}," + ",".join([str(d[i]) for d in self.log_data.values()]) + "\n")
                        
                self.status_message.append(" Log saved as: " + str(log_path.absolute()))