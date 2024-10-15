import numpy as np
from scipy import signal
import oe_pyprocessor

def butter_bandpass(lowcut, highcut, sample_rate, order=2):
    """
    Create an Nth-order analog Butterworth bandpass filter.

    Parameters:
    lowcut = lowpass frequency
    highcut = highpass frequency
    sample_rate = sampling frequency of incoming data
    order = the order of the filter

    Returns:
    Second-order sections representation of the bandpass filter
    """
    sos = signal.butter(order, [lowcut, highcut], analog=False, btype='bandpass', output='sos', fs=sample_rate)
    return sos

def butter_bandpass_filter(sos, data):
    """
    Filter data using the bandpass filter 

    Parameters:
    sos = Array of second-order filter coefficients
    data = data to filter

    Returns:
    Second-order sections representation of the bandpass filter
    """
    y = signal.sosfilt(sos, data)
    return y

class PyProcessor:
    
    def __init__(self, processor, num_channels, sample_rate):
        """ A new bandpass filter is initialized whenever the plugin settings are updated """
        
        self.num_chans = num_channels
        self.sample_rate = sample_rate
        self.processor = processor
        
        self.sos = []
        sos_t = butter_bandpass(500, 2000, sample_rate)
        for i in range(self.num_chans):
            self.sos.append(sos_t)
        
        self.sos = np.asarray(self.sos)
    
    def process(self, data):
        """
        Filter each incoming data buffer.
        
        Parameters:
        data - numpy array.
        """
        for i in range(self.num_chans):
            data[i] = butter_bandpass_filter(self.sos[i], data[i])
        
    def start_acquisition(self):
        """ Called at start of acquisition """
        pass
    
    def stop_acquisition(self):
        """ Called when acquisition is stopped """
        pass
    
    def handle_ttl_event(self, source_node, channel, sample_number, line, state):
        """ Handle each incoming ttl event """
        pass

    def handle_spike(self, source_node, electrode_name, num_channels, num_samples, sample_number, sorted_id, spike_data):
        """ Handle each incoming spike """
        pass

    def start_recording(self, recording_dir):
        """ Called when recording starts """
        pass
    
    def stop_recording(self):
        """ Called when recording stops """
        pass