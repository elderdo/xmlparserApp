#ifndef PDM_MASTER_H
#define PDM_MASTER_H
#include "xml_parser.h"

typedef struct PdmMaster {
    char* itemNo;
    char* cageCode;
    char* ecl;
    char* bemsId;
    int* version;
    char* releaseStatus;
    char* createdDate;
    char* modifyDate;
    char* familyCode;
    char* description;
    char* statusDate;
    char* project;
    char* owner;
    char* comments;
} PdmMaster;

PdmMaster* createTcePdmMasterCopy();
int* getVersion(Node* product);
void extractValuesFromAttributes(Node* product, PdmMaster* pdmMaster);
char* toDate(const char* date);
char* createSql(PdmMaster* pdmMaster);
char* makeInsert(Node* product);

#endif // PDM_MASTER_H

