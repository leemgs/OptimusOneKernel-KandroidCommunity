



#include <acpi/acpi.h>
#include "accommon.h"
#include "acresrc.h"
#include "acnamesp.h"

#define _COMPONENT          ACPI_RESOURCES
ACPI_MODULE_NAME("rscalc")


static u8 acpi_rs_count_set_bits(u16 bit_field);

static acpi_rs_length
acpi_rs_struct_option_length(struct acpi_resource_source *resource_source);

static u32
acpi_rs_stream_option_length(u32 resource_length, u32 minimum_total_length);



static u8 acpi_rs_count_set_bits(u16 bit_field)
{
	u8 bits_set;

	ACPI_FUNCTION_ENTRY();

	for (bits_set = 0; bit_field; bits_set++) {

		

		bit_field &= (u16) (bit_field - 1);
	}

	return bits_set;
}



static acpi_rs_length
acpi_rs_struct_option_length(struct acpi_resource_source *resource_source)
{
	ACPI_FUNCTION_ENTRY();

	
	if (resource_source->string_ptr) {
		return ((acpi_rs_length) (resource_source->string_length + 1));
	}

	return (0);
}



static u32
acpi_rs_stream_option_length(u32 resource_length,
			     u32 minimum_aml_resource_length)
{
	u32 string_length = 0;

	ACPI_FUNCTION_ENTRY();

	

	
	if (resource_length > minimum_aml_resource_length) {

		

		string_length =
		    resource_length - minimum_aml_resource_length - 1;
	}

	
	return ((u32) ACPI_ROUND_UP_TO_NATIVE_WORD(string_length));
}



acpi_status
acpi_rs_get_aml_length(struct acpi_resource * resource, acpi_size * size_needed)
{
	acpi_size aml_size_needed = 0;
	acpi_rs_length total_size;

	ACPI_FUNCTION_TRACE(rs_get_aml_length);

	

	while (resource) {

		

		if (resource->type > ACPI_RESOURCE_TYPE_MAX) {
			return_ACPI_STATUS(AE_AML_INVALID_RESOURCE_TYPE);
		}

		

		total_size = acpi_gbl_aml_resource_sizes[resource->type];

		
		switch (resource->type) {
		case ACPI_RESOURCE_TYPE_IRQ:

			

			if (resource->data.irq.descriptor_length == 2) {
				total_size--;
			}
			break;

		case ACPI_RESOURCE_TYPE_START_DEPENDENT:

			

			if (resource->data.irq.descriptor_length == 0) {
				total_size--;
			}
			break;

		case ACPI_RESOURCE_TYPE_VENDOR:
			
			if (resource->data.vendor.byte_length > 7) {

				

				total_size =
				    sizeof(struct aml_resource_large_header);
			}

			

			total_size = (acpi_rs_length)
			    (total_size + resource->data.vendor.byte_length);
			break;

		case ACPI_RESOURCE_TYPE_END_TAG:
			
			*size_needed = aml_size_needed + total_size;

			

			return_ACPI_STATUS(AE_OK);

		case ACPI_RESOURCE_TYPE_ADDRESS16:
			
			total_size = (acpi_rs_length)
			    (total_size +
			     acpi_rs_struct_option_length(&resource->data.
							  address16.
							  resource_source));
			break;

		case ACPI_RESOURCE_TYPE_ADDRESS32:
			
			total_size = (acpi_rs_length)
			    (total_size +
			     acpi_rs_struct_option_length(&resource->data.
							  address32.
							  resource_source));
			break;

		case ACPI_RESOURCE_TYPE_ADDRESS64:
			
			total_size = (acpi_rs_length)
			    (total_size +
			     acpi_rs_struct_option_length(&resource->data.
							  address64.
							  resource_source));
			break;

		case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
			
			total_size = (acpi_rs_length)
			    (total_size +
			     ((resource->data.extended_irq.interrupt_count -
			       1) * 4) +
			     
			     acpi_rs_struct_option_length(&resource->data.
							  extended_irq.
							  resource_source));
			break;

		default:
			break;
		}

		

		aml_size_needed += total_size;

		

		resource =
		    ACPI_ADD_PTR(struct acpi_resource, resource,
				 resource->length);
	}

	

	return_ACPI_STATUS(AE_AML_NO_RESOURCE_END_TAG);
}



