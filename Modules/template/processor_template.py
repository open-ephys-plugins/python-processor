import numpy as np
import oe_pyprocessor
# Add additional imports here

class PyProcessor:
    
    def __init__(self, processor, num_channels, sample_rate):
        """ 
        A new processor is initialized whenever the plugin settings are updated
        
        Parameters:
        processor (object): Python Processor class object used for adding events from python.
        num_channels (int): number of input channels in the selected stream.
        sample_rate (float): sample rate of the selected stream
        """
        print("Num Channels: ", num_channels, " | Sample Rate: ", sample_rate)
        # pass
    
    def process(self, data):
        """
        Process each incoming data buffer.
        
        Parameters:
        data - N x M numpy array, where N = num_channles, M = num of samples in the buffer.
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
        state (bool): event state True (ON) or False (OFF)
        """
        pass

    def handle_spike(self, source_node, electrode_name, num_channels, num_samples, sample_number, sorted_id, spike_data):
        """
        Handle each incoming spike.
        
        Parameters:
        source_node (int): id of the processor this spike was generated from
        electrode_name (str): name of the electrode
        num_channels (int): number of channels associated with the electrode type
        num_samples (int): total number of samples in the spike waveform 
        sample_number (int): sample number of the spike
        sorted_id (int): the sorted ID for this spike
        spike_data (numpy array): N x M numpy array, where N = num_channels & M = num_samples (read-only).
        """
        # print("SPIKE RECEIVED! Source Node:", source_node, ", Electrode name:", electrode_name, ", Num channels:", num_channels, ", Sample num:", sample_number, ", Sorted ID:", sorted_id)
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