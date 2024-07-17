
# add_executable(HandsakeClientTest HandsakeClientTest.cpp)

# target_link_libraries(HandsakeClientTest base network mmedia )

# add_executable(HandsakeServerTest HandsakeServerTest.cpp)

# target_link_libraries(HandsakeServerTest base network mmedia )

# add_executable(RtmpServerTest RtmpServerTest.cpp)

# target_link_libraries(RtmpServerTest base network mmedia )

add_executable(RtmpClientTest RtmpClientTest.cpp)

target_link_libraries(RtmpClientTest base network mmedia  jsoncpp.a crypto)