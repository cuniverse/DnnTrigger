
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
 *	@file	hash_table.c
 *	@ingroup common_base_src
 *	@date	2007/06/01
 *	@author	최인정(ijchoi@hcilab.co.kr) (주)HCILAB(http://www.hcilab.co.kr)
 *	@brief	hash table module.
 *
 * This hash tables are intended for associating a pointer/integer
 * "value" with a char string "key", (e.g., an ID with a word string).
 * Subsequently, one can retrieve the value by providing the string
 * key.  (The reverse functionality--obtaining the string given the
 * value--is not provided with the hash table module.)
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/hash_table.h"
#include "base/hci_msg.h"
#include "base/hci_malloc.h"
#include "base/case.h"


/*
 * HACK!!  Initial hash table size is restricted by this set of primes.  (Of course,
 * collision resolution by chaining will accommodate more entries indefinitely, but
 * efficiency will drop.)
 */
const hci_int32 prime[] = {
    101, 211, 307, 401, 503, 601, 701, 809, 907,
    1009, 1201, 1601, 2003, 2411, 3001, 4001, 5003, 6007, 7001, 8009,
    9001,
    10007, 12007, 16001, 20011, 24001, 30011, 40009, 50021, 60013,
    70001, 80021, 90001,
    100003, 120011, 160001, 200003, 240007, 300007, 400009, 500009,
    600011, 700001, 800011, 900001,
    -1
};


// local functions

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * This function returns a very large prime. 
 */
HCILAB_PRIVATE hci_int32
_prime_size(hci_int32 size);

/**
 * Compute hash value for given key string.
 * Somewhat tuned for English text word strings.
 */
HCILAB_PRIVATE hci_uint32
_key2hash(hash_table_t * h, const char *key);

HCILAB_PRIVATE char *
_makekey(hci_uint8 * data, hci_int32 len, char *key);

HCILAB_PRIVATE hci_int32
_keycmp_nocase(hash_entry_t * entry, const char *key);

HCILAB_PRIVATE hci_int32
_keycmp_case(hash_entry_t * entry, const char *key);

/**
 * Lookup entry with hash-value hash in table h for given key
 * @return Return value: hash_entry_t for key
 */
HCILAB_PRIVATE hash_entry_t *
_lookup(hash_table_t * h, hci_uint32 hash, const char *key, size_t len);

/**
 * add a new entry into hash table
 */
HCILAB_PRIVATE void *
_enter(hash_table_t * h, hci_uint32 hash, const char *key, size_t len, void *val, hci_int32 replace);

/**
 * delete a key from a hash table
 */
HCILAB_PRIVATE void *
_delete(hash_table_t * h, hci_uint32 hash, const char *key, size_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus */


/**
 * Allocate a new hash table for a given expected size.
 *
 * @return Return value: READ-ONLY handle to allocated hash table.
 */
HCILAB_PUBLIC HCI_BASE_API hash_table_t *
PowerASR_Base_newHashTable(hci_int32 size,		/**< In: Expected #entries in the table */
						   hci_int32 casearg )  /**< In: Whether case insensitive for key
                                                   comparisons. When 1, case is insentitive,
                                                   0, case is sensitive. */
{
    hash_table_t *h = 0;

    h = (hash_table_t *) hci_calloc(1, sizeof(hash_table_t));
    h->size = _prime_size(size + (size >> 1));
    h->nocase = (casearg == HASH_CASE_NO);
    h->table = (hash_entry_t *) hci_calloc(h->size, sizeof(hash_entry_t));
    /* The above calloc clears h->table[*].key and .next to NULL, i.e. an empty table */

    return h;
}


/**
 * Free the specified hash table; the caller is responsible for freeing the key strings
 * pointed to by the table entries.
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_freeHashTable(hash_table_t *h	)	/**< In: Handle of hash table to free */
{
    hash_entry_t *e = 0;
	hash_entry_t *e2 = 0;
    hci_int32 i = 0;

    /* Free additional entries created for key collision cases */
    for (i = 0; i < h->size; i++) {
        for (e = h->table[i].next; e; e = e2) {
            e2 = e->next;
            hci_free((void *) e);
        }
    }

    hci_free((void *) h->table);
    hci_free((void *) h);
}


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
							 void *val )		/**< In: Value to be associated with above key */
{
    hci_uint32 hash = 0;
    size_t len = 0;

    hash = _key2hash(h, key);
    len = strlen(key);
    return (_enter(h, hash, key, len, val, 0));
}


/**
 * Add a new entry with given key and value to hash table h.  If the
 * key already exists, its value is replaced with the given value, and
 * the previous value is returned, otherwise val is returned.
 */
HCILAB_PUBLIC HCI_BASE_API void *
PowerASR_Base_replaceHashTable(hash_table_t *h,	/**< In: Handle of hash table in which to create entry */
							   const char *key, /**< In: C-style NULL-terminated key string
												for the new entry */
							   void *val )		/**< In: Value to be associated with above key */
{
    hci_uint32 hash = 0;
    size_t len = 0;

    hash = _key2hash(h, key);
    len = strlen(key);
    return (_enter(h, hash, key, len, val, 1));
}


/**
 * Delete an entry with given key and associated value to hash table
 * h.  Return the value associated with the key (NULL if it did not exist)
 */

HCILAB_PUBLIC HCI_BASE_API void *
PowerASR_Base_deleteHashTable(hash_table_t *h,	/**< In: Handle of hash table in
													which a key will be deleted */
							  const char *key )	/**< In: C-style NULL-terminated
													key string for the new entry */
{
    hci_uint32 hash = 0;
    size_t len = 0;

    hash = _key2hash(h, key);
    len = strlen(key);

    return (_delete(h, hash, key, len));
}


/**
 * Delete all entries from a hash_table.
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_emptyHashTable(hash_table_t *h )	/**< In: Handle of hash table */
{
    hash_entry_t *e = 0;
	hash_entry_t *e2 = 0;
    hci_int32 i = 0;

    for (i = 0; i < h->size; i++) {
        /* Free collision lists. */
        for (e = h->table[i].next; e; e = e2) {
            e2 = e->next;
            hci_free((void *) e);
        }
        memset(&h->table[i], 0, sizeof(h->table[i]));
    }
    h->inuse = 0;
}


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
										  void *val	)		/**< In: Value to be associated with above key */
{
    hci_uint32 hash = 0;
    char *str = 0;

    str = _makekey((hci_uint8 *) key, len, NULL);
    hash = _key2hash(h, str);
    hci_free(str);

    return (_enter(h, hash, key, len, val, 0));
}


