#include "defs_itf.h"
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char **colnames;
double *d_cla; // Contains different classes which is double TODO: free
char **s_cla; // Contains different classes which is string TODO: free all the elements
int elm_count; // Count how many different classes
int t_count;
pthread_mutex_t lock;

typedef struct T_Arg{
	Tree **trees;
	Table *tbl;
	Row **rows;
} T_arg;

int d_cmp(const void *a, const void *b) {
	if( *(double*)a > *(double*)b ) {
		return 1;
	}
	else if( *(double*)a == *(double*)b ) {
		return 0;
	}
	else {
		return -1;
	}
}

int s_cmp(const void *a, const void *b) {
	return strcmp(*(char**)a, *(char**)b);
}

/*
 * Mapping the class which type is D
 * save the extract class into the list
 * return the number of the element in the list
 */
double* d_map(Table *tbl, int *elm) {
	int nr = tbl_row_count(tbl); // Number of rows
	int ci = tbl_column_count(tbl) - 1; // The index of the class
	double *t_list = (double*)calloc( nr, sizeof(double) ); // Temp list which save all the classes
	for(int w = 0; w < nr; w++) {
		t_list[w] = tbl_double_at( tbl_row_at(tbl, w), ci );
	}
	qsort( t_list, nr, sizeof(double), d_cmp );	
	double temp = t_list[0]; // Temp double
	*elm = 1; // number of different classes
	for(int w = 1; w < nr; w++) {
		if(temp < t_list[w]) {
			(*elm)++;
			temp = t_list[w];
		}
	}
	double *list = (double*)calloc(*elm, sizeof(double));
	int index = 0; // index of the list	
	temp = t_list[0];
	list[index] = temp;
	index++;
	for(int w = 1; w < nr; w++) {
		if(temp < t_list[w]) {
			temp = t_list[w];
			list[index] = temp;
			index++;
		}
	}
	free(t_list);
	return list;
}

/*
 * Mapping the class which type is S
 * save the extract class into the list
 * return the number of the element in the list
 */
char** s_map(Table *tbl, int *elm) {
	int nr = tbl_row_count(tbl); // Number of rows
	int ci = tbl_column_count(tbl) - 1; // The index of the class
	char **t_list = (char**)calloc( nr, sizeof(char*) ); // Temp list which save all the classes
	for(int w = 0; w < nr; w++) {
		t_list[w] = tbl_string_at( tbl_row_at(tbl, w), ci );
	}
	qsort( t_list, nr, sizeof(char*), s_cmp );	
	char* temp = t_list[0]; // Temp string
	*elm = 1; // number of different classes
	for(int w = 1; w < nr; w++) {
		if( strcmp(temp, t_list[w]) != 0 ) {
			(*elm)++;
			temp = t_list[w];
		}
	}
	char **list = (char**)calloc(*elm, sizeof(char*));
	int index = 0; // index of the list	
	temp = t_list[0];
	list[index] = (char*)calloc( strlen(temp)+1, sizeof(char) );
	strcpy( list[index], temp );
	index++;
	for(int w = 1; w < nr; w++) {
		if( strcmp(temp, t_list[w]) != 0 ) {
			temp = t_list[w];
			list[index] = (char*)calloc( strlen(temp)+1, sizeof(char) );
			strcpy( list[index], temp );
			index++;
		}
	}
	free(t_list);
	return list;
}

/*
 * Resample the training table
 */
