#include "defs_itf.h"
#include <float.h>

void dt_recur(Row **rows, Tree *tree, int nc, int nr, char *type) {
	//fprintf(stderr, "err\n");
	double final_entropy = DBL_MAX; 
	int column_type = 0; 
	double final_d_value = 0.0;
	char *final_s_value = NULL; 
	int final_split_column = 0;
	int splitless = 1; 
	int *classes = (int*)calloc( nr, sizeof(unsigned int));
	for(int i = 0; i < nr; i++) {
		classes[i] = (int)tbl_double_at( rows[i], nc-1);
	}
	
	// use the 2/3 of the columns
	int num_use = ((nc-1)*2/3 + 1);
	for(int w = 0; w < num_use; w++) {
		int i = rand() % (nc - 1);
		if(type[i] == 'D') {
			//double* fields = (double*)calloc(nr, sizeof(double)); 
			double fields[nr]; 
			for(int j = 0; j < nr; j++) {
				fields[j] = tbl_double_at( rows[j], i );
			}
			Column* column = make_double_column(fields, classes, nr);
			//free(fields);
			if(is_impossible_split(column) || has_single_value(column)) {
				free_column(column);
				continue;
			}
			double d_value = find_double_split_value(column);
			double d_entropy = find_double_split_entropy(column);
			if(d_entropy < final_entropy) {
				final_entropy = d_entropy;
				column_type = 1;
				final_d_value = d_value;
				final_split_column = i;
				splitless = 0;
			}
			free_column(column);
		}
		else if(type[i] == 'S') {
			//char** fields = (char**)calloc(nr, sizeof(char*)); 
			char* fields[nr];
			for(int j = 0; j < nr; j++) {
				fields[j] = tbl_string_at( rows[j], i );
			}
			Column* column = make_string_column(fields, classes, nr);
			//free(fields);
			if(is_impossible_split(column) || has_single_value(column)) {
				free_column(column);
				continue;
			}
			char* s_value = find_string_split_value(column);
			double s_entropy = find_string_split_entropy(column);
			if(s_entropy < final_entropy) {
				final_entropy = s_entropy;
				column_type = 0;
				if(final_s_value != NULL) {
					free(final_s_value);
					final_s_value = NULL;
					final_s_value = (char*)calloc( strlen(s_value)+1, sizeof(char) ); 
					strcpy(final_s_value, s_value);
				}
				else {
					final_s_value = (char*)calloc( strlen(s_value)+1, sizeof(char) ); 
					strcpy(final_s_value, s_value);
				}
				final_split_column = i;
				splitless = 0;
			}
			free_column(column);
		}
	}
	// set the data into the tree
	Node* node = (Node*)calloc(1, sizeof(Node)); // TODO:FREE
	if(splitless) {
		node->leaf = 1;
		unsigned int cla_maj = 0;
		unsigned int cla = 0;
		int maj_times = 0;
		int times = 0;
		for(int i = 0; i < nr; i++) {
			cla = classes[i];
			for(int j = 0; j < nr; j++) {
				if(cla == classes[j]) {
					times++;
				}
			}
			if(times > maj_times) {
				maj_times = times;
				cla_maj = cla;
			}
			times = 0;
			cla = 0;
		}
		node->class = cla_maj;
		t_set_data(tree, node);
		free(classes);
		return;
	}
	else {
		node->entropy = final_entropy;
		if(column_type) {
			node->field.d = final_d_value;
			node->type = 'D';
			node->leaf = 0;
			node->column = final_split_column;
		}
		else {
			node->field.s = final_s_value;
			node->type = 'S';
			node->leaf = 0;
			node->column = final_split_column;
		}
	}
	t_set_data(tree, node);
	free(classes);
	// split data into two Rows
	int left_nr = 0;
	int right_nr = 0;
	if(column_type) {
		for(int i = 0; i < nr; i++) {
			if( tbl_double_at( rows[i], final_split_column) < final_d_value) {
				left_nr++;
			}
			else {
				right_nr++;
			}
		}
	}
	else {
		for(int i = 0; i < nr; i++) {
			if( strcmp( tbl_string_at( rows[i], final_split_column) , final_s_value ) == 0) {
				left_nr++;
			}
			else {
				right_nr++;
			}
		}
	}
	Row* l_rows[left_nr]; 
	Row* r_rows[right_nr];
	Tree* l_tree = t_make();
	Tree* r_tree = t_make();
	t_set_left(tree, l_tree);
	t_set_right(tree, r_tree);
	int l_index = 0;
	int r_index = 0;
	if(column_type) {
		for(int i = 0; i < nr; i++) {
			if( tbl_double_at( rows[i], final_split_column) < final_d_value) {
				l_rows[l_index] = rows[i];
				l_index++;
			}
			else {
				r_rows[r_index] = rows[i];
				r_index++;
			}
		}
	}
	else {
		for(int i = 0; i < nr; i++) {
			if( strcmp( tbl_string_at( rows[i], final_split_column) , final_s_value ) == 0) {
				l_rows[l_index] = rows[i];
				l_index++;
			}
			else {
				r_rows[r_index] = rows[i];
				r_index++;
			}
		}
	}
	dt_recur(l_rows, l_tree, nc, left_nr, type);
	//free(l_rows);
	dt_recur(r_rows, r_tree, nc, right_nr, type);
	//free(r_rows);
}

/*
 * This function builds a decision tree.
 *
 * @param tbl is the table from which the decision tree should be built
 * @param tree is where the decision tree should be stored
 */
void dt_build(Table *tbl, Tree *tree) {
	if(tbl == NULL || tree == NULL || tbl_type(tbl) == NULL) {
		return;
	}
	Row** rows = tbl_rows(tbl);
	int num_columns = tbl_column_count(tbl);
	int num_rows = tbl_row_count(tbl);
	char* table_type = tbl_type(tbl); 
	dt_recur(rows, tree, num_columns, num_rows, table_type);
}

/*
 * This function should be passed to t_free() with the decision tree
 * to free all the memory allocated for your decision tree.
 *
 * @param data is the data that will be freed
 */
void dt_free(void *data) {
	Node* node = (Node*)data;
	if(!node->leaf) {
		if(node->type == 'S') {
			free(node->field.s);
		}
	}
	free(node);
}

/*
 * This function should be passed to t_print() to print your tree. The tree
 * should be printed as follows:
 * 
 * colname=value
 *  C:class
 *  etc..
 *
 * @param data is the data to be printed
 */
void dt_print(void *data) {
	Node* node = (Node*)data;
	if(node->leaf) {
		printf("C:%d\n", node->class);
	}
	else {
		if(node->type == 'D') {
			printf("%s=%.2f\n", colnames[node->column], node->field.d);
		}
		else {
			printf("%s=%s\n", colnames[node->column], node->field.s);
		}
	}
	
}
