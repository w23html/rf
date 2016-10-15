#include "defs_itf.h"

static int i = 0;

/*set the data on the tree node */
void  t_set_data(Tree* t, void* data) {
	t->data = data;
}

/*let l be the left node of tree *t */
void  t_set_left(Tree* t, Tree* l) {
	t->left = l;
}

/*let r be the left node of tree *t */
void  t_set_right(Tree* t, Tree* r) {
	t->right = r;
}

/* simply return left node of the tree */
Tree* t_left(Tree* t) {
	return t->left;
}

/* simply return right node of the tree */
Tree* t_right(Tree* t) {
	return t->right;
}

/* simply return the data of the tree */
void* t_data(Tree* t) {
	return t->data;
}

/* make the node of the tree and allocate space for it*/
Tree* t_make() {
	Tree* t = (Tree*)calloc(1, sizeof(Tree));
	return t;
}

/* 

print the whole tree in the following format

Root is printed with zero trailing spaces to the left
Every node/subtree is printed with one additional space
Below is an example for a tree with depth 2:

Root
<space>Left-Child
<space><space>Left-Left-Child
<space><space>Left-Right-Child
<space>Right-Child 
     .... and so on ...


Use (void(* p))(function pointer) to print.

 */
void  t_print( Tree* t ,int space, void(* p)(void*) ) {
	i = 0;
	for(; i < space; i++) {
		putchar(' ');
	}
	(*p)(t->data);
	if(t_left(t) != NULL) {
		++space;
		t_print( t->left, space, p);
		--space;
	}
	if(t_right(t) != NULL) {
		++space;
		t_print( t->right, space, p);
		--space;
	}
}

/* 
   find a node using compare pointer (*p(d,t->data)) to navigate
   d is the data to be found
   do f at the node. return d if found, otherwise return NULL

   f is a function pointer which returns a void pointer. 
   Hint: You could use it to print the node or perform other functions.
   Note: f is more useful for future projects. You may not need f for this lab.

*/

void* t_navigate( Tree* t , void* d, int(*p)(void*, void*), void*(*f)(void*)) {
	if(t->data != NULL) {
		if((*p)(d, t->data) > 0 && t->right != NULL) {
			t_navigate( t->right , d, p, f);
		}
		else if((*p)(d, t->data) < 0 && t->left != NULL) {
			t_navigate( t->left, d, p, f);
		}
		else if((*p)(d, t->data) == 0) {
			return (*f)(t->data);
		}
		else {
			return NULL;
		}
	}
}

void t_free(Tree *t, void (*free_function)(void *)) {
	if(t_left(t) != NULL) {
		t_free(t_left(t), free_function);	
	}
	if(t_right(t) != NULL) {
		t_free(t_right(t), free_function);
	}
	(*free_function)(t_data(t));
	free(t);	
}
