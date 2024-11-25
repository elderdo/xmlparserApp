#!/bin/csh
# dos2unit.sh
# Convert DOS line endings to UNIX line endings
# Usage: dos2unix.sh filename
# Example: dos2unix.sh loadPdmMaster.sh
# To view cr in a file use od -c filename
# A carriage return is represented by \r
# A line feed is represented by \n
sed 's/\r/$//' $1 > /tmp/$1
mv /tmp/$1 $1