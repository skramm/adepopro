adepopro: ADE(Campus)POstPROcess

(see https://www.adesoft.com/ade-campus/)

* Status: preliminar
* Language: C++ (C++11 actually)
* Licence: GPL v3
* Author: S. Kramm
* Hosting: https://github.com/skramm/adepopro
* Manual (fr-only): [online here](https://github.com/skramm/adepopro/blob/master/docs/man_fr.md)

Motivation: when your institution uses ADE Campus for planning, it is not very easy
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


Error handling: at present, in case of malformed input file, an assert is raised and execution stops.
This could be improved.


(1) - ADE-Campus provides an API that theorically could avoid this manual data extraction, however the documentation is too sparse to make it possible at present. Maybe that could be an improvement in the future.
