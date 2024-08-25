# cpath

Provides a filepath and dirpath class.

# useage

```c++
dirpath_t appdir = root->device_root("appdir");
dirpath_t bins = appdir.down("bin"); // even if this folder doesn't exist, it will be 'registered' but not 'created on disk'
filepath_t coolexe = bins.file("cool.exe");
npath::istring_t datafilename; npath::istring_t dataextension;
root->filename("data.txt", datafilename, dataextension);
filepath_t datafilepath = bins.file(datafilename, dataextension);
filestream_t datastream = open_filestream(datafilepath); // if this file doesn't exist, it will be created
   // .. read from the datastream
datastream.close();
```
