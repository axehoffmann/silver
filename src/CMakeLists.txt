# Lazy. compile times should be fast enough anyway.
file(GLOB_RECURSE comp_src CONFIGURE_DEPENDS
     "*.hpp"
     "*.cpp"
)

target_sources(silver PRIVATE
    ${comp_src}
)