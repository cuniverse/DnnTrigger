
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
 *	@file	hash_table.h
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	Header file to define API functions related to hash table module.
 *
 * This hash tables are intended for associating a pointer/integer
 * "value" with a char string "key", (e.g., an ID with a word string).
 * Subsequently, one can retrieve the value by providing the string
 * key.  (The reverse functionality--obtaining the string given the
 * value--is not provided with the hash table module.)
 */


#ifndef __BASELIB_HASH_TABLE_H__
#define __BASELIB_HASH_TABLE_H__

#include "base/hci_type.h"
#include "base/base_lib.h"
#include "base/glist.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct hash_entry_s
 *
 * The hash table structures.
 * Each hash table is identified by a hash_table_t structure.  hash_table_t.table is
 * pre-allocated for a user-controlled max size, and is initially empty.  As new
 * entries are created (using hash_enter()), the empty entries get filled.  If multiple
 * keys hash to the same entry, new entries are allocated and linked together in a
 * linear list.
 */

typedef struct hash_entry_s {
	const char *key;		/**< Key string, NULL if this is an empty slot.
					    NOTE that the key must not be changed once the entry
					    has been made. */
	size_t len;			/**< Key-length; the key string does not have to be a C-style NULL
					    terminated string; it can have arbitrary binary bytes */
	void *val;			/**< Value associated with above key */
	struct hash_entry_s *next;	/**< For collision resolution */
} hash_entry_t;

typedef struct {
	hash_entry_t *table;	/**< Primary hash table, excluding entries that collide */
	hci_int32 size;		/**< Primary hash table size, (is a prime#); NOTE: This is the
						number of primary entries ALLOCATED, NOT the number of valid
						entries in the table */
	hci_int32 inuse;		/**< Number of valid entries in the table. */
	hci_int32 nocase;		/**< Whether case insensitive for key comparisons */
} hash_table_t;


/** Access macros */
#define hash_entry_val(e)	((e)->val)
#define hash_entry_key(e)	((e)->key)
#define hash_entry_len(e)	((e)->len)
#define hash_table_inuse(h)	((h)->inuse)
#define hash_table_size(h)	((h)->size)


/**
 * Allocate a new hash table for a given expected size.
 *
 * @return Return value: READ-ONLY handle to allocated hash table.
 */
HCILAB_PUBLIC HCI_BASE_API hash_table_t *
PowerASR_Base_newHashTable(hci_int32 size,		/**< In: Expected #entries in the table */
						   hci_int32 casearg  	/**< In: Whether case insensitive for key
                                                   comparisons. When 1, case is insensitive,
                                                   0, case is sensitive. */
);

#define HASH_CASE_YES		0
#define HASH_CASE_NO		1

/**
 * Free the specified hash table; the caller is responsible for freeing the key strings
 * pointed to by the table entries.
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_freeHashTable(hash_table_t *h		/**< In: Handle of hash table to free */
);

/**
 * Try to add a new entry with given key and associated value to hash table h.  If key doesn't
 * already exist in hash table, the addition is successful, and the return value is val.  But
 * if key already exists, return its existing associated value.  (The hash table is unchanged;
 * it is up to the caller to resolve the conflict.)
 */
HCILAB_PUBLIC HCI_BASE_API void *
PowerASR_Base_enterHashTable(hash_table_t *h,	/**< In: Handle of hash table in which to create entry */
							 const char *key,	/**< In: C-style NULL-terminated key string
												for the new entry */
							 void *val			/**< In: Value to be associated with above key */
);

/**
 * Add a new entry with given key and value to hash table h.  If the
 * key already exists, its value is replaced with the given value, and
 * the previous value is returned, otherwise val is returned.
 */
HCILAB_PUBLIC HCI_BASE_API void *
PowerASR_Base_replaceHashTable(hash_table_t *h,	/**< In: Handle of hash table in which to create entry */
							   const char *key, /**< In: C-style NULL-terminated key string
												for the new entry */
							   void *val		/**< In: Value to be associated with above key */
);


/**
 * Delete an entry with given key and associated value to hash table
 * h.  Return the value associated with the key (NULL if it did not exist)
 */

HCILAB_PUBLIC HCI_BASE_API void *
PowerASR_Base_deleteHashTable(hash_table_t *h,	/**< In: Handle of hash table in
													which a key will be deleted */
							  const char *key	/**< In: C-style NULL-terminated
													key string for the new entry */
);

/**
 * Delete all entries from a hash_table.
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_emptyHashTable(hash_table_t *h	/**< In: Handle of hash table */
);

/**
 * Like hash_table_enter, but with an explicitly specified key length,
 * instead of a NULL-terminated, C-style key string.  So the key
 * string is a binary key (or bkey).  Hash tables containing such keys
 * should be created with the HASH_CASE_YES option.  Otherwise, the
 * results are unpredictable.
 */
HCILAB_PUBLIC HCI_BASE_API void *
PowerASR_Base_enterHashTableWithBinaryKey(hash_table_t *h,	/**< In: Handle of hash table
															in which to create entry */
										  const char *key,	/**< In: Key buffer */
										  size_t len,		/**< In: Length of above key buffer */
										  void *val			/**< In: Value to be associated with above key */
);

/**
 * Lookup hash table h for given key and return the associated value in *val.
 * Return value: 0 if key found in hash table, else -1.
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_lookupHashTable(hash_table_t *h,	/**< In: Handle of hash table being searched */
							  const char *key,	/**< In: C-style NULL-terminated string whose value is sought */
							  void **val	  	/**< Out: *val = value associated with key */
);

/**
 * Like hash_lookup, but with an explicitly specified key length, instead of a NULL-terminated,
 * C-style key string.  So the key string is a binary key (or bkey).  Hash tables containing
 * such keys should be created with the HASH_CASE_YES option.  Otherwise, the results are
 * unpredictable.
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_lookupHashTableWithBinaryKey(hash_table_t *h,	/**< In: Handle of hash table being searched */
										   const char *key,	/**< In: Key buffer */
										   size_t len,		/**< In: Length of above key buffer */
										   void **val		/**< Out: *val = value associated with key */
);

/**
 * Build a glist of valid hash_entry_t pointers from the given hash table.  Return the list.
 */
HCILAB_PUBLIC HCI_BASE_API glist_t
PowerASR_Base_buildListFromHashTable(hash_table_t *h,		/**< In: Hash table from which list is to be generated */
									 hci_int32 *count		/**< Out: #entries in the list */
);

/**
 * Display a hash-with-chaining representation on the screen.
 * Currently, it will only works for situation where hash_enter was
 * used to enter the keys. 
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_displayHashTable(hash_table_t *h,		/**< In: Hash table to display */
							   hci_int32 showkey    /**< In: Show the string or not,
														Use 0 if hash_enter_bkey was
														used. */
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __BASELIB_HASH_TABLE_H__
