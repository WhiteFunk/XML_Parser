#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif


int ends_with(const char* haystack, const char* needle)
{
    int h_len = strlen(haystack);
    int n_len = strlen(needle);

    if (h_len < n_len)
        return FALSE;

    for (int i = 0; i < n_len; i++) {
        if (haystack[h_len - n_len + i] != needle[i])
            return FALSE;
    }

    return TRUE;
}

struct XML_Attribute
{
    char* key;
    char* value;
};

void XML_Attribute_free(XML_Attribute* attr);

struct XML_AttributeList
{
    int heap_size;
    int size;
    XML_Attribute* data;
};

void XML_AttributeList_init(XML_AttributeList* list);
void XML_AttributeList_add(XML_AttributeList* list, XML_Attribute* attr);

struct XML_NodeList
{
    int heap_size;
    int size;
    struct XML_Node** data;
};

void XML_NodeList_init(XML_NodeList* list);
void XML_NodeList_add(XML_NodeList* list, struct XML_Node* node);
struct XML_Node* XML_NodeList_at(XML_NodeList* list, int index);
void XML_NodeList_free(XML_NodeList* list);

struct XML_Node
{
    char* tag;
    char* inner_text;
    struct XML_Node* parent;
    XML_AttributeList attributes;
    XML_NodeList children;
};

XML_Node* XML_Node_new(XML_Node* parent);
void XML_Node_free(XML_Node* node);
XML_Node* XML_Node_child(XML_Node* parent, int index);
XML_NodeList* XML_Node_children(XML_Node* parent, const char* tag);
char* XML_Node_attr_val(XML_Node* node,const char* key);
XML_Attribute* XML_Node_attr(XML_Node* node,const char* key);

struct _XMLDocument
{
    XML_Node* root;
    char* version;
    char* encoding;
};
typedef struct _XMLDocument XMLDocument;

int XML_Document_load(XMLDocument* doc, const char* path);
int XML_Document_write(XMLDocument* doc, const char* path, int indent);
void XML_Document_free(XMLDocument* doc);