/**
 * Lookup hash table h for given key and return the associated value in *val.
 * Return value: 0 if key found in hash table, else -1.
 */
HCILAB_PUBLIC HCI_BASE_API hci_int32
PowerASR_Base_lookupHashTable(hash_table_t *h,	/**< In: Handle of hash table being searched */
							  const char *key,	/**< In: C-style NULL-terminated string whose value is sought */
							  void **val )	  	/**< Out: *val = value associated with key */
{
    hash_entry_t *entry = 0;
    hci_uint32 hash = 0;
    hci_int32 len = 0;

    hash = _key2hash(h, key);
    len = strlen(key);

    entry = _lookup(h, hash, key, len);
    if (entry) {
        *val = entry->val;
        return 0;
    }
    else {
        return -1;
	}
}


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
										   void **val )		/**< Out: *val = value associated with key */
{
    hash_entry_t *entry = 0;
    hci_uint32 hash = 0;
    char *str = 0;

    str = _makekey((hci_uint8 *) key, len, NULL);
    hash = _key2hash(h, str);
    hci_free(str);

    entry = _lookup(h, hash, key, len);
    if (entry) {
        *val = entry->val;
        return 0;
    }
    else {
        return -1;
	}
}


/**
 * Build a glist of valid hash_entry_t pointers from the given hash table.  Return the list.
 */
HCILAB_PUBLIC HCI_BASE_API glist_t
PowerASR_Base_buildListFromHashTable(hash_table_t *h,		/**< In: Hash table from which list is to be generated */
									 hci_int32 *count )		/**< Out: #entries in the list */
{
    glist_t g = 0;
    hash_entry_t *e = 0;
    hci_int32 i = 0;
	hci_int32 j = 0;

    j = 0;
    for (i = 0; i < h->size; i++) {
        e = &(h->table[i]);

        if (e->key != NULL) {
            g = PowerASR_Base_addGListPtr(g, (void *) e);
            j++;

            for (e = e->next; e; e = e->next) {
                g = PowerASR_Base_addGListPtr(g, (void *) e);
                j++;
            }
        }
    }

    *count = j;

    return g;
}


