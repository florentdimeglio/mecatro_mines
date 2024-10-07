# A generic interface class to talk to the Arduino, either using a socket (WiFi) or
# a serial port

import serial
import socket
import typing as tp

from mecatro_telemetry.utils import send_floats_to_arduino, list_serial_ports

ARDUINO_IP = "192.168.4.1"
ARDUINO_PORT_NUMBER = 80

class ArduinoInterface:
    def __init__(self, port_name:str, baudrate:int = 1000000):
        self.socket = None
        self.serial = None
        self.baudrate = baudrate
        self.port_name = port_name

        if port_name == "wifi":
                self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.socket.settimeout(0.5)

    def connect(self, float_list: tp.List[float] = None):
        if self.socket:
            try:
                self.socket.connect((ARDUINO_IP, ARDUINO_PORT_NUMBER))
            except TimeoutError:
                print("Failed to connect to Arduino over WiFi. Are you connected to the right network?")
                print("If you want to use USB instead, use argument '-p' and one of the following serial ports:")
                for r in list_serial_ports():
                    print(f" - {r}")
                exit()
        else:
            self.serial = serial.Serial(self.port_name, self.baudrate, timeout=0.5, write_timeout=0.5)
        # Send start
        self.send(b"s")

        # Send gains
        if float_list:
            send_floats_to_arduino(float_list, self.send)

    def send(self, data: bytearray):
        """
        Send bytearray to the Arduino
        """
        if self.socket:
            self.socket.sendall(data)
        else:
            try:
                self.serial.write(data)
            except serial.serialutil.SerialTimeoutException:
                print("Failed to write data to Arduino. Is the correct port serial selected?")
                exit()

    def read(self, n_bytes:int):
        """
        Receive n_bytes from the Arduino
        """
        if self.socket:
            n_recv = 0
            total_data = b""
            while n_recv < n_bytes:
                try:
                    total_data += self.socket.recv(n_bytes - n_recv)
                except TimeoutError:
                    print("Timeout waiting for data. You probably need to reset the Arduino")
                    exit()
                n_recv = len(total_data)
            return total_data
        else:
            try:
                return self.serial.read(n_bytes)
            except serial.serialutil.SerialException:
                print("Timeout waiting for data. You probably need to reset the Arduino")
                exit()
