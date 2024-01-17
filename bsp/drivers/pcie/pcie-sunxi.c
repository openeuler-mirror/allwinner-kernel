// SPDX_License-Identifier: GPL-2.0
/*
 * allwinner PCIe host controller driver
 *
 * Copyright (c) 2007-2022 Allwinnertech Co., Ltd.
 *
 * Author: songjundong <songjundong@allwinnertech.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/msi.h>
#include <linux/of_address.h>
#include <linux/of_pci.h>
#include <linux/pci.h>
#include <linux/pci_regs.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#include "pcie-sunxi.h"
#include "pcie-sunxi-dma.h"

static inline struct pcie_port *sys_to_pcie(struct pci_sys_data *sys)
{
	return sys->private_data;
}

static int sunxi_pcie_cfg_read(void __iomem *addr, int size, u32 *val)
{
	if ((uintptr_t)addr & (size - 1)) {
		*val = 0;
		return PCIBIOS_BAD_REGISTER_NUMBER;
	}

	if (size == 4) {
		*val = readl(addr);
	} else if (size == 2) {
		*val = readw(addr);
	} else if (size == 1) {
		*val = readb(addr);
	} else {
		*val = 0;
		return PCIBIOS_BAD_REGISTER_NUMBER;
	}

	return PCIBIOS_SUCCESSFUL;
}

static int sunxi_pcie_cfg_write(void __iomem *addr, int size, u32 val)
{
	if ((uintptr_t)addr & (size - 1))
		return PCIBIOS_BAD_REGISTER_NUMBER;

	if (size == 4)
		writel(val, addr);
	else if (size == 2)
		writew(val, addr);
	else if (size == 1)
		writeb(val, addr);
	else
		return PCIBIOS_BAD_REGISTER_NUMBER;

	return PCIBIOS_SUCCESSFUL;
}

static inline void sunxi_pcie_readl_rc(struct pcie_port *pp, u32 reg, u32 *val)
{
	if (pp->ops->readl_rc)
		pp->ops->readl_rc(pp, pp->dbi_base + reg, val);
	else
		*val = readl(pp->dbi_base + reg);
}

static inline void sunxi_pcie_writel_rc(struct pcie_port *pp, u32 val, u32 reg)
{
	if (pp->ops->writel_rc)
		pp->ops->writel_rc(pp, val, pp->dbi_base + reg);
	else
		writel(val, pp->dbi_base + reg);
}

static int sunxi_pcie_rd_own_conf(struct pcie_port *pp, int where, int size,
			       u32 *val)
{
	int ret;

	if (pp->ops->rd_own_conf)
		ret = pp->ops->rd_own_conf(pp, where, size, val);
	else
		ret = sunxi_pcie_cfg_read(pp->dbi_base + where, size, val);

	return ret;
}

static int sunxi_pcie_wr_own_conf(struct pcie_port *pp, int where, int size,
			       u32 val)
{
	int ret;

	if (pp->ops->wr_own_conf)
		ret = pp->ops->wr_own_conf(pp, where, size, val);
	else
		ret = sunxi_pcie_cfg_write(pp->dbi_base + where, size, val);

	return ret;
}

irqreturn_t sunxi_handle_msi_irq(struct pcie_port *pp)
{
	unsigned long val;
	int i, pos;
	u32 status;
	irqreturn_t ret = IRQ_NONE;

	for (i = 0; i < MAX_MSI_CTRLS; i++) {
		sunxi_pcie_rd_own_conf(pp, PCIE_MSI_INTR_STATUS + (i * 12), 4, &status);

		if (!status)
			continue;

		ret = IRQ_HANDLED;
		pos = 0;
		val = status;
		while ((pos = find_next_bit(&val, 32, pos)) != 32) {

			generic_handle_domain_irq(pp->irq_domain, i * 32 + pos);

			sunxi_pcie_wr_own_conf(pp,
					PCIE_MSI_INTR_STATUS + (i * 12),
					4, 1 << pos);
			pos++;
		}
	}

	return ret;
}

static void sunxi_msi_top_irq_ack(struct irq_data *d)
{
	/* NULL */
}

static struct irq_chip sunxi_msi_top_chip = {
	.name		= "Sunxi-PCIe-MSI",
	.irq_ack	= sunxi_msi_top_irq_ack,
};

static int sunxi_msi_set_affinity(struct irq_data *d, const struct cpumask *mask, bool force)
{
	return -EINVAL;
}

