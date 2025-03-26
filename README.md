# cpath (WIP)

Provides a filepath and dirpath class.

## usage

```c++
dirpath_t appdir = root->device_root("appdir");
dirpath_t datadir = appdir.down("bin"); // even if this folder doesn't exist, it will be 'inserted' but not 'created on disk'
filepath_t datafile = datadir.filename("data.txt");

// if the file doesn't exist, the stream will contain no data to read from
filestream_t datastream = open_filestream(datafile);
   // .. read from the datastream
datastream.close();

```
