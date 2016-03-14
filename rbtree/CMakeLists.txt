add_library(rbtree STATIC rbtree.c)
add_library(rbtree_compact STATIC rbtree_compact.c)

if(HAS_UNIT_TEST)
	add_executable(test_rbtree test_rbtree.c)
	target_link_libraries(test_rbtree rbtree libcmocka)
	add_test(rbtree test_rbtree)

	add_executable(test_rbtree_compact test_rbtree.c)
	target_compile_definitions(test_rbtree_compact PUBLIC RB_COMPACT)
	target_link_libraries(test_rbtree_compact rbtree_compact libcmocka)
	add_test(rbtree_compact test_rbtree_compact)
endif()
