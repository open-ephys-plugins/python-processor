import numpy as np
# Add additional imports here

class PyProcessor:
    
    def __init__(self, num_channels, sample_rate):
        """ A new processor is initialized whenever the plugin settings are updated """
        print("Num Channels: ", num_channels, " | Sample Rate: ", sample_rate)
        # pass
    
    def process(self, data):
        """
        Process each incoming data buffer.
        
        Parameters:
        data - numpy array.
        """
        try:
            pass
        except:
            pass
        
    def start_acquisition(self):
        """ Called at start of acquisition """
        pass
    
    def stop_acquisition(self):
        """ Called when acquisition is stopped """
        pass
    
    def start_recording(self, recording_dir):
        """ 
        Called when recording starts

        Parameters:
        recording_dir - recording directory to be used by future record nodes.
        """
        pass
    
    def stop_recording(self):
        """ Called when recording stops """
        pass