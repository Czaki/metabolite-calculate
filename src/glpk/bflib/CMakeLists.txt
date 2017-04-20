INCLUDE_DIRECTORIES(.. ../env ../misc)

AUX_SOURCE_DIRECTORY(. FILES)

add_library(bflib ${FILES})
