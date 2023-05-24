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

#ifndef PYTHONPROCESSOREDITOR_H_DEFINED
#define PYTHONPROCESSOREDITOR_H_DEFINED

#include <EditorHeaders.h>

class PythonProcessor;

/** Custom parameter editor for changing the script path*/
class ScriptPathButton : public ParameterEditor,
	public Button::Listener
{
public:

	/** Constructor */
	ScriptPathButton(Parameter* param);

	/** Destructor*/
	virtual ~ScriptPathButton() { }

	/** Respond to trigger button clicks*/
	void buttonClicked(Button* label) override;

	/** Update view of the parameter editor component*/
	void updateView() {};

	/** Sets component layout*/
	void resized() override;

private:
	std::unique_ptr<UtilityButton> utilButton;
};



class PythonProcessorEditor :
	public GenericEditor,
	public Button::Listener,
	public ComboBox::Listener
{
public:

	/** Constructor */
	PythonProcessorEditor(PythonProcessor* parentNode);

	/** Destructor */
	~PythonProcessorEditor() { }

	/** Called when underlying settings are updated */
	void updateSettings() override;

	/** Called just prior to the start of acquisition, to allow custom commands. */
	void startAcquisition() override;

	/** Called after the end of acquisition, to allow custom commands .*/
	void stopAcquisition() override;

	/** Respond to button clicks*/
	void buttonClicked(Button* button) override;

	/** Called when a ComboBox changes*/
	void comboBoxChanged(ComboBox* comboBox) override;

	/** Sets the text & tooltip of the path label */
	void setPathLabelText(String text, String tooltip);

private:

	PythonProcessor* pythonProcessor;

	std::unique_ptr<Label> scriptPathLabel;
	std::unique_ptr<Button> scriptPathButton;
	std::unique_ptr<Button> reloadButton;
	std::unique_ptr<ComboBox> streamSelection;

	uint16 currentStream = 0;


	/** Generates an assertion if this class leaks */
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PythonProcessorEditor);
};

#endif // PythonProcessorEDITOR_H_DEFINED