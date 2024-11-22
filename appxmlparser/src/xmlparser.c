#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifndef _WIN32
char* my_strdup(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    // Allocate memory for the new string, including the null terminator
    char* dup = malloc(strlen(str) + 1);

    if (dup != NULL) {
        // Copy the original string to the newly allocated memory
        strcpy(dup, str);
    }
    else {
        logError("Error: my_strdup: Memory allocation failed. Aborting");
        exit(EXIT_FAILURE);
    }

    return dup;
}
#endif


char sqlFileName[256];
char logFileName[256];

// Node structure
typedef struct Node {
    char* tagname;
    char* attributes;
    char* content;
    struct Node** children;  // Array of pointers to child nodes
    int childCount;          // Number of children
    struct Node* next;
} Node;


Node* createNode(const char* tagname) {
    Node* node = (Node*)malloc(sizeof(Node));
#ifdef _WIN32
    node->tagname = _strdup(tagname);
#else
	node->tagname = my_strdup(tagname);
#endif
    node->attributes = NULL;
    node->content = NULL;
    node->children = NULL;
    node->childCount = 0;
    node->next = NULL;
    return node;
}

void logError(const char* message) {
    FILE* logFile = fopen(logFileName, "a");
    if (logFile) {
        time_t now;
        time(&now);
        struct tm* local = localtime(&now);
        fprintf(logFile, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
            local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
            local->tm_hour, local->tm_min, local->tm_sec, message);
        fclose(logFile);
    }
}


