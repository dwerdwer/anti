#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <tinyxml.h>

using namespace std;

#define MODULES     "modules"
#define NAME_ATTR   "name"
#define LOAD_ATTR   "load"

TiXmlAttribute* find_target_attribute(TiXmlDocument &xml_obj, const char *p_module)
{
    TiXmlAttribute *result = NULL;

    TiXmlNode *p_node = NULL;
    TiXmlElement *p_element = NULL;

    p_node = xml_obj.RootElement()->IterateChildren(MODULES, p_node);

    if (NULL == p_node)
    {
        printf("Can't find \"%s\"\n", MODULES);
        return result;
    }
    for (p_node = p_node->FirstChild(); p_node != NULL; p_node = p_node->NextSibling())
    {
        p_element = p_node->ToElement();
        
        if (0 == strcmp(p_element->Attribute(NAME_ATTR), p_module)) {
            break;
        }
    }
    TiXmlAttribute *p_attr = p_element->FirstAttribute();

    for (; p_attr != NULL; p_attr = p_attr->Next())
    {
        if (0 == strcmp(p_attr->Name(), LOAD_ATTR)) {
            break;
        }
    }
    result = p_attr;

    return result;
}

int change_load_status(const char *p_file, const char *p_module, bool load_flag)
{
    int result = -1;
    
    TiXmlDocument xml_obj(p_file);
    
    if (!xml_obj.LoadFile()) 
    {
        printf("\"%s\" %s\n", p_file, xml_obj.ErrorDesc());
        return result;
    }
    TiXmlAttribute *p_attr = find_target_attribute(xml_obj, p_module);
    
    if (NULL == p_attr) {
        return result;
    }

    if (load_flag)
    {
        printf("set %s load attribute \"yes\"\n", p_module);
        p_attr->SetValue("yes");
    }
    else {
        printf("set %s load attribute \"no\"\n", p_module);
        p_attr->SetValue("no");
    }

    xml_obj.SaveFile();

    return result;
}

int main(int argc, char *argv[])
{   
    int opt = 0;
    const char *p_file = NULL;
    const char *p_module = NULL;
    const char *p_load = NULL;
    bool load_flag;

    while ((opt = getopt(argc, argv, "f:m:l:")) != -1)
    {
        switch (opt) {
        case 'f':
            p_file = optarg;
            break;
        case 'm':
            p_module = optarg;
            break;
        case 'l':
            p_load = optarg;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s -f file_path -m module_name -l [y/n]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (NULL == p_file || NULL == p_module || NULL == p_load) {
        goto args_error;
    }
    if (access(p_file, F_OK) == -1)                                  
    {
        printf("\"%s\" %s\n", p_file, strerror(errno));
        return -1;
    }
    if (strcmp("y", p_load) == 0 || strcmp("Y", p_load) == 0) {
        load_flag = true;
    }
    else if (strcmp("n", p_load) == 0 || strcmp("N", p_load) == 0) {
        load_flag = false;
    }
    else {
        goto args_error;
    }
    return change_load_status(p_file, p_module, load_flag);

args_error:
    printf("Usage: %s -f file_path -m module_name -l [y/n]\n", argv[0]);
    exit(EXIT_FAILURE);
}