Table* resample(Row **rows, int rn, int cn, char *type) {
	Table *ntbl = tbl_make();
	int w = 0;
	for(int q = 0; q < rn; q++) {
		w = (rand() % rn);
		tbl_start_row(ntbl, cn);
		for(int i = 0; i < cn-1; i++) {
			if(type[i] == 'D') {
				tbl_add_double_to_row( ntbl, tbl_double_at(rows[w], i) );
			}
			else if(type[i] == 'S') {
				char *temp = tbl_string_at(rows[w], i);
				tbl_add_string_to_row( ntbl, temp );
			}
		}
		if(type[cn-1] == 'D') {
			double temp = tbl_double_at(rows[w],cn-1);
			for(int w = 0; w < elm_count; w++) {
				if(temp == d_cla[w]) {
					tbl_add_double_to_row( ntbl, w );
				}
			}
		}
		else if(type[cn-1] == 'S') {
			char *temp = tbl_string_at(rows[w], cn-1);
			for(int w = 0; w < elm_count; w++) {
				if( strcmp(temp, s_cla[w]) == 0 ) {
					tbl_add_double_to_row( ntbl, w );
				}
			} 
		}
	}
	return tbl_done_building(ntbl);
}

/*
 * Renew the validation table
 */
Table* renew(Row **rows, int rn, int cn, char *type) {
	Table *ntbl = tbl_make();
	int w = 0;
	for(int w = 0; w < rn; w++) {
		tbl_start_row(ntbl, cn);
		for(int i = 0; i < cn-1; i++) {
			if(type[i] == 'D') {
				tbl_add_double_to_row( ntbl, tbl_double_at(rows[w], i) );
			}
			else if(type[i] == 'S') {
				char *temp = tbl_string_at(rows[w], i);
				char *adding = (char*)calloc( strlen(temp)+1, sizeof(char) );
				strcpy(adding, temp);
				tbl_add_string_to_row( ntbl, adding );
			}
		}
		if(type[cn-1] == 'D') {
			double temp = tbl_double_at(rows[w],cn-1);
			for(int w = 0; w < elm_count; w++) {
				if(temp == d_cla[w]) {
					tbl_add_double_to_row( ntbl, w );
				}
			}
		}
		else if(type[cn-1] == 'S') {
			char *temp = tbl_string_at(rows[w], cn-1);
			for(int w = 0; w < elm_count; w++) {
				if( strcmp(temp, s_cla[w]) == 0 ) {
					tbl_add_double_to_row( ntbl, w );
				}
			} 
		}
	}
	return tbl_done_building(ntbl);
}

void* build_trees(void *t_arg) {
	T_arg *arg = (T_arg*)t_arg; 
	Table *train_budtbl = arg->tbl;
	Tree **ntree = arg->trees;
	Row **rows = arg->rows;
	
	Table *ntrain_tbl = resample( rows, tbl_row_count(train_budtbl), tbl_column_count(train_budtbl), tbl_type(train_budtbl) );
	Tree *sa_tree = t_make();
	dt_build(ntrain_tbl, sa_tree); // choose 2/3 column in dt.c
	pthread_mutex_lock(&lock);
	ntree[t_count] = sa_tree;
	t_count++;
	pthread_mutex_unlock(&lock);
	
	tbl_free_without_string(ntrain_tbl);
	return NULL;
}

