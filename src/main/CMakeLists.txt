aux_source_directory(. SRCS)
add_executable(videoserver ${SRCS})

add_dependencies(base jsoncpp)
target_link_libraries(videoserver base jsoncpp.a)

install(TARGETS videoserver RUNTIME DESTINATION sbin)