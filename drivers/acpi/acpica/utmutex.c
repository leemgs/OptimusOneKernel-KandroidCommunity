



#include <acpi/acpi.h>
#include "accommon.h"

#define _COMPONENT          ACPI_UTILITIES
ACPI_MODULE_NAME("utmutex")


static acpi_status acpi_ut_create_mutex(acpi_mutex_handle mutex_id);

static acpi_status acpi_ut_delete_mutex(acpi_mutex_handle mutex_id);



acpi_status acpi_ut_mutex_initialize(void)
{
	u32 i;
	acpi_status status;

	ACPI_FUNCTION_TRACE(ut_mutex_initialize);

	

	for (i = 0; i < ACPI_NUM_MUTEX; i++) {
		status = acpi_ut_create_mutex(i);
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}
	}

	

	spin_lock_init(acpi_gbl_gpe_lock);
	spin_lock_init(acpi_gbl_hardware_lock);

	

	status = acpi_ut_create_rw_lock(&acpi_gbl_namespace_rw_lock);
	return_ACPI_STATUS(status);
}



void acpi_ut_mutex_terminate(void)
{
	u32 i;

	ACPI_FUNCTION_TRACE(ut_mutex_terminate);

	

	for (i = 0; i < ACPI_NUM_MUTEX; i++) {
		(void)acpi_ut_delete_mutex(i);
	}

	

	acpi_os_delete_lock(acpi_gbl_gpe_lock);
	acpi_os_delete_lock(acpi_gbl_hardware_lock);

	

	acpi_ut_delete_rw_lock(&acpi_gbl_namespace_rw_lock);
	return_VOID;
}



static acpi_status acpi_ut_create_mutex(acpi_mutex_handle mutex_id)
{
	acpi_status status = AE_OK;

	ACPI_FUNCTION_TRACE_U32(ut_create_mutex, mutex_id);

	if (mutex_id > ACPI_MAX_MUTEX) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	if (!acpi_gbl_mutex_info[mutex_id].mutex) {
		status =
		    acpi_os_create_mutex(&acpi_gbl_mutex_info[mutex_id].mutex);
		acpi_gbl_mutex_info[mutex_id].thread_id =
		    ACPI_MUTEX_NOT_ACQUIRED;
		acpi_gbl_mutex_info[mutex_id].use_count = 0;
	}

	return_ACPI_STATUS(status);
}



static acpi_status acpi_ut_delete_mutex(acpi_mutex_handle mutex_id)
{

	ACPI_FUNCTION_TRACE_U32(ut_delete_mutex, mutex_id);

	if (mutex_id > ACPI_MAX_MUTEX) {
		return_ACPI_STATUS(AE_BAD_PARAMETER);
	}

	acpi_os_delete_mutex(acpi_gbl_mutex_info[mutex_id].mutex);

	acpi_gbl_mutex_info[mutex_id].mutex = NULL;
	acpi_gbl_mutex_info[mutex_id].thread_id = ACPI_MUTEX_NOT_ACQUIRED;

	return_ACPI_STATUS(AE_OK);
}



acpi_status acpi_ut_acquire_mutex(acpi_mutex_handle mutex_id)
{
	acpi_status status;
	acpi_thread_id this_thread_id;

	ACPI_FUNCTION_NAME(ut_acquire_mutex);

	if (mutex_id > ACPI_MAX_MUTEX) {
		return (AE_BAD_PARAMETER);
	}

	this_thread_id = acpi_os_get_thread_id();

#ifdef ACPI_MUTEX_DEBUG
	{
		u32 i;
		
		for (i = mutex_id; i < ACPI_NUM_MUTEX; i++) {
			if (acpi_gbl_mutex_info[i].thread_id == this_thread_id) {
				if (i == mutex_id) {
					ACPI_ERROR((AE_INFO,
						    "Mutex [%s] already acquired by this thread [%p]",
						    acpi_ut_get_mutex_name
						    (mutex_id),
						    ACPI_CAST_PTR(void,
								  this_thread_id)));

					return (AE_ALREADY_ACQUIRED);
				}

				ACPI_ERROR((AE_INFO,
					    "Invalid acquire order: Thread %p owns [%s], wants [%s]",
					    ACPI_CAST_PTR(void, this_thread_id),
					    acpi_ut_get_mutex_name(i),
					    acpi_ut_get_mutex_name(mutex_id)));

				return (AE_ACQUIRE_DEADLOCK);
			}
		}
	}
#endif

	ACPI_DEBUG_PRINT((ACPI_DB_MUTEX,
			  "Thread %p attempting to acquire Mutex [%s]\n",
			  ACPI_CAST_PTR(void, this_thread_id),
			  acpi_ut_get_mutex_name(mutex_id)));

	status = acpi_os_acquire_mutex(acpi_gbl_mutex_info[mutex_id].mutex,
				       ACPI_WAIT_FOREVER);
	if (ACPI_SUCCESS(status)) {
		ACPI_DEBUG_PRINT((ACPI_DB_MUTEX,
				  "Thread %p acquired Mutex [%s]\n",
				  ACPI_CAST_PTR(void, this_thread_id),
				  acpi_ut_get_mutex_name(mutex_id)));

		acpi_gbl_mutex_info[mutex_id].use_count++;
		acpi_gbl_mutex_info[mutex_id].thread_id = this_thread_id;
	} else {
		ACPI_EXCEPTION((AE_INFO, status,
				"Thread %p could not acquire Mutex [%X]",
				ACPI_CAST_PTR(void, this_thread_id), mutex_id));
	}

	return (status);
}



acpi_status acpi_ut_release_mutex(acpi_mutex_handle mutex_id)
{
	acpi_thread_id this_thread_id;

	ACPI_FUNCTION_NAME(ut_release_mutex);

	this_thread_id = acpi_os_get_thread_id();
	ACPI_DEBUG_PRINT((ACPI_DB_MUTEX, "Thread %p releasing Mutex [%s]\n",
			  ACPI_CAST_PTR(void, this_thread_id),
			  acpi_ut_get_mutex_name(mutex_id)));

	if (mutex_id > ACPI_MAX_MUTEX) {
		return (AE_BAD_PARAMETER);
	}

	
	if (acpi_gbl_mutex_info[mutex_id].thread_id == ACPI_MUTEX_NOT_ACQUIRED) {
		ACPI_ERROR((AE_INFO,
			    "Mutex [%X] is not acquired, cannot release",
			    mutex_id));

		return (AE_NOT_ACQUIRED);
	}
#ifdef ACPI_MUTEX_DEBUG
	{
		u32 i;
		
		for (i = mutex_id; i < ACPI_NUM_MUTEX; i++) {
			if (acpi_gbl_mutex_info[i].thread_id == this_thread_id) {
				if (i == mutex_id) {
					continue;
				}

				ACPI_ERROR((AE_INFO,
					    "Invalid release order: owns [%s], releasing [%s]",
					    acpi_ut_get_mutex_name(i),
					    acpi_ut_get_mutex_name(mutex_id)));

				return (AE_RELEASE_DEADLOCK);
			}
		}
	}
#endif

	

	acpi_gbl_mutex_info[mutex_id].thread_id = ACPI_MUTEX_NOT_ACQUIRED;

	acpi_os_release_mutex(acpi_gbl_mutex_info[mutex_id].mutex);
	return (AE_OK);
}
