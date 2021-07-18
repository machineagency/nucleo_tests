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

# Plotting/Data Collection Constants
SAMPLE_INTERVAL_S = 10 # number of seconds worth of data pts to plot.
PLOTTING_RATE_HZ = 1. # update rate for plot window. Can't go too fast.
NUM_DATAPTS = 100
USB_READ_TIMEOUT_MS = 3000 # How long (in ms) to wait until giving up on incoming data.

# USB Device-related constants. These must agree with the settings on the device.
USB_ID_VENDOR = 0xcafe
USB_ID_PRODUCT = 0xcafe
BULK_READ_ADDR = 0x81 # bulk transfer address.
BULK_PKT_SIZE = 64 # size of the bulk transfer packet in bytes.
SAMPLING_FREQ_HZ = 5000. # sampling rate in samples/sec.
DATAPTS_PER_PACKET = 15
USB_PACKET_DELIVERY_FREQ_HZ = SAMPLING_FREQ_HZ/DATAPTS_PER_PACKET # approx usb packet delivery rate (samples/sec).

# Setup data collection in FIFOs for plotting like an oscilloscope.
amt_fifo = deque(maxlen=NUM_DATAPTS)
heds_fifo = deque(maxlen=NUM_DATAPTS)


# Connect to device over USB.
dev = None
try:
    dev = usb.core.find(idVendor=USB_ID_VENDOR, idProduct=USB_ID_PRODUCT)
    if dev is None:
        raise ValueError('Device not found. Is it plugged in?')
    dev.set_configuration()
except usb.core.USBError as e:
    print(e)
    print("Error: this program must be run with root, or you must apply udev rules.")
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
    data = dev.read(BULK_READ_ADDR, BULK_PKT_SIZE, timeout=USB_READ_TIMEOUT_MS)
    # Extract data from the packet and put in the respective queues.
    last_pkt_index = UInt16(struct.unpack("<H", data[0:2])[0])
    amt_data = struct.unpack(fmt, data[2:int(BULK_PKT_SIZE/2)])
    heds_data = struct.unpack(fmt, data[int(BULK_PKT_SIZE/2):-2])
    # Store data into the plotting buffers.
    amt_fifo.extend(amt_data)
    heds_fifo.extend(heds_data)

    # Extract data in a loop forever.
    while True:
        # Check for new packets at least twice as fast as delivery rate.
        # Skip sleeping at all if packet delivery is sufficiently fast.
        if USB_PACKET_DELIVERY_FREQ_HZ < 100: # Hz
            time.sleep(1/(2*USB_PACKET_DELIVERY_FREQ_HZ))
        # Pull data from the usb device.
        data = dev.read(BULK_READ_ADDR, BULK_PKT_SIZE, timeout=USB_READ_TIMEOUT_MS)
        # Extract data from the packet and put in the respective queues.
        pkt_index = UInt16(struct.unpack("<H", data[0:2])[0])
        #print(f"pkt_index: {pkt_index}")
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
            amt_fifo.extend(amt_data)
            heds_fifo.extend(heds_data)


# Launch data collection thread.
Thread(target=collect_data_worker, name="collect_data_worker", daemon=True).start()

# Setup plotting.
t = np.linspace(0, NUM_DATAPTS/SAMPLING_FREQ_HZ, NUM_DATAPTS, endpoint=False)
plt.ion()
fig = plt.figure()
ax = fig.add_subplot(111)

# Don't try plotting until we have enough data to begin with.
print("Waiting to fill up buffer with enough data....")
while len(amt_fifo) < len(t) or len(heds_fifo) < len(t):
    pass

# Create lines with empty data.
line1 = ax.plot(t, np.zeros(len(t)), 'b-')[0]
line2 = ax.plot(t, np.zeros(len(t)), 'g-')[0]

# Main Loop for plotting:
# Plot on an absolute schedule. Throw warnings if we can't plot fast enough.
loop_interval_start = time.perf_counter()
curr_time = loop_interval_start
scheduled_time = loop_interval_start + 1.0/PLOTTING_RATE_HZ
while True:

    if curr_time < scheduled_time:
        time.sleep(scheduled_time - curr_time)
    # Update the graphs.
    x1 = np.asarray(amt_fifo)
    x2 = np.asarray(heds_fifo)
    line1.set_ydata(x1) # Cute graph for testing
    line2.set_ydata(x2) # Cute graph for testing
    #processing_start_time = time.perf_counter() # For processing metrics.
    #line1_data = np.sin(0.5 * t + curr_time)
    #line2_data = np.cos(0.5 * t + curr_time)
    #line1.set_ydata(line1_data) # Cute graph for testing
    #line2.set_ydata(line2_data) # Cute graph for testing
    #ax.relim()
    plt.ylim(min(amt_fifo), max(amt_fifo))
    fig.canvas.draw()
    #print(f"processing time: {time.perf_counter() - processing_start_time}")

    curr_time = time.perf_counter()
    scheduled_time += 1.0/PLOTTING_RATE_HZ

    # Loop-too-fast-check:
    if curr_time >= scheduled_time:
        print("Warning: Plotting interval is running faster than we can process the data.")

