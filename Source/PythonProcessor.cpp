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
    pyModule = NULL;
    pyObject = NULL;
    moduleReady = false;
    scriptPath = "";
    moduleName = "";
    editorPtr = NULL;
    currentStream = 0;

    addStringParameter(Parameter::GLOBAL_SCOPE, "python_home", "Path to python home", String());
    addStringParameter(Parameter::GLOBAL_SCOPE, "script_path", "Path to python script", String(), true);
    addIntParameter(Parameter::GLOBAL_SCOPE,
        "current_stream", "Currently selected stream",
        0, 0, 200000);
}

PythonProcessor::~PythonProcessor()
{
    if(Py_IsInitialized() > 0)
    {
        {
            delete pyModule;
            delete pyObject;
            py::gil_scoped_release release;
        }
        py::finalize_interpreter();
    }
}


AudioProcessorEditor* PythonProcessor::createEditor()
{
    editor = std::make_unique<PythonProcessorEditor>(this);
    editorPtr = (PythonProcessorEditor*) editor.get();
    return editor.get();
}


void PythonProcessor::updateSettings()
{
    if (getDataStreams().size() == 0)
        currentStream = 0;
}

void PythonProcessor::initialize(bool signalChainIsLoading)
{
    if(!signalChainIsLoading
       && Py_IsInitialized() == 0)
    {
        if( !initInterpreter() )
            LOGE("Unable to initialize python interpreter!!");
    }
}

void PythonProcessor::process(AudioBuffer<float>& buffer)
{
    if( !moduleReady )
        return;

    checkForEvents(true);

    for (auto stream : getDataStreams())
    {

        if (stream->getStreamId() == currentStream)
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
                pyObject->attr("process")(numpyArray);


                // // Write back from numpy array
                for (int i = 0; i < numChannels; ++i) {
                    int globalChannelIndex = getGlobalChannelIndex(stream->getStreamId(), i);

                    float* bufferChannelPtr = buffer.getWritePointer(globalChannelIndex);
                    const float* numpyChannelPtr = numpyArray.data(i, 0);
                    memcpy(bufferChannelPtr, numpyChannelPtr, sizeof(float) * numSamples);
                }
            }
            // py::gil_scoped_release release;
        
        }
    }
}

void PythonProcessor::handleTTLEvent(TTLEventPtr event)
{
    if (event->getStreamId() == currentStream)
    {
        // Get ttl info
        auto chanInfo = event->getChannelInfo();
        auto channelName = chanInfo->getName();
        const int sourceNodeId = chanInfo->getSourceNodeId();
        const int64 sampleNumber = event->getSampleNumber();
        const uint8 line = event->getLine();
        const int state = event->getState() ? 1 : 0;

        // Give to python
        // py::gil_scoped_acquire acquire;

        try {
            pyObject->attr("handle_ttl_event")(sourceNodeId, channelName.toRawUTF8(), sampleNumber, line, state);
        }
        catch (py::error_already_set& e) {
            handlePythonException(e);
        }
    }
}


// void PythonProcessor::handleSpike(SpikePtr event)
// {
//     // py::gil_scoped_acquire acquire;
//     try {
//         pyObject->attr("handle_spike_event")();
//     }
//     catch (py::error_already_set& e) {
//         handlePythonException(e);
//     }
// }


// void PythonProcessor::handleBroadcastMessage(String message)
// {

// }

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
    }
    return true;
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

bool PythonProcessor::streamExists(uint16 streamId)
{
    for (auto stream : getDataStreams())
    {
        if (stream->getStreamId() == streamId)
            return true;
    }
    
    return false;
}

void PythonProcessor::parameterValueChanged(Parameter* param)
{
    if (param->getName().equalsIgnoreCase("script_path")) 
    {
        // Initialize python interpreter if not already
        if(Py_IsInitialized() == 0)
            initInterpreter(getParameter("python_home")->getValueAsString());

        String newScriptPath = param->getValueAsString();

        if( !scriptPath.equalsIgnoreCase(newScriptPath) )
        {
            scriptPath = newScriptPath;
            importModule();
            initModule();
        }
    }
    else if (param->getName().equalsIgnoreCase("python_home")) 
    {
        initInterpreter(param->getValueAsString());
    }
    else if (param->getName().equalsIgnoreCase("current_stream"))
    {
        uint16 candidateStream = (uint16) (int) param->getValue();

        if(streamExists(candidateStream)
           && currentStream != candidateStream)
        {
            currentStream = candidateStream;

            if(moduleReady)
                initModule();
        }
    }
}


