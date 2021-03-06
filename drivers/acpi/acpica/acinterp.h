



#ifndef __ACINTERP_H__
#define __ACINTERP_H__

#define ACPI_WALK_OPERANDS          (&(walk_state->operands [walk_state->num_operands -1]))



#define ACPI_EXD_OFFSET(f)          (u8) ACPI_OFFSET (union acpi_operand_object,f)
#define ACPI_EXD_NSOFFSET(f)        (u8) ACPI_OFFSET (struct acpi_namespace_node,f)
#define ACPI_EXD_TABLE_SIZE(name)   (sizeof(name) / sizeof (struct acpi_exdump_info))


#if (!defined(ACPI_MISALIGNMENT_NOT_SUPPORTED) && !defined(ACPI_PACKED_POINTERS_NOT_SUPPORTED))
#pragma pack(1)
#endif

typedef const struct acpi_exdump_info {
	u8 opcode;
	u8 offset;
	char *name;

} acpi_exdump_info;



#define ACPI_EXD_INIT                   0
#define ACPI_EXD_TYPE                   1
#define ACPI_EXD_UINT8                  2
#define ACPI_EXD_UINT16                 3
#define ACPI_EXD_UINT32                 4
#define ACPI_EXD_UINT64                 5
#define ACPI_EXD_LITERAL                6
#define ACPI_EXD_POINTER                7
#define ACPI_EXD_ADDRESS                8
#define ACPI_EXD_STRING                 9
#define ACPI_EXD_BUFFER                 10
#define ACPI_EXD_PACKAGE                11
#define ACPI_EXD_FIELD                  12
#define ACPI_EXD_REFERENCE              13



#pragma pack()


acpi_status
acpi_ex_convert_to_integer(union acpi_operand_object *obj_desc,
			   union acpi_operand_object **result_desc, u32 flags);

acpi_status
acpi_ex_convert_to_buffer(union acpi_operand_object *obj_desc,
			  union acpi_operand_object **result_desc);

acpi_status
acpi_ex_convert_to_string(union acpi_operand_object *obj_desc,
			  union acpi_operand_object **result_desc, u32 type);



#define ACPI_EXPLICIT_BYTE_COPY         0x00000000
#define ACPI_EXPLICIT_CONVERT_HEX       0x00000001
#define ACPI_IMPLICIT_CONVERT_HEX       0x00000002
#define ACPI_EXPLICIT_CONVERT_DECIMAL   0x00000003

acpi_status
acpi_ex_convert_to_target_type(acpi_object_type destination_type,
			       union acpi_operand_object *source_desc,
			       union acpi_operand_object **result_desc,
			       struct acpi_walk_state *walk_state);


acpi_status
acpi_ex_common_buffer_setup(union acpi_operand_object *obj_desc,
			    u32 buffer_length, u32 * datum_count);

acpi_status
acpi_ex_write_with_update_rule(union acpi_operand_object *obj_desc,
			       acpi_integer mask,
			       acpi_integer field_value,
			       u32 field_datum_byte_offset);

void
acpi_ex_get_buffer_datum(acpi_integer * datum,
			 void *buffer,
			 u32 buffer_length,
			 u32 byte_granularity, u32 buffer_offset);

void
acpi_ex_set_buffer_datum(acpi_integer merged_datum,
			 void *buffer,
			 u32 buffer_length,
			 u32 byte_granularity, u32 buffer_offset);

acpi_status
acpi_ex_read_data_from_field(struct acpi_walk_state *walk_state,
			     union acpi_operand_object *obj_desc,
			     union acpi_operand_object **ret_buffer_desc);

acpi_status
acpi_ex_write_data_to_field(union acpi_operand_object *source_desc,
			    union acpi_operand_object *obj_desc,
			    union acpi_operand_object **result_desc);


acpi_status
acpi_ex_extract_from_field(union acpi_operand_object *obj_desc,
			   void *buffer, u32 buffer_length);

acpi_status
acpi_ex_insert_into_field(union acpi_operand_object *obj_desc,
			  void *buffer, u32 buffer_length);

acpi_status
acpi_ex_access_region(union acpi_operand_object *obj_desc,
		      u32 field_datum_byte_offset,
		      acpi_integer * value, u32 read_write);


acpi_status
acpi_ex_get_object_reference(union acpi_operand_object *obj_desc,
			     union acpi_operand_object **return_desc,
			     struct acpi_walk_state *walk_state);

acpi_status
acpi_ex_concat_template(union acpi_operand_object *obj_desc,
			union acpi_operand_object *obj_desc2,
			union acpi_operand_object **actual_return_desc,
			struct acpi_walk_state *walk_state);

acpi_status
acpi_ex_do_concatenate(union acpi_operand_object *obj_desc,
		       union acpi_operand_object *obj_desc2,
		       union acpi_operand_object **actual_return_desc,
		       struct acpi_walk_state *walk_state);

acpi_status
acpi_ex_do_logical_numeric_op(u16 opcode,
			      acpi_integer integer0,
			      acpi_integer integer1, u8 * logical_result);