static void sunxi_compose_msi_msg(struct irq_data *data, struct msi_msg *msg)
{
	struct pcie_port *pcie = irq_data_get_irq_chip_data(data);
	phys_addr_t pa = ALIGN_DOWN(virt_to_phys(pcie), SZ_4K);

	msg->address_lo = lower_32_bits(pa);
	msg->address_hi = upper_32_bits(pa);
	msg->data = data->hwirq;
}

/*
 * In the future, test whether the following interface needs to be added on A523 or T736:
 * .irq_ack, .irq_mask, .irq_unmask and the xxx_bottom_irq_chip.
 */
static struct irq_chip sunxi_msi_bottom_chip = {
	.name			= "Sunxi MSI",
	.irq_set_affinity 	= sunxi_msi_set_affinity,
	.irq_compose_msi_msg	= sunxi_compose_msi_msg,
};

static int sunxi_msi_domain_alloc(struct irq_domain *domain, unsigned int virq,
				  unsigned int nr_irqs, void *args)
{
	struct pcie_port *pp = domain->host_data;
	int hwirq, i;
	unsigned long flags;

	raw_spin_lock_irqsave(&pp->lock, flags);

	hwirq = bitmap_find_free_region(pp->msi_map, INT_PCI_MSI_NR, order_base_2(nr_irqs));

	raw_spin_unlock_irqrestore(&pp->lock, flags);

	if (unlikely(hwirq < 0)) {
		dev_err(pp->dev, "failed to alloc hwirq\n");
		return -ENOSPC;
	}

	for (i = 0; i < nr_irqs; i++)
		irq_domain_set_info(domain, virq + i, hwirq + i,
				    &sunxi_msi_bottom_chip, pp,
				    handle_edge_irq, NULL, NULL);

	return 0;
}

static void sunxi_msi_domain_free(struct irq_domain *domain, unsigned int virq,
				  unsigned int nr_irqs)
{
	struct irq_data *d = irq_domain_get_irq_data(domain, virq);
	struct pcie_port *pp = domain->host_data;
	unsigned long flags;

	raw_spin_lock_irqsave(&pp->lock, flags);

	bitmap_release_region(pp->msi_map, d->hwirq, order_base_2(nr_irqs));

	raw_spin_unlock_irqrestore(&pp->lock, flags);
}

static const struct irq_domain_ops sunxi_msi_domain_ops = {
	.alloc	= sunxi_msi_domain_alloc,
	.free	= sunxi_msi_domain_free,
};

static struct msi_domain_info sunxi_msi_info = {
	.flags	= (MSI_FLAG_USE_DEF_DOM_OPS | MSI_FLAG_USE_DEF_CHIP_OPS),
	.chip	= &sunxi_msi_top_chip,
};

static int sunxi_allocate_msi_domains(struct pcie_port *pcie)
{
	struct fwnode_handle *fwnode = dev_fwnode(pcie->dev);

	pcie->irq_domain = irq_domain_create_linear(fwnode, INT_PCI_MSI_NR,
							  &sunxi_msi_domain_ops, pcie);
	if (!pcie->irq_domain) {
		dev_err(pcie->dev, "failed to create IRQ domain\n");
		return -ENOMEM;
	}
	irq_domain_update_bus_token(pcie->irq_domain, DOMAIN_BUS_NEXUS);

	pcie->msi_domain = pci_msi_create_irq_domain(fwnode, &sunxi_msi_info, pcie->irq_domain);
	if (!pcie->msi_domain) {
		dev_err(pcie->dev, "failed to create MSI domain\n");
		irq_domain_remove(pcie->irq_domain);
		return -ENOMEM;
	}

	return 0;
}

static void sunxi_free_msi_domains(struct pcie_port *pcie)
{
	irq_domain_remove(pcie->msi_domain);
	irq_domain_remove(pcie->irq_domain);
}

int sunxi_pcie_link_up(struct pcie_port *pp)
{
	if (pp->ops->link_up)
		return pp->ops->link_up(pp);
	else
		return 0;
}

void sunxi_pcie_prog_outbound_atu(struct pcie_port *pp, int index, int type,
					u64 cpu_addr, u64 pci_addr, u32 size)
{
	sunxi_pcie_writel_rc(pp, lower_32_bits(cpu_addr), PCIE_ATU_LOWER_BASE_OUTBOUND(index));
	sunxi_pcie_writel_rc(pp, upper_32_bits(cpu_addr), PCIE_ATU_UPPER_BASE_OUTBOUND(index));
	sunxi_pcie_writel_rc(pp, lower_32_bits(cpu_addr + size - 1), PCIE_ATU_LIMIT_OUTBOUND(index));
	sunxi_pcie_writel_rc(pp, lower_32_bits(pci_addr), PCIE_ATU_LOWER_TARGET_OUTBOUND(index));
	sunxi_pcie_writel_rc(pp, upper_32_bits(pci_addr), PCIE_ATU_UPPER_TARGET_OUTBOUND(index));
	sunxi_pcie_writel_rc(pp, type, PCIE_ATU_CR1_OUTBOUND(index));
	sunxi_pcie_writel_rc(pp, PCIE_ATU_ENABLE, PCIE_ATU_CR2_OUTBOUND(index));
}

