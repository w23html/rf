#include "defs_itf.h"

/*
 * returns the type of the table.  Undefined if called before tbl_done_building()
 */
char *tbl_type(Table *t) {
	if(t->done == 1 || t->final_type == NULL) {
		return NULL;
	}
	return t->final_type;
}

/*
 * Return a newly allocated table with all 
 * members set to NULL/zero
 */
Table* tbl_make() {
	Table *tbl = (Table*)calloc(1, sizeof(Table));
	tbl->count = 0;
	tbl->done = 1;
	tbl->num_row = 0;
	return tbl;
}

/*
 * Prepare the table for the addition of a row with num_field values.
 * This will add the row to the tail of the list.
 * Undefined behavior if tbl_done_building() has already been called.
 */
void tbl_start_row(Table* tbl, int num_fields) {
	if(tbl->done) {
		// create a new Row pointer and allocate the memory to each of its part
		Row *temp_row = (Row*)calloc(1, sizeof(Row)); 
		temp_row->data = (u_value*)calloc(num_fields, sizeof(u_value));
		temp_row->type = (char*)calloc(num_fields + 1, sizeof(char));
		temp_row->nfields = num_fields;
		temp_row->findex = 0;
		temp_row->tindex = 0;
		// linked to the first and last of the table and linked to the previous Row
		if(tbl->first == NULL) {//TODO: undefined behavior, the first row is empty, set data is NULL
			tbl->first = temp_row;
		}
		if(tbl->count != 0) {
			tbl->last->next = temp_row;
		}
		tbl->last = temp_row;
		tbl->count = tbl->count + 1;
	}
	return;
}

/* 
 * Add a NULL-terminated string as a field of the current row. Undefined
 * behavior if tbl_done_building() has been called or if the row's fields are already
 * full.
 */
void tbl_add_string_to_row(Table* tbl, char * str) {
	if(tbl->done && tbl->last->findex < tbl->last->nfields) {
		Row* temp_row = tbl->last;
		int index = temp_row->findex;
		(temp_row->data[index]).s_value = str;
		
		*(tbl->last->type + tbl->last->tindex) = 'S';
		if(tbl->last->tindex == tbl->last->nfields) {
			tbl->last->findex = tbl->last->findex + 1;
			tbl->last->tindex = tbl->last->tindex + 1;
			*(tbl->last->type + tbl->last->tindex) = '\0';		
		}
		else {
			tbl->last->findex = tbl->last->findex + 1;
			tbl->last->tindex = tbl->last->tindex + 1;
		}
		str = NULL;
	}
	return;
}

/* 
 * Add a double as a field of the current row. Undefined behavior if
 * tbl_done_building() has been called or if the row's fields are already full.
 */
void tbl_add_double_to_row(Table* tbl, double d) {
	if(tbl->done && tbl->last->findex < tbl->last->nfields) {
		(*(tbl->last->data + tbl->last->findex)).d_value = d;
		*(tbl->last->type + tbl->last->tindex) = 'D';
		if(tbl->last->tindex == tbl->last->nfields) {
			tbl->last->findex = tbl->last->findex + 1;
			tbl->last->tindex = tbl->last->tindex + 1;
			*(tbl->last->type + tbl->last->tindex) = '\0';		
		}
		else {
			tbl->last->findex = tbl->last->findex + 1;
			tbl->last->tindex = tbl->last->tindex + 1;
		}
	}
	return;
}

/* 
 * Ends construction of the table:
 * Finds table type,
 * Removes all rows differing from table type
 * A completed array for Table is finished
 */
Table *tbl_done_building(Table* tbl) {
	tbl->done = 0;
	if(tbl->first == NULL) {
		tbl_free(tbl);
		return tbl_make();
	}
	if(tbl->count < 2) {
		tbl->final_type = tbl->first->type;
		return tbl;
	}

	int i = 1;

	// temporary type, which is the type of the former Row
	char* temp_type = tbl->first->type;
	
	// temporary Row, which is the former one
	Row *temp_rowf = tbl->first;
	
	// temporary Row, which is the later one
	Row *temp_rown = tbl->first->next;
	
	// find type
	for(; i < tbl->count; i++) {
		if(strcmp(temp_type, temp_rown->type) == 0) {
			tbl->final_type = temp_type;
			break;
		}
		temp_type = temp_rown->type;
		temp_rowf = temp_rown;
		temp_rown = temp_rowf->next;
	}
	
	if(tbl->final_type == NULL) {
		tbl_free(tbl);
		return tbl_make();
	}
	
	int j = 0;
	int new_first = 0;
	
	// repoint to the first Row of the former table
	temp_rowf = tbl->first;
	
	for(; j < tbl->count; j++) {
		if(strcmp(temp_rowf->type, tbl->final_type) == 0) {
			if(!new_first) {
				tbl->first = temp_rowf;
				++new_first;
			}
			if(tbl->num_row > 0) {
				tbl->last->next = temp_rowf;
			}
			tbl->last = temp_rowf;
			temp_rowf = temp_rowf->next;
			tbl->num_row = tbl->num_row + 1;
		}
		else {
			temp_rown = temp_rowf;
			temp_rowf = temp_rowf->next; 
			for(int i = 0; i < temp_rown->nfields; i++) {
				if(*(temp_rown->type + i) == 'S') {
					free((temp_rown->data[i]).s_value);
				}
			}
			free(temp_rown->data);
			free(temp_rown->type);
			free(temp_rown);
		}
	}
	tbl->count = tbl->num_row;
	return tbl;
}

