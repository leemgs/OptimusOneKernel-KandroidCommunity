
#include "drmP.h"
#include "drm.h"
#include "i915_drm.h"
#include "i915_drv.h"
#include "intel_bios.h"

#define	SLAVE_ADDR1	0x70
#define	SLAVE_ADDR2	0x72

static void *
find_section(struct bdb_header *bdb, int section_id)
{
	u8 *base = (u8 *)bdb;
	int index = 0;
	u16 total, current_size;
	u8 current_id;

	
	index += bdb->header_size;
	total = bdb->bdb_size;

	
	while (index < total) {
		current_id = *(base + index);
		index++;
		current_size = *((u16 *)(base + index));
		index += 2;
		if (current_id == section_id)
			return base + index;
		index += current_size;
	}

	return NULL;
}

static u16
get_blocksize(void *p)
{
	u16 *block_ptr, block_size;

	block_ptr = (u16 *)((char *)p - 2);
	block_size = *block_ptr;
	return block_size;
}

static void
fill_detail_timing_data(struct drm_display_mode *panel_fixed_mode,
			struct lvds_dvo_timing *dvo_timing)
{
	panel_fixed_mode->hdisplay = (dvo_timing->hactive_hi << 8) |
		dvo_timing->hactive_lo;
	panel_fixed_mode->hsync_start = panel_fixed_mode->hdisplay +
		((dvo_timing->hsync_off_hi << 8) | dvo_timing->hsync_off_lo);
	panel_fixed_mode->hsync_end = panel_fixed_mode->hsync_start +
		dvo_timing->hsync_pulse_width;
	panel_fixed_mode->htotal = panel_fixed_mode->hdisplay +
		((dvo_timing->hblank_hi << 8) | dvo_timing->hblank_lo);

	panel_fixed_mode->vdisplay = (dvo_timing->vactive_hi << 8) |
		dvo_timing->vactive_lo;
	panel_fixed_mode->vsync_start = panel_fixed_mode->vdisplay +
		dvo_timing->vsync_off;
	panel_fixed_mode->vsync_end = panel_fixed_mode->vsync_start +
		dvo_timing->vsync_pulse_width;
	panel_fixed_mode->vtotal = panel_fixed_mode->vdisplay +
		((dvo_timing->vblank_hi << 8) | dvo_timing->vblank_lo);
	panel_fixed_mode->clock = dvo_timing->clock * 10;
	panel_fixed_mode->type = DRM_MODE_TYPE_PREFERRED;

	
	if (panel_fixed_mode->hsync_end > panel_fixed_mode->htotal)
		panel_fixed_mode->htotal = panel_fixed_mode->hsync_end + 1;
	if (panel_fixed_mode->vsync_end > panel_fixed_mode->vtotal)
		panel_fixed_mode->vtotal = panel_fixed_mode->vsync_end + 1;

	drm_mode_set_name(panel_fixed_mode);
}


static void
parse_lfp_panel_data(struct drm_i915_private *dev_priv,
			    struct bdb_header *bdb)
{
	struct bdb_lvds_options *lvds_options;
	struct bdb_lvds_lfp_data *lvds_lfp_data;
	struct bdb_lvds_lfp_data_ptrs *lvds_lfp_data_ptrs;
	struct bdb_lvds_lfp_data_entry *entry;
	struct lvds_dvo_timing *dvo_timing;
	struct drm_display_mode *panel_fixed_mode;
	int lfp_data_size, dvo_timing_offset;

	
	dev_priv->lvds_dither = 0;
	dev_priv->lvds_vbt = 0;

	lvds_options = find_section(bdb, BDB_LVDS_OPTIONS);
	if (!lvds_options)
		return;

	dev_priv->lvds_dither = lvds_options->pixel_dither;
	if (lvds_options->panel_type == 0xff)
		return;

	lvds_lfp_data = find_section(bdb, BDB_LVDS_LFP_DATA);
	if (!lvds_lfp_data)
		return;

	lvds_lfp_data_ptrs = find_section(bdb, BDB_LVDS_LFP_DATA_PTRS);
	if (!lvds_lfp_data_ptrs)
		return;

	dev_priv->lvds_vbt = 1;

