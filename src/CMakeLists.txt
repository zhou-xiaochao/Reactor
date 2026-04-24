aux_source_directory(./core SRC_CORE)
aux_source_directory(./net SRC_NET)
aux_source_directory(./protocol SRC_PROTOCOL)
aux_source_directory(./thread SRC_THREAD)
aux_source_directory(./config SRC_CONFIG)
aux_source_directory(./ SRC_MAIN)

# 合并所有源文件到一个变量里
set(SRC_ALL
    ${SRC_CORE}
    ${SRC_NET}
    ${SRC_PROTOCOL}
    ${SRC_THREAD}
    ${SRC_CONFIG}
    ${SRC_MAIN}
)

add_executable(Reactor ${SRC_ALL})