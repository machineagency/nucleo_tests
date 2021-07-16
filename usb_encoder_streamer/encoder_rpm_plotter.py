#!/usr/bin/env python3
import sys
import usb.core # import pyusb
import numpy as np
import struct
from collections import deque
from threading import Thread
import time
import matplotlib.pyplot as plt
from fixedint import UInt8

# Constants
TIME_WINDOW = 10 # number of seconds worth of data pts to plot.
PLOTTING_RATE = 10. # update rate for plot window. Can't go too fast.

# USB Device-related constants. These must agree with the settings on the device.
BULK_READ_ADDR = 0x81 # bulk transfer address.
BULK_PKT_SIZE = 64 # size of the bulk transfer packet in bytes.
SAMPLING_RATE = 50. # sampling rate in samples/sec.
USB_PACKET_DELIVERY_RATE = 10. # usb packet delivery rate (samples/sec).


# Setup data collection in FIFOs for plotting like an oscilloscope.
amt_fifo = deque(maxlen=NUM_DATAPTS)
heds_fifo = deque(maxlen=NUM_DATAPTS)


# Connect to device over USB.
dev = None
try:
    dev = usb.core.find(idVendor=0xcafe, idProduct=0xcafe)
    if dev is None:
        raise ValueError('Device not found. Is it plugged in?')
    dev.set_configuration()
except usb.core.USBError:
    print("Error: this program must be run with root.")
    sys.exit()


# Data collection routine. This will run in a thread.
def collect_data_worker():
    """Pull data from usb and format into each container."""
    # encoder data in the usb packet is formatted as:
    # 2 contiguous chunks, one per encoder, little-endian, 16-bit (2-byte) signed value.
    fmt = f"<{BULK_PKT_SIZE/2/2 - 1}l" # if pkt is 64 bytes, this should be 15 int32_t's
    # -1 is because the first word is reserved for the packet index.

    # Each packet has an index. We will treat it as a fixed int so we can do
    # subtraction to figure out if the packet that we just read is new.


    # Extract data for the first time.
    # Pull data from the usb device and store it.
    data = dev.read(BULK_READ_ADDR, BULK_PKT_SIZE)
    # Extract data from the packet and put in the respective queues.
    last_pkt_index = UInt8(struct.unpack("<B", data[0]))
    amt_data = struct.unpack(fmt, data[2:BULK_PKT_SIZE/2])
    heds_data = struct.unpack(fmt, data[BULK_PKT_SIZE/2:-2])
    amt_fifo.extend(amt_data)
    heds_data.extend(heds_data)

    # Do data collection in a loop forever.
    while True:
        # Check for new packets at least twice as fast as delivery rate.
        time.sleep(1/(2*USB_PACKET_DELIVERY_RATE))
        # Pull data from the usb device.
        data = dev.read(BULK_READ_ADDR, BULK_PKT_SIZE)
        # Extract data from the packet and put in the respective queues.
        pkt_index = UInt8(struct.unpack("<B", data[0]))
        amt_data = struct.unpack(fmt, data[4:BULK_PKT_SIZE/2])
        heds_data = struct.unpack(fmt, data[BULK_PKT_SIZE/2:-4])
        # Check for new pkt with fixed point subtraction.
        new_pkt_distance = pkt_index - last_pkt_index
        if new_pkt_distance > 0:
            if new_pkt_distance > 1:
                print(f"ERROR: We have missed {new_pkt_distance - 1} usb packets.")
                #TODO: in this case we should stuff the fifo with zeros
                # to make up for the missing data.
            # Default case: we caught the next sequential data pkt.
            last_pkt_index = pkt_index
            amt_fifo.extend(amt_data)
            heds_data.extend(heds_data)


# Launch data collection thread.
threading.Thread(target=collect_data_worker, name="collect_data_worker", daemom=True).start()

# Setup plotting.
x1 = np.asarray(amt_fifo)
x2 = np.asarray(heds_fifo)


#    value = 0
#    while True:
#        key = input("press ENTER to toggle the LED.")
#        value^=1
#        dev.ctrl_transfer(0x40, 0, value, 0, 'Hello World!')


# Main Loop:
# Plot on an absolute schedule. Throw warnings if we can't plot fast enough.
    loop_interval_start = time.perf_counter()
    curr_time = loop_interval_start
    scheduled_time = loop_interval_start + LOOP_TIME_S
    while True:
        if curr_time < scheduled_time:
            time.sleep(scheduled_time - curr_time)
        # Update the graphs.
        #processing_start_time = time.perf_counter() # For processing metrics.
        line1_data = np.sin(0.5 * t + curr_time)
        line2_data = np.cos(0.5 * t + curr_time)
        line1.set_ydata(line1_data) # Cute graph for testing
        line2.set_ydata(line2_data) # Cute graph for testing
        fig.canvas.draw()
        #print(f"processing time: {time.perf_counter() - processing_start_time}")

        curr_time = time.perf_counter()
        scheduled_time += LOOP_TIME_S

        # Loop-too-fast-check:
        if curr_time >= scheduled_time:
            print("Warning: loop time too fast")


# TODO: kill all threads.
