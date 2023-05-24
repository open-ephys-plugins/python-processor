# Python Processor

![Python Processor Editor](https://open-ephys.github.io/gui-docs/_images/pythonprocessor-01.png)

Modifies incoming continuous data and handles events and spikes using custom code written in Python.
## Installation

The Python Processor plugin is not included by default in the Open Ephys GUI. To install, use ctrl-P to access the Plugin Installer, browse to the "Processor plugin" plugin, and click the "Install" button.

## Usage

Instructions for using the Python Processor plugin are available [here](https://open-ephys.github.io/gui-docs/User-Manual/Plugins/Python-Processor.html).

## Building from source

First, follow the instructions on [this page](https://open-ephys.github.io/gui-docs/Developer-Guide/Compiling-the-GUI.html) to build the Open Ephys GUI.

**Important:** This plugin is intended for use with the GUI version 0.6.0 and higher.

Be sure to clone the `python-processor` repository into a directory at the same level as the `plugin-GUI`, e.g.:
 
```
Code
├── plugin-GUI
│   ├── Build
│   ├── Source
│   └── ...
├── OEPlugins
│   └── python-processor
│       ├── Build
│       ├── Source
│       └── ...
```
After that, you also need to initialize the [pybind11](https://pybind11.readthedocs.io/en/stable/) submodule required for building the plugin by entering the entering this command inside the python-processor plugin directory.

```bash
git submodule update --init
```

**Note:** Minimum Python version required for building the Python Processor plugin is `3.8`. Refer to the plugin docs to understand how to setup a Conda environment for this.

### Windows

**Requirements:** [Visual Studio](https://visualstudio.microsoft.com/) and [CMake](https://cmake.org/install/)

From the `Build` directory, generate the build files by entering:

```bash
cmake -G "Visual Studio 17 2022" -A x64 -DPython_ROOT_DIR="path/to/python_home" ..
```

Next, launch Visual Studio and open the `OE_PLUGIN_python-processor.sln` file that was just created. Select the appropriate configuration (Debug/Release) and build the solution.

Selecting the `INSTALL` project and manually building it will copy the `.dll` and any other required files into the GUI's `plugins` directory. The next time you launch the GUI from Visual Studio, the Python Processor plugin should be available.


### Linux

**Requirements:** [CMake](https://cmake.org/install/)

From the `Build` directory, enter:

```bash
cmake -G "Unix Makefiles" -DPython_ROOT_DIR="path/to/python_home" ..
cd Debug
make -j
make install
```

This will build the plugin and copy the `.so` file into the GUI's `plugins` directory. The next time you launch the GUI compiled version of the GUI, the Python Processor plugin should be available.


### macOS

**Requirements:** [Xcode](https://developer.apple.com/xcode/) and [CMake](https://cmake.org/install/)

From the `Build` directory, enter:

```bash
cmake -G "Xcode" -DPython_ROOT_DIR="path/to/python_home" ..
```

Next, launch Xcode and open the `python-processor.xcodeproj` file that now lives in the “Build” directory.

Running the `ALL_BUILD` scheme will compile the plugin; running the `INSTALL` scheme will install the `.bundle` file to `/Users/<username>/Library/Application Support/open-ephys/plugins-api8`. The Python Processor plugin should be available the next time you launch the GUI from Xcode.


**Note:** `Python_ROOT_DIR` is the path to the Python installation on your system you want to use. E.g. `${HOME}/miniconda3/envs/ENVNAME`.



