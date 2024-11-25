#!/usr/bin/ksh
# loadPdmMaster.sh
# Author: Douglas S. Elder
# Date: 11/25/2024
# Description: This is a Korn Shell Linux script
# toLoad PDM Master data from XML file, sample.xml
# the output contains SQL INSERT statements
# to load the data into the PDM Master table
# Usage: loadPdmMaster.sh [filename]
# Example: loadPdmMaster.sh someotherXml.xml
# otherwise use the default file sample.xml
if (($#==0)) ; then
	INPUT=../data/sample.xml
else
	INPUT=$1
fi

SQL=../data/insertStatements.sql
LOG=../log/xmlparser.log
XMLTREE=../data/xmltree.txt
echo xmlparser $INPUT $SQL $LOG
# Run the xmlparser program send the screen output to tee 
# to display on the console and save to the $XMLTEE
xmlparser $INPUT $SQL $LOG | tee $XMLTREE
# run the SQL*Plus program using the output from xmlparser
sqlplus jitoper/NteiNJ29UI_ido@sdcdsdev @$SQL