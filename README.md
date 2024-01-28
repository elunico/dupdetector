# Duplicate Detector

Duplicate Detector is a CLI program that can be used to recursively find duplicate files in folders. The program uses SHA256 hashing to determine when the content of two files is the same (regardless of filename). The program can output the discovered duplicates and, optionally, automatically remove all but one of the duplicates.

## Use 

The program can be downloaded and make with CMake and then run on a directory. Beware the `-r` option which will remove files!!
