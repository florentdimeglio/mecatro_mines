from tkinter import *
from tkinter import filedialog
from tkinter.ttk import Combobox
import tkinter.scrolledtext
from multiprocessing import Process
from multiprocessing.managers import BaseManager
from contextlib import suppress

import serial.tools.list_ports
from pathlib import Path
import threading
import time
import datetime

from .telemetry_listener import TelemetryListener
from .telemetry_plotter import Plotter


# custom manager to support custom classes
class CustomManager(BaseManager):
    # nothing
    pass

class GUIWindow(Tk):
    def __init__(self, telemetry_listener):
        '''
        GUI main window
        '''
        super().__init__()

        self.rowconfigure(2, weight=1)
        self.columnconfigure(0, weight=1)


        top_frame = Frame(self)
        top_frame.grid(row=0, sticky="ew")
        top_frame.rowconfigure(3, weight=1)
        top_frame.columnconfigure(1, weight=1)
        top_frame.columnconfigure(3, weight=1)

        center_frame = Frame(self)
        center_frame.grid(row=2, sticky="news")
        center_frame.rowconfigure(0, weight=1)

        self.telemetry_listener = telemetry_listener


        label = Label(top_frame, text="Port:")
        label.grid(row = 0, column = 0, ipadx = 5)

        self.port_combo = Combobox(top_frame, state="readonly")
        self.port_combo.grid(row = 0, column = 1, sticky="news")
        self.port_combo.bind('<<ComboboxSelected>>', self.port_selected)

        refresh = Button(top_frame, command = self.update_port_list, text="Refresh")
        refresh.grid(row=0, column = 2, ipadx = 5)

        self.update_plot_option = BooleanVar()
        update_plot_button = Checkbutton(self, variable=self.update_plot_option, text="Disable plot update (improve performance)")
        update_plot_button.grid(row=1, column = 0, sticky="w")

        label = Label(top_frame, text="Baudrate:")
        label.grid(row = 0, column = 3, ipadx = 5)

        self.baudrate_combo = Combobox(top_frame, state="readonly", values=(1000000, 921600, 230400, 115200))
        self.baudrate_combo.set(230400)
        self.baudrate_combo.grid(row = 0, column = 4, sticky="news")
        self.baudrate_combo.bind('<<ComboboxSelected>>', self.baudrate_selected)


        label = Label(top_frame, text="Target folder:")
        label.grid(row = 0, column = 5, ipadx = 5)
        self.folder_entry = Entry(top_frame, width=50)
        self.telemetry_listener.set_folder(Path.home() )
        self.folder_entry.insert(0, str(Path.home() ))
        self.folder_entry["state"] = "disabled"

        self.folder_entry.grid(row = 0, column = 6, padx = 10)

        self.folder_selector_button = Button(top_frame, text="Select target folder", command=self.set_folder)
        self.folder_selector_button.grid(row = 0, column = 7, padx = 10)

        self.connect_button = Button(top_frame, text="Connect", command=self.toogle_connect, fg='green')
        self.connect_button.grid(row = 0, column = 8, padx = 10)

        scrollbar = Scrollbar(center_frame, orient = 'vertical')
        scrollbar.pack(side = RIGHT, fill = Y)
        self.plotter = Plotter(center_frame)
        self.plotter.canvas.get_tk_widget().config(bg='#FFFFFF',scrollregion=(0,0,500,3000))
        self.plotter.canvas.get_tk_widget().config(height=300)
        scrollbar.config(command=self.plotter.canvas.get_tk_widget().yview)
        # self.plotter.canvas.get_tk_widget().config(yscrollcommand=scrollbar.set)

        self.title('Mecatro Telemetry')
        self.geometry("1080x720+10+10")

        self.text = tkinter.scrolledtext.ScrolledText(self, wrap=WORD, background='white', height=5, state ='disabled')
        self.text.grid(row=3, sticky="ew")

        self.update_port_list()

        self.bind('<KeyPress>', self.key_pressed)
        self.after(50, self.refresh)

    def update_port_list(self):
        self.port_list = serial.tools.list_ports.comports()
        self.port_combo['values'] = [p.description for p in self.port_list]
        arduino_found = False
        for p in self.port_list:
            if "Arduino" in p.description or p.vid == 9025:
                self.port_combo.set(p.description)
                self.port_selected(None)        
                arduino_found = True 
        if not(arduino_found):
            self.port_combo.set(self.port_list[0].description)
            self.port_selected(None)


    def port_selected(self, event):
        for p in self.port_list:
            if p.description == self.port_combo.get():
                self.telemetry_listener.set_port_name(p.device)

    def baudrate_selected(self, event):
        self.telemetry_listener.set_baudrate(int(self.baudrate_combo.get()))

    def toogle_connect(self):
        if not(self.telemetry_listener.is_connected()):
            self.telemetry_listener.attempt_connection()
        else:
            self.telemetry_listener.disconnect()
            self.plotter.lines = {}

    def set_folder(self):
        folder_path = filedialog.askdirectory()
        self.folder_entry["state"] = "normal"
        self.folder_entry.delete(0, END)
        self.folder_entry.insert(0, folder_path)
        self.folder_entry["state"] = "disabled"
        self.telemetry_listener.set_folder(Path(folder_path))

    def refresh(self):
        start_time = time.time()
        # Refresh state based on connection status
        if self.telemetry_listener.is_connected():
            self.connect_button["text"] = "Disconnect"
            self.connect_button["fg"] = "red"
            self.port_combo["state"] = "disabled"
            self.baudrate_combo["state"] = "disabled"
            self.folder_selector_button["state"] = "disabled"
            if not(self.update_plot_option.get()):
                self.plotter.update(self.telemetry_listener)
        else:
            self.connect_button["text"] = "Connect"
            self.connect_button["fg"] = "green"
            self.port_combo["state"] = "readonly"
            self.baudrate_combo["state"] = "readonly"
            self.folder_selector_button["state"] = "normal"
        status = self.telemetry_listener.get_status()
        for s in status:
            s = s.strip()
            if s != "":
                self.text["state"] = "normal"
                # self.text.insert(tkinter.INSERT, f"[{datetime.datetime.now().strftime('%H:%M:%S.%f')[:-3]}] {s}\n")
                self.text.insert(tkinter.INSERT, f"{s}\n")
                self.text["state"] = "disabled"
                self.text.yview(END)

        delay = max(1, 50 - int((time.time() - start_time)* 1000))
        self.after(delay, self.refresh)

    def key_pressed(self, e):
        self.telemetry_listener.send(e.char)


def run_gui(telemetry_listener):
    window = GUIWindow(telemetry_listener)
    with suppress(ModuleNotFoundError):
        import pyi_splash  
        pyi_splash.close()
    window.mainloop()

def main():
    CustomManager.register('TelemetryListener', TelemetryListener)
    with CustomManager() as manager:
        # create a shared set instance
        telemetry_listener = manager.TelemetryListener()
        proc = Process(target=run_gui, args=(telemetry_listener,))
        proc.start()

        thread = threading.Thread(target=telemetry_listener.listener_thread)
        thread.daemon = True
        thread.start()
        proc.join()
        telemetry_listener.terminate()
        thread.join()