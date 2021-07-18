#!/usr/bin/env python3
import sys
import usb.core # import pyusb
import numpy as np
import struct
from collections import deque
from threading import Thread
import time
import matplotlib.pyplot as plt
from fixedint import UInt16

# Constants
TIME_WINDOW = 10 # number of seconds worth of data pts to plot.
PLOTTING_RATE_HZ = 10. # update rate for plot window. Can't go too fast.
NUM_DATAPTS = 100

# USB Device-related constants. These must agree with the settings on the device.
BULK_READ_ADDR = 0x81 # bulk transfer address.
BULK_PKT_SIZE = 64 # size of the bulk transfer packet in bytes.
SAMPLING_RATE = 50. # sampling rate in samples/sec.
USB_PACKET_DELIVERY_FREQ = 2. # usb packet delivery rate (samples/sec).


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
except usb.core.USBError as e:
    print(e)
    print("Error: this program must be run with root, or you will need to apply udev rules.")
    sys.exit()


# Data collection routine. This will run in a thread.
def collect_data_worker():
    """Pull data from usb and format into each container."""
    # encoder data in the usb packet is formatted as:
    # 2 contiguous chunks, one per encoder, little-endian, 16-bit (2-byte) signed value.
    fmt = f"<{int(BULK_PKT_SIZE/2/2) - 1}H" # if pkt is 64 bytes, this should be 15 int32_t's
    # -1 is because the first two bytes are reserved for the packet index.

    # Each packet has an index. We will treat it as a fixed int so we can do
    # subtraction to figure out if the packet that we just read is new.


    # Extract data for the first time.
    # Pull data from the usb device and store it.
    data = dev.read(BULK_READ_ADDR, BULK_PKT_SIZE, timeout=3000)
    # Extract data from the packet and put in the respective queues.
    last_pkt_index = UInt16(struct.unpack("<H", data[0:2])[0])
    print(f"pkt_index: {last_pkt_index}")
    amt_data = struct.unpack(fmt, data[2:int(BULK_PKT_SIZE/2)])
    heds_data = struct.unpack(fmt, data[int(BULK_PKT_SIZE/2):-2])
    print(amt_data)
    print(heds_data)
    amt_fifo.extend(amt_data)
    heds_fifo.extend(heds_data)

    # Do data collection in a loop forever.
    while True:
        # Check for new packets at least twice as fast as delivery rate.
        time.sleep(1/(2*USB_PACKET_DELIVERY_FREQ))
        # Pull data from the usb device.
        data = dev.read(BULK_READ_ADDR, BULK_PKT_SIZE, timeout=3000)
        # Extract data from the packet and put in the respective queues.
        pkt_index = UInt16(struct.unpack("<H", data[0:2])[0])
        print(f"pkt_index: {pkt_index}")
        amt_data = struct.unpack(fmt, data[2:int(BULK_PKT_SIZE/2)])
        heds_data = struct.unpack(fmt, data[int(BULK_PKT_SIZE/2):-2])
        # Check for new pkt with fixed point subtraction.
        new_pkt_distance = pkt_index - last_pkt_index
        if new_pkt_distance > 0:
            if new_pkt_distance > 1:
                print(f"ERROR: We have missed {new_pkt_distance - 1} usb packets.")
                #TODO: in this case we should stuff the fifo with zeros
                # to make up for the missing data.
            # Default case: we caught the next sequential data pkt.
            last_pkt_index = pkt_index
            print(amt_data)
            print(heds_data)
            amt_fifo.extend(amt_data)
            heds_fifo.extend(heds_data)


# Launch data collection thread.
Thread(target=collect_data_worker, name="collect_data_worker", daemon=True).start()

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
#loop_interval_start = time.perf_counter()
#curr_time = loop_interval_start
#scheduled_time = loop_interval_start + 1.0/PLOTTING_RATE_HZ
#while True:
#    if curr_time < scheduled_time:
#        time.sleep(scheduled_time - curr_time)
#    # Update the graphs.
#    #processing_start_time = time.perf_counter() # For processing metrics.
#    line1_data = np.sin(0.5 * t + curr_time)
#    line2_data = np.cos(0.5 * t + curr_time)
#    line1.set_ydata(line1_data) # Cute graph for testing
#    line2.set_ydata(line2_data) # Cute graph for testing
#    fig.canvas.draw()
#    #print(f"processing time: {time.perf_counter() - processing_start_time}")
#
#    curr_time = time.perf_counter()
#    scheduled_time += 1.0/PLOTTING_RATE_HZ
#
#    # Loop-too-fast-check:
#    if curr_time >= scheduled_time:
#        print("Warning: loop time too fast")


# TODO: kill all threads.

while True:
    pass
