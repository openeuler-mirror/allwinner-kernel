/*
 * Based on the tcs4838 driver and the previous tcs4838 driver
 */

#ifndef __LINUX_MFD_TCS4838_H
#define __LINUX_MFD_TCS4838_H

#include <linux/regmap.h>

/* List of registers for tcs4838 */
#define TCS4838_VSEL0		0x00
#define TCS4838_VSEL1		0x01
#define TCS4838_CTRL		0x02
#define TCS4838_ID1		    0x03
#define TCS4838_ID2		    0x04
#define TCS4838_TCS_PGOOD	0x05

/*
 * struct tcs4838 - state holder for the tcs4838 driver
 *
 * Device data may be used to access the tcs4838 chip
 */
struct tcs4838 {
	struct device *dev;
	struct regmap *regmap;
};

enum {
	TCS4838_DCDC0,
	TCS4838_DCDC1,
	TCS4838_REG_ID_MAX,
};

extern const struct regmap_config tcs4838_regmap_config;

int tcs4838_device_init(struct tcs4838 *tps);
int tcs4838_device_exit(struct tcs4838 *tps);

#endif /*  __LINUX_MFD_TCS4838_H */
