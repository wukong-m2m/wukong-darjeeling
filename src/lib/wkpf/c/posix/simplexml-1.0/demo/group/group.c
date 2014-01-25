// /* $Id: group.c,v 1.1.1.1 2002/08/23 10:38:59 essmann Exp $
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

// /* ---- types */

// typedef struct person {
// 	char* szAlias;
// 	char* szFirstname;
// 	char* szLastname;
// 	char* szEmail;
// } TPerson, *Person;

// typedef struct person_list {
// 	Person person;
// 	struct person_list* next;
// } TPersonList, *PersonList;

// typedef struct mailgroup {
// 	char* szName;
// 	char* szDescription;
// 	PersonList members;
// } TMailgroup, *Mailgroup;

// typedef struct mailgroup_list {
// 	Mailgroup mailgroup;
// 	struct mailgroup_list* next;
// } TMailgroupList, *MailgroupList;

// /* ---- prototypes */

// /* group and person handling */
// Person personCreate ();
// PersonList personListCreate ();
// int personListAddPerson (PersonList pl, Person person);
// Person personListGetPerson (PersonList pl, const char* szAlias);
// Mailgroup mailgroupCreate ();
// int mailgroupAddPerson (Mailgroup mailgroup, Person person);
// MailgroupList mailgroupListCreate ();
// int mailgroupListAddMailgroup (MailgroupList ml, Mailgroup mailgroup);
// Mailgroup mailgroupListGetMailgroup (MailgroupList ml, const char* szName);

// /* group parser */
// char* parserGetErrorDescription (int nError);
// void* personContentHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue);
// void* personHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue);
// void* groupPassOneHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue);
// void* passOneHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue);
// void* memberContentHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue);
// void* mailgroupHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue);
// void* groupPassTwoHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue);
// void* passTwoHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue);
// int parse (char* sData, long nDataLen, 
// 	PersonList* pPersonList, MailgroupList* pMailgroupList);

// /* helper functions */
// int isWhitespace (const char* szInput);
// char* getReadFileDataErrorDescription (int nError);
// int readFileData (char* sFileName, char** sData, long *pnDataLen);
// void usage (char* szApp);

// /* ---- group and person handling */

// Person personCreate () {
// 	Person person= malloc(sizeof(TPerson));
// 	if (person != NULL) {
// 		person->szAlias= NULL;
// 		person->szFirstname= NULL;
// 		person->szLastname= NULL;
// 		person->szEmail= NULL;
// 	}
// 	return person;
// }

// PersonList personListCreate () {
// 	PersonList pl= malloc(sizeof(TPersonList));
// 	if (pl != NULL) {
// 		pl->person= NULL;
// 		pl->next= NULL;
// 	}
// 	return pl;
// }

// int personListAddPerson (PersonList pl, Person person) {
// 	if (pl == NULL) {
// 		return 0;
// 	}
// 	if (pl->person == NULL) {
// 		pl->person= person;
// 		return 1;
// 	}
// 	while (pl->person != person && pl->next != NULL) {
// 		pl= pl->next;
// 	}
// 	if (pl->next == NULL) {
// 		pl->next= personListCreate();
// 		if (pl->next == NULL) {
// 			return 0;
// 		}
// 		pl->next->person= person;
// 	}
// 	return 1;
// }

// Person personListGetPerson (PersonList pl, const char* szAlias) {
// 	while (pl != NULL) {
// 		if (pl->person->szAlias != NULL) {
// 			if (strcmp(pl->person->szAlias, szAlias) == 0) {
// 				return pl->person;
// 			}
// 		}
// 		pl= pl->next;
// 	}
// 	return NULL;
// }

// Mailgroup mailgroupCreate () {
// 	Mailgroup mailgroup= malloc(sizeof(TMailgroup));
// 	if (mailgroup != NULL) {
// 		mailgroup->szName= NULL;
// 		mailgroup->szDescription= NULL;
// 		mailgroup->members= personListCreate();
// 		if (mailgroup->members == NULL) {
// 			free(mailgroup);
// 			return NULL;
// 		}
// 	}
// 	return mailgroup;
// }

