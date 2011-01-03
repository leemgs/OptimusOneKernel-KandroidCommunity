



#include <acpi/acpi.h>
#include "accommon.h"
#include "acnamesp.h"

#define _COMPONENT          ACPI_NAMESPACE
ACPI_MODULE_NAME("nswalk")


struct acpi_namespace_node *acpi_ns_get_next_node(struct acpi_namespace_node
						  *parent_node,
						  struct acpi_namespace_node
						  *child_node)
{
	ACPI_FUNCTION_ENTRY();

	if (!child_node) {

		

		return parent_node->child;
	}

	
	if (child_node->flags & ANOBJ_END_OF_PEER_LIST) {
		return NULL;
	}

	

	return child_node->peer;
}



struct acpi_namespace_node *acpi_ns_get_next_node_typed(acpi_object_type type,
							struct
							acpi_namespace_node
							*parent_node,
							struct
							acpi_namespace_node
							*child_node)
{
	struct acpi_namespace_node *next_node = NULL;

	ACPI_FUNCTION_ENTRY();

	next_node = acpi_ns_get_next_node(parent_node, child_node);


	

	if (type == ACPI_TYPE_ANY) {

		

		return (next_node);
	}

	

	while (next_node) {

		

		if (next_node->type == type) {
			return (next_node);
		}

		

		next_node = acpi_ns_get_next_valid_node(next_node);
	}

	

	return (NULL);
}



acpi_status
acpi_ns_walk_namespace(acpi_object_type type,
		       acpi_handle start_node,
		       u32 max_depth,
		       u32 flags,
		       acpi_walk_callback user_function,
		       void *context, void **return_value)
{
	acpi_status status;
	acpi_status mutex_status;
	struct acpi_namespace_node *child_node;
	struct acpi_namespace_node *parent_node;
	acpi_object_type child_type;
	u32 level;

	ACPI_FUNCTION_TRACE(ns_walk_namespace);

	

	if (start_node == ACPI_ROOT_OBJECT) {
		start_node = acpi_gbl_root_node;
	}

	

	parent_node = start_node;
	child_node = NULL;
	child_type = ACPI_TYPE_ANY;
	level = 1;

	
	while (level > 0) {

		

		status = AE_OK;
		child_node = acpi_ns_get_next_node(parent_node, child_node);
		if (child_node) {

			

			if (type != ACPI_TYPE_ANY) {
				child_type = child_node->type;
			}

			
			if ((child_node->flags & ANOBJ_TEMPORARY) &&
			    !(flags & ACPI_NS_WALK_TEMP_NODES)) {
				status = AE_CTRL_DEPTH;
			}

			

			else if (child_type == type) {
				
				if (flags & ACPI_NS_WALK_UNLOCK) {
					mutex_status =
					    acpi_ut_release_mutex
					    (ACPI_MTX_NAMESPACE);
					if (ACPI_FAILURE(mutex_status)) {
						return_ACPI_STATUS
						    (mutex_status);
					}
				}

				status =
				    user_function(child_node, level, context,
						  return_value);

				if (flags & ACPI_NS_WALK_UNLOCK) {
					mutex_status =
					    acpi_ut_acquire_mutex
					    (ACPI_MTX_NAMESPACE);
					if (ACPI_FAILURE(mutex_status)) {
						return_ACPI_STATUS
						    (mutex_status);
					}
				}

				switch (status) {
				case AE_OK:
				case AE_CTRL_DEPTH:

					
					break;

				case AE_CTRL_TERMINATE:

					

					return_ACPI_STATUS(AE_OK);

				default:

					

					return_ACPI_STATUS(status);
				}
			}

			
			if ((level < max_depth) && (status != AE_CTRL_DEPTH)) {
				if (child_node->child) {

					

					level++;
					parent_node = child_node;
					child_node = NULL;
				}
			}
		} else {
			
			level--;
			child_node = parent_node;
			parent_node = acpi_ns_get_parent_node(parent_node);
		}
	}

	

	return_ACPI_STATUS(AE_OK);
}
