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



#include <filesystem>

#include "PythonProcessor.h"
#include "PythonProcessorEditor.h"

namespace py = pybind11;


PythonProcessor::PythonProcessor()
    : GenericProcessor("Python Processor")
{
    if(Py_IsInitialized() > 0)
    {
        LOGC("[***Before]Python Interpreter already initialized from: ", String(Py_GetPath()));

    }
    else
    {

        AlertWindow::showMessageBox (AlertWindow::InfoIcon,
                                    "Select Python installation path",
                                    "To use the plugin you need provide the path to your preferred Python installation. "
                                    "Please select the path where your python dll is located in the next step.");
                                    
        FileChooser chooser ("Please select the path to your preferred Python installation",
                            { File::getSpecialLocation(File::userHomeDirectory) });

        if (chooser.browseForDirectory())
        {
            auto targetFolder = chooser.getResult();
            if (targetFolder == File{})
                return;
            
            Py_SetPythonHome(targetFolder.getFullPathName().toWideCharPointer());
        }
        
        
        py::initialize_interpreter();
        {
            py::gil_scoped_acquire acquire;

            if(Py_IsInitialized() > 0)
            {
                LOGD("Python Interpreter initialized successfully from: ", String(Py_GetPythonHome()));
                LOGC("Python interpreter build info: ", String(Py_GetBuildInfo()));
            }
        }
    }
    
    pyModule = NULL;
    pyObject = NULL;
    moduleReady = false;
    scriptPath = "";
    moduleName = "";
    editorPtr = NULL;

    addStringParameter(Parameter::GLOBAL_SCOPE, "script_path", "Path to python script", String());
}

PythonProcessor::~PythonProcessor()
{
    {
        delete pyModule;
        delete pyObject;
        py::gil_scoped_release release;
    }
    py::finalize_interpreter();
}


AudioProcessorEditor* PythonProcessor::createEditor()
{
    editor = std::make_unique<PythonProcessorEditor>(this);
    editorPtr = (PythonProcessorEditor*) editor.get();
    return editor.get();
}


void PythonProcessor::updateSettings()
{

    int numContinuousChannels = continuousChannels.size();
    // int numEventChannels = eventChannels.size();
    // int numSpikeChannels = spikeChannels.size();

    float sampleRate = 0;
    if (numContinuousChannels > 0) {
        sampleRate = continuousChannels.getFirst()->getSampleRate();
    }
    

    if (moduleReady)
    {
        // py::gil_scoped_acquire acquire;
        if (pyObject)
        {
            delete pyObject;
            pyObject = NULL;
        }

        try {
            pyObject = new py::object(pyModule->attr("PyProcessor")(numContinuousChannels, sampleRate));
        }

        catch (py::error_already_set& e) {
            handlePythonException(e);
        }
    }
}

void PythonProcessor::initialize(bool signalChainIsLoading)
{
    
}

void PythonProcessor::process(AudioBuffer<float>& buffer)
{
    checkForEvents(true);


    if (moduleReady)
    {

        for (auto stream : getDataStreams())
        {

            if ((*stream)["enable_stream"])
            {

                const uint16 streamId = stream->getStreamId();

                const int numSamples = getNumSamplesInBlock(streamId);
                const int numChannels = stream->getChannelCount();

                // Only for blocks bigger than 0
                // py::gil_scoped_acquire acquire;
                if (numSamples > 0) 
                {
                    py::array_t<float> numpyArray = py::array_t<float>({ numChannels, numSamples });

                    // Read into numpy array
                    for (int i = 0; i < numChannels; ++i) {
                        int globalChannelIndex = getGlobalChannelIndex(stream->getStreamId(), i);

                        const float* bufferChannelPtr = buffer.getReadPointer(globalChannelIndex);
                        float* numpyChannelPtr = numpyArray.mutable_data(i, 0);
                        memcpy(numpyChannelPtr, bufferChannelPtr, sizeof(float) * numSamples);
                    }

                    // Call python script on this block

                    try {
                        pyObject->attr("process")(numpyArray);
                    }
                    catch (py::error_already_set& e) {
                        handlePythonException(e);
                    }


                    // // Write from numpy array?
                    // for (int i = 0; i < numChannels; ++i) {
                    //     int globalChannelIndex = getGlobalChannelIndex(stream->getStreamId(), i);

                    //     float* bufferChannelPtr = buffer.getWritePointer(globalChannelIndex);
                    //     const float* numpyChannelPtr = numpyArray.data(i, 0);
                    //     memcpy(bufferChannelPtr, numpyChannelPtr, sizeof(float) * numSamples);
                    // }
                }
                // py::gil_scoped_release release;
            
            }
        }
    }
}

