#include "defs_itf.h"
#include <float.h>
#include <time.h>

void swap(int *a, int *b) {
	int temp = *a;
	*a = *b;
	*b = temp;
	return;
}

void dt_recur(Row **rows, Tree *tree, int nc, int nr, char *type) {
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
	
	// make a random array, use the 2/3 in front
	int *rand_list = (int*)calloc(nc, sizeof(int));
	int random;
	for(int w = 0; w < nc; w++) {
		rand_list[w] = w;
	}
	for(int w = 0; w < nc; w++) {
		random = rand() % nc;
		swap(&rand_list[w], &rand_list[random]);
	}
	
	// use the 2/3 of the columns
	int num_use = 0;
	num_use = (int)((nc-1)*2/3);
	Column* column = NULL;
	for(int w = 0; w < num_use; w++) {
		int i = rand_list[w];
		if(type[i] == 'D') {
			double *fields = (double*)calloc(nr, sizeof(double)); 
			for(int j = 0; j < nr; j++) {
				fields[j] = tbl_double_at( rows[j], i );
			}
			Column* temp_column = make_double_column(fields, classes, nr);
			double d_entropy = find_double_split_entropy(temp_column);
			if(d_entropy < final_entropy) {
				final_entropy = d_entropy;
				column_type = 1;
				final_split_column = i;
				splitless = 0;
				if(column != NULL) {
					if(column->type) {
						free(column->s_fields);
					}
					else {
						free(column->d_fields);
					}
					free_column(column);
					column = NULL;
				}
				column = temp_column;
			}
			else {
				free_column(temp_column);
				free(fields);
			}
		}
		else if(type[i] == 'S') {
			char **fields = (char**)calloc(nr, sizeof(char*));
			for(int j = 0; j < nr; j++) {
				fields[j] = tbl_string_at( rows[j], i );
			}
			Column* temp_column = make_string_column(fields, classes, nr);
			double s_entropy = find_string_split_entropy(temp_column);
			if(s_entropy < final_entropy) {
				final_entropy = s_entropy;
				column_type = 0;
				final_split_column = i;
				splitless = 0;
				if(column != NULL) {
					if(column->type) {
						free(column->s_fields);
					}
					else {
						free(column->d_fields);
					}
					free_column(column);
					column = NULL;
				}
				column = temp_column;
			}
			else {
				free_column(temp_column);
				free(fields);
			}
		}
	}
	free(rand_list);
	
	if(!splitless) {
		if(type[final_split_column] == 'D') {
			final_d_value = find_double_split_value(column);
			if(column->type) {
				free(column->s_fields);
			}
			else {
				free(column->d_fields);
			}
			free_column(column);
		} 
		else {
			final_s_value = find_string_split_value(column);
			if(column->type) {
				free(column->s_fields);
			}
			else {
				free(column->d_fields);
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
	Row** l_rows = (Row**)calloc(left_nr, sizeof(Row*));
	Row** r_rows = (Row**)calloc(right_nr, sizeof(Row*));
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
	free(l_rows);
	dt_recur(r_rows, r_tree, nc, right_nr, type);
	free(r_rows);
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
	free(data);
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
