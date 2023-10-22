import numpy as np
import pandas as pd
import math

SAMPLE_RATE = 1000000
NOISE_STRENGTH = 1
BIAS = 100

# generate noise data
noise = np.random.normal(0, NOISE_STRENGTH, SAMPLE_RATE)

# generate interference signal
interference = np.zeros(SAMPLE_RATE)
INTERFERENCE_FREQUENCIES = [60, 50]
for i in range(0, SAMPLE_RATE):
    for frequency in INTERFERENCE_FREQUENCIES:
        #interference[i] += 10 *  math.sin(2 * math.pi * frequency * i / SAMPLE_RATE)
        pass

# 1/e time constant (in seconds)
# https://www.ncbi.nlm.nih.gov/pmc/articles/PMC1538960/ found the time constant to be 424 us.
FLUORESCENCE_TIME_CONSTANT = 424e-6
FLUORESCENCE_TIME_CONSTANT_SAMPLES = math.floor(FLUORESCENCE_TIME_CONSTANT * SAMPLE_RATE)

EXCITATION_INTERVAL = 10e-3
EXCITATION_INTERVAL_SAMPLES = math.floor(EXCITATION_INTERVAL * SAMPLE_RATE)

def u(t):
    if t > 0:
        return 1
    else:
        return 0

# generate emissions waveform
emissions = np.zeros(SAMPLE_RATE)
for i in range(0, math.floor(SAMPLE_RATE / EXCITATION_INTERVAL_SAMPLES)):
    for j in range(i * EXCITATION_INTERVAL_SAMPLES, min(i * EXCITATION_INTERVAL_SAMPLES + FLUORESCENCE_TIME_CONSTANT_SAMPLES * 10, SAMPLE_RATE)):
        emissions[j] += math.exp(-(j - i * EXCITATION_INTERVAL_SAMPLES) / FLUORESCENCE_TIME_CONSTANT_SAMPLES) * u(j - i * EXCITATION_INTERVAL_SAMPLES)

data = noise + emissions + interference + BIAS

# output original waveform
pd.DataFrame(np.real(data)).to_csv("original_data.csv")

# filtering
fft = np.fft.fft(data)
fundamental_index = math.floor(SAMPLE_RATE / EXCITATION_INTERVAL_SAMPLES)
for i in range(0, math.floor(len(fft) / 2)):
    if (i < fundamental_index / 8) or (i > 100 * fundamental_index) or ((i % fundamental_index) > 5 and (fundamental_index - i % fundamental_index) > 5):
        fft[i] = 0
        fft[-i - 1] = 0
data = np.fft.ifft(fft)

pd.DataFrame(np.real(data)).to_csv("processed_data.csv")
