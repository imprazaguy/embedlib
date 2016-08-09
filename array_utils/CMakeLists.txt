if(HAS_UNIT_TEST)
	add_executable(test_array_map test_array_map.c)
	target_link_libraries(test_array_map libcmocka)
	add_test(array_map test_array_map)

	add_executable(test_array_queue test_array_queue.c)
	target_link_libraries(test_array_queue libcmocka)
	add_test(array_queue test_array_queue)
endif()