acpi_status
acpi_rs_get_list_length(u8 * aml_buffer,
			u32 aml_buffer_length, acpi_size * size_needed)
{
	acpi_status status;
	u8 *end_aml;
	u8 *buffer;
	u32 buffer_size;
	u16 temp16;
	u16 resource_length;
	u32 extra_struct_bytes;
	u8 resource_index;
	u8 minimum_aml_resource_length;

	ACPI_FUNCTION_TRACE(rs_get_list_length);

	*size_needed = 0;
	end_aml = aml_buffer + aml_buffer_length;

	

	while (aml_buffer < end_aml) {

		

		status = acpi_ut_validate_resource(aml_buffer, &resource_index);
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}

		

		resource_length = acpi_ut_get_resource_length(aml_buffer);
		minimum_aml_resource_length =
		    acpi_gbl_resource_aml_sizes[resource_index];

		
		extra_struct_bytes = 0;
		buffer =
		    aml_buffer + acpi_ut_get_resource_header_length(aml_buffer);

		switch (acpi_ut_get_resource_type(aml_buffer)) {
		case ACPI_RESOURCE_NAME_IRQ:
			
			ACPI_MOVE_16_TO_16(&temp16, buffer);
			extra_struct_bytes = acpi_rs_count_set_bits(temp16);
			break;

		case ACPI_RESOURCE_NAME_DMA:
			
			extra_struct_bytes = acpi_rs_count_set_bits(*buffer);
			break;

		case ACPI_RESOURCE_NAME_VENDOR_SMALL:
		case ACPI_RESOURCE_NAME_VENDOR_LARGE:
			
			extra_struct_bytes = resource_length;
			break;

		case ACPI_RESOURCE_NAME_END_TAG:
			
			*size_needed += ACPI_RS_SIZE_MIN;
			return_ACPI_STATUS(AE_OK);

		case ACPI_RESOURCE_NAME_ADDRESS32:
		case ACPI_RESOURCE_NAME_ADDRESS16:
		case ACPI_RESOURCE_NAME_ADDRESS64:
			
			extra_struct_bytes =
			    acpi_rs_stream_option_length(resource_length,
							 minimum_aml_resource_length);
			break;

		case ACPI_RESOURCE_NAME_EXTENDED_IRQ:
			
			extra_struct_bytes = (buffer[1] - 1) * sizeof(u32);

			

			extra_struct_bytes +=
			    acpi_rs_stream_option_length(resource_length -
							 extra_struct_bytes,
							 minimum_aml_resource_length);
			break;

		default:
			break;
		}

		
		buffer_size = acpi_gbl_resource_struct_sizes[resource_index] +
		    extra_struct_bytes;
		buffer_size = (u32) ACPI_ROUND_UP_TO_NATIVE_WORD(buffer_size);

		*size_needed += buffer_size;

		ACPI_DEBUG_PRINT((ACPI_DB_RESOURCES,
				  "Type %.2X, AmlLength %.2X InternalLength %.2X\n",
				  acpi_ut_get_resource_type(aml_buffer),
				  acpi_ut_get_descriptor_length(aml_buffer),
				  buffer_size));

		
		aml_buffer += acpi_ut_get_descriptor_length(aml_buffer);
	}

	

	return_ACPI_STATUS(AE_AML_NO_RESOURCE_END_TAG);
}



acpi_status
acpi_rs_get_pci_routing_table_length(union acpi_operand_object *package_object,
				     acpi_size * buffer_size_needed)
{
	u32 number_of_elements;
	acpi_size temp_size_needed = 0;
	union acpi_operand_object **top_object_list;
	u32 index;
	union acpi_operand_object *package_element;
	union acpi_operand_object **sub_object_list;
	u8 name_found;
	u32 table_index;

	ACPI_FUNCTION_TRACE(rs_get_pci_routing_table_length);

	number_of_elements = package_object->package.count;

	
	top_object_list = package_object->package.elements;

	for (index = 0; index < number_of_elements; index++) {

		

		package_element = *top_object_list;

		

		if (!package_element ||
		    (package_element->common.type != ACPI_TYPE_PACKAGE)) {
			return_ACPI_STATUS(AE_AML_OPERAND_TYPE);
		}

		
		sub_object_list = package_element->package.elements;

		

		name_found = FALSE;

		for (table_index = 0; table_index < 4 && !name_found;
		     table_index++) {
			if (*sub_object_list &&	
			    ((ACPI_TYPE_STRING ==
			      (*sub_object_list)->common.type) ||
			     ((ACPI_TYPE_LOCAL_REFERENCE ==
			       (*sub_object_list)->common.type) &&
			      ((*sub_object_list)->reference.class ==
			       ACPI_REFCLASS_NAME)))) {
				name_found = TRUE;
			} else {
				

				sub_object_list++;
			}
		}

		temp_size_needed += (sizeof(struct acpi_pci_routing_table) - 4);

		

		if (name_found) {
			if ((*sub_object_list)->common.type == ACPI_TYPE_STRING) {
				
				temp_size_needed += ((acpi_size)
						     (*sub_object_list)->string.
						     length + 1);
			} else {
				temp_size_needed +=
				    acpi_ns_get_pathname_length((*sub_object_list)->reference.node);
			}
		} else {
			
			temp_size_needed += sizeof(u32);
		}

		

		temp_size_needed = ACPI_ROUND_UP_TO_64BIT(temp_size_needed);

		

		top_object_list++;
	}

	
	*buffer_size_needed =
	    temp_size_needed + sizeof(struct acpi_pci_routing_table);
	return_ACPI_STATUS(AE_OK);
}
