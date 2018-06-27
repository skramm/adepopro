### adepopro: ADE(Campus)POstPROcess

(see https://www.adesoft.com/ade-campus/)

* Status: beta
* Language: C++ (C++11 actually)
* Licence: GPL v3
* Author: S. Kramm
* Hosting: https://github.com/skramm/adepopro
* Manual (fr-only): [online here](https://github.com/skramm/adepopro/blob/master/docs/man_fr.md)

This is a tool related to the software ADE Campus, if you are not a user of this, you probably will not be concerned.<br>
See https://www.adesoft.com/ade-campus/ for details.

#### Motivation:
When your institution uses ADE Campus for planning, it is not very easy
(even for the person in charge for the planning)
to access to some basic information.
Thus this tool is designed to do some post process, to extract some information such as:
 * what are the modules in which a given instructor has some teaching ?
 * what are the instructors that give teaching in a given course module ?
 * over how many weeks and days does a given module spread ?
 * how many days is an instructor present ?

This is a command-line tool, thus to use it you will need minimal knowledge on how to handle this. It will take as input  a raw data CSV file. This file needs to be **generated manually** from the ADE Campus client (1).
<br>
This program will generate from the input file several .csv and text files holding the data described above.
You can afterward load these in your favorite spreadsheet application and format them as required.

No guarantee is given on its usefulness, as it is targeted on our specific organization.

Usage: this is only a C++ source file, thus you need to compile it before using it.
Once this is done, the program is launched with:
```
adepopro <csv_input_file>
```
To give it a try, a sample data file is provided, so once built, you can enter:
```
adepopro -s sample_input.csv
```
and checkout the 4 files that it will generate.

#### Building:

You will need a C++11 compliant compiler installed on your machine.
This program also has dependencies on two Boost libraries:
* boost::format
* boost::property_tree

* If you have the CodeBlocks IDE installed on your machine, you should be able to build by opening the project file ```adepopro.cbp``` and hitting F9
* If you have GnuMake, you can build the app by entering:
```
make
```
* As the program is contained in a single cpp file, you can also enter the following in a shell:
```
g++ -std=c++11 -o adepopro adepopro.cpp
```

**Error handling**: most of the errors are handled with exceptions, an error message is provided so that the user should be able to correct the error.
Please [post issue on Github](https://github.com/skramm/adepopro/issues) in case of trouble.


(1) - ADE-Campus provides an API that theorically could avoid this manual data extraction,
however the documentation is too sparse to make it possible at present. Maybe that could be an improvement in the future.