static int sunxi_pcie_rd_other_conf(struct pcie_port *pp, struct pci_bus *bus,
		u32 devfn, int where, int size, u32 *val)
{
	int ret = PCIBIOS_SUCCESSFUL, type;
	u32 busdev, cfg_size;
	u64 cpu_addr;
	void __iomem *va_cfg_base;

	busdev = PCIE_ATU_BUS(bus->number) | PCIE_ATU_DEV(PCI_SLOT(devfn)) |
		 PCIE_ATU_FUNC(PCI_FUNC(devfn));

	/* cfg1 maybe need't, please verify it */
	if (bus->parent->number == pp->root_bus_nr) {
		type = PCIE_ATU_TYPE_CFG0;
		cpu_addr = pp->cfg0_base - PCIE_CPU_BASE;
		cfg_size = pp->cfg0_size;
		va_cfg_base = pp->va_cfg0_base;
	} else {
		type = PCIE_ATU_TYPE_CFG1;
		cpu_addr = pp->cfg1_base - PCIE_CPU_BASE;
		cfg_size = pp->cfg1_size;
		va_cfg_base = pp->va_cfg1_base;
	}

	sunxi_pcie_prog_outbound_atu(pp, PCIE_ATU_INDEX0, type, cpu_addr, busdev, cfg_size);

	ret = sunxi_pcie_cfg_read(va_cfg_base + where, size, val);

	return ret;
}

static int sunxi_pcie_wr_other_conf(struct pcie_port *pp, struct pci_bus *bus,
		u32 devfn, int where, int size, u32 val)
{
	int ret = PCIBIOS_SUCCESSFUL, type;
	u32 busdev, cfg_size;
	u64 cpu_addr;
	void __iomem *va_cfg_base;

	busdev = PCIE_ATU_BUS(bus->number) | PCIE_ATU_DEV(PCI_SLOT(devfn)) |
		 PCIE_ATU_FUNC(PCI_FUNC(devfn));

	if (bus->parent->number == pp->root_bus_nr) {
		type = PCIE_ATU_TYPE_CFG0;
		cpu_addr = pp->cfg0_base - PCIE_CPU_BASE;
		cfg_size = pp->cfg0_size;
		va_cfg_base = pp->va_cfg0_base;

	} else {
		type = PCIE_ATU_TYPE_CFG1;
		cpu_addr = pp->cfg1_base - PCIE_CPU_BASE;
		cfg_size = pp->cfg1_size;
		va_cfg_base = pp->va_cfg1_base;
	}

	sunxi_pcie_prog_outbound_atu(pp, PCIE_ATU_INDEX0, type, cpu_addr, busdev, cfg_size);

	ret = sunxi_pcie_cfg_write(va_cfg_base + where, size, val);

	return ret;
}

static int sunxi_pcie_valid_config(struct pcie_port *pp,
				struct pci_bus *bus, int dev)
{
	/* If there is no link, then there is no device */
	if (!pci_is_root_bus(bus)) {
		if (!sunxi_pcie_link_up(pp))
			return 0;
	} else if (dev > 0)
		/* Access only one slot on each root port */
		return 0;

	return 1;
}

static int sunxi_pcie_rd_conf(struct pci_bus *bus, u32 devfn, int where,
			int size, u32 *val)
{
	struct pcie_port *pp = (bus->sysdata);
	int ret;

	if (!pp) {
		BUG();
		return -EINVAL;
	}

	if (!sunxi_pcie_valid_config(pp, bus, PCI_SLOT(devfn))) {
		*val = 0xffffffff;
		return PCIBIOS_DEVICE_NOT_FOUND;
	}


	if (!pci_is_root_bus(bus))
		ret = sunxi_pcie_rd_other_conf(pp, bus, devfn,
						where, size, val);
	else
		ret = sunxi_pcie_rd_own_conf(pp, where, size, val);

	return ret;
}

