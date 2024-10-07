import serial.tools.list_ports
import struct
import typing as tp

def list_serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    ports = serial.tools.list_ports.comports()
    return [f"{p} ({desc})" for p, desc, _ in sorted(ports)]



def send_floats_to_arduino(float_list:tp.List[float], send_function):
    """
    Send gains to robot.
        - gains: list of float32 to send
        - send_function: function to send a byte array
    """
    send_array = bytearray(4 * len(float_list) + 2)
    send_array[0] = 0xFF
    for i, f in enumerate(float_list):
        send_array[4 * i +1:4 * (i + 1) + 1] = struct.pack("f", f)

    checksum = sum(send_array[1:-1]) % 256
    send_array[-1] = 255 - checksum
    send_function(send_array)