acpi_status
acpi_ex_do_logical_op(u16 opcode,
		      union acpi_operand_object *operand0,
		      union acpi_operand_object *operand1, u8 * logical_result);

acpi_integer
acpi_ex_do_math_op(u16 opcode, acpi_integer operand0, acpi_integer operand1);

acpi_status acpi_ex_create_mutex(struct acpi_walk_state *walk_state);

acpi_status acpi_ex_create_processor(struct acpi_walk_state *walk_state);

acpi_status acpi_ex_create_power_resource(struct acpi_walk_state *walk_state);

acpi_status
acpi_ex_create_region(u8 * aml_start,
		      u32 aml_length,
		      u8 region_space, struct acpi_walk_state *walk_state);

acpi_status acpi_ex_create_event(struct acpi_walk_state *walk_state);

acpi_status acpi_ex_create_alias(struct acpi_walk_state *walk_state);

acpi_status
acpi_ex_create_method(u8 * aml_start,
		      u32 aml_length, struct acpi_walk_state *walk_state);


acpi_status
acpi_ex_load_op(union acpi_operand_object *obj_desc,
		union acpi_operand_object *target,
		struct acpi_walk_state *walk_state);

acpi_status
acpi_ex_load_table_op(struct acpi_walk_state *walk_state,
		      union acpi_operand_object **return_desc);

acpi_status acpi_ex_unload_table(union acpi_operand_object *ddb_handle);


acpi_status
acpi_ex_acquire_mutex(union acpi_operand_object *time_desc,
		      union acpi_operand_object *obj_desc,
		      struct acpi_walk_state *walk_state);

acpi_status
acpi_ex_acquire_mutex_object(u16 timeout,
			     union acpi_operand_object *obj_desc,
			     acpi_thread_id thread_id);

acpi_status
acpi_ex_release_mutex(union acpi_operand_object *obj_desc,
		      struct acpi_walk_state *walk_state);

acpi_status acpi_ex_release_mutex_object(union acpi_operand_object *obj_desc);

void acpi_ex_release_all_mutexes(struct acpi_thread_state *thread);

void acpi_ex_unlink_mutex(union acpi_operand_object *obj_desc);


acpi_status
acpi_ex_prep_common_field_object(union acpi_operand_object *obj_desc,
				 u8 field_flags,
				 u8 field_attribute,
				 u32 field_bit_position, u32 field_bit_length);

acpi_status acpi_ex_prep_field_value(struct acpi_create_field_info *info);


acpi_status
acpi_ex_system_do_notify_op(union acpi_operand_object *value,
			    union acpi_operand_object *obj_desc);

acpi_status acpi_ex_system_do_suspend(acpi_integer time);

acpi_status acpi_ex_system_do_stall(u32 time);

acpi_status acpi_ex_system_signal_event(union acpi_operand_object *obj_desc);

acpi_status
acpi_ex_system_wait_event(union acpi_operand_object *time,
			  union acpi_operand_object *obj_desc);

acpi_status acpi_ex_system_reset_event(union acpi_operand_object *obj_desc);

acpi_status
acpi_ex_system_wait_semaphore(acpi_semaphore semaphore, u16 timeout);

acpi_status acpi_ex_system_wait_mutex(acpi_mutex mutex, u16 timeout);


acpi_status acpi_ex_opcode_0A_0T_1R(struct acpi_walk_state *walk_state);

acpi_status acpi_ex_opcode_1A_0T_0R(struct acpi_walk_state *walk_state);

acpi_status acpi_ex_opcode_1A_0T_1R(struct acpi_walk_state *walk_state);

acpi_status acpi_ex_opcode_1A_1T_1R(struct acpi_walk_state *walk_state);

acpi_status acpi_ex_opcode_1A_1T_0R(struct acpi_walk_state *walk_state);


acpi_status acpi_ex_opcode_2A_0T_0R(struct acpi_walk_state *walk_state);

acpi_status acpi_ex_opcode_2A_0T_1R(struct acpi_walk_state *walk_state);

acpi_status acpi_ex_opcode_2A_1T_1R(struct acpi_walk_state *walk_state);

acpi_status acpi_ex_opcode_2A_2T_1R(struct acpi_walk_state *walk_state);


acpi_status acpi_ex_opcode_3A_0T_0R(struct acpi_walk_state *walk_state);

acpi_status acpi_ex_opcode_3A_1T_1R(struct acpi_walk_state *walk_state);


acpi_status acpi_ex_opcode_6A_0T_1R(struct acpi_walk_state *walk_state);


acpi_status
acpi_ex_resolve_to_value(union acpi_operand_object **stack_ptr,
			 struct acpi_walk_state *walk_state);

acpi_status
acpi_ex_resolve_multiple(struct acpi_walk_state *walk_state,
			 union acpi_operand_object *operand,
			 acpi_object_type * return_type,
			 union acpi_operand_object **return_desc);


