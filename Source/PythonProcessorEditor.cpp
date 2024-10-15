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
	utilButton = std::make_unique<UtilityButton>("...");
	utilButton->setFont (FontOptions (12.0f));
	utilButton->addListener(this);
	addAndMakeVisible(utilButton.get());

	setBounds(0, 0, 20, 20);
}


void ScriptPathButton::buttonClicked(Button* label)
{
	FileChooser chooseScriptDirectory("Please select a python script...", File(), "*.py");

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

	addSelectedStreamParameterEditor(Parameter::PROCESSOR_SCOPE, "current_stream", 20, 34);
	getParameterEditor("current_stream")->setLayout(ParameterEditor::Layout::nameHidden);
	getParameterEditor("current_stream")->setSize(150, 20);


	scriptPathLabel = std::make_unique<TextEditor>("Script Path Label");
	scriptPathLabel->setText("No Module Loaded", false);
	scriptPathLabel->setTooltip(scriptPathLabel->getText());
	scriptPathLabel->setMultiLine(false);
	scriptPathLabel->setReadOnly(true);
	scriptPathLabel->setCaretVisible(false);
	scriptPathLabel->setBounds(20, 65, 125, 20);
	scriptPathLabel->setJustification(Justification::centredLeft);
	addAndMakeVisible(scriptPathLabel.get());

	Parameter* scriptPathPtr = getProcessor()->getParameter("script_path");
	addCustomParameterEditor(new ScriptPathButton(scriptPathPtr), 150, 65);

	reloadButton = std::make_unique<UtilityButton>("Reload");
	reloadButton->setFont (FontOptions (13.0f));
	reloadButton->setBounds(60, 95, 70, 25);
	reloadButton->addListener(this);
	addAndMakeVisible(reloadButton.get());

}

void PythonProcessorEditor::startAcquisition()
{
	reloadButton->setEnabled(false);
}

void PythonProcessorEditor::stopAcquisition()
{
	reloadButton->setEnabled(true);
}

void PythonProcessorEditor::buttonClicked(Button* button)
{

	if (button == reloadButton.get())
	{
		pythonProcessor->reload();
	}

}

void PythonProcessorEditor::setPathLabelText(String text, String tooltip)
{
	scriptPathLabel->setText(text, false);
	scriptPathLabel->setTooltip(tooltip);
	scriptPathLabel->setCaretPosition(0);
}