/**
 * Display a hash-with-chaining representation on the screen.
 * Currently, it will only works for situation where hash_enter was
 * used to enter the keys. 
 */
HCILAB_PUBLIC HCI_BASE_API void
PowerASR_Base_displayHashTable(hash_table_t *h,			/**< In: Hash table to display */
							   hci_int32 showdisplay )	/**< In: Show the string or not,
														Use 0 if hash_enter_bkey was
														used. */
{
    hash_entry_t *e = 0;
    hci_int32 i = 0;
	hci_int32 j = 0;

    HCIMSG_INFOCONT("Hash with chaining representation of the hash table\n");

    for (i = 0; i < h->size; i++) {
        e = &(h->table[i]);
        if (e->key != NULL) {
            HCIMSG_INFOCONT("|key:");
            if (showdisplay)
                HCIMSG_INFOCONT("%s", e->key);

            HCIMSG_INFOCONT("|len:%d|val=%d|->", e->len, e->val);
            if (e->next == NULL) {
                HCIMSG_INFOCONT("NULL\n");
            }
            j++;

            for (e = e->next; e; e = e->next) {
                HCIMSG_INFOCONT("|key:");
                if (showdisplay)
                    HCIMSG_INFOCONT("%s", e->key);

                HCIMSG_INFOCONT("|len:%d|val=%d|->", e->len, e->val);
                if (e->next == NULL) {
                    HCIMSG_INFOCONT("NULL\n");
                }
                j++;
            }
        }
    }

    HCIMSG_INFOCONT("The total number of keys =%d\n", j);
}


/**
 * This function returns a very large prime. 
 */
HCILAB_PRIVATE hci_int32
_prime_size(hci_int32 size)
{
    hci_int32 i = 0;

    for (i = 0; (prime[i] > 0) && (prime[i] < size); i++);
    if (prime[i] <= 0) {
        HCIMSG_WARN("Very large hash table requested (%d entries)\n", size);
        --i;
    }
    return (prime[i]);
}


/**
 * Compute hash value for given key string.
 * Somewhat tuned for English text word strings.
 */
HCILAB_PRIVATE hci_uint32
_key2hash(hash_table_t * h,
		  const char *key)
{

    register const char *cp = 0;

    /*
	Apply suggested hack of fixing the hash table such that
	it can work with extended ascii code . This is a hack because
	the best way to solve it is to make sure all character
	representation is unsigned character in the first place. (or
	better unicode.)
    */

    /*register char c; */
    register unsigned char c = 0;
    register hci_int32 s = 0;
    register hci_uint32 hash = 0;

    hash = 0;
    s = 0;

    if (h->nocase) {
        for (cp = key; *cp; cp++) {
            c = *cp;
            c = UPPER_CASE(c);
            hash += c << s;
            s += 5;
            if (s >= 25)
                s -= 24;
        }
    }
    else {
        for (cp = key; *cp; cp++) {
            hash += (*cp) << s;
            s += 5;
            if (s >= 25)
                s -= 24;
        }
    }

    return (hash % h->size);
}


HCILAB_PRIVATE char *
_makekey(hci_uint8 * data,
		 hci_int32 len,
		 char *key)
{
    hci_int32 i = 0;
	hci_int32 j = 0;

    if (!key) {
        key = (char *) hci_calloc(len * 2 + 1, sizeof(char));
	}

    for (i = 0, j = 0; i < len; i++, j += 2) {
        key[j] = 'A' + (data[i] & 0x000f);
        key[j + 1] = 'J' + ((data[i] >> 4) & 0x000f);
    }
    key[j] = '\0';

    return key;
}


HCILAB_PRIVATE hci_int32
_keycmp_nocase(hash_entry_t * entry,
			   const char *key)
{
    char c1 = 0;
	char c2 = 0;
    size_t i = 0;
    const char *str = 0;

    str = entry->key;
    for (i = 0; i < entry->len; i++) {
        c1 = *(str++);
        c1 = UPPER_CASE(c1);
        c2 = *(key++);
        c2 = UPPER_CASE(c2);
        if (c1 != c2)
            return (c1 - c2);
    }

    return 0;
}


