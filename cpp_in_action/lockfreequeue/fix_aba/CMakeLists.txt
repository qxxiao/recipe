
include_directories(${PROJECT_SOURCE_DIR}/fix_aba/include)
aux_source_directory(src SRCS)

add_executable(lockfreequeue_fix_test ${SRCS})

