#!/usr/bin/ksh
INPUT=../data/sample.xml
SQL=../data/insertStatements.sql
LOG=../log/xmlparser.log
echo xmlparser $INPUT $SQL $LOG
xmlparser $INPUT $SQL $LOG
sqlplus jitoper/NteiNJ29UI_ido@sdcdsdev @$SQL