// int mailgroupAddPerson (Mailgroup mailgroup, Person person) {
// 	if (mailgroup == NULL) {
// 		return 0;
// 	}
// 	return personListAddPerson(mailgroup->members, person);
// }

// MailgroupList mailgroupListCreate () {
// 	MailgroupList ml= malloc(sizeof(TMailgroupList));
// 	if (ml != NULL) {
// 		ml->mailgroup= NULL;
// 		ml->next= NULL;
// 	}
// 	return ml;
// }

// int mailgroupListAddMailgroup (MailgroupList ml, Mailgroup mailgroup) {
// 	if (ml == NULL) {
// 		return 0;
// 	}
// 	if (ml->mailgroup == NULL) {
// 		ml->mailgroup= mailgroup;
// 		return 1;
// 	}
// 	while (ml->mailgroup != mailgroup && ml->next != NULL) {
// 		ml= ml->next;
// 	}
// 	if (ml->next == NULL) {
// 		ml->next= mailgroupListCreate();
// 		if (ml->next == NULL) {
// 			return 0;
// 		}
// 		ml->next->mailgroup= mailgroup;
// 	}
// 	return 1;
// }

// Mailgroup mailgroupListGetMailgroup (MailgroupList ml, const char* szName) {
// 	while (ml != NULL) {
// 		if (ml->mailgroup->szName != NULL) {
// 			if (strcmp(ml->mailgroup->szName, szName) == 0) {
// 				return ml->mailgroup;
// 			}
// 		}
// 		ml= ml->next;
// 	}
// 	return NULL;
// }

// /* ---- example xml handler */

// #define OUT_OF_MEMORY (SIMPLE_XML_USER_ERROR + 0)
// #define ILLEGAL_TAG (SIMPLE_XML_USER_ERROR + 10)
// #define ILLEGAL_ATTRIBUTE (SIMPLE_XML_USER_ERROR + 11)
// #define ILLEGAL_CONTENT (SIMPLE_XML_USER_ERROR + 12)
// #define DOUBLY_DEFINED_TAG (SIMPLE_XML_USER_ERROR + 20)
// #define DOUBLY_DEFINED_ATTRIBUTE (SIMPLE_XML_USER_ERROR + 21)
// #define DOUBLY_DEFINED_PERSON (SIMPLE_XML_USER_ERROR + 22)
// #define DOUBLY_DEFINED_MAILGROUP (SIMPLE_XML_USER_ERROR + 23)
// #define INCOMPLETE_PERSON (SIMPLE_XML_USER_ERROR + 30)
// #define UNKNOWN_PERSON (SIMPLE_XML_USER_ERROR + 31)
// #define ATTRIBUTE_ALIAS_EXPECTED (SIMPLE_XML_USER_ERROR + 40)
// #define ATTRIBUTE_NAME_EXPECTED (SIMPLE_XML_USER_ERROR + 41)

// char* parserGetErrorDescription (int nError) {
// 	switch (nError) {
// 		case OUT_OF_MEMORY: return "out of memory";
// 		case ILLEGAL_TAG: return "illegal tag encountered";
// 		case ILLEGAL_ATTRIBUTE: return "illegal attribute encountered";
// 		case ILLEGAL_CONTENT: return "illegal content encountered";
// 		case DOUBLY_DEFINED_TAG: return "doubly defined tag";
// 		case DOUBLY_DEFINED_ATTRIBUTE: return "doubly defined attribute";
// 		case DOUBLY_DEFINED_PERSON: return "doubly defined person";
// 		case DOUBLY_DEFINED_MAILGROUP: return "doubly defined mailgroup";
// 		case INCOMPLETE_PERSON: return "person is incomplete";
// 		case UNKNOWN_PERSON: return "unknown person alias";
// 		case ATTRIBUTE_ALIAS_EXPECTED: return "attribute 'alias' missing";
// 		case ATTRIBUTE_NAME_EXPECTED: return "attribute 'name' missing";
// 	}
// 	return "unknown error";
// }

