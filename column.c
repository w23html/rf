#include "defs_itf.h"
#include <float.h>

Column *make_double_column(double *fields, unsigned int *classes, unsigned int n) {
	Column* col = (Column*)calloc(1, sizeof(Column));
	col->d_fields = fields;
	col->classes = classes;
	col->num_field = n;
	col->type = 0;
	return col;
}

Column *make_string_column(char **fields, unsigned int *classes, unsigned int n) {
	Column* col = (Column*)calloc(1, sizeof(Column));
	col->s_fields = fields;
	col->classes = classes;
	col->num_field = n;
	col->type = 1;
	return col;
}

void free_column(Column *column) {
	free(column);
}

int has_single_value(Column *column) {
	if(column->num_field == 0 || column->num_field == 1) return 1;
	if(column->type == 0) {
		for(int w = 1; w < column->num_field; w++) {
			if( (column->d_fields[w] - column->d_fields[0]) <= 0.0001 && (column->d_fields[w] - column->d_fields[0]) >= 0
			 || (column->d_fields[0] - column->d_fields[w]) <= 0.0001 && (column->d_fields[0] - column->d_fields[w]) >= 0) {
				;
			}
			else {
				return 0;
			}
		}
		return 1;
	}
	else {
		for(int w = 1; w < column->num_field; w++) {
			if( strcmp(column->s_fields[0], column->s_fields[w]) == 0 ) {
				;
			}
			else {
				return 0;
			}
		}
		return 1;
		
	}
}

int is_impossible_split(Column *column) {
	if(column->num_field == 0 || column->num_field == 1) return 1;
	for(int w = 1; w < column->num_field; w++) {
		if(column->classes[0] != column->classes[w]) {
			return 0;
		}
	}
	return 1;
}

double calc_entropy(Column *column) {
	if(column->num_field == 0) {
		return 0.0;
	}
	if(column->num_field == 1) {
		return 0.0;
	}
	
	int M = column->classes[0]; // M is the largest class in the column
	double H = 0.0; // H: entropy
	int N = column->num_field; // N: number of classes
	int Ni = 0; // Ni: the number of items in the column belonging to class i
	double Pi = 0.0;
	
	for(int w = 1; w < column->num_field; w++) {
		if(column->classes[w] > M) {
			M = column->classes[w];
		}
	}
	for(int i = 0; i < M + 1; i++) {
		for(int w = 0; w < N; w++) {
			if(column->classes[w] == i) {
				Ni++;
			}
		}
		if(Ni == 0) {
			;
		}
		else {
			Pi = (double)Ni/N;
			H = H + Pi * log2(Pi);
		}
		Ni = 0;
		Pi = 0.0;
	}
	H = -H;
	return H;
}

double calc_entropy_sedo(double* fields, int max_class, int num_class) {
	
	int M = max_class; // M is the largest class in the column
	double H = 0.0; // H: entropy
	int N = num_class; // N: number of classes
	int Ni = 0; // Ni: the number of items in the column belonging to class i
	double Pi = 0.0;
	
	for(int i = 0; i < max_class + 1; i++) {
		Ni = fields[i];
		if(Ni == 0) {
			;
		}
		else {
			Pi = (double)Ni/N;
			H = H + Pi * log2(Pi);
		}
		Ni = 0;
		Pi = 0.0;
	}
	H = -H;
	return H;
}

int compare_d(const void* p1, const void* p2) {
	double *a = (double*)p1;
	double *b = (double*)p2;
	if(*a > *b) {
		return 1;
	}
	else if(*a < *b) {
		return -1;
	}
	else {
		return 0;
	}
}

int compare_s(const void *a, const void *b) {
	return strcmp( *(char**)a, *(char**)b );
}

