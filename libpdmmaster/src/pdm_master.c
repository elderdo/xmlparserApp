#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pdm_master.h"
#include "xml_parser.h"
#include "log.h"


PdmMaster* createTcePdmMasterCopy() {
#if DEBUG
	printf("createTcePdmMasterCopy: Creating PdmMaster\n");
	fprintf(logFile, "createTcePdmMasterCopy: Creating PdmMaster\n");
#endif

	PdmMaster* pdmMaster = (PdmMaster*)malloc(sizeof(PdmMaster));
	if (pdmMaster == NULL) {
		perror("Failed to allocate memory for PdmMaster");
		return NULL;
	}


	pdmMaster->itemNo = NULL;  // itemNo varchar2
	pdmMaster->cageCode = NULL;	// cageCode varchar2
	pdmMaster->ecl = NULL; // ecl varchar2
	pdmMaster->bemsId = NULL; // bemsId varchar2
	pdmMaster->version = NULL; // version number
	pdmMaster->releaseStatus = NULL; // releaseStatus varchar2
	pdmMaster->createdDate = NULL;  // TO_DATE('2024-08-29T17:30:28Z', 'YYYY-MM-DD"T"HH24:MI:SS"Z"')
	pdmMaster->modifyDate = NULL;   // TO_DATE('2024-08-29T17:30:28Z', 'YYYY-MM-DD"T"HH24:MI:SS"Z"')
	pdmMaster->familyCode = NULL; // familyCode varchar2q
	pdmMaster->description = NULL; // description varchar2
	pdmMaster->statusDate = NULL;  // TO_DATE('2024-08-29T17:30:28Z', 'YYYY-MM-DD"T"HH24:MI:SS"Z"')
	pdmMaster->project = NULL; // project varchar2
	pdmMaster->owner = NULL; // owner varchar2
	pdmMaster->comments = NULL; // comments varchar2
	return pdmMaster;
}

int* getVersion(Node* product) {
#if DEBUG
	printf("getVersion: Searching for version\n");
	fprintf(logFile, "getVersion: Searching for version\n");
#endif

	NodeArray* result = createNodeArray();
	if (result == NULL) {
		perror("getVersion: Failed to create node array");
		return NULL;
	}
	int* resultValue = NULL;
	SearchParams* searchParams = (SearchParams*)malloc(sizeof(SearchParams));
	searchParams->tagName = "UserValue";
	searchParams->attrName = "title";
	searchParams->attrValue = "sequence_id";
	searchNodes(product, result, searchParams);
	free(searchParams);

	if (result->count == 0) {
		freeNodeArray(result);
		return NULL;
	}

	Node* node = result->nodes[0];
	for (int i = 0; i < node->attrCount - 1; i += 2) {
		if (strcmp(node->attributes[i]->name, "value") == 0) {
			resultValue = (int*)malloc(sizeof(int));
			*resultValue = atoi(node->attributes[i]->value);
			break;
		}
	}
	freeNodeArray(result);
	return resultValue;
}


void extractValuesFromAttributes(Node* product, PdmMaster* pdmMaster) {
#if DEBUG
	printf("extractValuesFromAttributes: Extracting values from attributes\n");
	fprintf(logFile, "extractValuesFromAttributes: Extracting values from attributes\n");
#endif

	pdmMaster->itemNo = getValueAttributeByTitleAttribute(product, "item_id");
	if (pdmMaster->itemNo == NULL) {
		return;
	}

	char* userValueAttributes[] = {
		"fnd0CurrentLocationCode", // cageCode varchar2	
		pdmMaster->cageCode,
		"item_revision_id", // ecl varchar2
		pdmMaster->ecl,
		"owning_user", // bemsId varchar2
		pdmMaster->bemsId,
		"release_status_list", // releaseStatus varchar2
		pdmMaster->releaseStatus,
		"creation_date", // createdDate date
		pdmMaster->createdDate,
		"last_mod_date", // modifyDate date
		pdmMaster->modifyDate,
		"ba6_sub_type", // familyCode varchar2
		pdmMaster->familyCode,
		"object_desc", // description varchar2
		pdmMaster->description,
		"date_released", // statusDate date
		pdmMaster->statusDate,
		"owning_group", // project varchar2
		pdmMaster->project,
		"owning_user", // owner varchar2
		pdmMaster->owner,
		"ba6_comments", // comments varchar2
		pdmMaster->comments
	};
	for (int i = 0; i < sizeof(userValueAttributes) / sizeof(userValueAttributes[0]); i += 2) {
		char* value = getValueAttributeByTitleAttribute(product, userValueAttributes[i]);
		if (value) {
			userValueAttributes[i + 1] = value;
		}
		else
		{
			userValueAttributes[i + 1] = NULL;
		}
	}
	pdmMaster->version = getVersion(product);

}

char* toDate(const char* date) {
#if DEBUG
	printf("toDate: Converting date to SQL format\n");
	fprintf(logFile, "toDate: Converting date to SQL format\n");
#endif

	char* sql = NULL;

	if (date == NULL) {
		sql = "'NULL'";
	}
	else
	{
		char* sql = (char*)malloc(67);
		sprintf(sql, "TO_DATE('%s', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"')", date);
	}
	return sql;
}


char* createSql(PdmMaster* pdmMaster) {
#if DEBUG
	printf("createSql: Generating SQL insert statement\n");
	fprintf(logFile, "createSql: Generating SQL insert statement\n");
#endif

	char* sql = (char*)malloc(1024);
	char version[10] = "NULL";
	if (pdmMaster->version) {
		sprintf(version, "%d", *pdmMaster->version);
	}
	sprintf(sql,
		"INSERT INTO TCE_PDM_MASTER_COPY (ITEM_NO, CAGE_CODE, ECL, BEMSID, VERSION, RELEASE_STATUS, CREATED_DATE, MODIFY_DATE, FAMILY_CODE, DESCRIPTION, STATUS_DATE, PROJECT, OWNER, COMMENTS) VALUES ('%s', %s, '%s', '%s', %s, '%s', %s, %s, '%s', '%s', %s, '%s', '%s', '%s','%s')",
		pdmMaster->itemNo,
		(pdmMaster->cageCode) ? pdmMaster->cageCode : "NULL",
		(pdmMaster->ecl) ? pdmMaster->ecl : "NULL",
		(pdmMaster->bemsId) ? pdmMaster->bemsId : "NULL",
		version,
		(pdmMaster->releaseStatus) ? pdmMaster->releaseStatus : "NULL",
		toDate(pdmMaster->createdDate),
		toDate(pdmMaster->createdDate),
		toDate(pdmMaster->modifyDate),
		(pdmMaster->familyCode) ? pdmMaster->familyCode : "NULL",
		(pdmMaster->description) ? pdmMaster->description : "NULL",
		toDate(pdmMaster->statusDate),
		(pdmMaster->project) ? pdmMaster->project : "NULL",
		(pdmMaster->owner) ? pdmMaster->owner : "NULL",
		(pdmMaster->comments) ? pdmMaster->comments : "NULL");
	return sql;
}


char* makeInsert(Node* product) {
#if DEBUG
	printf("makeInsert: Generating SQL insert statement\n");
	fprintf(logFile, "makeInsert: Generating SQL insert statement\n");
#endif

	PdmMaster* pdmMaster = createTcePdmMasterCopy();
	extractValuesFromAttributes(product, pdmMaster);

	// Generate the SQL insert statement using pdmMaster values
	return createSql(pdmMaster);

}
