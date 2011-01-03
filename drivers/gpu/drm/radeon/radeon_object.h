
#ifndef __RADEON_OBJECT_H__
#define __RADEON_OBJECT_H__

#include <ttm/ttm_bo_api.h>
#include <ttm/ttm_bo_driver.h>
#include <ttm/ttm_placement.h>
#include <ttm/ttm_module.h>


struct radeon_mman {
	struct ttm_bo_global_ref        bo_global_ref;
	struct ttm_global_reference	mem_global_ref;
	bool				mem_global_referenced;
	struct ttm_bo_device		bdev;
};

#endif