void addChild(Node* parent, Node* child) {
    if (parent->children == NULL) {
        parent->children = (Node**)malloc(sizeof(Node*));
        if (parent->children == NULL) {
            // Handle memory allocation failure
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
    }
    else {
        Node** temp = (Node**)realloc(parent->children, sizeof(Node*) * (parent->childCount + 1));
        if (temp == NULL) {
			char logMessage[256];
			sprintf(logMessage, "Memory allocation failed at %d of %s - aborting\n", __LINE__, __FILE__);
            logError(logMessage);
            exit(EXIT_FAILURE);
        }
        parent->children = temp;
    }
    parent->children[parent->childCount++] = child;
#ifdef DEBUG
    printf("Added child with tag %s to parent with tag %s. Total children: %d\n", child->tagname, parent->tagname, parent->childCount);
#endif
}


int my_strncasecmp(const char* s1, const char* s2, size_t n) {
	size_t i = 0;
    for (i = 0; i < n; i++) {
        if (tolower((unsigned char)s1[i]) != tolower((unsigned char)s2[i])) {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
        if (s1[i] == '\0' || s2[i] == '\0') {
            break;
        }
    }
    return 0;
}

int isValidTag(const char* tag) {
	const char* p = tag;
    if (tag == NULL || strlen(tag) == 0) return 0;

    // Check if the tag starts with a letter or underscore
    if (!(isalpha(tag[0]) || tag[0] == '_')) return 0;

    // Check if the tag starts with 'xml' (case insensitive)
    if (my_strncasecmp(tag, "xml", 3) == 0) return 0;

    // Check if the tag contains only valid characters
    for (p = tag; *p; ++p) {
        if (!(isalnum(*p) || *p == '-' || *p == '_' || *p == '.')) {
            return 0;
        }
    }

    return 1;
}
// end new
// Global declaration of userValueTitleAttributes
char* userValueTitleAttributes[] = {
    "fnd0CurrentLocationCode", // cageCode varchar2 data in UserValue's value attribute
    "item_revision_id",        // ecl varchar2 data in UserValue's value attribute
    "owning_user",             // bemsId varchar2 data in UserValue's value attribute
    "release_status_list",     // releaseStatus varchar2 data in UserValue's value attribute
    "creation_date",           // createdDate date data in UserValue's value attribute
    "last_mod_date",           // modifyDate date data in UserValue's value attribute
    "ba6_sub_type",            // familyCode varchar2 data in UserValue's value attribute
    "object_desc",             // description varchar2 data in UserValue's value attribute
    "date_released",           // statusDate date data in UserValue's value attribute
    "owning_group",            // project varchar2 data in UserValue's value attribute
    "owning_user",             // owner data in UserValue's value attribute varchar2
    "ba6_comments"             // comments varchar2 data in UserValue's value attribute
};

// Ensure PdmMaster struct is defined globally
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




void addNode(Node* parent, Node* child) {
    if (parent->children == NULL) {
        parent->children = (Node**)malloc(sizeof(Node*));
    }
    else {
        parent->children = (Node**)realloc(parent->children, sizeof(Node*) * (parent->childCount + 1));
    }
    parent->children[parent->childCount++] = child;
#ifdef DEBUG
    printf("Added child with tag %s to parent with tag %s. Total children: %d\n", child->tagname, parent->tagname, parent->childCount);
#endif
}

void parseAttributes(char* attrs, Node* node) {
    char* context = NULL;
#ifdef _WIN32
    char* attr = strtok_s(attrs, " ", &context);
    while (attr) {
        char* keyContext = NULL;
        char* key = strtok_s(attr, "=", &keyContext);
        char* value = strtok_s(NULL, "\"", &keyContext);
#else
    char* attr = strtok_r(attrs, " ", &context);
    while (attr) {
        char* keyContext = NULL;
        char* key = strtok_r(attr, "=", &keyContext);
        char* value = strtok_r(NULL, "\"", &keyContext);
#endif
        if (value) {
            size_t len = strlen(key) + strlen(value) + 4; // +4 for space, equals, and quotes
            char* attribute = (char*)malloc(len);
            snprintf(attribute, len, "%s=\"%s\"", key, value);
#ifdef DEBUG
            printf("Parsed attribute: %s\n", attribute);
#endif
            if (node->attributes == NULL) {
                node->attributes = attribute;
            } else {
                size_t new_len = strlen(node->attributes) + len + 1;
                char* temp = (char*)realloc(node->attributes, new_len);
                if (temp == NULL) {
					char logMessage[256];
					sprintf(logMessage, "Memory allocation failed at %d of %s - aborting\n", __LINE__, __FILE__);
                    logError(logMessage);
                    exit(EXIT_FAILURE);
                }
                node->attributes = temp;
                strncat(node->attributes, " ", new_len - strlen(node->attributes) - 1);
                if (attribute != NULL) {
                    strncat(node->attributes, attribute, new_len - strlen(node->attributes) - 1);
                }
                else {
#ifdef DEBUG
                    printf("Warning: attribute is NULL, cannot concatenate to node->attributes\n");
#endif
                }
                free(attribute);
            }
        }
#ifdef _WIN32
        attr = strtok_s(NULL, " ", &context);
#else
        attr = strtok_r(NULL, " ", &context);
#endif
    }
}

char* handleComments(char* pos) {
    return strstr(pos, "-->") + 3;
}

char* handleDeclarations(char* pos) {
    return strstr(pos, "?>") + 2;
}

char* handleEndTag(char* pos, Node** current, Node* parentStack[], int* stackIndex) {
    char* end = strstr(pos, ">");
    if (end) {
        pos = end + 1;
    }
    // Pop the parent stack
    if (*stackIndex > 0) {
        *current = parentStack[--(*stackIndex)];
    }
#ifdef DEBUG
    printf("Moved back to parent node: %s\n", (*current)->tagname);

#endif // DEBUG
    return pos;
}


char* extractTagName(char* pos, char* tag, size_t tagSize) {
    size_t tagLength = strcspn(pos + 1, " >");
    if (tagLength >= tagSize) {
        printf("Tag name too long, exiting.\n");
        return NULL;
    }
    strncpy(tag,pos + 1, tagLength);
    tag[tagLength] = '\0'; // Null-terminate the tag string
    return pos + tagLength + 1;
}

Node* createAndAddNode(Node** current, Node* parent, const char* tag) {
    if (isValidTag(tag)) {
        Node* newNode = createNode(tag);
        addChild(*current, newNode);
        printf("Created node with tag: %s\n", tag);
        return newNode;
    }
    return NULL;
}


void processAttributes(char* attrs, Node* current) {
    char* endAttrs = strstr(attrs, ">");
    if (endAttrs) {
        size_t attrsLength = endAttrs - attrs;
        char* attrString = (char*)malloc(attrsLength + 1);
        if (attrString != NULL) {
            strncpy(attrString, attrs, attrsLength);
        }
        else {
			char logMessage[256];
			sprintf(logMessage, "Error: processAttributes: Memory allocation for attrString failed at %d of %s - aborting\n", __LINE__, __FILE__);
            logError(logMessage);
			exit(EXIT_FAILURE);
        }
        attrString[attrsLength] = '\0'; // Null-terminate the attribute string
        parseAttributes(attrString, current);
        free(attrString);
    }
}

char* handleSelfClosingTag(char* endAttrs, Node** current, Node* parent) {
    if (*(endAttrs - 1) == '/') {
        *current = parent; // Move back to parent node
    }
    return endAttrs + 1;
}

char* processTag(char* pos, Node** current, Node* parentStack[], int* stackIndex) {
    char tag[100]; // Assuming tag length will not exceed 100 characters
    pos = extractTagName(pos, tag, sizeof(tag));
    if (pos == NULL) return NULL;

    Node* newNode = createAndAddNode(current, parentStack[*stackIndex - 1], tag);
    if (newNode != NULL) {
        parentStack[(*stackIndex)++] = *current;
        *current = newNode; // Move to the new node
        printf("Current node tag: %s\n", (*current)->tagname);
        processAttributes(pos, *current);
        char* endAttrs = strstr(pos, ">");
        if (endAttrs) {
            pos = handleSelfClosingTag(endAttrs, current, parentStack[*stackIndex - 1]);
        }
    }
    return pos;
}

void parseXML(char* xmlContent, Node* parent) {
    char* pos = xmlContent;
    Node* current = parent;
    Node* parentStack[100];
    int stackIndex = 0;
    parentStack[stackIndex++] = parent;

    printf("Starting XML parsing...\n");
    while ((pos = strstr(pos, "<")) != NULL) {
        printf("Processing tag starting at position: %lld\n", (long long int)(pos - xmlContent));
        if (strncmp(pos, "<!--", 4) == 0) {
            pos = handleComments(pos);
        }
        else if (strncmp(pos, "<?", 2) == 0) {
            pos = handleDeclarations(pos);
        }
        else if (pos[1] == '/') {
            pos = handleEndTag(pos, &current, parentStack, &stackIndex);
        }
        else {
            pos = processTag(pos, &current, parentStack, &stackIndex);
            if (pos == NULL) break; // Exit loop on error
        }
    }
    printf("XML parsing completed.\n");
}

void printTree(Node* node, int depth) {
    int i = 0;
    while (node) {
        for (i = 0; i < depth; i++) printf("  ");
        printf("Tag: %s\n", node->tagname);

        if (node->attributes) {
            for (int i = 0; i < depth; i++) printf("  ");
            printf("Attributes: %s\n", node->attributes);
        }

        if (node->content) {
            for (int i = 0; i < depth; i++) printf("  ");
            printf("Content: %s\n", node->content);
        }

        for (int i = 0; i < node->childCount; i++) {
            printTree(node->children[i], depth + 1);
        }

        node = node->next;
    }
}

// Function to search for nodes with the specified tag and attribute, and return a list of matching nodes
Node** searchNodes(Node* root, const char* tagname, const char* attribute, const char* value, int* count) {
	int i = 0;
    if (root == NULL) return NULL;

    int capacity = 10;
    Node** results = (Node**)malloc(capacity * sizeof(Node*));
    *count = 0;

    int stackCapacity = 100;
    Node** stack = (Node**)malloc(stackCapacity * sizeof(Node*));
    int stackIndex = 0;
    stack[stackIndex++] = root;

    while (stackIndex > 0) {
        Node* current = stack[--stackIndex];
        if (current == NULL) continue;

        if (strcmp(current->tagname, tagname) == 0) {
            if (attribute == NULL || (current->attributes && strstr(current->attributes, attribute))) {
                if (value == NULL || (current->attributes && strstr(current->attributes, value))) {
                    if (*count >= capacity) {
                        capacity *= 2;
                        results = (Node**)realloc(results, capacity * sizeof(Node*));
                    }
                    results[(*count)++] = current;
                }
            }
        }

        if (stackIndex >= stackCapacity) {
            stackCapacity *= 2;
            stack = (Node**)realloc(stack, stackCapacity * sizeof(Node*));
        }

        if (current->next) stack[stackIndex++] = current->next;

        if (current->children) {
            for (i = 0; i < current->childCount; i++) {
                if (current->children[i]) {
                    if (stackIndex >= stackCapacity) {
                        stackCapacity *= 2;
                        stack = (Node**)realloc(stack, stackCapacity * sizeof(Node*));
                    }
                    stack[stackIndex++] = current->children[i];
                }
            }
        }
    }

    free(stack);
    return results;
}


int isValidPdmMaster(PdmMaster* pdmMaster) {
    return pdmMaster->itemNo != NULL;
}

void processUserValue(Node* node, PdmMaster* pdmMaster) {
    char* title = NULL;
    char* value = NULL;
    char* context = NULL;
#ifdef _WIN32
    char* attr = strtok_s(node->attributes, " ", &context);
    while (attr) {
        char* keyContext = NULL;
        char* key = strtok_s(attr, "=", &keyContext);
        char* val = strtok_s(NULL, "\"", &keyContext);
#else
	char* attr = strtok_r(node->attributes, " ", &context);
	while (attr) {
		char* keyContext = NULL;
		char* key = strtok_r(attr, "=", &keyContext);
		char* val = strtok_r(NULL, "\"", &keyContext);
#endif
#ifdef _WIN32
        if (strcmp(key, "title") == 0) {
            title = _strdup(val);
        }
        else if (strcmp(key, "value") == 0) {
            value = _strdup(val);
        }
        attr = strtok_s(NULL, " ", &context);
#else
        if (strcmp(key, "title") == 0) {
            title = my_strdup(val);
    }
        else if (strcmp(key, "value") == 0) {
            value = my_strdup(val);
        }
        attr = strtok_r(NULL, " ", &context);
#endif
    }

    if (title != NULL && value != NULL) {
        if (strcmp(title, "fnd0CurrentLocationCode") == 0) pdmMaster->cageCode = value;
        else if (strcmp(title, "item_revision_id") == 0) pdmMaster->ecl = value;
        else if (strcmp(title, "owning_user") == 0) pdmMaster->bemsId = value;
        else if (strcmp(title, "release_status_list") == 0) pdmMaster->releaseStatus = value;
        else if (strcmp(title, "creation_date") == 0) pdmMaster->createdDate = value;
        else if (strcmp(title, "last_mod_date") == 0) pdmMaster->modifyDate = value;
        else if (strcmp(title, "ba6_sub_type") == 0) pdmMaster->familyCode = value;
        else if (strcmp(title, "object_desc") == 0) pdmMaster->description = value;
        else if (strcmp(title, "date_released") == 0) pdmMaster->statusDate = value;
        else if (strcmp(title, "owning_group") == 0) pdmMaster->project = value;
        else if (strcmp(title, "ba6_comments") == 0) pdmMaster->comments = value;
        else if (strcmp(title, "item_id") == 0) pdmMaster->itemNo = value;

        free(title);
    }
}

void traverseNodes(Node* node, PdmMaster* pdmMaster) {
    int i = 0;
    if (node == NULL) return;

    if (strcmp(node->tagname, "UserValue") == 0) {
        processUserValue(node, pdmMaster);
    }

    for (i = 0; i < node->childCount; i++) {
        traverseNodes(node->children[i], pdmMaster);
    }
}



void fetchNodeAttributes(Node* node, PdmMaster* pdmMaster) {
    traverseNodes(node, pdmMaster);

    // Log PdmMaster values
    char logMessage[1024];
    snprintf(logMessage, sizeof(logMessage),
        "PdmMaster values - itemNo: %s, cageCode: %s, ecl: %s, bemsId: %s, releaseStatus: %s, createdDate: %s, modifyDate: %s, familyCode: %s, description: %s, statusDate: %s, project: %s, comments: %s",
        pdmMaster->itemNo ? pdmMaster->itemNo : "NULL",
        pdmMaster->cageCode ? pdmMaster->cageCode : "NULL",
        pdmMaster->ecl ? pdmMaster->ecl : "NULL",
        pdmMaster->bemsId ? pdmMaster->bemsId : "NULL",
        pdmMaster->releaseStatus ? pdmMaster->releaseStatus : "NULL",
        pdmMaster->createdDate ? pdmMaster->createdDate : "NULL",
        pdmMaster->modifyDate ? pdmMaster->modifyDate : "NULL",
        pdmMaster->familyCode ? pdmMaster->familyCode : "NULL",
        pdmMaster->description ? pdmMaster->description : "NULL",
        pdmMaster->statusDate ? pdmMaster->statusDate : "NULL",
        pdmMaster->project ? pdmMaster->project : "NULL",
        pdmMaster->comments ? pdmMaster->comments : "NULL");
    logError(logMessage);
}


char* toDate(const char* date) {
    char* sql = NULL;

    if (date == NULL) {
        sql = "'NULL'";
    }
    else {
        sql = (char*)malloc(67);
        snprintf(sql, 67, "TO_DATE('%s', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"')", date);
    }
    return sql;
}

char* handleVarcharColumn(const char* value) {
    static char buffer[4096];
    if (value) {
        snprintf(buffer, sizeof(buffer), "'%s'", value);
    }
    else {
        snprintf(buffer, sizeof(buffer), "NULL");
    }
    return buffer;
}

char* handleDateColumn(const char* date) {
    static char buffer[4096];
    if (date) {
        snprintf(buffer, sizeof(buffer), "TO_DATE('%s', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"')", date);
    }
    else {
        snprintf(buffer, sizeof(buffer), "NULL");
    }
    return buffer;
}

char* handleNumberColumn(const int* number) {
    static char buffer[4096];
    if (number) {
        snprintf(buffer, sizeof(buffer), "%d", *number);
    }
    else {
        snprintf(buffer, sizeof(buffer), "NULL");
    }
    return buffer;
}


char* buildInsertStatement(PdmMaster* pdmMaster) {
    static char insertStatement[4096]; // Increased buffer size for safety

    snprintf(insertStatement, sizeof(insertStatement),
        "INSERT INTO TCE_PDM_MASTER_COPY "
        "(ITEM_NO, CAGE_CODE, ECL, BEMSID, VERSION, RELEASE_STATUS, CREATED_DATE, MODIFY_DATE, FAMILY_CODE, DESCRIPTION, STATUS_DATE, PROJECT, OWNER, COMMENTS) "
        "VALUES ('%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s);",
        pdmMaster->itemNo,
        handleVarcharColumn(pdmMaster->cageCode),
        handleVarcharColumn(pdmMaster->ecl),
        handleVarcharColumn(pdmMaster->bemsId),
        handleNumberColumn(pdmMaster->version),
        handleVarcharColumn(pdmMaster->releaseStatus),
        handleDateColumn(pdmMaster->createdDate),
        handleDateColumn(pdmMaster->modifyDate),
        handleVarcharColumn(pdmMaster->familyCode),
        handleVarcharColumn(pdmMaster->description),
        handleDateColumn(pdmMaster->statusDate),
        handleVarcharColumn(pdmMaster->project),
        handleVarcharColumn(pdmMaster->owner),
        handleVarcharColumn(pdmMaster->comments));

    return insertStatement;
}


void generateInsertStatement(PdmMaster* pdmMaster, FILE* outputFile) {
	int i = 0;
    if (pdmMaster == NULL || outputFile == NULL) return;

    // Ensure item_id is always present
    if (pdmMaster->itemNo == NULL) {
        logError("Error: item_id (ITEM_NO) cannot be NULL.");
        return;
    }

    char* insertStatement = buildInsertStatement(pdmMaster);
    fprintf(outputFile, "%s\n", insertStatement);
}

int main(int argc, char** argv) {
	int i = 0;
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <xml_file> <sql_file> <log_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    strncpy(sqlFileName, argv[2],sizeof(sqlFileName) - 1);
    strncpy(logFileName, argv[3],sizeof(logFileName) - 1);
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer != NULL) {
        fread(buffer, 1, fileSize, file);
    }
    else {
		char errorMessage[256];
		snprintf(errorMessage, sizeof(errorMessage),
			"Error: main: Memory allocation for buffer failed at %d of %s - aborting\n", __LINE__, __FILE__);
		logError(errorMessage);
        exit(EXIT_FAILURE);
    }
    buffer[fileSize] = '\0';
    fclose(file);

    Node* root = createNode("root");
    parseXML(buffer, root);

    printTree(root, 0);

    // Search for Product nodes with subType="BA6_Assembly"
    int count = 0;
    Node** foundNodes = searchNodes(root, "Product", "subType=\"BA6_Assembly\"", NULL, &count);
    if (foundNodes != NULL && count > 0) {
		printf("Found %d Product nodes with subType=\"BA6_Assembly\".\n", count);
        FILE* outputFile = fopen(sqlFileName, "w");
        if (outputFile != NULL) {
            for (i = 0; i < count; ++i) {
                PdmMaster pdmMaster = { 0 };
                fetchNodeAttributes(foundNodes[i], &pdmMaster);

                if (isValidPdmMaster(&pdmMaster)) {
                    generateInsertStatement(&pdmMaster, outputFile);
                }
                else {
                    char errorMessage[256];
                    snprintf(errorMessage, sizeof(errorMessage),
                        "Product with %s is invalid", foundNodes[i]->attributes);
                    logError(errorMessage);
                }
            }
            fclose(outputFile);
        }
        free(foundNodes);
    }
    else {
        logError("No Product nodes found with subType=\"BA6_Assembly\".");
    }

    free(buffer);
    return EXIT_SUCCESS;
}