int main(int argc, char **argv) {
	
	srand (time(NULL));
	
	/* initialize variables */
	ca_init(argc, argv);
	
	char *train = NULL; // the training data, which build the tree
	if( ca_defined("train") && ca_str_value("train") != NULL ) {
		train = ca_str_value("train"); 
	}
	else if( ca_defined("t") && ca_str_value("t") != NULL ) {
		train = ca_str_value("t");
	}
	
	char *validate = NULL; // the validation data
	if( ca_defined("validate") && ca_str_value("validate") != NULL ) {
		validate = ca_str_value("validate"); 
	}
	else if( ca_defined("v") && ca_str_value("v") != NULL ) {
		validate = ca_str_value("v");
	}
	
	int num_trees = 1;
	if( ca_defined("trees") && ca_int_value("trees") != -1 ) {
		num_trees = ca_int_value("trees");
	}
	if( num_trees < 1 ) {
		num_trees = 1;
	}
	
	char *class = NULL; // Assume the class is a string
	if( ca_defined("class") && ca_str_value("class") != NULL ) {
		class = ca_str_value("class");
	}
	else if( ca_defined("c") && ca_str_value("c") != NULL ) {
		class = ca_str_value("c");
	}
	
	int num_threads = 8;
	if( ca_defined("threads") && ca_int_value("threads") != -1 ) {
		num_threads = ca_int_value("threads");
		if(num_threads > num_trees) {
			num_threads = num_trees;
		}
	}
	else if( ca_defined("nothreads") ) {
		num_threads = 1;
	}
	
	char oneline[9000]; // a copy of a line
	int from = 0;
	int to = 0;
	int length = 0;
	char c;
	
	if(train == NULL || validate == NULL) {
		printf("Lack of training data or validation data!\n");
		ca_free();
		return 0; // TODO: return
	}
	
	/* start make the training table */
	Table *train_tbl = tbl_make();
	rd_open(train);
	c = rd_getchar();
	// Save the first line of the training data
	// First count the number of columns
	int num_columns = 0;
	for(length = 0; c != '\n' && c != EOF; length++) {
		oneline[length] = c;
		c = rd_getchar();
	}
	oneline[length] = '\0';
	while(from <= length) {
    	to = rd_field_length(oneline, from, length);
    	num_columns++;
    	from = to + 1;
	}
	// Then save in the colnames
	from = 0;
	to = 0;
	colnames = (char**)calloc(num_columns, sizeof(char*)); // initialized as global
	int index = 0;
	while(from <= length) {
    	to = rd_field_length(oneline, from, length);
		char *name = rd_parse_string(oneline, from, to);
		colnames[index] = name;
    	from = to + 1;
		index++;
	}
	// set other lines into the train_tbl
	int err = 0;
	while(1) {
		c = rd_getchar();
		for(length = 0; c != '\n' && c != EOF; length++) {
			oneline[length] = c;
			c = rd_getchar();
		}
		if(length == 0) {
			break;
		}
		oneline[length] = '\0';
		from = 0;
		to = 0;
		tbl_start_row(train_tbl, num_columns);
		for(int w = 0; w < num_columns; w++) {
			err = 0;
       		to = rd_field_length(oneline, from, length);
       		if(from > length) {
       			break;
       		}
       		double num_data = rd_parse_number(oneline, from, to, &err);
			if(err == 0) {
				tbl_add_double_to_row(train_tbl, num_data);
				from = to + 1;
			}
			else {
				char *string_data = rd_parse_string(oneline, from, to);
        		if(string_data != NULL) {
		    		tbl_add_string_to_row(train_tbl, string_data);
		    		from = to + 1;
        		}
			}
		}
       	if(c == EOF) {
			break;
		}
	}
	Table *train_budtbl = tbl_done_building(train_tbl); 
	rd_close();


	/* Build the validating table */
	from = 0;
	to = 0;
	length = 0;
	Table *vali_tbl = tbl_make();
	rd_open(validate);
	// set lines into the vali_tbl
	err = 0;
	while(1) {
		c = rd_getchar();
		for(length = 0; c != '\n' && c != EOF; length++) {
			oneline[length] = c;
			c = rd_getchar();
		}
		if(length == 0) {
			break;
		}
		oneline[length] = '\0';
		from = 0;
		to = 0;
		tbl_start_row(vali_tbl, num_columns);
		for(int w = 0; w < num_columns; w++) {
			err = 0;
       		to = rd_field_length(oneline, from, length);
       		if(from > length) {
       			break;
       		}
       		double num_data = rd_parse_number(oneline, from, to, &err);
			if(err == 0) {
				tbl_add_double_to_row(vali_tbl, num_data);
				from = to + 1;
			}
			else {
				char *string_data = rd_parse_string(oneline, from, to);
        		if(string_data != NULL) {
		    		tbl_add_string_to_row(vali_tbl, string_data);
		    		from = to + 1;
		    	}
			}
		}
       	if(c == EOF) {
			break;
		}
	}
	Table *vali_budtbl = tbl_done_building(vali_tbl); // finish build a table
	rd_close();
	
	/* Mapping the class through training table */
	char *type = tbl_type(train_budtbl);
	char cla_type = type[strlen(type)-1]; // The type of the class, D or S
	
	if(cla_type == 'D') {
		d_cla = d_map(train_budtbl, &elm_count);
	}
	else if(cla_type == 'S') {
		s_cla = s_map(train_budtbl, &elm_count);
	}
	
	/* Renew the validation table using the mapping data */
	Table *nvali_tbl = renew( tbl_rows(vali_budtbl), tbl_row_count(vali_budtbl), tbl_column_count(vali_budtbl), tbl_type(vali_budtbl) );
	tbl_free(vali_budtbl);
	
	/* 
	 * Resample the training table 
	 * Choose 2/3 column to build each decision tree
	 * Vote each line by using the tree
	 */
	int wrong = 999999;
	Tree **ntree = (Tree**)calloc(num_trees, sizeof(Tree*));
	
		
	/* create the threads */
	
	if(num_threads != 0) {
		pthread_t thread[num_threads];
		pthread_attr_t attr;
		pthread_mutex_init(&lock, NULL); 
	
		pthread_attr_init( &attr );
		pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	
		T_arg *t_arg = (T_arg*)calloc(1, sizeof(T_arg));
		t_arg->tbl = train_budtbl;
		t_arg->rows = tbl_rows(train_budtbl);
		t_arg->trees = ntree;
		int ntr = num_trees;
		int nth = num_threads;
		t_count = 0;
		while(1) {
			if(ntr < nth) {
				for(int w = 0; w < ntr; w++) {
					pthread_create(&(thread[w]), &attr, build_trees, t_arg );
				}
				for(int w = 0; w < ntr; w++) {
					pthread_join(thread[w], NULL);
				}
				break;
			}
			else {
				for(int w = 0; w < nth; w++) {
					pthread_create(&(thread[w]), &attr, build_trees, t_arg );
				}
				for(int w = 0; w < nth; w++) {
					pthread_join(thread[w], NULL);
				}
				ntr = ntr - nth;
			}
		}
		free(t_arg);
	}
	
	ConfMatrix* cm = cm_make( elm_count );
	cm_validate(cm, nvali_tbl, ntree, num_trees);
	if(class == NULL) {
		cm_print(cm);
	}
	else {
		int index = 0;
		if(cla_type == 'D') {
			double real_class = atof(class);
			for(int w = 0; w < elm_count; w++) {
				if(real_class == d_cla[w]) {
					index = w;
					break;
				}
			}
		}
		else {
			char *real_class = class;
			for(int w = 0; w < elm_count; w++) {
				if( strcmp(real_class, s_cla[w]) == 0 ) {
					index = w;
					break;
				}
			}
		}
		int err_count = 0;
		for(int w = 0; w < elm_count; w++) {
			if(w == index) {
				continue;
			}
			err_count = err_count + cm->matrix[index][w];
		}
		//cm_print(cm);
		printf("%d\n", err_count);
	}

	/* Mapping test */
	/*
	if(cla_type == 'D') {
		for(int w = 0; w < elm_count; w++) {
			printf("%.1f ---------- %d\n", d_cla[w], w);
		}
	}
	else if(cla_type == 'S') {
		for(int w = 0; w < elm_count; w++)
		printf("%s ---------- %d\n", s_cla[w], w);
	}
	printf("----------------------------------\n");
	*/
	
	/* Free function */
	// all the global variables
	for(int w = 0; w < num_columns; w++) {
		free( *(colnames+w) );
	}
	free(colnames);
	if(cla_type == 'D') {
		free(d_cla);
	}
	else if(cla_type == 'S') {
		for(int w = 0 ; w < elm_count; w++) {
			free(s_cla[w]);
		}
		free(s_cla);
	}
	// cargs
	ca_free();
	// tables
	tbl_free(train_budtbl);
	tbl_free(nvali_tbl);
	// trees
	for(int w = 0; w < num_trees; w++) {
		t_free(ntree[w], dt_free);
	}
	free(ntree);
	// confusion matrix
	cm_free(cm);
	
	return 0;
}   


