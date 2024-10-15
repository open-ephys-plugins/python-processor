/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2022 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PYTHONPROCESSOR_H_DEFINED
#define PYTHONPROCESSOR_H_DEFINED

#include <ProcessorHeaders.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>

#include <queue>

#include "PythonProcessorEditor.h"

namespace py = pybind11;

class PythonProcessor : public GenericProcessor
{

private:

	/** Manage all python objects with raw pointers, so they
	can be explicitly deleted with the GIL held*/

	/** Custom python module */
	py::module_* pyModule;

	/** Instance of user-defined python class*/
	py::object* pyObject;

	/** File path to python script */
	String scriptPath;

	/** Name of module */
	std::string moduleName;

	/** True if there is an module loaded with no exceptions*/
	bool moduleReady;

	/** Pointer to editor */
	PythonProcessorEditor* editorPtr;

	/** Stream to process*/
	uint16 currentStream;

	std::map<uint16, EventChannel*> localEventChannels;

	struct StringTTL
    {
        int eventLine;
		bool state;
    };

	std::queue<StringTTL> TTLQueue;
	CriticalSection TTLqueueLock;

	/**Check whether data stream exists */
	bool streamExists(uint16 streamId);

public:
	/** The class constructor, used to initialize any members. */
	PythonProcessor();

	/** The class destructor, used to deallocate memory */
	~PythonProcessor();

	/** Registers the parameters of the processor */
	void registerParameters() override;

	/** If the processor has a custom editor, this method must be defined to instantiate it. */
	AudioProcessorEditor* createEditor() override;

	/** Called every time the settings of an upstream plugin are changed.
		Allows the processor to handle variations in the channel configuration or any other parameter
		passed through signal chain. The processor can use this function to modify channel objects that
		will be passed to downstream plugins. */
	void updateSettings() override;

	/** Allows the processor to load initialize the Python interpreter. */
	void initialize(bool signalChainIsLoading) override;

	/** Defines the functionality of the processor.
		The process method is called every time a new data buffer is available.
		Visualizer plugins typically use this method to send data to the canvas for display purposes */
	void process(AudioBuffer<float>& buffer) override;

	/** Handles events received by the processor
		Called automatically for each received event whenever checkForEvents() is called from
		the plugin's process() method */
	void handleTTLEvent(TTLEventPtr event) override;

	/** Handles spikes received by the processor
		Called automatically for each received spike whenever checkForEvents(true) is called from
		the plugin's process() method */
	void handleSpike(SpikePtr spike) override;

	// /** Handles broadcast messages sent during acquisition
	// 	Called automatically whenever a broadcast message is sent through the signal chain */
	// void handleBroadcastMessage(String message) override;

	/**Add events from python to C++. Bound to Python as an embedded module*/
	void addPythonEvent(int line, bool state);

	void triggerTTLEvent(StringTTL TTLmsg, juce::int64 sampleNum);

	/** Called at the start of acquisition.*/
	bool startAcquisition() override;

	/** Called when acquisition is stopped.*/
	bool stopAcquisition() override;

	/** Called when recording starts */
	void startRecording() override;

	/** Called when recording stops */
	void stopRecording() override;

	/** Called whenever a parameter's value is changed */
	void parameterValueChanged(Parameter* param) override;

	/** Allows the processor to initialize the Python interpreter. */
	bool initInterpreter(String pythonHome = String());

	/** Imports the python script from scriptPath and rebuilds the processor object. Returns false
		if the import fails*/
	bool importModule();

	/** Reloads the current python module if one is loaded */
	void reload();
	
	/** Initializes the python script by calling __init__() */
	void initModule();

	/** Deals with python exceptions (print and turn off module for now) */
	void handlePythonException(const String& title, const String& msg, py::error_already_set e);

};

#endif