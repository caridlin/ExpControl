How to Compile ExperimentContrik.cws
==============

* Install AdWin
* Under Instrument-->Search Directories... add "includes" if it is not there already
* Under Edit-->Add Files to Project-->Library (*.lib)... add "lib/CORE_RL_magick_.lib" and "lib/CORE_RL_wand_.lib" if they are not there already
* Under Edit-->Add Files to Project-->Instrument (*.fp)... add "%CVIDIR%/toolslib/datasock/dataskt.fp" and "%CVIDIR%/toolslib/toolbox/inifile.fp"

==============
* After compilation, the files are saved in ./cvibuild.ExperimentControl folder
* Copy the .exe file to ./EXE folder and run