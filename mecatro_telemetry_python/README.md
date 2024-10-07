# `mecatro_telemetry` package

This python package provides a command-line utility, `mecatro_telemetry`, that listen
to data coming from the Arduino, and stores it to a CSV file

## Installation

Simply install this python package, for instance in a virtual environment using pip:

`pip install .`

You will probably need to install `matplotlib` as well for plots, though this is not a
direct requirement.

## Getting data from the Arduino

Once the package is installed, simply type `mecatro_telemetry` to launch data acquisition.
With no extra arguments, it will try to connect using WiFi, and store data in the current
folder under the name `arduino_log.csv`. The main arguments are:
 - `-p` to specify a serial port name, instead of WiFi (`-b` for baudrate ; use `--list_serial`
 to get the list of ports available).
 - `-o`: output file name
 - `--arduino_parameters <float1>:<float2>:<...>`: list of floats to send to the Arduino

Type `mecatro_telemetry -h` or `mecatro_telemetry --help` for the full list of arguments.

## Reading the log file

`mecatro_telemetry` stores data in a simple `csv` file, each column corresponding to a variable.
This can easily be open using any spreadsheet software.

In `python`, the following example code will plot all variables from a log into a single plot:

```python
import csv
import matplotlib.pyplot as plt

# Load all data into a dictionnary
def load_data(logfile:str):
    with open(logfile) as csv_file:
        reader = csv.DictReader(csv_file)

        data_dict = {}
        for row in reader:
            for k, v in row.items():
                if k not in data_dict:
                    data_dict[k] = []
                data_dict[k].append(float(v))
    return data_dict

data = load_data("arduino_log.csv")

for x in data:
    if not(x == "Time"):
        plt.plot(data["Time"], data[x], label=x)
plt.grid()
plt.legend()
plt.show()
```