#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include "wkpf.h"
#include "posix_utils.h"

extern uint8_t wkpf_process_enabled_wuclass(char* wuclassname, bool appCanCreateInstances, int createInstancesAtStartup);

uint8_t wkpf_process_enabled_wuclasses_xml(void) {
    xmlDoc         *document;
    xmlNode        *root, *first_child, *node;

    document = xmlReadFile(posix_enabled_wuclasses_xml, NULL, 0);
    root = xmlDocGetRootElement(document);
    first_child = root->children;
    for (node = first_child; node; node = node->next) {
        if (!xmlStrcmp(node->name, (const xmlChar *) "WuClass")) {
            char* name = (char *)xmlGetProp(node, (const xmlChar *)"name");
            char* appCanCreateInstancesString = (char *)xmlGetProp(node, (const xmlChar *)"appCanCreateInstances");
            char* createInstancesAtStartupString = (char *)xmlGetProp(node, (const xmlChar *)"createInstancesAtStartup");
            bool appCanCreateInstances = strcmp("true", appCanCreateInstancesString) == 0
                                        || strcmp("True", appCanCreateInstancesString) == 0
                                        || strcmp("1", appCanCreateInstancesString) == 0;
            int createInstancesAtStartup = atoi(createInstancesAtStartupString);
            if (wkpf_process_enabled_wuclass(name, appCanCreateInstances, createInstancesAtStartup) != WKPF_OK) {
                printf("Initialisation failed.\n");
                exit(1);
            }
        }
    }
    return 0;
}
