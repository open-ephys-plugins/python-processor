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

#include "PythonProcessorEditor.h"
#include "PythonProcessor.h"

ScriptPathButton::ScriptPathButton(Parameter* param) : ParameterEditor(param)
{
	utilButton = std::make_unique<UtilityButton>("...", Font(12));
	utilButton->addListener(this);
	addAndMakeVisible(utilButton.get());

	setBounds(0, 0, 20, 20);
}


void ScriptPathButton::buttonClicked(Button* label)
{
	FileChooser chooseScriptDirectory("Please select a python script...", File(CoreServices::getDefaultUserSaveDirectory()), "*.py");

	if (chooseScriptDirectory.browseForFileToOpen())
	{
		param->setNextValue(chooseScriptDirectory.getResult().getFullPathName());
	}
}

void ScriptPathButton::resized()
{
	utilButton->setBounds(0, 0, 20, 20);

}



PythonProcessorEditor::PythonProcessorEditor(PythonProcessor* parentNode) 
    : GenericEditor(parentNode)
{
	// Set ptr to parent
	pythonProcessor = parentNode;

    desiredWidth = 190;

	streamSelection = std::make_unique<ComboBox>("Stream Selector");
    streamSelection->setBounds(20, 32, 155, 20);
    streamSelection->addListener(this);
    addAndMakeVisible(streamSelection.get());

	scriptPathLabel = std::make_unique<Label>("Script Path Label", "No Module Loaded");
	scriptPathLabel->setTooltip(scriptPathLabel->getText());
	scriptPathLabel->setMinimumHorizontalScale(0.7f);
	scriptPathLabel->setBounds(20, 65, 135, 20);
	scriptPathLabel->setColour(Label::backgroundColourId, Colours::grey);
	scriptPathLabel->setColour(Label::backgroundWhenEditingColourId, Colours::white);
	scriptPathLabel->setJustificationType(Justification::centredLeft);
	addAndMakeVisible(scriptPathLabel.get());

	Parameter* scriptPathPtr = getProcessor()->getParameter("script_path");
	addCustomParameterEditor(new ScriptPathButton(scriptPathPtr), 160, 65);

	reloadButton = std::make_unique<UtilityButton>("Reload", Font(12));
	reloadButton->setBounds(60, 95, 80, 25);
	reloadButton->addListener(this);
	addAndMakeVisible(reloadButton.get());

}

void PythonProcessorEditor::updateSettings()
{
 
    streamSelection->clear();

	for (auto stream : getProcessor()->getDataStreams())
	{
        if (currentStream == 0)
            currentStream = stream->getStreamId();
        
		streamSelection->addItem(stream->getName(), stream->getStreamId());
	}

    if (streamSelection->indexOfItemId(currentStream) == -1)
    {
        if (streamSelection->getNumItems() > 0)
            currentStream = streamSelection->getItemId(0);
        else
            currentStream = 0;
    }
		
    if (currentStream > 0)
    {
        streamSelection->setSelectedId(currentStream, sendNotification);
    }
        
    
}

void PythonProcessorEditor::startAcquisition()
{
	streamSelector->setEnabled(false);
	scriptPathButton->setEnabled(false);
	reloadButton->setEnabled(false);
}

void PythonProcessorEditor::stopAcquisition()
{
	streamSelector->setEnabled(true);
	scriptPathButton->setEnabled(true);
	reloadButton->setEnabled(true);
}

void PythonProcessorEditor::buttonClicked(Button* button)
{

	if (button == reloadButton.get())
	{
		pythonProcessor->reload();
	}

}

void PythonProcessorEditor::comboBoxChanged(ComboBox* cb)
{
    if (cb == streamSelection.get())
    {
		currentStream = cb->getSelectedId();
        
        if (currentStream > 0)
        	getProcessor()->getParameter("current_stream")->setNextValue(currentStream);
    }

}


void PythonProcessorEditor::setPathLabelText(String s)
{
	scriptPathLabel->setText(s, dontSendNotification);
	scriptPathLabel->setTooltip(s);
}


