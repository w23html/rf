#include "defs_itf.h"


static FILE *f = NULL;

/*
 * Opens a file identified by NULL-terminated string fn for reading.  Has
 * undefined behavior if called more than once or if the file can not be
 * opened.
 */
void rd_open(char* fn){
    /* COMPLETE THIS FUNCTION */
    if(fn == NULL) {
    	return;
    } 
    else {
		f = fopen(fn, "r");
		return;
   	}
    
}

void rd_close() {
	if(f == NULL) {
		return;
	}
	else {
		fclose(f);
		return;
	}
}

/*
 * Returns the next character in the input file or EOF. Has undefined
 * behavior if the file is not open.
 */
int rd_getchar(){
    /* COMPLETE THIS FUNCTION */
    if(f == NULL) {
    	return 0;
    }
    return fgetc(f);
}

/*
 * Parse the character array buf (from "from" index to "to" index, inclusive) 
 * to extract a string discarding leading and trailing spaces and well-formed 
 * double quotes. The format is:
 *
 *   <space>*  '"' <not-quote> '"' <space>*  |
 *   <space>* <not-space-quote-comma> ( <not-quote-comma>* <not-space-quote-comma> )? <space>*
 *
 * where <space> is a space character, <not-quote> is any ASCII character
 * other a double quote, <not-quote-comma> is any ASCII character other the 
 * double quote or comma, <not-space-quote-comma> excludes spaces,
 * commas, and double quotes. A \C{*} means zero or more occurences, \C{?}
 * means zero or one.  If the string does not conform to this format return
 * NULL. The string returned is a newly allocated that is zero terminated.
 */
char* rd_parse_string(char buf[], int from, int to){
    /* COMPLETE THIS FUNCTION */
    if(from == to)
    	return NULL;
    char *str = (char*)calloc(to-from+1, sizeof(char));
    int i = from;
    int j = 0;
    int k = to;
    int quotenum = 0;
	while(buf[i] == ' ') {
		++i;
	}
	if(buf[i] == '"') {
		++i;
		++quotenum;
	}
	if(quotenum == 1) {
		while(buf[i] != '"' && i < to) {
			*(str + j) = buf[i];
			++i;
			++j;
		}
		if(buf[i] == '"') {
			++quotenum;
			++i;
		}
		while(i < to) {
			if(buf[i] != ' ') {
				return NULL;
			}
			++i;
		}
		if(quotenum != 2) {
			return NULL;
		}
		*(str + j) = '\0';
		return str;
	}
	else {
		while(buf[k - 1] == ' ') {
			--k;
		}
		while(buf[i] != '"' && buf[i] != ',' && i < k) {
			*(str + j) = buf[i];
			++i;
			++j;
		}
		if(buf[i] == '"' || buf[i] == ',') {
			if(i != k || buf[i] == '"') {
				return NULL;
			}
		}
		*(str + j) = '\0';
		return str;
	}
}

/*
 * Parse the character array buf (from "from" index to "to" index, inclusive) 
 * to extract a double discarding leading and trailing spaces. 
 * The format for a number is:
 *  <space>* ('+' | '-')?  (<digit>* '.')? <digit>+ <space>*
 * where <space> is a space character, <digit> is a single digit.  If the
 * buf does not contain a number, err is set to 1 and the return value is
 * undefined. Otherwise, the value of the number is returned and err is set
 * to 0.
 */
double rd_parse_number(char buf[], int from, int to, int* err){
    /* COMPLETE THIS FUNCTION */
    if(from == to) {
    	*err = 1;
    	return -1;
    }
    char *doub = (char*)calloc(to-from+1, sizeof(char));
    int i = from;
    int j = 0;
	int dotnum = 0;
	while(buf[i] == ' ') {
		++i;
	}
	if(buf[i] == '-') {
		*doub = '-';
		++i;
		++j;
	}
	if(buf[i] == '+') {
		++i;
	}
	for(; i < to && buf[i] >= '0' && buf[i] <= '9' || buf[i] == '.'; ++i) {
		if(buf[i] == '.') {
			*(doub + j) = '.';
			++j;
			++dotnum;
		}
		else {
			*(doub + j) = buf[i];
			++j;
		}
	}
	while(i < to) {
		if(buf[i] != ' ') {
			*err = 1;
			free(doub);
			return 0.0;
		}
		++i;
	}
	if(dotnum > 1) {
		*err = 1;
		free(doub);
		return 0.0;
	}
	*err = 0;
	double d_return = atof(doub);
	free(doub);
	return d_return;
}


/*
 * Return the number of comma separated values in the array buf.  end has
 * the length of the buffer. Returns -1 if it cannot calculate due to
 * odd number of double quotes. 
 */
int rd_num_fields(char buf[], int end ){
    /* COMPLETE THIS FUNCTION */
    char c;
    int i = 0;
    int even = 0;
    int num = 0;
	int length = 0;
    if(end == 0) {
    	return 0;
    }
    while(1) {
		for(; i < end; ++i) {
			c = buf[i];
			if(c == '"') {
				++even;
			}
			if(c == ',' && even % 2 == 0) {
				break;
			}
			++length;
		}
		++num;
		if(length == 0) {
			return -1;
		}
		if(i == end) {
			if(even % 2 == 0) {
				return num;
			}
			else {
				return -1;
			}
		}
		++i;
		length = 0;
    }
}

/*
 * Scan a sequence of characters from cur to end in buf[] and return the
 * position in the array of the first comma encountered. Return -1 if there 
 * is an uneven number of double quotes.
 */
int rd_field_length(char buf[], int cur, int end ){
    /* COMPLETE THIS FUNCTION */
	char c = 0;
	int iseven = 0;
	int length = cur;
	if(strlen(buf) == 0) {
		return 0;
	}
	if(cur == end) {
		if(buf[cur] == ',' || buf[cur] == '\0') {
			return cur;
		}
		else {
			return -1;
		}
	}
	for(c = buf[length]; length < end; length++) {
		if(buf[length] == '"') {
			++iseven;
		}
		if(buf[length] == ',') {
			if(iseven % 2 == 0) {
				break;
			}
		}		
	}
	if(length == cur) {
		if(buf[length] == ',') {
			return length;
		} else {
			return -1;
		}
	}
	if(iseven % 2 != 0) {
		return -1;
	}
	else {
		return length;
	}
}
