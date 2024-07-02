aux_source_directory(. DIR_LIB_SRCS)
aux_source_directory(base DIR_LIB_SRCS)
aux_source_directory(rtmp DIR_LIB_SRCS)

add_library (mmedia ${DIR_LIB_SRCS})

target_link_libraries(mmedia base network crypto)
aux_source_directory(tests DIR_LIB_SRCS)

add_subdirectory(tests)


# add_executable(base_test ${DIR_LIB_SRCS})