bool PythonProcessor::initInterpreter(String pythonHome)
{
    File targetFolder;

    if(Py_IsInitialized() > 0)
    {
        LOGD("Python Interpreter already initialized from: ", String(Py_GetPythonHome()));
        return true;
    }

    if(pythonHome == String())
    {
        AlertWindow::showMessageBox (AlertWindow::InfoIcon,
                                    "Select Python Home path",
                                    "To use the plugin you need provide the path to your preferred Python installation. "
                                    "Please select the folder where your python is located in the next step.");
                                    
        FileChooser chooser ("Please select the path to your preferred Python installation",
                            { File::getSpecialLocation(File::userHomeDirectory) });

        if (chooser.browseForDirectory())
        {
            targetFolder = chooser.getResult();
            if (targetFolder.getFullPathName().isEmpty())
            {
                return false;
            }
        }
        else
        {
            return false;
        }

    }
    else
    {
        targetFolder = File(pythonHome);
    }

    try
    {
        getParameter("python_home")->currentValue = targetFolder.getFullPathName();
        Py_SetPythonHome(targetFolder.getFullPathName().toWideCharPointer());

        String pythonPaths = String();

    #if JUCE_WINDOWS
        pythonPaths = targetFolder.getFullPathName()
                        + ";"
                        + targetFolder.getChildFile("lib").getFullPathName()
                        + ";"
                        + targetFolder.getChildFile("lib/site-packages").getFullPathName()
                        + ";"
                        + targetFolder.getChildFile("DLLs").getFullPathName();
        
        Py_SetPath(pythonPaths.toWideCharPointer());
    #endif

        py::initialize_interpreter();
        {
            py::gil_scoped_acquire acquire;

            if(Py_IsInitialized() > 0)
                LOGC("Python Interpreter initialized successfully! Python Home: ", String(Py_GetPythonHome()));
            
            py::module sys = py::module::import("sys");
            py::list path = sys.attr("path");

            LOGD("Python sys paths:")
            // Check if the path was added successfully
            for (auto p : path) {
                LOGD(p.cast<std::string>());
            }
        }

        return true;
    }
    catch(std::exception& e)
    {   
        PyErr_Print();
        LOGE(e.what());
        return false;
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
        // Clear for new class
        if (pyModule)
        {
            delete pyModule;
            pyModule = NULL;
        }

        // Get module info (change to get from editor)
        std::filesystem::path path(scriptPath.toRawUTF8());
        std::string moduleDir = path.parent_path().string();
        std::string fileName = path.filename().string();
        moduleName = fileName.substr(0, fileName.find_last_of("."));
        

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
        initModule();
    }
    else 
    {
        LOGC("There is no module to reload");
    }
}

void PythonProcessor::initModule()
{
    int numChans = 0;
    float sampleRate = 0;
    
    if(currentStream > 0)
    {
        numChans = getDataStream(currentStream)->getChannelCount();
        sampleRate = getDataStream(currentStream)->getSampleRate();
    }
    else
    {
        return;
    }

    if (moduleReady)
    {
        LOGC("Initializing module with ", numChans, " channels at ", sampleRate, " Hz");
        // py::gil_scoped_acquire acquire;
        if (pyObject)
        {
            delete pyObject;
            pyObject = NULL;
        }

        try {
            pyObject = new py::object(pyModule->attr("PyProcessor")(numChans, sampleRate));
        }

        catch (std::exception& exc)
        {
            LOGC("Failed to initialize Python module.");
            LOGC(exc.what());

            editorPtr->setPathLabelText("(ERROR) " + moduleName);
            moduleReady = false;
        }
    }
}

void PythonProcessor::handlePythonException(py::error_already_set e)
{
    LOGE("Python Exception:", e.what());
    moduleReady = false;
    editorPtr->setPathLabelText("(ERROR) " + moduleName);
}