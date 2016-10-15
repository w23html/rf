#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "reader.h"
#include "cargs.h"
typedef union u{
	double d_value;
	char* s_value;
} u_value;

typedef struct Row_s{
	u_value *data; // a pointer to a set of data
	struct Row_s *next; // next Row
	char* type; // using D or S to defined
	int nfields; // number of fields
	int findex; // index of fields
	int tindex; // index of type
} Row;

typedef struct{
	Row *first; // first Row of table
	Row *last; // last Row of table
	int count; // count how many Rows are already in the table
	int done;
	char* final_type;
	int num_row;
	Row** rows;
} Table;

typedef struct confmatrix {
	int** matrix; // the matrix 
	int count; // how many classes in the matrix
	int total;
	int wrong;
} ConfMatrix;

typedef struct column {
	double *d_fields;
	char **s_fields;
	unsigned int* classes;
	unsigned int num_field;
	int type; // 0 double, 1 str
} Column;

#include "tree.h"
#include "table.h"
#include "column.h"
#include "dt.h"
#include "confmatrix.h"