double find_double_split_value(Column *column)  {
	if(has_single_value(column) || is_impossible_split(column)) {
		return DBL_MAX;
	}
	double splitD_f = 0.0;
	double sum_f = DBL_MAX;
	int num_field = column->num_field;
	
	int largest_class = -1; // the largest class in the column
	
	double *copy_fields = (double*)calloc(num_field, sizeof(double));
	for(int w = 0; w < num_field; w++) {
		copy_fields[w] = column->d_fields[w];
		if( (int)(column->classes[w]) > largest_class) {
			largest_class = (int)(column->classes[w]);
		}
	}
	qsort(copy_fields, num_field, sizeof(double), compare_d);	
	
	double former_split = DBL_MIN;
	double this = copy_fields[0];
	double next = copy_fields[1];
	double* fields_l = (double*)calloc(largest_class + 1, sizeof(double));
	double* fields_r = (double*)calloc(largest_class + 1, sizeof(double));
	for(int w = 0; w < num_field; w++) {
		fields_r[column->classes[w]]++;
	}
	int NL = 0;
	int NR = num_field;	
	for(int w = 0; w < num_field; w++) {
		if(this == next) {
			if(w < num_field - 2) {
				this = copy_fields[w+1];
				next = copy_fields[w+2];
				continue; // 0 0 1 2 3	
			}
			else {
				this = next;
				continue;
			}
		}
		double sum = 0;
		double splitD = 0.0;
		double HL = 0;
		double HR = 0;
		splitD = this; // split value -- double
		for(int i = 0; i < num_field; i++) {
			if(column->d_fields[i] <= splitD && column->d_fields[i] > former_split) {
				fields_l[column->classes[i]]++;
				fields_r[column->classes[i]]--;	
				NL++;
				NR--;			
			}
		}
		HL = calc_entropy_sedo(fields_l, largest_class, NL);
		HR = calc_entropy_sedo(fields_r, largest_class, NR);
		sum = HL * NL + HR * NR;
		if(sum < sum_f) {
			sum_f = sum;
			splitD_f = splitD;
			former_split = splitD;
		}
		else if(sum - sum_f < 0.00001 && sum - sum_f > 0 || sum_f - sum < 0.00001 && sum_f - sum > 0 || sum == sum_f) {
			if(splitD_f > splitD) {
				splitD_f = splitD;
				former_split = splitD;
			}
		}
		else {
			former_split = splitD;
		}
		
		if(w < num_field - 2) {
			this = copy_fields[w+1];
			next = copy_fields[w+2];
		}
		else {
			this = next;
			next = next + 1;
		}
	}
	double bigger = DBL_MAX;
	for(int w = 0; w < num_field; w++) {
		if(column->d_fields[w] - splitD_f > 0) {
			if(bigger > column->d_fields[w]) {
				bigger = column->d_fields[w];
			}
		}	
	}
	free(fields_l);
	free(fields_r);
	free(copy_fields);
	return (splitD_f + bigger) / 2;
}

double find_double_split_entropy(Column *column) {
	if(has_single_value(column) || is_impossible_split(column)) {
		return DBL_MAX;
	}
	double splitD_f = 0.0;
	double sum_f = DBL_MAX;
	int num_field = column->num_field;
	
	int largest_class = -1; // the largest class in the column
	double *copy_fields = (double*)calloc(num_field, sizeof(double));
	for(int w = 0; w < num_field; w++) {
		copy_fields[w] = column->d_fields[w];
		if( (int)(column->classes[w]) > largest_class) {
			largest_class = (int)(column->classes[w]);
		}
	}
	qsort(copy_fields, num_field, sizeof(double), compare_d);

	double former_split = DBL_MIN;
	double this = copy_fields[0];
	double next = copy_fields[1];
	double* fields_l = (double*)calloc(largest_class + 1, sizeof(double));
	double* fields_r = (double*)calloc(largest_class + 1, sizeof(double));
	for(int w = 0; w < num_field; w++) {
		fields_r[column->classes[w]]++;
	}	
	int NL = 0;
	int NR = num_field;	
	for(int w = 0; w < num_field; w++) {
		if(this == next) {
			if(w < num_field - 2) {
				this = copy_fields[w+1];
				next = copy_fields[w+2];
				continue; // 0 0 1 2 3	
			}
			else {
				this = next;
				continue;
			}
		}
		double sum = 0;
		double splitD = 0.0;
		double HL = 0;
		double HR = 0;
		splitD = this; // split value -- double
		for(int i = 0; i < num_field; i++) {
			if(column->d_fields[i] <= splitD && column->d_fields[i] > former_split) {
				fields_l[column->classes[i]]++;
				fields_r[column->classes[i]]--;	
				NL++;
				NR--;			
			}
		}
		HL = calc_entropy_sedo(fields_l, largest_class, NL);
		HR = calc_entropy_sedo(fields_r, largest_class, NR);
		sum = HL * NL + HR * NR;
		if(sum < sum_f) {
			sum_f = sum;
			splitD_f = splitD;
			former_split = splitD;
		}
		else if(sum - sum_f < 0.00001 && sum - sum_f > 0 || sum_f - sum < 0.00001 && sum_f - sum > 0 || sum == sum_f) {
			if(splitD_f > splitD) {
				splitD_f = splitD;
				former_split = splitD;
			}
		}
		else {
			former_split = splitD;
		}
		
		if(w < num_field - 2) {
			this = copy_fields[w+1];
			next = copy_fields[w+2];
		}
		else {
			this = next;
			next = next + 1;
		}
	}
	free(fields_l);
	free(fields_r);
	free(copy_fields);
	return sum_f;
}