	lfp_data_size = lvds_lfp_data_ptrs->ptr[1].dvo_timing_offset -
		lvds_lfp_data_ptrs->ptr[0].dvo_timing_offset;
	entry = (struct bdb_lvds_lfp_data_entry *)
		((uint8_t *)lvds_lfp_data->data + (lfp_data_size *
						   lvds_options->panel_type));
	dvo_timing_offset = lvds_lfp_data_ptrs->ptr[0].dvo_timing_offset -
		lvds_lfp_data_ptrs->ptr[0].fp_timing_offset;

	
	dvo_timing = (struct lvds_dvo_timing *)
			((unsigned char *)entry + dvo_timing_offset);

	panel_fixed_mode = kzalloc(sizeof(*panel_fixed_mode), GFP_KERNEL);

	fill_detail_timing_data(panel_fixed_mode, dvo_timing);

	dev_priv->lfp_lvds_vbt_mode = panel_fixed_mode;

	DRM_DEBUG("Found panel mode in BIOS VBT tables:\n");
	drm_mode_debug_printmodeline(panel_fixed_mode);

	return;
}


static void
parse_sdvo_panel_data(struct drm_i915_private *dev_priv,
		      struct bdb_header *bdb)
{
	struct bdb_sdvo_lvds_options *sdvo_lvds_options;
	struct lvds_dvo_timing *dvo_timing;
	struct drm_display_mode *panel_fixed_mode;

	dev_priv->sdvo_lvds_vbt_mode = NULL;

	sdvo_lvds_options = find_section(bdb, BDB_SDVO_LVDS_OPTIONS);
	if (!sdvo_lvds_options)
		return;

	dvo_timing = find_section(bdb, BDB_SDVO_PANEL_DTDS);
	if (!dvo_timing)
		return;

	panel_fixed_mode = kzalloc(sizeof(*panel_fixed_mode), GFP_KERNEL);

	if (!panel_fixed_mode)
		return;

	fill_detail_timing_data(panel_fixed_mode,
			dvo_timing + sdvo_lvds_options->panel_type);

	dev_priv->sdvo_lvds_vbt_mode = panel_fixed_mode;

	return;
}

static void
parse_general_features(struct drm_i915_private *dev_priv,
		       struct bdb_header *bdb)
{
	struct bdb_general_features *general;

	
	dev_priv->int_tv_support = 1;
	dev_priv->int_crt_support = 1;

	general = find_section(bdb, BDB_GENERAL_FEATURES);
	if (general) {
		dev_priv->int_tv_support = general->int_tv_support;
		dev_priv->int_crt_support = general->int_crt_support;
		dev_priv->lvds_use_ssc = general->enable_ssc;

		if (dev_priv->lvds_use_ssc) {
			if (IS_I85X(dev_priv->dev))
				dev_priv->lvds_ssc_freq =
					general->ssc_freq ? 66 : 48;
			else if (IS_IGDNG(dev_priv->dev))
				dev_priv->lvds_ssc_freq =
					general->ssc_freq ? 100 : 120;
			else
				dev_priv->lvds_ssc_freq =
					general->ssc_freq ? 100 : 96;
		}
	}
}

static void
parse_general_definitions(struct drm_i915_private *dev_priv,
			  struct bdb_header *bdb)
{
	struct bdb_general_definitions *general;
	const int crt_bus_map_table[] = {
		GPIOB,
		GPIOA,
		GPIOC,
		GPIOD,
		GPIOE,
		GPIOF,
	};

	
	dev_priv->crt_ddc_bus = -1;

	general = find_section(bdb, BDB_GENERAL_DEFINITIONS);
	if (general) {
		u16 block_size = get_blocksize(general);
		if (block_size >= sizeof(*general)) {
			int bus_pin = general->crt_ddc_gmbus_pin;
			DRM_DEBUG("crt_ddc_bus_pin: %d\n", bus_pin);
			if ((bus_pin >= 1) && (bus_pin <= 6)) {
				dev_priv->crt_ddc_bus =
					crt_bus_map_table[bus_pin-1];
			}
		} else {
			DRM_DEBUG("BDB_GD too small (%d). Invalid.\n",
				  block_size);
		}
	}
}

