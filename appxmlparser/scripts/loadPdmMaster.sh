#!/usr/bin/ksh
INPUT=../data/sample.xml
SQL=../data/insertStatements.sql
LOG=../log/xmlparser.log
XMLTREE=../data/xmltree.txt
echo xmlparser $INPUT $SQL $LOG
xmlparser $INPUT $SQL $LOG | tee $XMLTREE
sqlplus jitoper/NteiNJ29UI_ido@sdcdsdev @$SQL