HCILAB_PRIVATE hci_int32
_keycmp_case(hash_entry_t * entry,
			 const char *key)
{
    char c1 = 0;
	char c2 = 0;
    size_t i = 0;
    const char *str = 0;

    str = entry->key;
    for (i = 0; i < entry->len; i++) {
        c1 = *(str++);
        c2 = *(key++);
        if (c1 != c2)
            return (c1 - c2);
    }

    return 0;
}


/**
 * Lookup entry with hash-value hash in table h for given key
 * Return value: hash_entry_t for key
 */
HCILAB_PRIVATE hash_entry_t *
_lookup(hash_table_t * h,
		hci_uint32 hash,
		const char *key,
		size_t len)
{
    hash_entry_t *entry = 0;

    entry = &(h->table[hash]);
    if (entry->key == NULL) {
        return NULL;
	}

    if (h->nocase) {
        while (entry && ((entry->len != len)
			|| (_keycmp_nocase(entry, key) != 0))) {
            entry = entry->next;
		}
    }
    else {
        while (entry && ((entry->len != len)
			|| (_keycmp_case(entry, key) != 0))) {
            entry = entry->next;
		}
    }

    return entry;
}


/**
 * add a new entry into hash table
 */
HCILAB_PRIVATE void *
_enter(hash_table_t * h,
	   hci_uint32 hash,
	   const char *key,
	   size_t len,
	   void *val,
	   hci_int32 replace)
{
    hash_entry_t *cur = 0;
	hash_entry_t *newentry = 0;

    if ((cur = _lookup(h, hash, key, len)) != NULL) {
        /* Key already exists. */
        if (replace)
            cur->val = val;
        return cur->val;
    }

    cur = &(h->table[hash]);
    if (cur->key == NULL) {
        /* Empty slot at hashed location; add this entry */
        cur->key = key;
        cur->len = len;
        cur->val = val;

        /* Added by ARCHAN at 20050515. This allows deletion could work. */
        cur->next = NULL;

    }
    else {
        /* Key collision; create new entry and link to hashed location */
        newentry = (hash_entry_t *) hci_calloc(1, sizeof(hash_entry_t));
        newentry->key = key;
        newentry->len = len;
        newentry->val = val;
        newentry->next = cur->next;
        cur->next = newentry;
    }
    ++h->inuse;

    return val;
}

/**
 * delete a key from a hash table
 */
HCILAB_PRIVATE void *
_delete(hash_table_t * h,
		hci_uint32 hash,
		const char *key,
		size_t len)
{
    hash_entry_t *entry = 0;
	hash_entry_t *prev = 0;
    void *val = 0;

    prev = NULL;
    entry = &(h->table[hash]);
    if (entry->key == NULL) {
        return NULL;
	}

    if (h->nocase) {
        while (entry && ((entry->len != len)
                         || (_keycmp_nocase(entry, key) != 0))) {
            prev = entry;
            entry = entry->next;
        }
    }
    else {
        while (entry && ((entry->len != len)
                         || (_keycmp_case(entry, key) != 0))) {
            prev = entry;
            entry = entry->next;
        }
    }

    if (entry == NULL) {
        return NULL;
	}

    /* At this point, entry will be the one required to be deleted, prev
       will contain the previous entry
     */
    val = entry->val;

    if (prev == NULL) {
        /* That is to say the entry in the hash table (not the chain) matched the key. */
        /* We will then copy the things from the next entry to the hash table */
        prev = entry;
        if (entry->next) {      /* There is a next entry, great, copy it. */
            entry = entry->next;
            prev->key = entry->key;
            prev->len = entry->len;
            prev->val = entry->val;
            prev->next = entry->next;
            hci_free(entry);
        }
        else {                  /* There is not a next entry, just set the key to null */
            prev->key = NULL;
            prev->len = 0;
            prev->next = NULL;
        }

    }
    else {                      /* This case is simple */
        prev->next = entry->next;
        hci_free(entry);
    }

    /* Do wiring and free the entry */

    --h->inuse;

    return val;
}

// end of file