void PythonProcessor::handleTTLEvent(TTLEventPtr event)
{
    // Get ttl info
    const int state = event->getState() ? 1 : 0;
    const int64 sampleNumber = event->getSampleNumber();
    const int channel = event->getChannelIndex();
    const uint8 line = event->getLine();
    const uint16 streamId = event->getStreamId();

    // Give to python
    // py::gil_scoped_acquire acquire;

    try {
        pyObject->attr("handle_ttl_event")(state, sampleNumber, channel, line, streamId);
    }
    catch (py::error_already_set& e) {
        handlePythonException(e);
    }
}


void PythonProcessor::handleSpike(SpikePtr event)
{
    // py::gil_scoped_acquire acquire;
    try {
        pyObject->attr("handle_spike_event")();
    }
    catch (py::error_already_set& e) {
        handlePythonException(e);
    }
}


void PythonProcessor::handleBroadcastMessage(String message)
{

}


void PythonProcessor::saveCustomParametersToXml(XmlElement* parentElement)
{

}


void PythonProcessor::loadCustomParametersFromXml(XmlElement* parentElement)
{

}

bool PythonProcessor::startAcquisition() 
{
    if (moduleReady)
    {
        // py::gil_scoped_acquire acquire;

        try {
            pyObject->attr("start_acquisition")();
        }
        catch (py::error_already_set& e) {
            handlePythonException(e);
        }
        return true;
    }
    return false;
}

bool PythonProcessor::stopAcquisition() {
    if (moduleReady)
    {
        // py::gil_scoped_acquire acquire;
        try {
            pyObject->attr("stop_acquisition")();
        }
        catch (py::error_already_set& e) {
            handlePythonException(e);
        }
        return true;
    }
    return false;
}

void PythonProcessor::startRecording() {
    String recordingDirectory = CoreServices::getRecordingDirectoryName();

    // py::gil_scoped_acquire acquire;
    try {
        pyObject->attr("start_recording")(recordingDirectory.toRawUTF8());
    }
    catch (py::error_already_set& e) {
        handlePythonException(e);
    }
}



void PythonProcessor::stopRecording() {
    // py::gil_scoped_acquire acquire;
    try {
        pyObject->attr("stop_recording")();
    }
    catch (py::error_already_set& e) {
        handlePythonException(e);
    }
}



void PythonProcessor::parameterValueChanged(Parameter* param)
{
    if (param->getName().equalsIgnoreCase("script_path")) 
    {
        scriptPath = param->getValueAsString();
        importModule();
        updateSettings();
    }
}


bool PythonProcessor::importModule()
{

    if (scriptPath == "")
    {
        return false;
    }

    LOGC("Importing Python module from ", scriptPath.toRawUTF8());

    try
    {
        LOGC("Acquiring scope...");
        // py::gil_scoped_acquire acquire;

        LOGC("Clearing previoud module info...");
        // Clear for new class
        if (pyModule)
        {
            delete pyModule;
            pyModule = NULL;
        }

        LOGC("Getting module info...");
        // Get module info (change to get from editor)
        std::filesystem::path path(scriptPath.toRawUTF8());
        std::string moduleDir = path.parent_path().string();
        std::string fileName = path.filename().string();
        moduleName = fileName.substr(0, fileName.find_last_of("."));
        

        LOGC("Adding module to sys.path...");
        // Add module directory to sys.path
        py::module_ sys = py::module_::import("sys");
        py::object append = sys.attr("path").attr("append");
        append(moduleDir);

        pyModule = new py::module_(py::module_::import(moduleName.c_str()));

        LOGC("Successfully imported ", moduleName);

        editorPtr->setPathLabelText(moduleName);
        moduleReady = true;
        return true;
    }

    catch (std::exception& exc)
    {
        LOGC("Failed to import Python module.");
        LOGC(exc.what());

        editorPtr->setPathLabelText("No Module Loaded");
        moduleReady = false;
        return false;
    } catch (py::error_already_set &e)
    {
        handlePythonException(e);
    }
}

void PythonProcessor::reload() 
{
    // py::gil_scoped_acquire acquire;

    if (pyModule)
    {
        LOGC("Reloading module...");
        try
        {
            pyModule->reload();
        }
        catch (py::error_already_set& e) {
            handlePythonException(e);
            return;
        }
        
        LOGC("Module successfully reloaded");
        moduleReady = true;
        editorPtr->setPathLabelText(moduleName);
    }
    else 
    {
        LOGC("There is no module to reload");
    }
}

void PythonProcessor::handlePythonException(py::error_already_set e)
{
    LOGC("Python Exception:\n", e.what());
    moduleReady = false;
    editorPtr->setPathLabelText("(ERROR) " + moduleName);
}