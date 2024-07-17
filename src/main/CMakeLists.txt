aux_source_directory(. SRCS)
add_executable(videoserver ${SRCS})

add_dependencies(base jsoncpp)
target_link_libraries(videoserver base network mmedia live jsoncpp.a crypto)

install(TARGETS videoserver RUNTIME DESTINATION sbin)