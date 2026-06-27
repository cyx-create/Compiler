add_executable(fdmjc main.cc)
target_link_libraries(fdmjc PUBLIC
  pipeline util reg instr opt quadflow quad ir frontend ast vendor_xml
)
