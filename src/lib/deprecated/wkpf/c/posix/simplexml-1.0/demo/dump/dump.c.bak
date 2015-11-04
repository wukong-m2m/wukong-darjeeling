// /* $Id: dump.c,v 1.1.1.1 2002/08/23 10:38:58 essmann Exp $
//  *
//  * Copyright (c) 2001-2002 Bruno Essmann <essmann@users.sourceforge.net>
//  * All rights reserved.
//  */

// /* ---- includes */

// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <unistd.h>
// #include "simplexml.h"

// /* ---- prototypes */

// void* handler (SimpleXmlParser parser, SimpleXmlEvent event, 
// 	const char* szName, const char* szAttribute, const char* szValue);
// void parse (char* sData, long nDataLen);
	
// void trim (const char* szInput, char* szOutput);
// char* getIndent (int nDepth);
// char* getReadFileDataErrorDescription (int nError);
// int readFileData (char* sFileName, char** sData, long *pnDataLen);

// /* ---- example xml handler */

// void* handler (SimpleXmlParser parser, SimpleXmlEvent event, 
// 	const char* szName, const char* szAttribute, const char* szValue)
// {
// 	static int nDepth= 0;
// 	char szHandlerName[32], szHandlerAttribute[32], szHandlerValue[32];

// 	if (szName != NULL) {
// 		trim(szName, szHandlerName);
// 	}
// 	if (szAttribute != NULL) {
// 		trim(szAttribute, szHandlerAttribute);
// 	}
// 	if (szValue != NULL) {
// 		trim(szValue, szHandlerValue);
// 	}

// 	if (event == ADD_SUBTAG) {
// 		fprintf(stdout, "%6li: %s add subtag (%s)\n", 
// 			simpleXmlGetLineNumber(parser), getIndent(nDepth), szHandlerName);
// 		nDepth++;
// 	} else if (event == ADD_ATTRIBUTE) {
// 		fprintf(stdout, "%6li: %s add attribute to tag %s (%s=%s)\n", 
// 			simpleXmlGetLineNumber(parser), getIndent(nDepth), szHandlerName, szHandlerAttribute, szHandlerValue);
// 	} else if (event == ADD_CONTENT) {
// 		fprintf(stdout, "%6li: %s add content to tag %s (%s)\n", 
// 			simpleXmlGetLineNumber(parser), getIndent(nDepth), szHandlerName, szHandlerValue);
// 	} else if (event == FINISH_ATTRIBUTES) {
// 		fprintf(stdout, "%6li: %s finish attributes (%s)\n", 
// 			simpleXmlGetLineNumber(parser), getIndent(nDepth), szHandlerName);
// 	} else if (event == FINISH_TAG) {
// 		fprintf(stdout, "%6li: %s finish tag (%s)\n", 
// 			simpleXmlGetLineNumber(parser), getIndent(nDepth), szHandlerName);
// 		nDepth--;
// 	}

// 	return handler;
// }

// void parse (char* sData, long nDataLen) {
// 	SimpleXmlParser parser= simpleXmlCreateParser(sData, nDataLen);
// 	if (parser == NULL) {
// 		fprintf(stderr, "couldn't create parser");
// 		return;
// 	}
// 	if (simpleXmlParse(parser, handler) != 0) {
// 		fprintf(stderr, "parse error on line %li:\n%s\n", 
// 			simpleXmlGetLineNumber(parser), simpleXmlGetErrorDescription(parser));
// 	}
// }

// /* ---- helper functions */

// /**
//  * Copies the input to the output string.
//  *
//  * If the string is less than 32 characters it is
//  * simply copied, otherwise the first 28 characters
//  * are copied and an elipsis (...) is appended.
//  *
//  * @param szInput the input string.
//  * @param szOutput the output string (of at least
//  * 32 characters length).
//  */
// void trim (const char* szInput, char* szOutput) {
// 	int i= 0;
// 	while (szInput[i] != 0 && i < 32) {
// 		if (szInput[i] < ' ') {
// 			szOutput[i]= ' ';
// 		} else {
// 			szOutput[i]= szInput[i];
// 		}
// 		i++;
// 	}
// 	if (i < 32) {
// 		szOutput[i]= '\0';
// 	} else {
// 		szOutput[28]= '.';
// 		szOutput[29]= '.';
// 		szOutput[30]= '.';
// 		szOutput[31]= '\0';
// 	}
// }

