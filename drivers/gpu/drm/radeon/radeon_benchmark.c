
#include <drm/drmP.h>
#include <drm/radeon_drm.h>
#include "radeon_reg.h"
#include "radeon.h"

void radeon_benchmark_move(struct radeon_device *rdev, unsigned bsize,
			   unsigned sdomain, unsigned ddomain)
{
	struct radeon_object *dobj = NULL;
	struct radeon_object *sobj = NULL;
	struct radeon_fence *fence = NULL;
	uint64_t saddr, daddr;
	unsigned long start_jiffies;
	unsigned long end_jiffies;
	unsigned long time;
	unsigned i, n, size;
	int r;

	size = bsize;
	n = 1024;
	r = radeon_object_create(rdev, NULL, size, true, sdomain, false, &sobj);
	if (r) {
		goto out_cleanup;
	}
	r = radeon_object_pin(sobj, sdomain, &saddr);
	if (r) {
		goto out_cleanup;
	}
	r = radeon_object_create(rdev, NULL, size, true, ddomain, false, &dobj);
	if (r) {
		goto out_cleanup;
	}
	r = radeon_object_pin(dobj, ddomain, &daddr);
	if (r) {
		goto out_cleanup;
	}
	start_jiffies = jiffies;
	for (i = 0; i < n; i++) {
		r = radeon_fence_create(rdev, &fence);
		if (r) {
			goto out_cleanup;
		}
		r = radeon_copy_dma(rdev, saddr, daddr, size / RADEON_GPU_PAGE_SIZE, fence);
		if (r) {
			goto out_cleanup;
		}
		r = radeon_fence_wait(fence, false);
		if (r) {
			goto out_cleanup;
		}
		radeon_fence_unref(&fence);
	}
	end_jiffies = jiffies;
	time = end_jiffies - start_jiffies;
	time = jiffies_to_msecs(time);
	if (time > 0) {
		i = ((n * size) >> 10) / time;
		printk(KERN_INFO "radeon: dma %u bo moves of %ukb from %d to %d"
		       " in %lums (%ukb/ms %ukb/s %uM/s)\n", n, size >> 10,
		       sdomain, ddomain, time, i, i * 1000, (i * 1000) / 1024);
	}
	start_jiffies = jiffies;
	for (i = 0; i < n; i++) {
		r = radeon_fence_create(rdev, &fence);
		if (r) {
			goto out_cleanup;
		}
		r = radeon_copy_blit(rdev, saddr, daddr, size / RADEON_GPU_PAGE_SIZE, fence);
		if (r) {
			goto out_cleanup;
		}
		r = radeon_fence_wait(fence, false);
		if (r) {
			goto out_cleanup;
		}
		radeon_fence_unref(&fence);
	}
	end_jiffies = jiffies;
	time = end_jiffies - start_jiffies;
	time = jiffies_to_msecs(time);
	if (time > 0) {
		i = ((n * size) >> 10) / time;
		printk(KERN_INFO "radeon: blit %u bo moves of %ukb from %d to %d"
		       " in %lums (%ukb/ms %ukb/s %uM/s)\n", n, size >> 10,
		       sdomain, ddomain, time, i, i * 1000, (i * 1000) / 1024);
	}
out_cleanup:
	if (sobj) {
		radeon_object_unpin(sobj);
		radeon_object_unref(&sobj);
	}
	if (dobj) {
		radeon_object_unpin(dobj);
		radeon_object_unref(&dobj);
	}
	if (fence) {
		radeon_fence_unref(&fence);
	}
	if (r) {
		printk(KERN_WARNING "Error while benchmarking BO move.\n");
	}
}

void radeon_benchmark(struct radeon_device *rdev)
{
	radeon_benchmark_move(rdev, 1024*1024, RADEON_GEM_DOMAIN_GTT,
			      RADEON_GEM_DOMAIN_VRAM);
	radeon_benchmark_move(rdev, 1024*1024, RADEON_GEM_DOMAIN_VRAM,
			      RADEON_GEM_DOMAIN_GTT);
}
