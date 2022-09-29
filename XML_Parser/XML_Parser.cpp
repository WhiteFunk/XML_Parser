#include <XML_Parser.h>


void XML_Attribute_free(XML_Attribute* attr)
{
    free(attr->key);
    free(attr->value);
}

void XML_AttributeList_init(XML_AttributeList* list)
{
    list->heap_size = 1;
    list->size = 0;
    list->data = (XML_Attribute*) malloc(sizeof(XML_Attribute) * list->heap_size);
}

void XML_AttributeList_add(XML_AttributeList* list, XML_Attribute* attr)
{
    while (list->size >= list->heap_size) {
        list->heap_size *= 2;
        list->data = (XML_Attribute*) realloc(list->data, sizeof(XML_Attribute) * list->heap_size);
    }

    list->data[list->size++] = *attr;
}

void XML_NodeList_init(XML_NodeList* list)
{
    list->heap_size = 1;
    list->size = 0;
    list->data = (XML_Node**) malloc(sizeof(XML_Node*) * list->heap_size);
}

void XML_NodeList_add(XML_NodeList* list, XML_Node* node)
{
    while (list->size >= list->heap_size) {
        list->heap_size *= 2;
        list->data = (XML_Node**) realloc(list->data, sizeof(XML_Node*) * list->heap_size);
    }

    list->data[list->size++] = node;
}

XML_Node* XML_NodeList_at(XML_NodeList* list, int index)
{
    return list->data[index];
}

void XML_NodeList_free(XML_NodeList* list)
{
    free(list);
}

XML_Node* XML_Node_new(XML_Node* parent)
{
    XML_Node* node = (XML_Node*) malloc(sizeof(XML_Node));
    node->parent = parent;
    node->tag = NULL;
    node->inner_text = NULL;
    XML_AttributeList_init(&node->attributes);
    XML_NodeList_init(&node->children);
    if (parent)
        XML_NodeList_add(&parent->children, node);
    return node;
}

void XML_Node_free(XML_Node* node)
{
    if (node->tag)
        free(node->tag);
    if (node->inner_text)
        free(node->inner_text);
    for (int i = 0; i < node->attributes.size; i++)
        XML_Attribute_free(&node->attributes.data[i]);
    free(node);
}

XML_Node* XML_Node_child(XML_Node* parent, int index)
{
    return parent->children.data[index];
}

XML_NodeList* XML_Node_children(XML_Node* parent, const char* tag)
{
    XML_NodeList* list = (XML_NodeList*) malloc(sizeof(XML_NodeList));
    XML_NodeList_init(list);

    for (int i = 0; i < parent->children.size; i++) {
        XML_Node* child = parent->children.data[i];
        if (!strcmp(child->tag, tag))
            XML_NodeList_add(list, child);
    }

    return list;
}

char* XML_Node_attr_val(XML_Node* node,const char* key)
{
    for (int i = 0; i < node->attributes.size; i++) {
        XML_Attribute attr = node->attributes.data[i];
        if (!strcmp(attr.key, key))
            return attr.value;
    }
    return NULL;
}

XML_Attribute* XML_Node_attr(XML_Node* node,const char* key)
{
    for (int i = 0; i < node->attributes.size; i++) {
        XML_Attribute* attr = &node->attributes.data[i];
        if (!strcmp(attr->key, key))
            return attr;
    }
    return NULL;
}

enum _TagType
{
    TAG_START,
    TAG_INLINE
};
typedef enum _TagType TagType;

static TagType parse_attrs(char* buf, int* i, char* lex, int* lexi, XML_Node* curr_node)
{
    XML_Attribute curr_attr = {0, 0};
    while (buf[*i] != '>') {
        lex[(*lexi)++] = buf[(*i)++];

        if (buf[*i] == ' ' && !curr_node->tag) {
            lex[*lexi] = '\0';
            curr_node->tag = strdup(lex);
            *lexi = 0;
            (*i)++;
            continue;
        }

        if (lex[*lexi-1] == ' ') {
            (*lexi)--;
        }

        if (buf[*i] == '=') {
            lex[*lexi] = '\0';
            curr_attr.key = strdup(lex);
            *lexi = 0;
            continue;
        }

        if (buf[*i] == '"') {
            if (!curr_attr.key) {
                fprintf(stderr, "Value has no key\n");
                return TAG_START;
            }

            *lexi = 0;
            (*i)++;

            while (buf[*i] != '"')
                lex[(*lexi)++] = buf[(*i)++];
            lex[*lexi] = '\0';
            curr_attr.value = strdup(lex);
            XML_AttributeList_add(&curr_node->attributes, &curr_attr);
            curr_attr.key = NULL;
            curr_attr.value = NULL;
            *lexi = 0;
            (*i)++;
            continue;
        }

        if (buf[*i - 1] == '/' && buf[*i] == '>') {
            lex[*lexi] = '\0';
            if (!curr_node->tag)
                curr_node->tag = strdup(lex);
            (*i)++;
            return TAG_INLINE;
        }
    }

    return TAG_START;
}