static int sunxi_pcie_wr_conf(struct pci_bus *bus, u32 devfn,
			int where, int size, u32 val)
{
	struct pcie_port *pp = (bus->sysdata);
	int ret;

	if (!pp) {
		BUG();
		return -EINVAL;
	}
	if (sunxi_pcie_valid_config(pp, bus, PCI_SLOT(devfn)) == 0)
		return PCIBIOS_DEVICE_NOT_FOUND;

	if (!pci_is_root_bus(bus))
		ret = sunxi_pcie_wr_other_conf(pp, bus, devfn,
						where, size, val);
	else
		ret = sunxi_pcie_wr_own_conf(pp, where, size, val);

	return ret;
}

static struct pci_ops sunxi_pcie_ops = {
	.read = sunxi_pcie_rd_conf,
	.write = sunxi_pcie_wr_conf,
};

int sunxi_pcie_host_init(struct pcie_port *pp)
{
	struct device_node *np = pp->dev->of_node;
	struct device *dev = pp->dev;
	struct resource_entry *win;
	struct pci_host_bridge *bridge;
	int ret, i;

	bridge = devm_pci_alloc_host_bridge(dev, 0);
	if (!bridge)
		return -ENOMEM;

	pp->bridge = bridge;

	/* Get the I/O and memory ranges from DT */
	resource_list_for_each_entry(win, &bridge->windows) {
		switch (resource_type(win->res)) {
		case IORESOURCE_IO:
			pp->io_size = resource_size(win->res);
			pp->io_bus_addr = win->res->start - win->offset;
			pp->io_base = pci_pio_to_address(win->res->start);
			break;
		case 0:
			pp->cfg0_size = resource_size(win->res);
			pp->cfg0_base = win->res->start;
			if (!pp->dbi_base) {
				pp->dbi_base = devm_pci_remap_cfgspace(dev,
								pp->cfg0_base,
								pp->cfg0_size);
				if (!pp->dbi_base) {
					dev_err(dev, "Error with ioremap\n");
					return -ENOMEM;
				}
			}
			break;
		}
	}

	if (!pp->va_cfg0_base) {
		pp->va_cfg0_base = devm_pci_remap_cfgspace(dev,
					pp->cfg0_base, pp->cfg0_size);
		if (!pp->va_cfg0_base) {
			dev_err(dev, "Error with ioremap in function\n");
			return -ENOMEM;
		}
	}

	ret = of_property_read_u32(np, "num-viewport", &pp->num_viewport);
	if (ret) {
		dev_err(pp->dev, "Failed to parse the number of num_viewport\n");
		return -EINVAL;
	}

	if (of_property_read_u32(np, "num-lanes", &pp->lanes)) {
		dev_err(pp->dev, "Failed to parse the number of lanes\n");
		return -EINVAL;
	}

	if (IS_ENABLED(CONFIG_PCI_MSI) && !pp->msi_ext) {

		phys_addr_t pa = ALIGN_DOWN(virt_to_phys(pp), SZ_4K);

		ret = sunxi_allocate_msi_domains(pp);
		if (ret)
			return ret;

		sunxi_pcie_wr_own_conf(pp, PCIE_MSI_ADDR_LO, 4,	lower_32_bits(pa));
		sunxi_pcie_wr_own_conf(pp, PCIE_MSI_ADDR_HI, 4, upper_32_bits(pa));

		for (i = 0; i < 8; i++) {
			sunxi_pcie_wr_own_conf(pp, PCIE_MSI_INTR_ENABLE(i), 4, ~0);
		}
	}

	if (pp->ops->host_init)
		pp->ops->host_init(pp);

	bridge->sysdata = pp;
	bridge->ops = &sunxi_pcie_ops;

	ret = pci_host_probe(bridge);

	if (ret) {
		if (IS_ENABLED(CONFIG_PCI_MSI) && !pp->msi_ext)
			sunxi_free_msi_domains(pp);
		dev_err(pp->dev, "Failed to probe host bridge\n");

		return ret;
	}

	return 0;
}

