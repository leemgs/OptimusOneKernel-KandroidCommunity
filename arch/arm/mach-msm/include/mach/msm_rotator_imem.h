

#ifndef __MSM_ROTATOR_IMEM_H__

enum {
	ROTATOR_REQUEST,
	JPEG_REQUEST
};


int msm_rotator_imem_allocate(int requestor);

void msm_rotator_imem_free(int requestor);

#endif
