# gen_type = both or proto
# com_scope = generator_relay, relay_marshall, 
#             marshall_relay and relay_generator
# when gen_type = both , of course.
macro(generate_rpc file_name)
  set(PROTO_FILES ${PROTO_FILES} ${file_name})
  
  set_source_files_properties (SOURCE ${file_name} 
    PROPERTIES R_C OFF M_S OFF M_C OFF R_S OFF G_C OFF  G_S OFF)


  foreach(com_scope ${ARGN})
    if      (com_scope STREQUAL "relay_marshall")
      set_source_files_properties (SOURCE ${file_name} PROPERTIES R_C ON M_S ON)
    elseif  (com_scope STREQUAL "marshall_relay")
      set_source_files_properties (SOURCE ${file_name} PROPERTIES M_C ON R_S ON)
    elseif  (com_scope STREQUAL "generator_relay")
      set_source_files_properties (SOURCE ${file_name} PROPERTIES G_C ON R_S ON)
    elseif  (com_scope STREQUAL "relay_generator")
      set_source_files_properties (SOURCE ${file_name} PROPERTIES R_C ON G_S ON)
    else    ()
      message(FATAL_ERROR "com_scope arguement for file ${file_name} invalid.")
    endif   (com_scope STREQUAL "relay_marshall")
  endforeach(com_scope)
endmacro(generate_rpc)

generate_rpc(BasicProtocol.proto generator_relay relay_marshall)
# generate_rpc(dummy.proto  relay_marshall)

foreach(proto_var ${PROTO_FILES})
  get_source_file_property(R_C_ ${proto_var} R_C)  
  get_source_file_property(M_S_ ${proto_var} M_S)
  get_source_file_property(M_C_ ${proto_var} M_C)
  get_source_file_property(R_S_ ${proto_var} R_S) 
  get_source_file_property(G_C_ ${proto_var} G_C)
  get_source_file_property(G_S_ ${proto_var} G_S)

  get_filename_component(fn ${proto_var} NAME_WE )
  set(fn_c ${fn}-client.rblrpc.h)
  set(fn_s ${fn}-server.rblrpc.h)
  set(fn_p_c ${fn}.pb.cc)
  set(fn_p_h ${fn}.pb.h)

  message(STATUS ${fn_c})
  message(STATUS ${fn_s})

  
  add_custom_command( OUTPUT ${fn_c} ${fn_s} ${fn_p_c} ${fn_p_h}
                      COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}  ${CMAKE_CURRENT_SOURCE_DIR}/${proto_var} --cpp_out=. --plugin=protoc-gen-RBLRPC=${CMAKE_BINARY_DIR}/tools/protoc-gen-RBLRPC --RBLRPC_out=. 
                      DEPENDS ${proto_var}
  )
  
  if(${M_S_} OR ${M_C_})
    set(MARSHALL_RPC_FILES ${MARSHALL_RPC_FILES} ${fn_p_c})
  endif()

  if(${R_C_} OR  ${R_S_})
      set(RELAY_RPC_FILES ${RELAY_RPC_FILES} ${fn_p_c})
  endif()

  if(${G_C_} OR ${G_S_} )
    set(GENERATOR_RPC_FILES ${GENERATOR_RPC_FILES} ${fn_p_c})
  endif()

  
    message(STATUS "${proto_var} - R_C=${R_C_}, M_S=${M_S_}, M_C=${M_C_}, R_S=${R_S_},G_C=${G_C_}, G_S=${G_S_}")
endforeach(proto_var) 

add_library( relay_rpc_proto STATIC ${RELAY_RPC_FILES})
add_library( generator_rpc_proto STATIC ${GENERATOR_RPC_FILES})
add_library( marshall_rpc_proto STATIC ${MARSHALL_RPC_FILES})
