rf_sycro: rf_sycro.c dt.c confmatrix.c cargs.c reader.c table.c tree.c column.c confmatrix.c libreader.a libtree.a libcargs.a libcolumn.a libtable.a
	gcc -Werror -g -std=gnu99 -o rf rf_sycro.c dt.c confmatrix.c cargs.c tree.c column.c reader.c table.c -lm -L. -lpthread
	mv rf ./tests

rf_mutex: rf_mutex.c dt.c confmatrix.c libreader.a libtree.a libcargs.a libcolumn.a libtable.a
	gcc -Werror -g -std=gnu99 -o rf rf_mutex.c dt.c confmatrix.c cargs.c tree.c reader.c table.c column.c -lm -L. -lpthread
	mv rf ./tests

rf_sycro_pg: rf_sycro.c dt.c confmatrix.c cargs.c reader.c table.c tree.c column.c confmatrix.c libreader.a libtree.a libcargs.a libcolumn.a libtable.a
	gcc -Werror -g -std=gnu99 -o rf rf_sycro.c dt.c confmatrix.c cargs.c tree.c column.c reader.c table.c -lm -L. -lpthread -pg
	mv rf ./tests

rf_mutex_pg: rf_mutex.c dt.c confmatrix.c libreader.a libtree.a libcargs.a libcolumn.a libtable.a
	gcc -Werror -g -std=gnu99 -o rf rf_mutex.c dt.c confmatrix.c cargs.c tree.c reader.c table.c column.c -lm -L. -lpthread -pg
	mv rf ./tests

