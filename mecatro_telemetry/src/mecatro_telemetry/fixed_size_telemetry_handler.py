# This code is a python example to handle the fixed-size telemetry provided by the arduino.
# See format details in MecatroUtils.cpp

import threading
from datetime import datetime
import time
from pathlib import Path 

import struct
import serial
import serial.tools.list_ports
import matplotlib.pyplot as plt 

if __name__ == "__main__":
    # Look for Arduino serial port.
    #port_name = None
    #for p in serial.tools.list_ports.comports():
    #    print(p.description)
    #    if "Arduino" in p.description:
    #        port_name = p.name 
    #        break 
    #if port_name is None:
    port_name="/dev/cu.usbserial-220"
    # For efficiency, you don't want to allocate a new data point every time, but rather do a big 'chunk' allocation.
    # Now considering how small an Arduino is compared to a PC, we can grow lazy: the user will not send more than 
    # 20 data points every ms (the Arduino can't keep up). So we get 20 x 4 x 1000 = 80kBytes/s. This is nothing !
    # 1000s of experiemnt would only amount to 80Mb of RAM, this is clearly acceptable.
    # Thus, we can pre-allocate 1 million point per variable and not worry about reaching the buffer.
    MAX_DATA_LENGTH = 1000000

    # Init data holders
    log_time = [0] * 1000000
    data_values = {}
    
    # Store the index of where we are at in the time vector.
    current_idx = -1

    # Open serial port at 1Mbps
    with serial.Serial(port_name, 1000000, timeout=1.0) as ser:
        # The Arduino will reboot when the port is open, but we might still get some data from the previous
        # run. To avoid this, we wait for the Arduino to start its reboot sequence, then clear the input buffer.
        time.sleep(0.5)
        ser.flushInput()
        while True:
            # Wait for the start of a frame, aka @ 
            # b'@' is the way to tell python we want the ASCII value of @
            while (b:= ser.read(1)) != b'@':
                continue 

            # We have read the @, now we need to decode the message
            # The next 20 bytes are the variable's name, to be interpreted as ascii characters.
            name = str(ser.read(20), 'ascii')
            # Remove trailing spaces
            name = name.strip()

            # Next we read the next 4 bytes, the variable value.
            data = ser.read(4)

            # Now we have two cases: are we dealing with an unsigned int (timestamp) or a float ?
            if name == "TIMESTAMP":
                # Read unsigned int32, little endian (i.e. LSB first).    
                current_time = struct.unpack("<I", data)[0]

                # Increment index and store time
                current_idx += 1
                if (current_idx >= MAX_DATA_LENGTH):
                    raise ValueError("Experiment too long for buffer - you are very patient !")
                log_time[current_idx] = current_time / 1e6
            else:
                # We recieved a float variable, little endian
                value = struct.unpack("<f", data)[0]
                # If the variable does not exist, create it
                if name not in data_values:
                    data_values[name] = [0] * MAX_DATA_LENGTH
                data_values[name][current_idx] = value

            # For testing: stop after 1000 points i.e. 5s
            if current_idx > 1000:
                break
    
    # Crop to remove unused points
    log_time = log_time[:current_idx]
    for v in data_values:
        data_values[v] = data_values[v][:current_idx]

    # Plot test
    for d, v in data_values.items():
        plt.plot(log_time, v, label=d)
    plt.legend()
    plt.grid()
    plt.show()