double find_string_split_entropy(Column *column) {
	if(has_single_value(column) || is_impossible_split(column)) {
		return DBL_MAX;
	}
	char* splitS_f = NULL;
	double sum_f = DBL_MAX;
	int num_field = column->num_field;
	
	char **copy_fields = (char**)calloc(num_field, sizeof(char*));
	for(int w = 0; w < num_field; w++) {
		copy_fields[w] = column->s_fields[w];
	}
	qsort(copy_fields, num_field, sizeof(char*), compare_s);
	
	char *this = copy_fields[0];
	char *next = copy_fields[1];

	for(int w = 0; w < num_field; w++) {
		if( strcmp(this, next) == 0 ) {
			if(w < num_field - 2) {
				this = copy_fields[w+1];
				next = copy_fields[w+2];
				continue;
			}
			else {
				this = next;
				continue;
			}
		}
		double sum = 0;
		char* splitS = NULL;
		double HL = 0;
		int NL = 0;
		double HR = 0;
		int NR = 0;
		Column* left_column = NULL;
		Column* right_column = NULL;
		
		splitS = column->s_fields[w]; // split value -- string
		for(int i = 0; i < num_field; i++) {
			if(strcmp(splitS, column->s_fields[i]) == 0) {
				NL++;
			}
		}
		NR = num_field - NL;
		char** fields_l = (char**)calloc(NL, sizeof(char*));
		unsigned int* classes_l = (unsigned int*)calloc(NL, sizeof(unsigned int));
		char** fields_r = (char**)calloc(NR, sizeof(char*));
		unsigned int* classes_r = (unsigned int*)calloc(NR, sizeof(unsigned int));
		int index_l = 0;
		int index_r = 0;
		for(int i = 0; i < num_field; i++) {
			if(strcmp(splitS, column->s_fields[i]) == 0) {
				classes_l[index_l] = column->classes[i];
				fields_l[index_l] = splitS;
				index_l++;
			}
			else {
				classes_r[index_r] = column->classes[i];
				fields_r[index_r] = column->s_fields[i];
				index_r++;
			}
		}
		left_column = make_string_column(fields_l, classes_l, NL);
		HL = calc_entropy(left_column);
		right_column = make_string_column(fields_r, classes_r, NR);
		HR = calc_entropy(right_column);
		sum = HL * NL + HR * NR;
		if(sum < sum_f) {
			sum_f = sum;
			splitS_f = splitS;
		}
		free(fields_l);
		free(fields_r);
		free(classes_l);
		free(classes_r);
		free_column(left_column);
		free_column(right_column);
		
		if(w < num_field - 2) {
			this = copy_fields[w+1];
			next = copy_fields[w+2];
		}
		else {
			this = next;
			next = copy_fields[0];
		}
	}
	free(copy_fields);
	return sum_f;
}

char *find_string_split_value(Column *column) {
	if(has_single_value(column) || is_impossible_split(column)) {
		return column->s_fields[0];
	}
	char* splitS_f = NULL;
	double sum_f = DBL_MAX;
	int num_field = column->num_field;
	
	char **copy_fields = (char**)calloc(num_field, sizeof(char*));
	for(int w = 0; w < num_field; w++) {
		copy_fields[w] = column->s_fields[w];
	}
	qsort(copy_fields, num_field, sizeof(char*), compare_s);
	char *this = copy_fields[0];
	char *next = copy_fields[1];
	
	for(int w = 0; w < num_field; w++) {
		if(strcmp(this, next) == 0) {
			if(w < num_field - 2) {
				this = copy_fields[w+1];
				next = copy_fields[w+2];
				continue;
			}
			else {
				this = next;
				continue;
			}
		}
	
		double sum = 0;
		char* splitS = NULL;
		double HL = 0;
		int NL = 0;
		double HR = 0;
		int NR = 0;
		Column* left_column = NULL;
		Column* right_column = NULL;
		
		splitS = column->s_fields[w]; // split value -- string
		for(int i = 0; i < num_field; i++) {
			if(strcmp(splitS, column->s_fields[i]) == 0) {
				NL++;
			}
		}
		NR = num_field - NL;
		char** fields_l = (char**)calloc(NL, sizeof(char*));
		unsigned int* classes_l = (unsigned int*)calloc(NL, sizeof(unsigned int));
		char** fields_r = (char**)calloc(NR, sizeof(char*));
		unsigned int* classes_r = (unsigned int*)calloc(NR, sizeof(unsigned int));
		int index_l = 0;
		int index_r = 0;
		for(int i = 0; i < num_field; i++) {
			if(strcmp(splitS, column->s_fields[i]) == 0) {
				classes_l[index_l] = column->classes[i];
				fields_l[index_l] = splitS;
				index_l++;
			}
			else {
				classes_r[index_r] = column->classes[i];
				fields_r[index_r] = column->s_fields[i];
				index_r++;
			}
		}
		left_column = make_string_column(fields_l, classes_l, NL);
		HL = calc_entropy(left_column);
		right_column = make_string_column(fields_r, classes_r, NR);
		HR = calc_entropy(right_column);
		sum = HL * NL + HR * NR;
		if(sum < sum_f) {
			sum_f = sum;
			splitS_f = splitS;
		}
		free(fields_l);
		free(fields_r);
		free(classes_l);
		free(classes_r);
		free_column(left_column);
		free_column(right_column);
		
		if(w < num_field - 2) {
			this = copy_fields[w+1];
			next = copy_fields[w+2];
		}
		else {
			this = next;
			next = copy_fields[0];
		}
	}
	free(copy_fields);
	return splitS_f;
}
