# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)
list(FILTER app_sources EXCLUDE REGEX "/generate_zirkon12_iop\\.cpp$")

# Embed the IOP file using absolute path
idf_component_register(
    SRCS ${app_sources}
    EMBED_FILES ${CMAKE_SOURCE_DIR}/VT_object_pool/Zirkon12.iop
    #PRIV_REQUIRES micro_ros_espidf_component
)