int XML_Document_load(XMLDocument* doc, const char* path)
{
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Could not load file from '%s'\n", path);
        return FALSE;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buf = (char*) malloc(sizeof(char) * size + 1);
    fread(buf, 1, size, file);
    fclose(file);
    buf[size] = '\0';

    doc->root = XML_Node_new(NULL);

    char lex[256];
    int lexi = 0;
    int i = 0;

    XML_Node* curr_node = doc->root;

    while (buf[i] != '\0')
    {
        if (buf[i] == '<') {
            lex[lexi] = '\0';

            if (lexi > 0) {
                if (!curr_node) {
                    fprintf(stderr, "Text outside of document\n");
                    return FALSE;
                }

                curr_node->inner_text = strdup(lex);
                lexi = 0;
            }

            if (buf[i + 1] == '/') {
                i += 2;
                while (buf[i] != '>')
                    lex[lexi++] = buf[i++];
                lex[lexi] = '\0';

                if (!curr_node) {
                    fprintf(stderr, "Already at the root\n");
                    return FALSE;
                }

                if (strcmp(curr_node->tag, lex)) {
                    fprintf(stderr, "Mismatched tags (%s != %s)\n", curr_node->tag, lex);
                    return FALSE;
                }

                curr_node = curr_node->parent;
                i++;
                continue;
            }

            if (buf[i + 1] == '!') {
                while (buf[i] != ' ' && buf[i] != '>')
                    lex[lexi++] = buf[i++];
                lex[lexi] = '\0';

                if (!strcmp(lex, "<!--")) {
                    lex[lexi] = '\0';
                    while (!ends_with(lex, "-->")) {
                        lex[lexi++] = buf[i++];
                        lex[lexi] = '\0';
                    }
                    continue;
                }
            }

            if (buf[i + 1] == '?') {
                while (buf[i] != ' ' && buf[i] != '>')
                    lex[lexi++] = buf[i++];
                lex[lexi] = '\0';

                if (!strcmp(lex, "<?xml")) {
                    lexi = 0;
                    XML_Node* desc = XML_Node_new(NULL);
                    parse_attrs(buf, &i, lex, &lexi, desc);                   
                    doc->version = XML_Node_attr_val(desc, "version");
                    doc->encoding = XML_Node_attr_val(desc, "encoding");
                    continue;
                }
            }

            curr_node = XML_Node_new(curr_node);

            i++;
            if (parse_attrs(buf, &i, lex, &lexi, curr_node) == TAG_INLINE) {
                curr_node = curr_node->parent;
                i++;
                continue;
            }

            lex[lexi] = '\0';
            if (!curr_node->tag)
                curr_node->tag = strdup(lex);

            lexi = 0;
            i++;
            continue;
        } else {
            lex[lexi++] = buf[i++];
        }
    }

    return TRUE;
}

static void node_out(FILE* file, XML_Node* node, int indent, int times)
{
    for (int i = 0; i < node->children.size; i++) {
        XML_Node* child = node->children.data[i];

        if (times > 0)
            fprintf(file, "%0*s", indent * times, " ");

        fprintf(file, "<%s", child->tag);
        for (int i = 0; i < child->attributes.size; i++) {
            XML_Attribute attr = child->attributes.data[i];
            if (!attr.value || !strcmp(attr.value, ""))
                continue;
            fprintf(file, " %s=\"%s\"", attr.key, attr.value);
        }

        if (child->children.size == 0 && !child->inner_text)
            fprintf(file, " />\n");
        else {
            fprintf(file, ">");
            if (child->children.size == 0)
                fprintf(file, "%s</%s>\n", child->inner_text, child->tag);
            else {
                fprintf(file, "\n");
                node_out(file, child, indent, times + 1);
                if (times > 0)
                    fprintf(file, "%0*s", indent * times, " ");
                fprintf(file, "</%s>\n", child->tag);
            }
        }
    }
}

int XML_Document_write(XMLDocument* doc, const char* path, int indent)
{
    FILE* file = fopen(path, "w");
    if (!file) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return FALSE;
    }

    fprintf(
        file, "<?xml version=\"%s\" encoding=\"%s\" ?>\n",
        (doc->version) ? doc->version : "1.0",
        (doc->encoding) ? doc->encoding : "UTF-8"
    );
    node_out(file, doc->root, indent, 0);
    fclose(file);
}

void XML_Document_free(XMLDocument* doc)
{
    XML_Node_free(doc->root);
}



