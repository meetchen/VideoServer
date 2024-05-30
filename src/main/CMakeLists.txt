aux_source_directory(. SRCS)
add_executable(videoserver ${SRCS})

install(TARGETS videoserver RUNTIME DESTINATION sbin)