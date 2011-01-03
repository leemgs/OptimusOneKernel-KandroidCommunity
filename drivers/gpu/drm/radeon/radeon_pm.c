
#include "drmP.h"
#include "radeon.h"

int radeon_debugfs_pm_init(struct radeon_device *rdev);

int radeon_pm_init(struct radeon_device *rdev)
{
	if (radeon_debugfs_pm_init(rdev)) {
		DRM_ERROR("Failed to register debugfs file for CP !\n");
	}

	return 0;
}


#if defined(CONFIG_DEBUG_FS)

static int radeon_debugfs_pm_info(struct seq_file *m, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *) m->private;
	struct drm_device *dev = node->minor->dev;
	struct radeon_device *rdev = dev->dev_private;

	seq_printf(m, "engine clock: %u0 Hz\n", radeon_get_engine_clock(rdev));
	seq_printf(m, "memory clock: %u0 Hz\n", radeon_get_memory_clock(rdev));

	return 0;
}

static struct drm_info_list radeon_pm_info_list[] = {
	{"radeon_pm_info", radeon_debugfs_pm_info, 0, NULL},
};
#endif

int radeon_debugfs_pm_init(struct radeon_device *rdev)
{
#if defined(CONFIG_DEBUG_FS)
	return radeon_debugfs_add_files(rdev, radeon_pm_info_list, ARRAY_SIZE(radeon_pm_info_list));
#else
	return 0;
#endif
}
