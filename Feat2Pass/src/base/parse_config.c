
/* ====================================================================
 * Copyright (c) 2007 HCI LAB. 
 * ALL RIGHTS RESERVED.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are prohibited provided that permissions by HCI LAB
 * are not given.
 *
 * ====================================================================
 *
 */

/**
 *	@file	parse_config.c
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Command/Configuration line argument parsing and handling
 *
 *	- A command-line parsing routine that handle command-line input and file input of arguments.
 *  - [Format] name = values
 *	- arguments with '#' or ';' are regarded as comments.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/parse_config.h"
#include "base/hci_msg.h"
#include "base/hci_malloc.h"
#include "base/hash_table.h"
#include "base/case.h"
#include "base/str2words.h"


/** Storage for argument values */
typedef struct argval_s {
	char *arg;		/**< Argument string */
	char *doc;		/**< Documentation/description string */
} argval_t;

static hci_int32 num_argval = 0;
static argval_t *argval = 0;
static hash_table_t *ht = 0;        /**< Hash table */


/**
 * Parse each tokens of an arguments file [format: name = value]
 *  ; arguments with '#' or ';' are regarded as comments.
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_parseConfigFile(const char *filename)	/**< In: A file that contains all the arguments */ 
{
    FILE *fp = 0;
    char szLine[ARG_MAX_LENGTH];
	char szTemp[4][ARG_MAX_LENGTH];
	char *newArg = 0;
	hci_int32 nconfig = 0;
	hci_int32 i = 0;

    if ((fp = fopen(filename, "rt")) == NULL) {
        HCIMSG_ERROR("Cannot open configuration file %s for reading\n",
					 filename);
       return -1;
    }

	nconfig = 0;
	while ( fgets(szLine, ARG_MAX_LENGTH, fp) ) {
		if ( sscanf(szLine, "%s%s%s", szTemp[0], szTemp[1], szTemp[2])==3 &&
			strcmp(szTemp[1],"=")==0 && strncmp(szTemp[0],"#",1)!=0) {
			nconfig++;
		}
	}

	if ( !nconfig ) {
		fclose(fp);
		HCIMSG_ERROR("configuration file %s has none valid arguments !!\n",
					 filename);
       return -1;
	}
	num_argval = nconfig;

    // Allocate memory for argument values
    ht = PowerASR_Base_newHashTable(nconfig, 1 /* argument names are case-insensitive */ );
    argval = (argval_t *) hci_calloc((size_t)num_argval, sizeof(argval_t));
	memset(argval, 0, (size_t)num_argval*sizeof(argval_t));

	// rewind file pointer
	fseek(fp, 0L, SEEK_SET);

    // Enter argument names into hash table
	nconfig = 0;
	while ( fgets(szLine, ARG_MAX_LENGTH, fp) ) {
		if ( sscanf(szLine, "%s%s%s", szTemp[0], szTemp[1], szTemp[2])==3 &&
			strcmp(szTemp[1],"=")==0 && strncmp(szTemp[0],"#",1)!=0 ) {

			newArg = hci_salloc(szTemp[0]);

			// enter a new argument name into hash table
			if (PowerASR_Base_enterHashTable(ht, newArg, (void *)nconfig) != (void *)nconfig) {
				HCIMSG_ERROR("Duplicate argument name: %s\n", szTemp[0]);
				hci_free(newArg);
				goto lc_error_parseConfigFile;
			}

			// Check if argument has already been parsed before
			if (argval[nconfig].doc) {
				HCIMSG_ERROR("Multiple occurrences of argument %s\n", szTemp[0]);
				hci_free(newArg);
				goto lc_error_parseConfigFile;
			}

			// Enter argument value 
			argval[nconfig].arg = newArg;
			if ( szTemp[2][0] == '"' ) {
				char szValue[ARG_MAX_LENGTH];
				char *pStr = 0;
				memset(szValue, 0, sizeof(szValue));
				pStr = strchr(szLine, '"');
				if ( pStr ) strcpy(szValue, pStr + 1);
				else strcpy(szValue, szLine);
				pStr = strrchr(szValue, '"');
				if ( pStr ) *pStr = '\0';
				pStr = strrchr(szValue, '\n');
				if ( pStr ) *pStr = '\0';
				pStr = strrchr(szValue, '\r');
				if ( pStr ) *pStr = '\0';
				argval[nconfig].doc = hci_salloc(szValue);
			}
			else {
				argval[nconfig].doc = hci_salloc(szTemp[2]);
			}

			nconfig++;
		}
	}

    fclose(fp);

    return 0;

lc_error_parseConfigFile:

    if (ht) {
        PowerASR_Base_freeHashTable(ht);
    }
    if (argval) {
        hci_free(argval);
    }

    HCIMSG_ERROR("parseConfigFile failed\n");

    fclose(fp);

    return -1;
}


/**
 * free memory for configurations
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_closeConfigurations()
{
	if ( argval ) {
		hci_int32 i = 0;
		for ( i=0; i < num_argval; i++) {
			if ( argval[i].arg ) {
				hci_free((void *)argval[i].arg);
				argval[i].arg = 0;
			}
			if ( argval[i].doc ) {
				hci_free((void *)argval[i].doc);
				argval[i].doc = 0;
			}
		}
		hci_free((void *) argval);
		argval = 0;
		num_argval = 0;
	}

    if (ht) {
        PowerASR_Base_freeHashTable(ht);
		ht = 0;
	}
}


/**
 * get argument value from hash table & arg-value table
 */
HCILAB_PUBLIC HCI_BASE_API char *
PowerASR_Base_getArgumentValue(const char *strArg)
{
    void *val = 0;

    if (!argval) {
        HCIMSG_ERROR("getArgumentValue invoked before parseConfigFile\n");
		return 0;
	}

    if (PowerASR_Base_lookupHashTable(ht, strArg, &val) < 0) {
        //HCIMSG_ERROR("Unknown argument: %s\n", strArg);
		return 0;
	}
	else {
		return argval[(hci_int32)val].doc;
	}
}

// end of file