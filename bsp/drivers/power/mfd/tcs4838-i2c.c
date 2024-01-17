/*
 * Based on the TPS65218 driver and the previous TPS65912 driver
 */

#include <linux/acpi.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <power/tcs4838.h>
#include <linux/of.h>
#include <linux/regmap.h>
#include <linux/slab.h>


static const struct of_device_id tcs4838_i2c_of_match_table[] = {
	{ .compatible = "ti,tcs4838", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, tcs4838_i2c_of_match_table);

static int tcs4838_i2c_probe(struct i2c_client *client,
			      const struct i2c_device_id *ids)
{
	struct tcs4838 *tps;
	tps = devm_kzalloc(&client->dev, sizeof(*tps), GFP_KERNEL);
	if (!tps)
		return -ENOMEM;

	i2c_set_clientdata(client, tps);
	tps->dev = &client->dev;

	tps->regmap = devm_regmap_init_i2c(client, &tcs4838_regmap_config);
	if (IS_ERR(tps->regmap)) {
		dev_err(tps->dev, "Failed to initialize register map\n");
		return PTR_ERR(tps->regmap);
	}

	return tcs4838_device_init(tps);
}

static int tcs4838_i2c_remove(struct i2c_client *client)
{
	struct tcs4838 *tps = i2c_get_clientdata(client);

	return tcs4838_device_exit(tps);
}

static const struct i2c_device_id tcs4838_i2c_id_table[] = {
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(i2c, tcs4838_i2c_id_table);

static const struct acpi_device_id tcs4838_i2c_acpi_match[] = {
	{ },
};
MODULE_DEVICE_TABLE(acpi, tcs4838_i2c_acpi_match);

static struct i2c_driver tcs4838_i2c_driver = {
	.driver		= {
		.name	= "tcs4838-i2c",
		.of_match_table = of_match_ptr(tcs4838_i2c_of_match_table),
		.acpi_match_table = ACPI_PTR(tcs4838_i2c_acpi_match),
	},
	.probe		= tcs4838_i2c_probe,
	.remove		= tcs4838_i2c_remove,
	.id_table       = tcs4838_i2c_id_table,
};

static int __init tcs4838_i2c_init(void)
{
	int ret;

	ret = i2c_add_driver(&tcs4838_i2c_driver);
	if (ret != 0) {
		pr_err("tcs4838 i2c registration failed %d\n", ret);
		return ret;
	}

	return 0;
}
subsys_initcall(tcs4838_i2c_init);

static void __exit tcs4838_i2c_exit(void)
{
	i2c_del_driver(&tcs4838_i2c_driver);
}
module_exit(tcs4838_i2c_exit);

MODULE_AUTHOR("Andrew F. Davis <afd@ti.com>");
MODULE_DESCRIPTION("TPS65912x I2C Interface Driver");
MODULE_LICENSE("GPL v2");