acpi_status
acpi_ex_resolve_node_to_value(struct acpi_namespace_node **stack_ptr,
			      struct acpi_walk_state *walk_state);


acpi_status
acpi_ex_resolve_operands(u16 opcode,
			 union acpi_operand_object **stack_ptr,
			 struct acpi_walk_state *walk_state);


void acpi_ex_dump_operand(union acpi_operand_object *obj_desc, u32 depth);

void
acpi_ex_dump_operands(union acpi_operand_object **operands,
		      const char *opcode_name, u32 num_opcodes);

#ifdef	ACPI_FUTURE_USAGE
void
acpi_ex_dump_object_descriptor(union acpi_operand_object *object, u32 flags);

void acpi_ex_dump_namespace_node(struct acpi_namespace_node *node, u32 flags);
#endif				


acpi_status
acpi_ex_get_name_string(acpi_object_type data_type,
			u8 * in_aml_address,
			char **out_name_string, u32 * out_name_length);


acpi_status
acpi_ex_store(union acpi_operand_object *val_desc,
	      union acpi_operand_object *dest_desc,
	      struct acpi_walk_state *walk_state);

acpi_status
acpi_ex_store_object_to_node(union acpi_operand_object *source_desc,
			     struct acpi_namespace_node *node,
			     struct acpi_walk_state *walk_state,
			     u8 implicit_conversion);

#define ACPI_IMPLICIT_CONVERSION        TRUE
#define ACPI_NO_IMPLICIT_CONVERSION     FALSE


acpi_status
acpi_ex_resolve_object(union acpi_operand_object **source_desc_ptr,
		       acpi_object_type target_type,
		       struct acpi_walk_state *walk_state);

acpi_status
acpi_ex_store_object_to_object(union acpi_operand_object *source_desc,
			       union acpi_operand_object *dest_desc,
			       union acpi_operand_object **new_desc,
			       struct acpi_walk_state *walk_state);


acpi_status
acpi_ex_store_buffer_to_buffer(union acpi_operand_object *source_desc,
			       union acpi_operand_object *target_desc);

acpi_status
acpi_ex_store_string_to_string(union acpi_operand_object *source_desc,
			       union acpi_operand_object *target_desc);


acpi_status
acpi_ex_copy_integer_to_index_field(union acpi_operand_object *source_desc,
				    union acpi_operand_object *target_desc);

acpi_status
acpi_ex_copy_integer_to_bank_field(union acpi_operand_object *source_desc,
				   union acpi_operand_object *target_desc);

acpi_status
acpi_ex_copy_data_to_named_field(union acpi_operand_object *source_desc,
				 struct acpi_namespace_node *node);

acpi_status
acpi_ex_copy_integer_to_buffer_field(union acpi_operand_object *source_desc,
				     union acpi_operand_object *target_desc);


void acpi_ex_enter_interpreter(void);

void acpi_ex_exit_interpreter(void);

void acpi_ex_reacquire_interpreter(void);

void acpi_ex_relinquish_interpreter(void);

void acpi_ex_truncate_for32bit_table(union acpi_operand_object *obj_desc);

void acpi_ex_acquire_global_lock(u32 rule);

void acpi_ex_release_global_lock(u32 rule);

void acpi_ex_eisa_id_to_string(char *dest, acpi_integer compressed_id);

void acpi_ex_integer_to_string(char *dest, acpi_integer value);


acpi_status
acpi_ex_system_memory_space_handler(u32 function,
				    acpi_physical_address address,
				    u32 bit_width,
				    acpi_integer * value,
				    void *handler_context,
				    void *region_context);

acpi_status
acpi_ex_system_io_space_handler(u32 function,
				acpi_physical_address address,
				u32 bit_width,
				acpi_integer * value,
				void *handler_context, void *region_context);

acpi_status
acpi_ex_pci_config_space_handler(u32 function,
				 acpi_physical_address address,
				 u32 bit_width,
				 acpi_integer * value,
				 void *handler_context, void *region_context);

acpi_status
acpi_ex_cmos_space_handler(u32 function,
			   acpi_physical_address address,
			   u32 bit_width,
			   acpi_integer * value,
			   void *handler_context, void *region_context);

acpi_status
acpi_ex_pci_bar_space_handler(u32 function,
			      acpi_physical_address address,
			      u32 bit_width,
			      acpi_integer * value,
			      void *handler_context, void *region_context);

acpi_status
acpi_ex_embedded_controller_space_handler(u32 function,
					  acpi_physical_address address,
					  u32 bit_width,
					  acpi_integer * value,
					  void *handler_context,
					  void *region_context);

acpi_status
acpi_ex_sm_bus_space_handler(u32 function,
			     acpi_physical_address address,
			     u32 bit_width,
			     acpi_integer * value,
			     void *handler_context, void *region_context);

acpi_status
acpi_ex_data_table_space_handler(u32 function,
				 acpi_physical_address address,
				 u32 bit_width,
				 acpi_integer * value,
				 void *handler_context, void *region_context);

#endif				
