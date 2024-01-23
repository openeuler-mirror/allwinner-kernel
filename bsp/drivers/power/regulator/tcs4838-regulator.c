/*
 * Regulator driver for TI TCS4838x PMICs
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 *	Andrew F. Davis <afd@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether expressed or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * Based on the TPS65218 driver and the previous TCS4838 driver by
 * Margarita Olaya Cabrera <magi@slimlogic.co.uk>
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <power/tcs4838.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>

enum tcs4838_regulators { DCDC0, DCDC1 };

#define TCS4838_REGULATOR(_family, _id, _match, _supply, _ranges, _n_voltages,	\
			_vreg, _vmask, _ereg, _emask)				\
	[_family##_##_id] = {							\
		.name		= (_match),					\
		.supply_name	= (_supply),					\
		.of_match	= of_match_ptr(_match),				\
		.regulators_node = of_match_ptr("regulators"),			\
		.type		= REGULATOR_VOLTAGE,				\
		.id		= _family##_##_id,				\
		.n_voltages	= (_n_voltages),				\
		.owner		= THIS_MODULE,					\
		.vsel_reg	= (_vreg),					\
		.vsel_mask	= (_vmask),					\
		.enable_reg	= (_ereg),					\
		.enable_mask	= (_emask),					\
		.linear_ranges	= (_ranges),					\
		.n_linear_ranges = ARRAY_SIZE(_ranges),				\
		.ops		= &tcs4838_ops_dcdc,			\
	}

static const struct linear_range tcs4838_dcdc_ranges[] = {
	REGULATOR_LINEAR_RANGE(712500, 0x0, 0x3F, 12500),
};

/* Operations permitted on DCDCx */
static struct regulator_ops tcs4838_ops_dcdc = {
	.is_enabled		= regulator_is_enabled_regmap,
	.enable			= regulator_enable_regmap,
	.disable		= regulator_disable_regmap,
	.get_voltage_sel	= regulator_get_voltage_sel_regmap,
	.set_voltage_sel	= regulator_set_voltage_sel_regmap,
	.list_voltage		= regulator_list_voltage_linear_range,
};

static const struct regulator_desc tcs4838_regulators[] = {
	TCS4838_REGULATOR(TCS4838, DCDC0, "dcdc0", "vin1", tcs4838_dcdc_ranges,
			0x40, TCS4838_VSEL0, GENMASK(5, 0), TCS4838_VSEL0, BIT(7)),
	TCS4838_REGULATOR(TCS4838, DCDC1, "dcdc1", "vin1", tcs4838_dcdc_ranges,
			0x40, TCS4838_VSEL1, GENMASK(5, 0), TCS4838_VSEL1, BIT(7)),
};

static int tcs4838_regulator_probe(struct platform_device *pdev)
{
	struct regulator_dev *rdev;
	struct tcs4838 *tcs = dev_get_drvdata(pdev->dev.parent);
	const struct regulator_desc *regulators;
	struct regulator_config config = {
		.dev = pdev->dev.parent,
		.regmap = tcs->regmap,
		.driver_data = tcs,
	};
	int i, nregulators;

	regulators = tcs4838_regulators;
	nregulators = TCS4838_REG_ID_MAX;

	for (i = 0; i < nregulators; i++) {
		const struct regulator_desc *desc = &regulators[i];
		rdev = devm_regulator_register(&pdev->dev, desc, &config);
		if (IS_ERR(rdev)) {
			dev_err(&pdev->dev, "Failed to register %s\n",
				regulators[i].name);

			return PTR_ERR(rdev);
		}
	}

	return 0;
}

static int tcs4838_regulator_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver tcs4838_regulator_driver = {
	.driver = {
		.name = "tcs4838-regulator",
	},
	.probe = tcs4838_regulator_probe,
	.remove	= tcs4838_regulator_remove,
};

static int __init tcs4838_regulator_init(void)
{
	return platform_driver_register(&tcs4838_regulator_driver);
}

static void __exit tcs4838_regulator_exit(void)
{
	platform_driver_unregister(&tcs4838_regulator_driver);
}

subsys_initcall(tcs4838_regulator_init);
module_exit(tcs4838_regulator_exit);

MODULE_AUTHOR("Andrew F. Davis <afd@ti.com>");
MODULE_DESCRIPTION("TCS4838 voltage regulator driver");
MODULE_LICENSE("GPL v2");