// /** parses the 'firstname', 'lastname' and 'email' tags */
// void* personContentHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue)
// {
// 	if (event == ADD_SUBTAG) {
// 		simpleXmlParseAbort(parser, ILLEGAL_TAG);
// 		return NULL;
// 	} else if (event == ADD_ATTRIBUTE) {
// 		simpleXmlParseAbort(parser, ILLEGAL_ATTRIBUTE);
// 		return NULL;
// 	} else if (event == ADD_CONTENT) {
// 		Person p= (Person) simpleXmlGetUserData(parser);
// 		if (strcmp(szName, "firstname") == 0) {
// 			if (p->szFirstname != NULL) {
// 				simpleXmlParseAbort(parser, DOUBLY_DEFINED_TAG);
// 				return NULL;
// 			}
// 			p->szFirstname= strdup(szValue);
// 		} else if (strcmp(szName, "lastname") == 0) {
// 			if (p->szLastname != NULL) {
// 				simpleXmlParseAbort(parser, DOUBLY_DEFINED_TAG);
// 				return NULL;
// 			}
// 			p->szLastname= strdup(szValue);
// 		} else if (strcmp(szName, "email") == 0) {
// 			if (p->szEmail != NULL) {
// 				simpleXmlParseAbort(parser, DOUBLY_DEFINED_TAG);
// 				return NULL;
// 			}
// 			p->szEmail= strdup(szValue);
// 		}
// 	}
// 	return NULL;
// }

// /** parses the 'person' tag */
// void* personHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue)
// {
// 	if (event == ADD_SUBTAG) {
// 		if (
// 			strcmp(szName, "firstname") != 0 &&
// 			strcmp(szName, "lastname") != 0 &&
// 			strcmp(szName, "email") != 0
// 		) {
// 			simpleXmlParseAbort(parser, ILLEGAL_TAG);
// 			return NULL;
// 		}
// 		return personContentHandler;
// 	} else if (event == ADD_ATTRIBUTE) {
// 		if (strcmp(szAttribute, "alias") != 0) {
// 			simpleXmlParseAbort(parser, ILLEGAL_ATTRIBUTE);
// 			return NULL;
// 		} else {
// 			PersonList pl= (PersonList) simpleXmlGetUserDataAt(parser, 1);
// 			Person p= (Person) simpleXmlGetUserData(parser);
// 			if (p->szAlias != NULL) {
// 				simpleXmlParseAbort(parser, DOUBLY_DEFINED_ATTRIBUTE);
// 				return NULL;
// 			}
// 			if (personListGetPerson(pl, szValue) != NULL) {
// 				simpleXmlParseAbort(parser, DOUBLY_DEFINED_PERSON);
// 				return NULL;
// 			}
// 			p->szAlias= strdup(szValue);
// 		}
// 	} else if (event == FINISH_ATTRIBUTES) {
// 		Person p= (Person) simpleXmlGetUserData(parser);
// 		if (p->szAlias == NULL) {
// 			simpleXmlParseAbort(parser, ATTRIBUTE_ALIAS_EXPECTED);
// 			return NULL;
// 		}
// 	} else if (event == ADD_CONTENT) {
// 		if (!isWhitespace(szValue)) {
// 			simpleXmlParseAbort(parser, ILLEGAL_CONTENT);
// 			return NULL;
// 		}
// 	} else if (event == FINISH_TAG) {
// 		Person p= (Person) simpleXmlGetUserData(parser);
// 		if (
// 			p->szFirstname == NULL ||
// 			p->szLastname == NULL ||
// 			p->szEmail == NULL
// 		) {
// 			simpleXmlParseAbort(parser, INCOMPLETE_PERSON);
// 			return NULL;
// 		}
// 		/* pop off the current person */
// 		simpleXmlPopUserData(parser);
// 	}
// 	return NULL;
// }

