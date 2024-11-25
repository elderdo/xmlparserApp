#!/bin/csh
# loadPdmMaster.csh
# Author: Douglas S. Elder
# Date: 11/25/2024
# Description: This is a C Shell script to load PDM Master data from an XML file, sample.xml
# The output contains SQL INSERT statements to load the data into the PDM Master table
# Usage: loadPdmMaster.csh [filename]
# Example: loadPdmMaster.csh someotherXml.xml
# Otherwise use the default file sample.xml

if ($#argv == 0) then
	set INPUT = ../data/sample.xml
else
	set INPUT = $argv[1]
endif

set SQL = ../data/insertStatements.sql
set LOG = ../log/xmlparser.log
set XMLTREE = ../data/xmltree.txt	

echo "xmlparser $INPUT $SQL $LOG"
# Run the xmlparser program send the screen output to tee 
# to display on the console and save to the $XMLTEE
xmlparser $INPUT $SQL $LOG | tee $XMLTREE
# Run the SQL*Plus program using the output from xmlparser
sqlplus jitoper/NteiNJ29UI_ido@sdcdsdev @$SQL