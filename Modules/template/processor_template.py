import numpy as np
# Add additional imports here

class PyProcessor:
    
    def __init__(self, num_channels, sample_rate):
        """ 
        A new processor is initialized whenever the plugin settings are updated
        
        Parameters:
        num_channels (int): number of input channels in the selected stream.
        sample_rate (float): sample rate of the selected stream
        """
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

    def handle_ttl_event(self, source_node, channel, sample_number, line, state):
        """
        Handle each incoming ttl event.
        
        Parameters:
        source_node (int): id of the processor this event was generated from
        channel (str): name of the event channel
        sample_number (int): sample number of the event
        line (int): the line on which event was generated (0-255) 
        state (int): event state 0 (OFF) or 1 (ON)
        """
        pass
    
    def start_recording(self, recording_dir):
        """ 
        Called when recording starts

        Parameters:
        recording_dir (str): recording directory to be used by future record nodes.
        """
        pass
    
    def stop_recording(self):
        """ Called when recording stops """
        pass