// /** parses the 'group' tag on the first pass */
// void* groupPassOneHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue)
// {
// 	if (event == ADD_SUBTAG) {
// 		if (strcmp(szName, "person") == 0) {
// 			Person p= personCreate();
// 			if (p == NULL) {
// 				simpleXmlParseAbort(parser, OUT_OF_MEMORY);
// 				return NULL;
// 			} else {
// 				PersonList pl= (PersonList) simpleXmlGetUserData(parser);
// 				personListAddPerson(pl, p);
// 				/* push the person on the user data stack */
// 				simpleXmlPushUserData(parser, p);
// 				/* parse the "person" tag using the personHandler */
// 				return personHandler;
// 			}
// 		} else if (strcmp(szName, "mailgroup") == 0) {
// 			/* ignore mailgroup on pass one (this tag an all its
// 			   subtags are ignore by the parser */
// 			return NULL;
// 		} else {
// 			/* didn't expect any other tag at this stage */
// 			simpleXmlParseAbort(parser, ILLEGAL_TAG);
// 			return NULL;
// 		}
// 	} else if (event == ADD_ATTRIBUTE) {
// 		simpleXmlParseAbort(parser, ILLEGAL_ATTRIBUTE);
// 		return NULL;
// 	} else if (event == ADD_CONTENT) {
// 		if (!isWhitespace(szValue)) {
// 			simpleXmlParseAbort(parser, ILLEGAL_CONTENT);
// 			return NULL;
// 		}
// 	}
// 	return NULL;
// }

// /** document handler for the first pass */
// void* passOneHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue)
// {
// 	if (event == ADD_SUBTAG) {
// 		if (strcmp(szName, "group") == 0) {
// 			PersonList pl= personListCreate();
// 			if (pl == NULL) {
// 				simpleXmlParseAbort(parser, OUT_OF_MEMORY);
// 				return NULL;
// 			}
// 			/* push the person list on the user data stack */
// 			simpleXmlPushUserData(parser, pl);
// 			/* parse the "group" tag using this handler */
// 			return groupPassOneHandler;
// 		} else {
// 			/* didn't expect any other tag at this stage */
// 			simpleXmlParseAbort(parser, ILLEGAL_TAG);
// 			return NULL;
// 		}
// 	} else if (event == ADD_ATTRIBUTE) {
// 		simpleXmlParseAbort(parser, ILLEGAL_ATTRIBUTE);
// 		return NULL;
// 	} else if (event == ADD_CONTENT) {
// 		if (!isWhitespace(szValue)) {
// 			simpleXmlParseAbort(parser, ILLEGAL_CONTENT);
// 			return NULL;
// 		}
// 	}
// 	return NULL;
// }

// /** parses the 'member' tag */
// void* memberContentHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue)
// {
// 	if (event == ADD_SUBTAG) {
// 		simpleXmlParseAbort(parser, ILLEGAL_TAG);
// 		return NULL;
// 	} else if (event == ADD_ATTRIBUTE) {
// 		simpleXmlParseAbort(parser, ILLEGAL_ATTRIBUTE);
// 		return NULL;
// 	} else if (event == ADD_CONTENT) {
// 		/* retrieve person list pushed right before parsing 
// 		   (it's at level two since at level one is the mailgroup list 
// 			 pushed by the groupPassOneHandler */
// 		PersonList pl= (PersonList) simpleXmlGetUserDataAt(parser, 2);
// 		Mailgroup m= (Mailgroup) simpleXmlGetUserData(parser);
// 		Person p= personListGetPerson(pl, szValue);
// 		if (p == NULL) {
// 			simpleXmlParseAbort(parser, UNKNOWN_PERSON);
// 			return NULL;
// 		}
// 		mailgroupAddPerson(m, p);
// 	}
// 	return NULL;
// }

