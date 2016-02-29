add_library(rbtree STATIC rbtree.c)

if(HAS_UNIT_TEST)
	add_executable(test_rbtree test_rbtree.c)
	target_link_libraries(test_rbtree rbtree libcmocka)

	add_test(rbtree test_rbtree)
endif()
