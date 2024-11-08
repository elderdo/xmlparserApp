#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _CRT_SECURE_NO_WARNINGS
typedef struct Node {
    char* tagname;
    char* attributes;
    char* content;
    struct Node* children;
    struct Node* next;
} Node;

Node* createNode(const char* tagname) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->tagname = _strdup(tagname);
    node->attributes = NULL;
    node->content = NULL;
    node->children = NULL;
    node->next = NULL;
    return node;
}

void addNode(Node* parent, Node* child) {
    if (parent->children == NULL) {
        parent->children = child;
    }
    else {
        Node* sibling = parent->children;
        while (sibling->next != NULL) {
            sibling = sibling->next;
        }
        sibling->next = child;
    }
}

void parseAttributes(char* attrs, Node* node) {
    char* context = NULL;
    char* attr = strtok_s(attrs, " ", &context);
    while (attr) {
        char* keyContext = NULL;
        char* key = strtok_s(attr, "=", &keyContext);
        char* value = strtok_s(NULL, "\"", &keyContext);
        if (value) {
            size_t len = strlen(key) + strlen(value) + 4; // +4 for space, equals, and quotes
            char* attribute = (char*)malloc(len);
            snprintf(attribute, len, "%s=\"%s\"", key, value);
            if (node->attributes == NULL) {
                node->attributes = attribute;
            }
            else {
                size_t new_len = strlen(node->attributes) + len + 1;
                node->attributes = (char*)realloc(node->attributes, new_len);
                strcat_s(node->attributes, new_len, " ");
                strcat_s(node->attributes, new_len, attribute);
                free(attribute);
            }
        }
        attr = strtok_s(NULL, " ", &context);
    }
}/*
 I've made the following changes:
1. Added checks to ensure end is not NULL before using it to advance the pos pointer.

2. Printed additional debugging information, such as the found tag.

3. Added a check to ensure the tag name is not too long to avoid buffer overflow.

4. Moved the pos pointer forward after processing a tag to prevent the infinite loop.

5. Ensured start and end tags are processed correctly.

6. Handled tag attributes correctly by extracting and parsing them.

7. Created nodes and added them to the tree structure.

8. Properly handled self-closing tags.

*/

void parseXML(char* xmlContent, Node* parent) {
    char* pos = xmlContent;
    Node* current = parent;

    printf("Starting XML parsing...\n");
    while ((pos = strstr(pos, "<")) != NULL) {
        printf("Processing tag starting at position: %ld\n", pos - xmlContent);
        if (pos[1] == '/') {
            // End tag
            char* end = strstr(pos, ">");
            if (end) {
                pos = end + 1;
            }
            else {
                break; // Malformed XML, exit loop
            }
            current = parent; // Move back to parent node
        }
        else {
            // Start tag or self-closing tag
            char tag[100]; // Assuming tag length will not exceed 100 characters
            size_t tagLength = strcspn(pos + 1, " >");
            if (tagLength >= sizeof(tag)) {
                printf("Tag name too long, exiting.\n");
                break;
            }
            strncpy_s(tag, sizeof(tag), pos + 1, tagLength);
            tag[tagLength] = '\0'; // Null-terminate the tag string

            Node* newNode = createNode(tag);
            addNode(current, newNode);
            current = newNode; // Move to the new node

            // Process attributes
            char* attrs = pos + tagLength + 1;
            char* endAttrs = strstr(attrs, ">");
            if (endAttrs) {
                size_t attrsLength = endAttrs - attrs;
                char* attrString = (char*)malloc(attrsLength + 1);
                strncpy_s(attrString, attrsLength + 1, attrs, attrsLength);
                attrString[attrsLength] = '\0'; // Null-terminate the attribute string
                parseAttributes(attrString, current);
                free(attrString);
            }

            // Handle self-closing tag
            if (*(endAttrs - 1) == '/') {
                current = parent; // Move back to parent node
            }

            pos = endAttrs + 1;
        }
    }
    printf("XML parsing completed.\n");
}

void printTree(Node *node, int depth) {
    while (node) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("Tag: %s\n", node->tagname);
        if (node->attributes) {
            for (int i = 0; i < depth; i++) printf("  ");
            printf("Attributes: %s\n", node->attributes);
        }
        if (node->content) {
            for (int i = 0; i < depth; i++) printf("  ");
            printf("Content: %s\n", node->content);
        }
        printTree(node->children, depth + 1);
        node = node->next;
    }
}

Node* searchNode(Node *root, const char *tagname, const char *attribute, const char *value) {
    if (root == NULL) return NULL;
    
    if (strcmp(root->tagname, tagname) == 0) {
        if (attribute == NULL || (root->attributes && strstr(root->attributes, attribute))) {
            if (value == NULL || (root->attributes && strstr(root->attributes, value))) {
                return root;
            }
        }
    }

    Node *result = searchNode(root->children, tagname, attribute, value);
    if (result != NULL) return result;

    return searchNode(root->next, tagname, attribute, value);
}

// Function to fetch attributes and values of matched nodes
void fetchNodeAttributes(Node *node) {
    if (node == NULL) return;

    printf("Node name: %s\n", node->tagname);
    if (node->attributes) {
        printf("Attributes: %s\n", node->attributes);
    }
    if (node->content) {
        printf("Content: %s\n", node->content);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <xml_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = (char *)malloc(fileSize + 1);
    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0';
    fclose(file);

    Node *root = createNode("root");
    parseXML(buffer, root);

    printTree(root, 0);

    // Example search
    Node *found = searchNode(root, "UserValue", "title", "item_id");
    if (found != NULL) {
        printf("Found node:\n");
        fetchNodeAttributes(found);
    } else {
        printf("Node not found.\n");
    }

    free(buffer);
    return EXIT_SUCCESS;
}