// /** parses the 'mailgroup' tag */
// void* mailgroupHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue)
// {
// 	if (event == ADD_SUBTAG) {
// 		if (strcmp(szName, "member") != 0) {
// 			simpleXmlParseAbort(parser, ILLEGAL_TAG);
// 			return NULL;
// 		}
// 		return memberContentHandler;
// 	} else if (event == ADD_ATTRIBUTE) {
// 		if (strcmp(szAttribute, "name") == 0) {
// 			MailgroupList ml= (MailgroupList) simpleXmlGetUserDataAt(parser, 1);
// 			Mailgroup m= (Mailgroup) simpleXmlGetUserData(parser);
// 			if (m->szName != NULL) {
// 				simpleXmlParseAbort(parser, DOUBLY_DEFINED_ATTRIBUTE);
// 				return NULL;
// 			}
// 			if (mailgroupListGetMailgroup(ml, szValue) != NULL) {
// 				simpleXmlParseAbort(parser, DOUBLY_DEFINED_MAILGROUP);
// 				return NULL;
// 			}
// 			m->szName= strdup(szValue);
// 		} else if (strcmp(szAttribute, "desc") == 0) {
// 			Mailgroup m= (Mailgroup) simpleXmlGetUserData(parser);
// 			if (m->szDescription != NULL) {
// 				simpleXmlParseAbort(parser, DOUBLY_DEFINED_ATTRIBUTE);
// 				return NULL;
// 			}
// 			m->szDescription= strdup(szValue);
// 		} else {
// 			simpleXmlParseAbort(parser, ILLEGAL_ATTRIBUTE);
// 			return NULL;
// 		}
// 	} else if (event == FINISH_ATTRIBUTES) {
// 		Mailgroup m= (Mailgroup) simpleXmlGetUserData(parser);
// 		if (m->szName == NULL) {
// 			simpleXmlParseAbort(parser, ATTRIBUTE_NAME_EXPECTED);
// 			return NULL;
// 		}
// 	} else if (event == ADD_CONTENT) {
// 		if (!isWhitespace(szValue)) {
// 			printf("***%s***", szValue);
// 			simpleXmlParseAbort(parser, ILLEGAL_CONTENT);
// 			return NULL;
// 		}
// 	} else if (event == FINISH_TAG) {
// 		/* could check that there's at least one member 
// 		   in a mailgroup here, but we're going to allow
// 			 empty mailgroups ... */

// 		/* pop off the current person */
// 		simpleXmlPopUserData(parser);
// 	}
// 	return NULL;
// }

// /** parses the 'group' tag on the second pass */
// void* groupPassTwoHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue)
// {
// 	if (event == ADD_SUBTAG) {
// 		if (strcmp(szName, "mailgroup") == 0) {
// 			Mailgroup m= mailgroupCreate();
// 			if (m == NULL) {
// 				simpleXmlParseAbort(parser, OUT_OF_MEMORY);
// 				return NULL;
// 			} else {
// 				MailgroupList ml= (MailgroupList) simpleXmlGetUserData(parser);
// 				mailgroupListAddMailgroup(ml, m);
// 				/* push the mailgroup on the user data stack */
// 				simpleXmlPushUserData(parser, m);
// 				/* parse the "mailgroup" tag using the mailgroupHandler */
// 				return mailgroupHandler;
// 			}
// 		} else if (strcmp(szName, "person") == 0) {
// 			/* ignore person on pass two (this tag an all its
// 			   subtags are ignore by the parser */
// 			return NULL;
// 		} else {
// 			/* didn't expect any other tag at this stage */
// 			simpleXmlParseAbort(parser, ILLEGAL_TAG);
// 			return NULL;
// 		}
// 	} else if (event == ADD_ATTRIBUTE) {
// 		simpleXmlParseAbort(parser, ILLEGAL_ATTRIBUTE);
// 		return NULL;
// 	} else if (event == ADD_CONTENT) {
// 		if (!isWhitespace(szValue)) {
// 			simpleXmlParseAbort(parser, ILLEGAL_CONTENT);
// 			return NULL;
// 		}
// 	}
// 	return NULL;
// }

