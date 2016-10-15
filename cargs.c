#include "defs_itf.h"

//Function declaration
int compare(char *comp1, char *comp2);
int findpos(char *key);

/**
 * Definitions of Global Variables
 * DO NOT MODIFY THE VARIABLE NAMES
 * UPDATE THE VALUES CORRECTLY IN YOUR FUNCTIONS
 */

//the number of args in the table
int pos;

//the key-value table, both should store NULL-terminated strings
char **keys;
char **values;


/**
 *  Check if the null-terminated string key is defined. Answer 1 if either
 * "-key" or "-key=val" were passed at the command line, 0 otherwise.
 */
int ca_defined(char *key) {
 	int i = 0;
 	if(*key == '\0') {
 		return 0;
 	}
	for(i = 0; i < pos; i++) {	
		if(compare(key, *(keys + i))) {
			return 1;
		}
	}
	return 0;
}
/**
 * If "key" is associated to "val", returns "val". 
 * Undefined behavior, if "key" is not found.
 * You may need the helper function a2p.
 */
char *ca_str_value(char *key) {
	int i = 0;
	while(!compare(key, *(keys + i))) {
		++i;
	}
	if(*(values + i) != '\0') {
		return *(values + i);
	}
	else {
		return NULL;
	}
}


/**
 * If "key" is associated to "val", returns atoi(val). If "val" is NULL,
 * returns -1.  Undefined behavior, if "key" is not found.
 */
int ca_int_value(char *key) {
	int i = 0;
	while(!compare(key, *(keys + i))) {
		++i;
	}
	if(*(values + i) != '\0') {
		return atoi(*(values + i));
	}
	else {
		return -1;
	}
}

/**
 * Parse string with length len and add it to the key-values table.
 * The string should be of format "-key=value" or "-key" where key and value are
 * non-empty. If the format does not match, do nothing.
 *
 * Note: update pos correctly. It is the index of key-value table.
 */
void process_string(int len, char string[]) {
	int strloc = 0;
	int keyloc = 0;
	int valloc = 0;
	int k = 0;
	int p = 0;
	char keywords[len];
	char valuewords[len];
	
	if(string[strloc] != '-') {
		return; // incorrect format 
	}
	else {
		for(++strloc; string[strloc] != '\0' && string[strloc] != '='; ++strloc) {
			keywords[keyloc] = string[strloc];
			++keyloc;	
		}
		keywords[keyloc] = '\0';
		if(strloc == 1) {
			return; // NULL for the key
		}
		// null value key
		if(string[strloc] == '\0') {
			if(ca_defined(keywords)) {
				p = findpos(keywords);
				*(values + p) = "\0";
				return; // redefined key's value as NULL
			}
			else {
				keys[pos] = (char*)calloc(strlen(keywords) + 1, sizeof(char));
				strcpy(keys[pos], (char*) keywords);
				*(values + pos) = (char*)calloc(1, sizeof(char));
				strcpy(values[pos], "\0");
				++pos;
				return; // store a new key and set its value as NULL
			}
		}
		// key with the value
		if(string[strloc] == '=') {
			++strloc;
			valloc = 0;
			for(; string[strloc] != '\0'; ++valloc) {
				valuewords[valloc] = string[strloc];
				++strloc;
			}
			valuewords[valloc] = '\0';
			if(valloc == 0) {
				return; // NULL for the value
			}
			else if(string[strloc] == '\0'){
				if(ca_defined(keywords)) {
					p = findpos(keywords);
					if(strlen(valuewords) > strlen(*(values + p))) {
						values[p] = (char*)realloc(values[p], strlen(valuewords) * sizeof(char) + 1);
						strcpy(values[p], valuewords);
					}
					else {
						strcpy(values[p], (char*) valuewords);	
					}
					return; // redefined key's value as a new non-NULL value
				}
				else {
					keys[pos] = (char*)calloc(strlen(keywords) + 1, sizeof(char));
					strcpy(keys[pos], (char*) keywords);
					*(values + pos) = (char*)calloc(strlen(valuewords) + 1, sizeof(char));
					strcpy(*(values + pos), valuewords);
					++pos;					
					return; //	store a new key and set its value as valuewords	
				}
			}
			else {
				return; // value is exceed the max number of the words
			}
		}
		// key is exceed the max number of the words 
		else {
			return;
		}
	}
}

/**
 * Compare two strings, if they are the same, return 1,
 * else return 0
 */
int compare(char *comp1, char *comp2) {
	int i = 0;
	for(; i < strlen(comp1) && i < strlen(comp2); ++i) {
		if(*(comp1 + i) != *(comp2 + i)) {
			return 0;
		}
	}
	if(*(comp1 + i) == '\0' && *(comp2 + i) == '\0') {
		return 1;
	}
	else {
		return 0;
	}
}

/**
 * Find the position of a defined key
 */
int findpos(char *key) {
 	int i = 0;
	for(i = 0; i < pos; i++) {	
		if(compare(key, *(keys + i))) {
			return i;
		}
	}
	return 0;
}

/*
 * Frees all memory used by cargs
 */
void ca_free() {
	int i = 0;
	for(; i < pos; i++) {
		free(keys[i]);
		free(values[i]);
	}
	free(keys);
	free(values);
}

/**
 * Read and parse command line arguments. argv is an array of argc strings.
 * The format is "-key" or "-key=val" where "key" and "val" are sequences of
 * alpha-numeric characters. All other string are silently discarded.  For
 * any key, only its last definition is retained.  "-key" has a NULL value.
 */
/* NO NEED TO MODIFY. */
void ca_init(int argc, char **argv) {
	if (argc == 0 || argv == NULL) return;
	keys = (char**)malloc(sizeof(char*) * argc);
	values = (char**)malloc(sizeof(char*) * argc);
	for (int i = 1; i < argc; i++)
		process_string(strlen(argv[i]), argv[i]);
}
