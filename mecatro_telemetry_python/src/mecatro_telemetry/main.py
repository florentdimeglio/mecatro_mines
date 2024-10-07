import argparse
from pathlib import Path
import time
from collections import OrderedDict
from tqdm import tqdm
import typing as tp
import struct

from mecatro_telemetry.utils import list_serial_ports
from mecatro_telemetry.arduino_interface import ArduinoInterface


def process_telemetry(arduino:ArduinoInterface, output_name:Path, log_duration:float, arduino_parameters:tp.Optional[tp.List[float]] = None):
    """
    This function connects to the Arduino, gathers telemetry data and
    writes it to the target output file.
    """
    arduino.connect(arduino_parameters)

    # Wait for arduino to boot and send header
    print("Connected, waiting for Arduino to start sending data")
    TIMEOUT = 2.0
    start_time  = time.time()
    desired_update_period_us = -1
    while time.time() - start_time < TIMEOUT:
        lineDone = False
        line = ""
        while not lineDone:
            d = arduino.read(1)
            try:
                line = line + str(d, 'ascii')
            except UnicodeDecodeError:
                break
            lineDone = d == b'\n'
        line = line[:-2]
        if line.startswith("mecatro@"):
            # Decode header
            header = line.split("mecatro@")[1].split("@")
            desired_update_period_us = int(header[0])
            log_data = OrderedDict()
            log_time = []
            for h in header[1:]:
                log_data[h.strip()] = []
            break
        time.sleep(0.01)

    if desired_update_period_us < 0:
        print(f"Timeout on Arduino boot. Is the correct port / baudrate selected? Is Arduino really sending telemetry?")
        return

    print(f"Arduino code started. Target period: {desired_update_period_us}us.")

    max_time = 1.05 * desired_update_period_us / 1e6
    start_time = time.time()

    pbar = tqdm(total=log_duration, bar_format='Reading data: {percentage:3.0f}%|{bar}')
    while time.time() - start_time < log_duration:
        pbar.update(time.time() - start_time - pbar.n)
        # Read data byte by byte until an @ is found
        if arduino.read(1) == b'@':
            data = arduino.read(4 * (1 + len(log_data.keys()))) # Number of byts expected: n variables + 1 for time.

            # Decode data
            log_time.append(struct.unpack('<I', data[:4])[0] / 1.0e6)
            for i, h in enumerate(log_data):
                log_data[h].append(struct.unpack('f', data[4*(i+1):4*(i+2)])[0])

            # Check arduino speed
            if len(log_time) > 1:
                dt = log_time[-1] - log_time[-2]
                if dt > max_time:
                    print(f"Warning: lag detected on the Arduino: iteration duration {int(1e6 * dt)}us > {desired_update_period_us}us.")

    print(f"\nAcquisition done, saving to {output_name}")

    with open(output_name, 'w') as f:
        f.write("Time," + ",".join(log_data.keys()) + "\n")
        for i in range(len(log_time)):
            f.write(f"{log_time[i]:.5f}," + ",".join([str(d[i]) for d in log_data.values()]) + "\n")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--port_name", default = "wifi", help = "Serial port name (use --list_serial to see list) or 'wifi'")
    parser.add_argument("-o", "--output", type=Path, default = Path("arduino_log.csv"), help = "Optional, pass to output csv file")
    parser.add_argument("-b", "--baudrate", type=int, default=1000000, help = "Baudrate, for serial communication only (no effect over WiFi)")
    parser.add_argument("-t", "--max_time", type=float, default=20.0, help = "Log maximum duration, in s")
    parser.add_argument("--list_serial", action="store_true", help="If set, print the list of available serial port and exit")
    parser.add_argument("--arduino_parameters", type=str, default=None, help="Semi-colon separate list of floats to send to the Arduino")
    args = parser.parse_args()

    if args.list_serial:
        print("Available serial ports:")
        for r in list_serial_ports():
            print(f" - {r}")
        exit()

    arduino_parameters = None
    if args.arduino_parameters:
        try:
            arduino_parameters = [float(x) for x in args.arduino_parameters.split(":")]
        except ValueError:
            print("Error: failed to convert arduino_parameters to floats.")
            exit()
    arduino = ArduinoInterface(args.port_name, args.baudrate)
    process_telemetry(arduino, args.output, args.max_time, arduino_parameters)
