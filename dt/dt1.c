#include "defs_itf.h"
#include <float.h>

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
	int num_columns = tbl_column_count(tbl); 
	char* table_type = tbl_type(tbl); 
	double final_entropy = DBL_MAX; 
	int column_type = 0; 
	double final_d_value = 0.0;
	char* final_s_value = NULL; 
	int final_split_column = 0;
	int splitless = 1; 
	unsigned int n = (unsigned int)tbl_row_count(tbl); 
	unsigned int* classes = (unsigned int*)calloc( n, sizeof(unsigned int)); // make an array of classes TODO:FREE

	// get all the values of the classes
	for(int i = 0; i < n; i++) {
		classes[i] = (unsigned int)tbl_double_at( tbl_row_at( tbl, i), num_columns-1);
	}

	// find the lowest entropy and its value, get index of which column has the lowest entropy
	// use only 2/3 of the columns
	int num_use = ((num_columns-1)*2/3 + 1); // 2/3 columns
	for(int w = 0; w < num_use; w++) {
		int i = rand() % (num_columns - 1);
		if(table_type[i] == 'D') {
			double* fields = (double*)calloc(n, sizeof(double)); 
			for(int j = 0; j < n; j++) {
				fields[j] = tbl_double_at( tbl_row_at( tbl, j), i);
			}
			Column* column = make_double_column(fields, classes, n);
			free(fields);
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
		else if(table_type[i] == 'S') {
			char** fields = (char**)calloc(n, sizeof(char*)); 
			for(int j = 0; j < n; j++) {
				fields[j] = tbl_string_at( tbl_row_at( tbl, j), i);
			}
			Column* column = make_string_column(fields, classes, n);
			free(fields);
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
		for(int i = 0; i < n; i++) {
			cla = classes[i];
			for(int j = 0; j < n; j++) {
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
	
	// split data into two tables
	Table* l_table = tbl_make();
	Table* r_table = tbl_make();
	Tree* l_tree = t_make();
	Tree* r_tree = t_make();
	t_set_left(tree, l_tree);
	t_set_right(tree, r_tree);
	
	// if the column is double
	if(column_type) {
		for(int i = 0; i < n; i++) {
			if( tbl_double_at( tbl_row_at(tbl, i), final_split_column) < final_d_value) {
				tbl_start_row(l_table, num_columns);
				for(int j = 0; j < num_columns; j++) {
					if(table_type[j] == 'D') {
						tbl_add_double_to_row( l_table, tbl_double_at( tbl_row_at(tbl, i), j ) ); 
					}
					else {
						char* temp_str = (char*)calloc( strlen(tbl_string_at( tbl_row_at(tbl, i), j ))+1, sizeof(char) );
						strcpy(temp_str, tbl_string_at( tbl_row_at(tbl, i), j ) );
						tbl_add_string_to_row( l_table, temp_str ); 
					}
				}
			}
			else {
				tbl_start_row(r_table, num_columns);
				for(int j = 0; j < num_columns; j++) {
					if(table_type[j] == 'D') {
						tbl_add_double_to_row( r_table, tbl_double_at( tbl_row_at(tbl, i), j ) ); 
					}
					else {
						char* temp_str = (char*)calloc( strlen(tbl_string_at( tbl_row_at(tbl, i), j ))+1, sizeof(char) );
						strcpy(temp_str, tbl_string_at( tbl_row_at(tbl, i), j ) );
						tbl_add_string_to_row( r_table, temp_str ); 
					}
				}
			}
		}
		free(final_s_value);
	}
	else {
		for(int i = 0; i < n; i++) {
			if( strcmp( tbl_string_at( tbl_row_at(tbl, i), final_split_column) , final_s_value ) == 0) {
				tbl_start_row(l_table, num_columns);
				for(int j = 0; j < num_columns; j++) {
					if(table_type[j] == 'D') {
						tbl_add_double_to_row( l_table, tbl_double_at( tbl_row_at(tbl, i), j ) ); 
					}
					else {
						char* temp_str = (char*)calloc( strlen(tbl_string_at( tbl_row_at(tbl, i), j ))+1, sizeof(char) );
						strcpy(temp_str, tbl_string_at( tbl_row_at(tbl, i), j ) );
						tbl_add_string_to_row( l_table, temp_str ); 
					}
				}
			}
			else {
				tbl_start_row(r_table, num_columns);
				for(int j = 0; j < num_columns; j++) {
					if(table_type[j] == 'D') {
						tbl_add_double_to_row( r_table, tbl_double_at( tbl_row_at(tbl, i), j ) ); 
					}
					else {
						char* temp_str = (char*)calloc( strlen(tbl_string_at( tbl_row_at(tbl, i), j ))+1, sizeof(char) );
						strcpy(temp_str, tbl_string_at( tbl_row_at(tbl, i), j ) );
						tbl_add_string_to_row( r_table, temp_str ); 
					}
				}
			}
		}
	}
	tbl_done_building(l_table);
	tbl_done_building(r_table);
	dt_build(l_table, l_tree);
	tbl_free(l_table);
	dt_build(r_table, r_tree);
	tbl_free(r_table);
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
