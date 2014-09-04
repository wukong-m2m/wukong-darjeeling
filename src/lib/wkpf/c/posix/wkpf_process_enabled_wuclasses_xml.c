#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include "simplexml-1.0/src/simplexml.h"
#include "wkpf.h"
#include "posix_utils.h"
#include "wkpf_wuclasses.h"
#include "wkpf_wuobjects.h"

// GENERATED:
extern wuclass_t* wkpf_process_enabled_wuclasses_xml_register_class(char* wuclassname, bool appCanCreateInstances);

int wkpf_process_enabled_wuclasses_readfile (char* sFileName, char** psData, long *pnDataLen) {
    struct stat fstat;
    *psData= NULL;
    *pnDataLen= 0;
    if (stat(sFileName, &fstat) == -1) {
        return -1;
    } else {
        FILE *file= fopen(sFileName, "r");
        if (file == NULL) {
            return -1;
        } else {
            *psData= malloc(fstat.st_size);
            if (*psData == NULL) {
                return -1;
            } else {
                size_t len= fread(*psData, 1, fstat.st_size, file);
                fclose(file);
                if (len != fstat.st_size) {
                    free(*psData);
                    *psData= NULL;
                    return -1;
                }
                *pnDataLen= len;
                return 0;
            }
        }
    }   
}


void* wkpf_process_enabled_wuclasses_handler (SimpleXmlParser parser, SimpleXmlEvent event, 
    const char* szName, const char* szAttribute, const char* szValue)
{
    static int next_free_port = 1;
    static char wuclassname[1024];
    static bool appCanCreateInstances;
    static wuclass_t *wuclass;

    uint8_t retval;

    if (event == ADD_SUBTAG && !strcmp(szName, "WuClass")) {
        wuclassname[0] = 0;
        appCanCreateInstances = false;
        wuclass = NULL;
    } else if (event == ADD_SUBTAG && !strcmp(szName, "CreateInstance")) {
        if (wuclass == NULL)
            wuclass = wkpf_process_enabled_wuclasses_xml_register_class(wuclassname, appCanCreateInstances);
        if (wuclass == NULL) {
            printf("Couldn't register wuclass '%s'\n", wuclassname);
            exit(1);
        }

        retval = wkpf_create_wuobject(wuclass->wuclass_id, next_free_port++, 0, true);
        if (retval != WKPF_OK) {
            printf("Couldn't create wuobject for class '%s'. Error %d\n", wuclassname, retval);
            exit(1);
        }
    } else if (event == ADD_ATTRIBUTE) {
        if (!strcmp(szAttribute, "name"))
            strncpy(wuclassname, szValue, 1024);
        else if (!strcmp(szAttribute, "appCanCreateInstances"))
            appCanCreateInstances = strcmp(szValue, "true") == 0
                                        || strcmp(szValue, "True") == 0
                                        || strcmp(szValue, "1") == 0;
    } else if (event == FINISH_TAG && !strcmp(szName, "WuClass")) {
        if (wuclass == NULL)
            wuclass = wkpf_process_enabled_wuclasses_xml_register_class(wuclassname, appCanCreateInstances);
        if (wuclass == NULL) {
            printf("Couldn't register wuclass '%s'\n", wuclassname);
            exit(1);
        }
    }

    return wkpf_process_enabled_wuclasses_handler;
}

uint8_t wkpf_process_enabled_wuclasses_xml(void) {
    char* sData;
    long nDataLen;
    int nResult=wkpf_process_enabled_wuclasses_readfile(posix_enabled_wuclasses_xml, &sData, &nDataLen);
    if (nResult != 0) {
        fprintf(stderr, "couldn't read %s.\n", posix_enabled_wuclasses_xml);
        exit(1);
    }
    SimpleXmlParser parser = simpleXmlCreateParser(sData, nDataLen);
    if (simpleXmlParse(parser, wkpf_process_enabled_wuclasses_handler) != 0) {
        fprintf(stderr, "parse error on line %li:\n%s\n", simpleXmlGetLineNumber(parser), simpleXmlGetErrorDescription(parser));
        exit(1);
    }
    free(sData);
    return WKPF_OK;
}
