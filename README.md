# GreatSort

A code for sorting raw data in MIDAS GREAT format and producing ROOT trees.

## Instructions and user guide

For the full details of how to use GreatSort, please take a look at the Wiki here on GitHub: https://github.com/lpgaff/GreatSort/wiki (doesn't exist yet)

Some basic intructions are included in the below README.

## Download

```bash
git clone https://github.com/lpgaff/GreatSort
```

## Compile

```bash
make clean
make
```


## Execute

```
great_sort
```
if you add the GreatSort/bin to your PATH variable. You can also add GreatSort/lib to your (DY)LD_LIBRARY_PATH too.

or
```
./bin/great_sort
```

If you start the code without any flags, it will launch the GUI. To run in batch mode, simple pass at least one file to the programme with the -i flag.

The input options are described below.

```
use great_sort with following flags:
        [-i           <vector<string>>: List of input files]
        [-o           <string        >: Output file for histogram file]
        [-s           <string        >: Settings file]
        [-c           <string        >: Calibration file]
        [-r           <string        >: Reaction file]
        [-f                           : Flag to force new ROOT conversion]
        [-e                           : Flag to force new event builder (new calibration)]
        [-source                      : Flag to define an source only run]
        [-spy                         : Flag to run the DataSpy]
        [-m           <int           >: Monitor input file every X seconds]
        [-p           <int           >: Port number for web server (default 8030)]
        [-d           <string        >: Output directory for sorted files]
        [-g                           : Launch the GUI]
        [-h                           : Print this help]
```

## Sorting Philosophy

The code can be run entirely with default values, meaning that none of the additional input files are required in order to sort the data.

### Step 1: Converter
Running great_sort with a list of input files, using the `-i` flag, will simple convert them to ROOT format.
This step produces one output file per input file, which has the name of the input file, appended with .root.

If a calibration file is added with the `-c` flag, the ADC data is calibrated for energy.
An example calibration file is included in the source of this code, including a description of the format.

A settings file can also be included with the `-s` flag to overwrite any of the defaults for the configuration of the electronics and detectors.
An example settings file is included in the source of this code, including a description of the format.

The ouptut file contains a single ROOT tree of the data and a series of diagnostic histograms and singles spectra.
If the output file already exists, `great_sort` will skip this step unless the `-f` flag is used.

If this is a calibration source run, declare the -source flag, which skips the following unnecessary stages of analysis and produces only the energy histograms.
The output file in this case will not have any tree data and will be appended with `_source.root`.

### Step 2: Time Sorting
In order to combine timestamp and ADC data, the time sorting step needs to be performed.
This step is contained within the conversion step as long as you are not using the `-source` flag and time-sorted data will then be written to the `great_sort` tree.
This is potentially the slowest part of the process if there is a lot of data out of order due to the number of I/O operations.

### Step 3: Event Builder
The next step is the event builder, which runs if the `-e` flag is used, or automatically if a new file has been converted.
This uses the calibrated, time sorted data from the previous step to produce one output file per input, appended with `_events.root`.
The same settings file from the Converter step is reused for the same parameters, plus the length of the build window (default 3 µs).
There is a plan to have these setting written in to the ROOT file itself, so the file doesn't need to be passed again, but this isn't the case yet.

Events are built according to physical detectors or TAC units in to separate classes.
This format is all contained within the GreatEvts class, which you can browse to see which functions are available.
If you open the output file and want to draw directly from the `evt_tree`, you can load the library with `gSystem->Load("/path/to/GreatSort/lib/libgreat_sort.so")` or by adding it to your .rootlogon.C.
Then you have access to all the member functions like `gamma_event->GetEnergy()`, `tac_event->GetTdiff()` etc.

### Step 4: Histogramming
Finally a bunch of standard physics histograms are built using input from the reaction file, given with the `-r` flag.
An example reaction file is included in the source of this code, including a description of the format.

The code will now chain together all of the event trees from the previous step to produce a single output file given with the `-o` flag.
The default file name will be the first input file appended with `_hists.root`.

Users can edit this code as they please, producing their own plots.


## Dependencies

You also need to have ROOT installed with a minumum standard that your compiler supports C++14.
At the moment it works with v5 or v6, but let me know of any problems.
ROOT must be built with GSL library support, otherwise known as MathMore.
To check this is true, you can type: `root-config --has-mathmore` and hope the response is `yes`.
If it isn't, you will need to install the GSL libraries and reconfigure/rebuild ROOT.