// static char* szIndent= NULL;

// /**
//  * Returns an indent string for the specified depth.
//  *
//  * @param nDepth the depth.
//  * @return the indent string.
//  */
// char* getIndent (int nDepth) {
// 	if (nDepth > 500) {
// 		nDepth= 500;
// 	}
// 	if (szIndent == NULL) {
// 		szIndent= malloc(1024);
// 	}
// 	memset(szIndent, ' ', nDepth * 2);
// 	szIndent[nDepth * 2]= '\0';
// 	return szIndent;
// }

// #define READ_FILE_NO_ERROR 0
// #define READ_FILE_STAT_ERROR 1
// #define READ_FILE_OPEN_ERROR 2
// #define READ_FILE_OUT_OF_MEMORY 3
// #define READ_FILE_READ_ERROR 4

// /**
//  * Returns an error description for a readFileData error code.
//  *
//  * @param nError the error code.
//  * @return the error description.
//  */
// char* getReadFileDataErrorDescription (int nError) {
// 	switch (nError) {
// 		case READ_FILE_NO_ERROR: return "no error";
// 		case READ_FILE_STAT_ERROR: return "no such file";
// 		case READ_FILE_OPEN_ERROR: return "couldn't open file";
// 		case READ_FILE_OUT_OF_MEMORY: return "out of memory";
// 		case READ_FILE_READ_ERROR: return "couldn't read file";
// 	}
// 	return "unknown error";
// }

// /**
//  * Reads the complete contents of a file to a character array.
//  *
//  * @param sFileName the name of the file to read.
//  * @param psData pointer to a character array that will be
//  * allocated to read the file contents to.
//  * @param pnDataLen pointer to a long that will hold the 
//  * number of bytes read to the character array.
//  * @return 0 on success, > 0 if there was an error.
//  * @see #getReadFileDataErrorDescription
//  */
// int readFileData (char* sFileName, char** psData, long *pnDataLen) {
// 	struct stat fstat;
// 	*psData= NULL;
// 	*pnDataLen= 0;
// 	if (stat(sFileName, &fstat) == -1) {
// 		return READ_FILE_STAT_ERROR;
// 	} else {
// 		FILE *file= fopen(sFileName, "r");
// 		if (file == NULL) {
// 			return READ_FILE_OPEN_ERROR;
// 		} else {
// 			*psData= malloc(fstat.st_size);
// 			if (*psData == NULL) {
// 				return READ_FILE_OUT_OF_MEMORY;
// 			} else {
// 				size_t len= fread(*psData, 1, fstat.st_size, file);
// 				fclose(file);
// 				if (len != fstat.st_size) {
// 					free(*psData);
// 					*psData= NULL;
// 					return READ_FILE_READ_ERROR;
// 				}
// 				*pnDataLen= len;
// 				return READ_FILE_NO_ERROR;
// 			}
// 		}
// 	}	
// }

// /* ---- main */

// int main (int nArgc, char* szArgv[]) {
// 	int i;
// 	if (nArgc < 2) {
// 		fprintf(stderr, "usage: %s { file }\n", szArgv[0]);
// 		return 1;
// 	}
// 	for (i= 1; i < nArgc; i++) {
// 		char* sData;
// 		long nDataLen;
// 		int nResult= readFileData(szArgv[i], &sData, &nDataLen);
// 		if (nResult != 0) {
// 			fprintf(stderr, "couldn't read %s (%s).\n", szArgv[i], 
// 				getReadFileDataErrorDescription(nResult));
// 		} else {
// 			parse(sData, nDataLen);
// 			free(sData);
// 		}
// 	}
// 	return 0;
// }