// /** document handler for the second pass */
// void* passTwoHandler (SimpleXmlParser parser, SimpleXmlEvent event,
// 	const char* szName, const char* szAttribute, const char* szValue)
// {
// 	if (event == ADD_SUBTAG) {
// 		if (strcmp(szName, "group") == 0) {
// 			MailgroupList ml= mailgroupListCreate();
// 			if (ml == NULL) {
// 				simpleXmlParseAbort(parser, OUT_OF_MEMORY);
// 				return NULL;
// 			}
// 			/* push the mailgroup list on the user data stack */
// 			simpleXmlPushUserData(parser, ml);
// 			/* parse the "group" tag using this handler */
// 			return groupPassTwoHandler;
// 		} else {
// 			/* didn't expect any other tag at this stage */
// 			simpleXmlParseAbort(parser, ILLEGAL_TAG);
// 			return NULL;
// 		}
// 	} else if (event == ADD_ATTRIBUTE) {
// 		simpleXmlParseAbort(parser, ILLEGAL_ATTRIBUTE);
// 		return NULL;
// 	} else if (event == ADD_CONTENT) {
// 		if (!isWhitespace(szValue)) {
// 			simpleXmlParseAbort(parser, ILLEGAL_CONTENT);
// 			return NULL;
// 		}
// 	}
// 	return NULL;
// }

// /** parses the specified data and creates the person and mailgroup list */
// int parse (char* sData, long nDataLen, 
// 	PersonList* pPersonList, MailgroupList* pMailgroupList)
// {
// 	SimpleXmlParser parser;
// 	int nResult;
	
// 	*pPersonList= NULL;
// 	*pMailgroupList= NULL;
	
// 	parser= simpleXmlCreateParser(sData, nDataLen);
// 	if (parser == NULL) {
// 		fprintf(stderr, "couldn't create parser\n");
// 		return 0;
// 	}
	
// 	/* show off how to reuse a parser by parsing in two passes,
// 	   the first pass if for parsing all the person tags */
// 	if ((nResult= simpleXmlParse(parser, passOneHandler)) != 0) {
// 		if (nResult >= SIMPLE_XML_USER_ERROR) {
// 			fprintf(stderr, "parse error on line %li:\n%s\n",
// 				simpleXmlGetLineNumber(parser), 
// 				parserGetErrorDescription(nResult));
// 		} else {
// 			fprintf(stderr, "parse error on line %li:\n%s\n", 
// 				simpleXmlGetLineNumber(parser), 
// 				simpleXmlGetErrorDescription(parser));
// 		}
// 		return 0;
// 	}
// 	/* pop off the person list */
// 	*pPersonList= (PersonList) simpleXmlPopUserData(parser);

// 	/* second pass (initialize using the same data (now that we know
// 	   all person aliases)), parse all mailgroup tags */
// 	simpleXmlInitializeParser(parser, sData, nDataLen);
// 	/* as a specialty push the person list upfront on the user
// 	   data stack of the parser */
// 	simpleXmlPushUserData(parser, *pPersonList);
// 	if ((nResult= simpleXmlParse(parser, passTwoHandler)) != 0) {
// 		if (nResult >= SIMPLE_XML_USER_ERROR) {
// 			fprintf(stderr, "parse error on line %li:\n%s\n",
// 				simpleXmlGetLineNumber(parser), 
// 				parserGetErrorDescription(nResult));
// 		} else {
// 			fprintf(stderr, "parse error on line %li:\n%s\n", 
// 				simpleXmlGetLineNumber(parser), 
// 				simpleXmlGetErrorDescription(parser));
// 		}
// 		*pPersonList= NULL;
// 		return 0;
// 	}
// 	/* pop off the mailgroup list */
// 	*pMailgroupList= (MailgroupList) simpleXmlPopUserData(parser);
	
// 	simpleXmlDestroyParser(parser);
// 	return 1;
// }

// /* ---- helper functions */

// /**
//  * Check if the specified string solely consists of whitespace.
//  *
//  * @param szInput the input string to check.
//  * @return > 0 if it only consists of whitespace, 0 if it contains
//  * a non-whitespace character (anything > ' ').
//  */
// int isWhitespace (const char* szInput) {
// 	while (*szInput) {
// 		if (*szInput > ' ') {
// 			return 0;
// 		}
// 		szInput++;
// 	}
// 	return 1;
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