/* 
 * Return the number of columns/fields in the Table. 
 */
int tbl_column_count(Table* tbl) {
	return tbl->first->nfields;
}

/*
 * Return the Row at index at either S or D 
 * Undefined behavior if at is out of bounds
 * Undefined behavior if called before tbl_done_building();
 */
Row* tbl_row_at(Table* tbl, int at) {
	if(!tbl->done) {
		if(at < tbl->count) {
			Row *temp_row = tbl->first;
			int i = 0;
			for(; i < at; i++) {
				temp_row = temp_row->next;
			}
			return temp_row;
		}
	}
	return NULL;
}

/* 
 * Return the string value from the fields/columns of row in the at-th position
 * Undefined behavior if at is out of bounds or at-th field is not a char *.  
 */
char* tbl_string_at(Row* row, int at) {
	if(at < row->nfields) {
		if(*(row->type + at) == 'S') {
			return (*(row->data + at)).s_value;
		}
	}
	return NULL;
}

/*
 * Return the double value from the fields/columns of row in the at-th position
 * Undefined behavior if at is out of bounds or at-th field is not a double. 
 */
double tbl_double_at(Row* row, int at) {
	if(at < row->nfields) {
		if(*(row->type + at) == 'D') {
			return (*(row->data + at)).d_value;
		}
	}
	return -1;
}

/* 
 * Print each field of r separated by commas and ending with a newline
 */
void tbl_print_row(Row* row) {
	int i = 0;
	for(; i < row->nfields - 1; i++) {
		if(*(row->type + i) == 'D') {
			printf("%.2f,", (*(row->data + i)).d_value);
		}
		if(*(row->type + i) == 'S') {
			printf("%s,", (*(row->data + i)).s_value);
		}
	}
	if(*(row->type + i) == 'D') {
		printf("%.2f", (*(row->data + i)).d_value);
	}
	if(*(row->type + i) == 'S') {
		printf("%s", (*(row->data + i)).s_value);
	}
	putchar('\n');
	return;
}

/*
 * Print the fields of each row belonging to tbl.
 * Hint use tbl_print_row() for correct formating
 * Undefined behavior if called before tbl_done_building()
 */
void tbl_print(Table* tbl) {
	if(!tbl->done && tbl->first != NULL) {
		int i = 0;
		Row *temp_row = tbl->first;
		for(; i < tbl->count; i++) {
			tbl_print_row(temp_row);
			temp_row = temp_row->next;
		}
	}
	return;
}

/* 
 * Free all memory in a table.  In no case should it cause a 
 * segmentation fault, even it NULL is passed in.
 */
void tbl_free(Table *tbl) {
	if(tbl->first == NULL) {
		free(tbl);
		tbl = NULL;
		return;
	}
	if(tbl->first != NULL && tbl->first->next != NULL) {
		Row *temp_row = tbl->first;
		tbl->first = tbl->first->next;	
		for(int i = 0; i < temp_row->nfields; i++) {
			if(temp_row->type[i] == 'S') {
				free((temp_row->data[i]).s_value);
			}
		}
		free(temp_row->data);
		free(temp_row->type);
		free(temp_row);
		if(tbl->rows != NULL) {
			free(tbl->rows);
			tbl->rows = NULL;
		}
		tbl_free(tbl);
	}
	else if(tbl->first != NULL && tbl->first->next == NULL) {
		for(int i = 0; i <  tbl->first->nfields; i++) {
			if(*(tbl->first->type + i) == 'S') {
				free((tbl->first->data[i]).s_value);
			}
		}
		free(tbl->first->data);
		free(tbl->first->type);
		free(tbl->first);
		tbl->first = NULL;
		tbl->last = NULL;
		free(tbl);
		tbl = NULL;
	}
}


void tbl_free_without_string(Table *tbl) {
	if(tbl->first == NULL) {
		free(tbl);
		tbl = NULL;
		return;
	}
	if(tbl->first != NULL && tbl->first->next != NULL) {
		Row *temp_row = tbl->first;
		tbl->first = tbl->first->next;	
		free(temp_row->data);
		free(temp_row->type);
		free(temp_row);
		if(tbl->rows != NULL) {
			free(tbl->rows);
			tbl->rows = NULL;
		}
		tbl_free_without_string(tbl);
	}
	else if(tbl->first != NULL && tbl->first->next == NULL) {
		free(tbl->first->data);
		free(tbl->first->type);
		free(tbl->first);
		tbl->first = NULL;
		tbl->last = NULL;
		free(tbl);
		tbl = NULL;
	}
}


/* 
 * Return the type of a column, either S or D.
 * Undefined behavior if column is out of bounds
 */
char tbl_row_type_at(Row* row , int column) {
	if(column < row->nfields) {
			return *(row->type + column);
	}
	return 0;
}

/* 
 * Return the number of rows in the table
 */
int tbl_row_count(Table* tbl) {
	if(tbl == NULL) {
		return 0;
	}
	return tbl->count;
}

/*
 * Return an array with all rows of this table.
 * Undefined behavior if called before tbl_done_building()
 */
Row** tbl_rows(Table* tbl) {
	if(!tbl->done) {
		tbl->rows = (Row**)calloc(tbl->count, sizeof(Row*));
		Row* temp_row = tbl->first;
		int i = 0;
		for(; i < tbl->count; i++) {
			*(tbl->rows + i) = temp_row;
			temp_row = temp_row->next;
		}
		return tbl->rows;
	}
}
