project(Login)

SET(LIBRARY_OUTPUT_PATH ${lib_path}/mylib)

add_library(${PROJECT_NAME} 
                Login.cpp 
                bcrypt.cpp
                blowfish.cpp    
            )

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui)