
include_directories(${PROJECT_SOURCE_DIR}/normal/include)
aux_source_directory(src/mutexqueue/ SRCS1)
aux_source_directory(src/lockfreequeue/ SRCS2)


add_executable(mutexqueue_test ${SRCS1})
add_executable(lockfreequeue_test ${SRCS2})


# TARGET_LINK_LIBRARIES(lockfreequeue_test pthread)
# TARGET_LINK_LIBRARIES(mutexqueue_test pthread)
