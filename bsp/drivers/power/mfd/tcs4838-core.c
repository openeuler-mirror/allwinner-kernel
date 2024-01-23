/*
 * Based on the tcs4838 driver and the previous tcs4838 driver
 */

#include <linux/interrupt.h>
#include <linux/mfd/core.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/acpi.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <power/tcs4838.h>

static const struct mfd_cell tcs4838_cells[] = {
	{ .name = "tcs4838-regulator", },
};

static const struct regmap_range tcs4838_yes_ranges[] = {
	regmap_reg_range(TCS4838_VSEL0, TCS4838_TCS_PGOOD),
};

static const struct regmap_access_table tcs4838_volatile_table = {
	.yes_ranges = tcs4838_yes_ranges,
	.n_yes_ranges = ARRAY_SIZE(tcs4838_yes_ranges),
};

const struct regmap_config tcs4838_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.volatile_table = &tcs4838_volatile_table,
	.max_register   = TCS4838_TCS_PGOOD,
	.use_single_read = true,
	.use_single_write = true,
	.cache_type     = REGCACHE_NONE,
};
EXPORT_SYMBOL_GPL(tcs4838_regmap_config);


static void tcs4838_dts_parse(struct tcs4838 *tcs)
{
	struct device_node *node = tcs->dev->of_node;
	struct regmap *map = tcs->regmap;
	u32 val;

	/* init powerok reset function */
	if (of_property_read_u32(node, "tcs4838_delay", &val))
		val = 0;
	if (val) {
		val = val << 4;
		regmap_update_bits(map, TCS4838_CTRL, GENMASK(6, 4), val);
	}
}


int tcs4838_device_init(struct tcs4838 *tcs)
{
	int ret;
	tcs4838_dts_parse(tcs);
	ret = mfd_add_devices(tcs->dev, 0, tcs4838_cells,
			      ARRAY_SIZE(tcs4838_cells), NULL, 0, NULL);

	return 0;
}
EXPORT_SYMBOL_GPL(tcs4838_device_init);

int tcs4838_device_exit(struct tcs4838 *tcs)
{
	mfd_remove_devices(tcs->dev);
	return 0;
}
EXPORT_SYMBOL_GPL(tcs4838_device_exit);

MODULE_AUTHOR("Andrew F. Davis <afd@ti.com>");
MODULE_DESCRIPTION("TPS65912x MFD Driver");
MODULE_LICENSE("GPL v2");
