INCLUDE_DIRECTORIES(.. ../misc)

AUX_SOURCE_DIRECTORY(. FILES)

add_library(glpk_env ${FILES})
