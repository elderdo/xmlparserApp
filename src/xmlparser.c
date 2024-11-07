#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char *tagname;
    char *attributes;
    char *content;
    struct Node *children;
    struct Node *next;
} Node;

Node* createNode(const char *tagname) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->tagname = strdup(tagname);
    node->attributes = NULL;
    node->content = NULL;
    node->children = NULL;
    node->next = NULL;
    return node;
}

void addNode(Node *parent, Node *child) {
    if (parent->children == NULL) {
        parent->children = child;
    } else {
        Node *sibling = parent->children;
        while (sibling->next != NULL) {
            sibling = sibling->next;
        }
        sibling->next = child;
    }
}

void parseAttributes(char *attrs, Node *node) {
    char *attr = strtok(attrs, " ");
    while (attr) {
        char *key = strtok(attr, "=");
        char *value = strtok(NULL, "\"");
        if (value) {
            size_t len = strlen(key) + strlen(value) + 4; // +4 for space, equals, and quotes
            char *attribute = (char *)malloc(len);
            snprintf(attribute, len, "%s=\"%s\"", key, value);
            if (node->attributes == NULL) {
                node->attributes = attribute;
            } else {
                size_t new_len = strlen(node->attributes) + len + 1;
                node->attributes = (char *)realloc(node->attributes, new_len);
                strcat(node->attributes, " ");
                strcat(node->attributes, attribute);
                free(attribute);
            }
        }
        attr = strtok(NULL, " ");
    }
}

void parseXML(char *xmlContent, Node *parent) {
    char *pos = xmlContent;
    Node *current = parent;

    printf("Starting XML parsing...\n");
    while ((pos = strstr(pos, "<")) != NULL) {
        printf("Processing tag starting at position: %ld\n", pos - xmlContent);
        if (pos[1] == '/') {
            // End tag
            char *end = strstr(pos, ">");
            pos = end + 1;
            current = parent; // Move back to parent node
        } else {
            // Start tag or self-closing tag
            char *end = strstr(pos, ">");
            size_t tagLength = end - pos - 1;
            char *tag = (char *)malloc(tagLength + 1);
            strncpy(tag, pos + 1, tagLength);
            tag[tagLength] = '\0';

            // Extract tagname and attributes
            char *space = strchr(tag, ' ');
            if (space) {
                *space = '\0';
                space++;
            }

            printf("Found tag: %s\n", tag);
            Node *newNode = createNode(tag);
            if (space) {
                printf("Found attributes: %s\n", space);
                parseAttributes(space, newNode);
            }

            addNode(current, newNode);

            if (*(end - 1) == '/') {
                // Self-closing tag, do not change current node
            } else {
                current = newNode; // Move to new child node
            }

            free(tag);

            // Look for content
            char *content_start = end + 1;
            char *content_end = strstr(content_start, "<");
            if (content_end && content_end > content_start) {
                size_t content_length = content_end - content_start;
                newNode->content = (char *)malloc(content_length + 1);
                strncpy(newNode->content, content_start, content_length);
                newNode->content[content_length] = '\0';
                printf("Found content: %s\n", newNode->content);
            }
            pos = end + 1;
        }
    }
    printf("Finished XML parsing.\n");
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