void sunxi_pcie_setup_rc(struct pcie_port *pp)
{
	u32 val;
	int atu_idx = 0;
	struct resource_entry *entry;

	/* set the number of lanes */
	sunxi_pcie_readl_rc(pp, PCIE_PORT_LINK_CONTROL, &val);
	val &= ~PORT_LINK_MODE_MASK;
	switch (pp->lanes) {
	case 1:
		val |= PORT_LINK_MODE_1_LANES;
		break;
	case 2:
		val |= PORT_LINK_MODE_2_LANES;
		break;
	case 4:
		val |= PORT_LINK_MODE_4_LANES;
		break;
	default:
		dev_err(pp->dev, "num-lanes %u: invalid value\n", pp->lanes);
		return;
	}
	sunxi_pcie_writel_rc(pp, val, PCIE_PORT_LINK_CONTROL);

	/* set link width speed control register */
	sunxi_pcie_readl_rc(pp, PCIE_LINK_WIDTH_SPEED_CONTROL, &val);
	val &= ~PORT_LOGIC_LINK_WIDTH_MASK;
	switch (pp->lanes) {
	case 1:
		val |= PORT_LOGIC_LINK_WIDTH_1_LANES;
		break;
	case 2:
		val |= PORT_LOGIC_LINK_WIDTH_2_LANES;
		break;
	case 4:
		val |= PORT_LOGIC_LINK_WIDTH_4_LANES;
		break;
	}
	sunxi_pcie_writel_rc(pp, val, PCIE_LINK_WIDTH_SPEED_CONTROL);

	/* set mode gen1 for FPGA */
	sunxi_pcie_readl_rc(pp, 0xA0, &val);
	val &= ~(0xf<<0);
	val |= (0x1<<0);
	sunxi_pcie_writel_rc(pp, val, 0xA0);

	/* setup RC BARs */
	sunxi_pcie_writel_rc(pp, 0x00000004, PCI_BASE_ADDRESS_0);
	sunxi_pcie_writel_rc(pp, 0x00000000, PCI_BASE_ADDRESS_1);

	/* setup interrupt pins */
	sunxi_pcie_readl_rc(pp, PCI_INTERRUPT_LINE, &val);
	val &= PCIE_INTERRUPT_LINE_MASK;
	val |= PCIE_INTERRUPT_LINE_ENABLE;
	sunxi_pcie_writel_rc(pp, val, PCI_INTERRUPT_LINE);

	/* setup bus numbers */
	sunxi_pcie_readl_rc(pp, PCI_PRIMARY_BUS, &val);
	val &= 0xff000000;
	val |= 0x00ff0100;
	sunxi_pcie_writel_rc(pp, val, PCI_PRIMARY_BUS);

	/* setup command register */
	sunxi_pcie_readl_rc(pp, PCI_COMMAND, &val);

	val &= PCIE_HIGH16_MASK;
	val |= PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
		PCI_COMMAND_MASTER | PCI_COMMAND_SERR;

	sunxi_pcie_writel_rc(pp, val, PCI_COMMAND);

	resource_list_for_each_entry(entry, &pp->bridge->windows) {
		if (resource_type(entry->res) != IORESOURCE_MEM)
			continue;

		if (pp->num_viewport <= ++atu_idx)
			break;

		sunxi_pcie_prog_outbound_atu(pp, atu_idx, PCIE_ATU_TYPE_MEM, entry->res->start - PCIE_CPU_BASE,
						  entry->res->start - entry->offset,
						  resource_size(entry->res));
	}

	if (pp->io_size) {
		if (pp->num_viewport > ++atu_idx)
			sunxi_pcie_prog_outbound_atu(pp, atu_idx, PCIE_ATU_TYPE_IO, pp->io_base - PCIE_CPU_BASE,
							pp->io_bus_addr, pp->io_size);
		else
			dev_err(pp->dev, "Resources exceed number of ATU entries (%d)",
							pp->num_viewport);
	}

	sunxi_pcie_wr_own_conf(pp, PCI_BASE_ADDRESS_0, 4, 0);

	sunxi_pcie_readl_rc(pp, PCIE_MISC_CONTROL_1_CFG, &val);
	val |= 0x1;
	sunxi_pcie_writel_rc(pp, val, PCIE_MISC_CONTROL_1_CFG);


	sunxi_pcie_wr_own_conf(pp, PCI_CLASS_DEVICE, 2, PCI_CLASS_BRIDGE_PCI);

	sunxi_pcie_readl_rc(pp, PCIE_MISC_CONTROL_1_CFG, &val);
	val &= ~(0x1<<0);
	sunxi_pcie_writel_rc(pp, val, PCIE_MISC_CONTROL_1_CFG);

	sunxi_pcie_rd_own_conf(pp, PCIE_LINK_WIDTH_SPEED_CONTROL, 4, &val);
	val |= PORT_LOGIC_SPEED_CHANGE;
	sunxi_pcie_wr_own_conf(pp, PCIE_LINK_WIDTH_SPEED_CONTROL, 4, val);
}

MODULE_AUTHOR("songjundong <songjundong@allwinnertech.com>");
MODULE_DESCRIPTION("sunxi PCIe host controller driver");
MODULE_VERSION("1.0.0");
MODULE_LICENSE("GPL");
