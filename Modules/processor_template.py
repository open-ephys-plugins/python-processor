import numpy as np

class PyProcessor:
    
    # A new processor is initialized whenever the plugin settings are updated
    def __init__(self, num_channels, sample_rate):
        print("Num channels", num_channels, " sample_rate: ", sample_rate)
    
    # Process each data buffer. Data is a numpy array.
    def process(self, data):
        try:
            print("Num Samples received: ", data[0].size)
        except:
            pass
        
    # Called at start of acquisition
    def start_acquisition(self):
        pass
    
    # Called when acquisition is stopped
    def stop_acquisition(self):
        pass
    
    # Respond to TTL events
    def handle_ttl_event(self, state, sample_number, channel, line, stream_id):
        pass
    
    # Respond to spike events
    def handle_spike_event(self):
        pass
    
    # Called when recording starts
    def start_recording(self, recording_dir):
        pass
    
    # Called when recording stops
    def stop_recording(self):
        pass