# -*- coding: utf-8 -*-
"""
Created on Wed Mar  9 14:19:49 2022

@author: mikeg
"""

from datetime import date
import sys, time, serial, matplotlib, random
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from collections import deque   

import numpy as np

try:
    import tkinter as tk
except ImportError:
    import tkinter as tk

matplotlib.style.use("default")


def init_gui():

    global root

    global p1_box
    global p2_box
    global p3_box

    global plot_size
    global x_values
    global p1_values
    global p2_values
    global p3_values
    global fig
    global ax1
    global line1
    global line2
    global line3

    global daq_toggle_button

    plot_size = 1000

    x_values = [i for i in range(plot_size)]
    p1_values = deque(np.zeros(plot_size))
    p2_values = deque(np.zeros(plot_size))

    # plt.ion()
    # fig = plt.figure(figsize=(20,7))
    fig = plt.figure(figsize=(15,5)) #may need to change depending on computer screen size
    ax1 = fig.add_subplot(111)
    ax1.set_title("Force Readings")
    ax1.set_xlim(0, plot_size)
    #ax1.set_ylim(0, 1100)
    ax1.set_ylim(0, 150)
    #ax1.set_yticks([0, 100, 200, 300, 400, 500, 600, 700, 800,900,1000,1100])
    ax1.set_yticks([0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150])
    ax1.axhline(8192, color="r", linestyle="--", linewidth=0.7)
    #line1, = ax1.plot(x_values, p1_values, 'r-', linewidth=0.6)
    line2, = ax1.plot(x_values, p2_values, 'g-', linewidth=0.6)
    # plt.grid()

    root = tk.Tk()
    root.protocol("WM_DELETE_WINDOW", _quit)
    root.wm_title("ISS SOLID SETS_2 DAQ SYSTEM")

    primary_frame = tk.Frame(root)

    canvas = FigureCanvasTkAgg(fig, master=primary_frame)  # A tk.DrawingArea.
    canvas.draw()
    canvas.get_tk_widget().grid(row=2, column = 1, columnspan = 2)

    pressure_frame = tk.Frame(primary_frame)

    p1_label = soc_label = tk.Label(pressure_frame, text="Time (s)", font=("Arial", 15, "bold"))
    p1_label.grid(row=1, column=1, columnspan=1)
    p1_box = tk.Text(pressure_frame, height = 3, width = 20)
    p1_box.insert(tk.END, ("0"))
    p1_box.tag_add("center", 1.0, "end")
    p1_box.tag_configure("center", justify="center")
    p1_box.configure(font=("Arial", 20))
    p1_box.grid(row = 1, column = 2, columnspan=1, padx = 30, pady = 5)

    p2_label = soc_label = tk.Label(pressure_frame, text="Thrust (N)", font=("Arial", 15, "bold"))
    p2_label.grid(row=2, column=1, columnspan=1)
    p2_box = tk.Text(pressure_frame, height = 3, width = 20)
    p2_box.insert(tk.END, ("0"))
    p2_box.tag_add("center", 1.0, "end")
    p2_box.tag_configure("center", justify="center")
    p2_box.configure(font=("Arial", 20))
    p2_box.grid(row = 2, column = 2, columnspan=1, padx = 30, pady = 5)


    pressure_frame.grid(row = 1, column = 2)

    serial_frame = tk.Frame(primary_frame)

    daq_toggle_button = tk.Button(serial_frame, text="TOGGLE DAQ", height=3, width = 20, command=lambda: daq_toggle())
    daq_toggle_button.config({"background" : "red"})
    daq_toggle_button.config({"justify" : "center"})
    daq_toggle_button.grid(row=2, column=1)

    serial_frame.grid(row=1, column=1)

    primary_frame.pack()


def daq_toggle():
    global ser
    global DAQ_ENABLE

    if DAQ_ENABLE:
        #ser.write("b".encode("utf-8"))
        daq_toggle_button.config({"background" : "red"})
        daq_toggle_button.config({"justify" : "center"})
        DAQ_ENABLE = False
    else:
        #ser.write("a".encode("utf-8"))
        daq_toggle_button.config({"background" : "green3"})
        daq_toggle_button.config({"justify" : "center"})
        DAQ_ENABLE = True


def update_plot(new_p1, new_p2):

    p1_values.popleft()
    p2_values.popleft()

    if new_p1 == None:
        p1_values.append(p1_values[len(p1_values)-1])
    else:
        p1_values.append(new_p1)

    if new_p2 == None:
        p2_values.append(p2_values[len(p2_values)-1])
    else:
        p2_values.append(new_p2)


    #line1.set_ydata(p1_values)
    line2.set_ydata(p2_values)

    fig.canvas.draw()

    fig.canvas.flush_events()

    p1_box.delete("1.0", tk.END)
    p1_box.insert(tk.END, str(new_p1))
    p1_box.tag_add("center", 1.0, "end")
    p2_box.delete("1.0", tk.END)
    p2_box.insert(tk.END, str(new_p2))
    p2_box.tag_add("center", 1.0, "end")


def _quit():
    root.quit()
    root.destroy()
    sys.exit()

timestamp = time.strftime("%Y-%m-%d_%H-%M-%S", time.localtime())
file_name = f"{timestamp}Data.csv"

# Open the file with the formatted timestamp
out_file = open(file_name, "at")


def mcu_loop():

    i = 0

    while True:
        start = time.time()

        try:
            data = str(ser.readline()[:-2].decode("utf-8"))
            ser.flush()

            if data:
                tStamp, val1 = data.split("\t")
                if DAQ_ENABLE == True:    
                    out_file.write(f"{float(tStamp)*0.001},{float(val1)*0.009807}\n")
                    out_file.flush()
                    update_plot(float(tStamp)*0.001, float(val1)*0.009807)

                #plot_end = time.time()
                #print("Plot Max Refresh Rate: " + str(1/(plot_end-plot_start)) + " Hz")

                    ser.flush()
                else:
                #print("Timestamp:{}\tPressure1:{}\tPressure2:{}\tPressure3:{}"
                #.format(tStamp, val1, val2, val3))
                #print(tStamp)

                #plot_start = time.time()

                    update_plot(float(tStamp)*0.001, float(val1)*0.009807)

                #plot_end = time.time()
                #print("Plot Max Refresh Rate: " + str(1/(plot_end-plot_start)) + " Hz")

                    ser.flush()

        # update_plot(np.sin(i)*3000 + 4000, np.cos(i)*500 + 3000, 2000)
        # i += 0.1
        # time.sleep(0.1)

            else:
                update_plot(None, None)


        except KeyboardInterrupt:
            ser.flush()
            ser.write("stahp".encode("utf-8"))

        except ValueError:
            ser.flush()

        end = time.time()
        #print("Refresh Rate: " + str(1/(end-start)) + " Hz")


if __name__ == "__main__":

    global ser
    global DAQ_ENABLE

    DAQ_ENABLE = False

    init_gui()

    update_plot(0, 0)

    # ser = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
    ser = serial.Serial("COM7", 9600, timeout=1) # Who use this code should change the port name.

    # while True:
    #     line = ser.readline().decode("utf-8")
    #     print(line)

    try:
        mcu_loop()
    except KeyboardInterrupt:
        sys.exit()
