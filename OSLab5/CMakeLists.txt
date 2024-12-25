add_executable(Server Server.cpp Employee.h)
add_executable(Client Client.cpp Employee.h)

if(WIN32)
    target_link_libraries(Server ws2_32)
    target_link_libraries(Client ws2_32)
endif()
