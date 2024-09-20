from tkinter import *
from bisect import bisect

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from matplotlib.figure import Figure
from matplotlib.gridspec import GridSpec



class Plotter:
    def __init__(self, frame):
        '''
        Plot log data in real time in application window
        '''

        self.fig = Figure(figsize=(3, 15), dpi=100)
        self.fig.subplots_adjust(left=0.05, right=0.98, bottom=0.05, top=0.90)
        self.ax = self.fig.add_subplot(111)

        self.canvas = FigureCanvasTkAgg(self.fig, master=frame)
        toolbar = NavigationToolbar2Tk(self.canvas, frame, pack_toolbar=False)
        toolbar.update()
        toolbar.pack(side=TOP, fill=X)
        self.canvas.get_tk_widget().pack(side=BOTTOM, fill=BOTH, expand=1)

        self.lines = {}

    def update(self, telemetry_listener):
        time, log_data, idx = telemetry_listener.get_data()

        if idx < 0:
            return

        time = time[:idx]
        # Display at most last 10 seconds
        sidx = bisect(time, time[-1] - 10)
        time = time[sidx:]

        if len(self.lines) < len(log_data):
            # New variable added: recreate the figure
            self.fig.legend().remove()
            for d in log_data:
                self.lines[d], = self.ax.plot(time, log_data[d][sidx:idx], label=d)
            self.fig.legend(self.lines.values(), self.lines.keys(), loc='upper center', ncols=6)
            self.ax.grid(True)
        else:
            # Simply update the lines
            ymin, ymax = 1e10, -1e10
            for d in self.lines:
                if d not in log_data:
                    continue
                self.lines[d].set_xdata(time)
                self.lines[d].set_ydata(log_data[d][sidx:idx])
                if len(log_data[d][:idx]) > 0:
                    ymin = min(ymin, min(-0.01, 1.1 * min(log_data[d][sidx:idx])))
                    ymax = max(ymax, max(0.01, 1.1 * max(log_data[d][sidx:idx])))
                self.ax.draw_artist(self.lines[d])
            self.ax.set_ylim(ymin, ymax)
            try:
                self.ax.set_xlim(time[0], time[-1])
            except IndexError:
                pass
        self.fig.canvas.draw()
