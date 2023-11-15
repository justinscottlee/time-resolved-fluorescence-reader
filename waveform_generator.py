import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import math

SAMPLE_RATE = 50000
NOISE_STRENGTH = 0.01
BIAS = 0

# generate noise data
noise = np.random.normal(0, NOISE_STRENGTH, SAMPLE_RATE)

# generate interference signal
interference = np.zeros(SAMPLE_RATE)
INTERFERENCE_FREQUENCIES = [60, 50, 287, 3418, 794]
for i in range(0, SAMPLE_RATE):
    for frequency in INTERFERENCE_FREQUENCIES:
        #interference[i] += 100 *  math.sin(2 * math.pi * frequency * i / SAMPLE_RATE)
        pass

# 1/e time constant (in seconds)
# https://www.ncbi.nlm.nih.gov/pmc/articles/PMC1538960/ found the time constant to be 424 us.
FLUORESCENCE_TIME_CONSTANT = 424e-6
FLUORESCENCE_TIME_CONSTANT_SAMPLES = math.floor(FLUORESCENCE_TIME_CONSTANT * SAMPLE_RATE)

EXCITATION_INTERVAL = 2e-3
EXCITATION_INTERVAL_SAMPLES = math.floor(EXCITATION_INTERVAL * SAMPLE_RATE)

# delay next pulse slightly longer than the previous pulse (chirp)
EXCITATION_CONSECUTIVE_DELAY = 0.0e-3
EXCITATION_CONSECUTIVE_DELAY_SAMPLES = math.floor(EXCITATION_CONSECUTIVE_DELAY * SAMPLE_RATE)

def u(t):
    if t > 0:
        return 1
    else:
        return 0

# generate emissions waveform
emissions = np.zeros(SAMPLE_RATE)
for i in range(0, math.floor(SAMPLE_RATE / EXCITATION_INTERVAL_SAMPLES)):
    for j in range(i * EXCITATION_INTERVAL_SAMPLES + i * i * EXCITATION_CONSECUTIVE_DELAY_SAMPLES, min(i * EXCITATION_INTERVAL_SAMPLES + i * i * EXCITATION_CONSECUTIVE_DELAY_SAMPLES + FLUORESCENCE_TIME_CONSTANT_SAMPLES * 10, SAMPLE_RATE)):
        emissions[j] += math.exp(-(j - (i * EXCITATION_INTERVAL_SAMPLES + i * i * EXCITATION_CONSECUTIVE_DELAY_SAMPLES)) / FLUORESCENCE_TIME_CONSTANT_SAMPLES) * u(j - (i * EXCITATION_INTERVAL_SAMPLES + i * i * EXCITATION_CONSECUTIVE_DELAY_SAMPLES))

data = noise + emissions + interference + BIAS

plt.title("Fluorescence Emission Waveform")
plt.xlabel("Sample No. (50 kHz)")
plt.ylabel("Emission Intensity (normalized)")
plt.plot(data[:150], label="Unfiltered")

# filtering
fft = np.fft.fft(data)
fundamental_index = math.floor(SAMPLE_RATE / EXCITATION_INTERVAL_SAMPLES)
for i in range(0, math.floor(len(fft) / 2)):
    if (i > 100 * fundamental_index) or ((i % fundamental_index) > 5 and (fundamental_index - i % fundamental_index) > 30):
        fft[i] = 0
        fft[-i - 1] = 0
data = np.real(np.fft.ifft(fft))

plt.plot(data[:150], label="Filtered")
plt.legend()
plt.savefig('data.png')