static void
parse_sdvo_device_mapping(struct drm_i915_private *dev_priv,
		       struct bdb_header *bdb)
{
	struct sdvo_device_mapping *p_mapping;
	struct bdb_general_definitions *p_defs;
	struct child_device_config *p_child;
	int i, child_device_num, count;
	u16	block_size;

	p_defs = find_section(bdb, BDB_GENERAL_DEFINITIONS);
	if (!p_defs) {
		DRM_DEBUG("No general definition block is found\n");
		return;
	}
	
	if (p_defs->child_dev_size != sizeof(*p_child)) {
		
		DRM_DEBUG("different child size is found. Invalid.\n");
		return;
	}
	
	block_size = get_blocksize(p_defs);
	
	child_device_num = (block_size - sizeof(*p_defs)) /
				sizeof(*p_child);
	count = 0;
	for (i = 0; i < child_device_num; i++) {
		p_child = &(p_defs->devices[i]);
		if (!p_child->device_type) {
			
			continue;
		}
		if (p_child->slave_addr != SLAVE_ADDR1 &&
			p_child->slave_addr != SLAVE_ADDR2) {
			
			continue;
		}
		if (p_child->dvo_port != DEVICE_PORT_DVOB &&
			p_child->dvo_port != DEVICE_PORT_DVOC) {
			
			DRM_DEBUG("Incorrect SDVO port. Skip it \n");
			continue;
		}
		DRM_DEBUG("the SDVO device with slave addr %2x is found on "
				"%s port\n",
				p_child->slave_addr,
				(p_child->dvo_port == DEVICE_PORT_DVOB) ?
					"SDVOB" : "SDVOC");
		p_mapping = &(dev_priv->sdvo_mappings[p_child->dvo_port - 1]);
		if (!p_mapping->initialized) {
			p_mapping->dvo_port = p_child->dvo_port;
			p_mapping->slave_addr = p_child->slave_addr;
			p_mapping->dvo_wiring = p_child->dvo_wiring;
			p_mapping->initialized = 1;
		} else {
			DRM_DEBUG("Maybe one SDVO port is shared by "
					 "two SDVO device.\n");
		}
		if (p_child->slave2_addr) {
			
			
			DRM_DEBUG("there exists the slave2_addr. Maybe this "
				"is a SDVO device with multiple inputs.\n");
		}
		count++;
	}

	if (!count) {
		
		DRM_DEBUG("No SDVO device info is found in VBT\n");
	}
	return;
}

static void
parse_driver_features(struct drm_i915_private *dev_priv,
		       struct bdb_header *bdb)
{
	struct drm_device *dev = dev_priv->dev;
	struct bdb_driver_features *driver;

	driver = find_section(bdb, BDB_DRIVER_FEATURES);
	if (!driver)
		return;

	if (driver && SUPPORTS_EDP(dev) &&
	    driver->lvds_config == BDB_DRIVER_FEATURE_EDP) {
		dev_priv->edp_support = 1;
	} else {
		dev_priv->edp_support = 0;
	}

	if (driver && driver->dual_frequency)
		dev_priv->render_reclock_avail = true;
}


bool
intel_init_bios(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct pci_dev *pdev = dev->pdev;
	struct vbt_header *vbt = NULL;
	struct bdb_header *bdb;
	u8 __iomem *bios;
	size_t size;
	int i;

	bios = pci_map_rom(pdev, &size);
	if (!bios)
		return -1;

	
	for (i = 0; i + 4 < size; i++) {
		if (!memcmp(bios + i, "$VBT", 4)) {
			vbt = (struct vbt_header *)(bios + i);
			break;
		}
	}

	if (!vbt) {
		DRM_ERROR("VBT signature missing\n");
		pci_unmap_rom(pdev, bios);
		return -1;
	}

	bdb = (struct bdb_header *)(bios + i + vbt->bdb_offset);

	
	parse_general_features(dev_priv, bdb);
	parse_general_definitions(dev_priv, bdb);
	parse_lfp_panel_data(dev_priv, bdb);
	parse_sdvo_panel_data(dev_priv, bdb);
	parse_sdvo_device_mapping(dev_priv, bdb);
	parse_driver_features(dev_priv, bdb);

	pci_unmap_rom(pdev, bios);

	return 0;
}