// void usage (char* szApp) {
// 	fprintf(stderr, 
// 		"usage: %s [-f groupfile] [-l] {name}\n" \
// 		"  -f groupfile  use the specified groupfile (default: group.xml)\n" \
// 		"  -l            lists all persons and mailgroups in the groupfile\n" \
// 		"  name          shows the description of a person or mailgroup\n", 
// 		szApp
// 	);
// }

// int main (int nArgc, char* szArgv[]) {
// 	char* szFile= "group.xml";
// 	PersonList persons= NULL;
// 	MailgroupList mailgroups= NULL;
// 	int bList= 0;
// 	int nArg;
// 	if (nArgc < 2) {
// 		usage(szArgv[0]);
// 		return 1;
// 	}
// 	/* check arguments */
// 	nArg= 1;
// 	while (nArg < nArgc && szArgv[nArg][0] == '-') {
// 		if (strcmp(szArgv[nArg], "-f") == 0) {
// 			nArg++;
// 			if (nArg < nArgc) {
// 				szFile= szArgv[nArg];
// 			} else {
// 				usage(szArgv[0]);
// 				return 1;
// 			}
// 		} else if (strcmp(szArgv[nArg], "-l") == 0) {
// 			bList= 1;
// 		}
// 		nArg++;
// 	}
// 	/* read the data file and fill 'persons' and 'mailgroups' */
// 	{
// 		char* sData;
// 		long nDataLen;
// 		int nResult= readFileData(szFile, &sData, &nDataLen);
// 		if (nResult != 0) {
// 			fprintf(stderr, "couldn't read %s (%s).\n", szFile, 
// 				getReadFileDataErrorDescription(nResult));
// 		} else {
// 			if (!parse(sData, nDataLen, &persons, &mailgroups)) {
// 				free(sData);
// 				return 1;
// 			}
// 			free(sData);
// 		}
// 	}
// 	/* list 'persons' and 'mailgroups' */
// 	if (bList) {
// 		MailgroupList ml= mailgroups;
// 		PersonList pl= persons;
// 		fprintf(stdout, "persons:\n\t");
// 		while (pl != NULL) {
// 			Person p= pl->person;
// 			if (pl != persons) {
// 				fprintf(stdout, ", ");
// 			}
// 			fprintf(stdout, p->szAlias);
// 			pl= pl->next;
// 		}
// 		fprintf(stdout, "\nmailgroups:\n\t");
// 		while (ml != NULL) {
// 			Mailgroup m= ml->mailgroup;
// 			if (ml != mailgroups) {
// 				fprintf(stdout, ", ");
// 			}
// 			fprintf(stdout, m->szName);
// 			ml= ml->next;
// 		}
// 		fprintf(stdout, "\n");
// 	}
// 	/* show description of specified persons and mailgroups */
// 	if (nArg >= nArgc && !bList) {
// 		usage(szArgv[0]);
// 		return 1;
// 	}
// 	while (nArg < nArgc) {
// 		Mailgroup m= mailgroupListGetMailgroup(mailgroups, szArgv[nArg]);
// 		Person p= personListGetPerson(persons, szArgv[nArg]);
// 		if (m != NULL) {
// 			PersonList pl= m->members;
// 			fprintf(stdout, "details for mailgroup %s (%s):",
// 				m->szName, m->szDescription != NULL ? m->szDescription : "<no description>");
// 			while (pl != NULL) {
// 				if (pl->person != NULL) {
// 					fprintf(stdout, "\n\t%s %s <%s>", pl->person->szFirstname, 
// 						pl->person->szLastname, pl->person->szEmail);
// 				} else {
// 					fprintf(stdout, "\n\t<group is empty>");
// 				}
// 				pl= pl->next;
// 			}
// 			fprintf(stdout, "\n");
// 		}
// 		if (p != NULL) {
// 			fprintf(stdout, "details for person %s:\n\t%s %s <%s>\n",
// 				p->szAlias, p->szFirstname, p->szLastname, p->szEmail);
// 		}
// 		if (p == NULL && m == NULL) {
// 			fprintf(stdout, "details for %s:\n" \
// 				"\tunknown person or mailgroup\n", szArgv[nArg]);
// 		}
// 		nArg++;
// 	}
// 	return 0;
// }

