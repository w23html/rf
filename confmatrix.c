#include "defs_itf.h"

/* cm_make:
 *   creates and returns a ConfMatrix, where classes is the number of
 *   classes the confusion matrix will be considering.
 */
ConfMatrix* cm_make(int classes) {
	ConfMatrix* cm = (ConfMatrix*)calloc(1, sizeof(ConfMatrix));
	cm->matrix = (int**)calloc(classes, sizeof(int*));
	for(int i = 0; i < classes; i++) {
		cm->matrix[i] = (int*)calloc(classes, sizeof(int));
		for(int j = 0; j < classes; j++) {
			cm->matrix[i][j] = 0;
		}
	}
	cm->count = classes;
	cm->total = 0;
	cm->wrong = 0;
	return cm;
}

int compares(const void* a, const void* b) {
	return *(int*)a - *(int*)b;
}

/*
 * use the tree to get the detect class of the row
 */
int vote(Tree* tree, Row* row) {
	if( ((Node*)(t_data(tree)))->leaf == 1 ) {
		int class = ((Node*)(t_data(tree)))->class;
		return class;
	}
	else {
		int column = ((Node*)(t_data(tree)))->column;
		if(((Node*)(t_data(tree)))->type == 'S') {
			if(strcmp( ((Node*)(t_data(tree)))->field.s, tbl_string_at(row, column) ) == 0 ) {
				return vote(t_left(tree), row);
			}
			else {
				return vote(t_right(tree), row);
			}
		}
		else {
			if( ((Node*)(t_data(tree)))->field.d > tbl_double_at(row, column) ) {
				return vote(t_left(tree), row);
			}
			else {
				return vote(t_right(tree), row);
			}
		}
	}
}

/*
 * return the majority of the class appears in the classes
 * 0 0 1 2 2 3 3 3, vote 3
 */
int majority(int* classes, int nc) {
	int class1 = classes[0];
	int class2 = classes[0];
	int maj_class = classes[0];
	int count_class = 1;
	int count_maj = 0;
	for(int w = 0; w < nc-1; w++) {
		class1 = classes[w];
		class2 = classes[w+1];
		if(class1 != class2) {
			if(count_class > count_maj) {
				count_maj = count_class;
				maj_class = class1;
			}
			count_class = 1;
		}
		else {
			count_class++;
		}
	}
	if(count_class > count_maj) {
		count_maj = count_class;
		maj_class = class1;
	}
	return maj_class;
}


void decided(ConfMatrix *cm, Row *row, Tree **trees, int nt, int ac) {
	int *classes = (int*)calloc(nt, sizeof(int));
	for(int w = 0; w < nt; w++) {
		classes[w] = vote(trees[w], row);
	}
	if(nt > 1) {
		qsort(classes, nt, sizeof(int), compares);
	}
	int dc = majority(classes, nt);
	/*
	printf("----------------------------------------\n");
	printf("vote class: %d, actual class: %d\n", dc, ac);
	printf("----------------------------------------\n");
	*/
	cm->matrix[ac][dc]++;
	cm->total++;
	if(ac != dc) {
		cm->wrong++;
	}
	free(classes);
}

/* 
 * cm_validate:
 *   This function takes a table [tbl], a "trained" decision tree [tree],
 *   and attempts to determine how well the tree predicts classes.
 *   It will take each row of the table, and compare the actual class
 *   in the table to the predicted class it finds from the tree.
 *   Each prediction will be recorded in the confusion matrix [cm]. 
 */
void cm_validate(ConfMatrix *cm, Table *tbl, Tree **trees, int nt) {
	// using the desicion tree for each row
	Row **rows = tbl_rows(tbl);
	for(int i = 0; i < tbl_row_count(tbl); i++) {
		int actual_class = (int)tbl_double_at( rows[i], tbl_column_count(tbl)-1 );
		decided(cm, rows[i], trees, nt, actual_class);
	}
}

/* 
 * cm_print:
 *   prints ConfMatrix cm with the format:
 *
 *   Confusion matrix: total=X, errors=Y
 *    | 0 1
 *   0|a,b,
 *   1|c,d,
 *
 *   where the X is the number of predictions, Y is the number
 *   of wrong predictions, and a,b,c,d,etc. are the specific numbers
 *   for each class that was predicted. Note that there is a trailing
 *   new line after the last row of text printed.
 */
void cm_print(ConfMatrix* cm) {
	printf("Confusion matrix: total=%d, errors=%d \n", cm->total, cm->wrong);
	printf(" |");
	for(int i = 0; i < cm->count; i++) {
		printf(" %d", i);
	}
	printf("\n");
	for(int i = 0; i < cm->count; i++) {
		printf("%d|", i);
		for(int j = 0; j < cm->count; j++) {
			printf("%d,", cm->matrix[i][j]);
		}
		printf("\n");
	}
}

/*
 * cm_free():
 *   You know the drill!
 */
void cm_free(ConfMatrix* cm) {
	int n = cm->count;
	for(int i = 0; i < n; i++) {
		free(cm->matrix[i]);
	}
	free(cm->matrix);
	free(cm);
}

/* 
 * tbl_classes_count:
 *   returns the number of classes in a table. You will need
 *   this to know how large to make the confusion matrix.
 */
int tbl_classes_count(Table *tbl) {
	int tbl_num_column = tbl_column_count(tbl);
	int tbl_num_row = tbl_row_count(tbl);
	int* classes = (int*)calloc(tbl_num_row, sizeof(int));
	for(int i = 0; i < tbl_num_row; i++) {
		classes[i] = tbl_double_at( tbl_row_at(tbl, i), tbl_num_column-1 );
	}
	qsort(classes, tbl_num_row, 4, compares);
	
	// give the name of each class to the classes_name in matrix
	int w = -1;
	int q = 0;
	for(int i = 0; i < tbl_num_row; i++) {
		if(classes[i] > w) {
			w = classes[i];
			q++;
		}
	}
	free(classes);
	return q;
}

