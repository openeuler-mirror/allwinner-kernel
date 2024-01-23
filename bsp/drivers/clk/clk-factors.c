#include <linux/types.h>
#include <asm/div64.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/io.h>

struct sunxi_clk_factor_freq{
    u32 factor;
    u32 freq;
};

struct clk_factors_value {
    u16 factorn;
    u16 factork;

    u16 factorm;
    u16 factorp;

    u16 factord1;
    u16 factord2;

    u16 frac_mode;
    u16 frac_freq;
};

struct sunxi_clk_factors_config {
    u8 nshift;
    u8 nwidth;
    u8 kshift;
    u8 kwidth;

    u8 mshift;
    u8 mwidth;
    u8 pshift;
    u8 pwidth;

    u8 d1shift;
    u8 d1width;
    u8 d2shift;
    u8 d2width;

    u8 frac;
    u8 outshift;
    u8 modeshift;
    u8 enshift;

    u8 lockshift;
    u8 sdmshift;
    u8 sdmwidth;

    unsigned long sdmpat;
    u32 sdmval;

    u32 updshift;
    u32 delay;
};

#define FACTOR_ALL(nv, ns, nw, kv, ks, kw, mv, ms, mw, \
           pv, ps, pw, d0v, d0s, d0w, d1v, d1s, d1w) \
          ((((nv & ((1 << nw) - 1)) << ns) | \
            ((kv & ((1 << kw) - 1)) << ks) | \
            ((mv & ((1 << mw) - 1)) << ms) | \
            ((pv & ((1 << pw) - 1)) << ps) | \
            ((d0v & ((1 << d0w) - 1)) << d0s) | \
            ((d1v & ((1 << d1w) - 1)) << d1s)))

#define F_N8X7_M0X4(nv,mv) FACTOR_ALL(nv,8,7,0,0,0,mv,0,4,0,0,0,0,0,0,0,0,0)

#define PLLVIDEO0(n,m,freq)     {F_N8X7_M0X4( n, m),  freq}

#define SUNXI_CLK_FACTORS(name, _nshift, _nwidth, _kshift, _kwidth, \
        _mshift, _mwidth,  _pshift, _pwidth, _d1shift, _d1width, \
        _d2shift, _d2width, _frac, _outshift, _modeshift,   \
        _enshift, _sdmshift, _sdmwidth, _sdmpat, _sdmval)     \
    static struct sunxi_clk_factors_config sunxi_clk_factor_##name = { \
        .nshift = _nshift,  \
        .nwidth = _nwidth,  \
        .kshift = _kshift,  \
        .kwidth = _kwidth,  \
        .mshift = _mshift,  \
        .mwidth = _mwidth,  \
        .pshift = _pshift,  \
        .pwidth = _pwidth,  \
        .d1shift = _d1shift,    \
        .d1width = _d1width,    \
        .d2shift = _d2shift,    \
        .d2width = _d2width,    \
        .frac = _frac,  \
        .outshift = _outshift,  \
        .modeshift =_modeshift,     \
        .enshift =_enshift,    \
        .sdmshift=_sdmshift,    \
        .sdmwidth=_sdmwidth,    \
        .sdmpat  =_sdmpat,    \
        .sdmval  =_sdmval,    \
        .updshift = 0\
    }

#define PLL_VIDEO0PAT       0x0288

SUNXI_CLK_FACTORS(pll_video0, 8, 7, 0, 0, 0, 4, 0, 0,
            0, 0, 0, 0, 1, 25, 24, 31, 20,
            0, PLL_VIDEO0PAT, 0xd1303333);

struct sunxi_clk_factor_freq factor_pllvideo0_tbl[] = {
PLLVIDEO0(6,    0,  168000000U),
PLLVIDEO0(14,   1,  180000000U),
PLLVIDEO0(22,   2,  184000000U),
PLLVIDEO0(31,   3,  192000000U),
PLLVIDEO0(24,   2,  200000000U),
PLLVIDEO0(16,   1,  204000000U),
PLLVIDEO0(25,   2,  208000000U),
PLLVIDEO0(8,    0,  216000000U),
PLLVIDEO0(36,   3,  222000000U),
PLLVIDEO0(27,   2,  224000000U),
PLLVIDEO0(74,   7,  225000000U),
PLLVIDEO0(18,   1,  228000000U),
PLLVIDEO0(28,   2,  232000000U),
PLLVIDEO0(38,   3,  234000000U),
PLLVIDEO0(39,   3,  240000000U),
PLLVIDEO0(80,   7,  243000000U),
PLLVIDEO0(40,   3,  246000000U),
PLLVIDEO0(30,   2,  248000000U),
PLLVIDEO0(20,   1,  252000000U),
PLLVIDEO0(84,   7,  255000000U),
PLLVIDEO0(31,   2,  256000000U),
PLLVIDEO0(42,   3,  258000000U),
PLLVIDEO0(86,   7,  261000000U),
PLLVIDEO0(10,   0,  264000000U),
PLLVIDEO0(88,   7,  267000000U),
PLLVIDEO0(44,   3,  270000000U),
PLLVIDEO0(90,   7,  273000000U),
PLLVIDEO0(45,   3,  276000000U),
PLLVIDEO0(92,   7,  279000000U),
PLLVIDEO0(34,   2,  280000000U),
PLLVIDEO0(46,   3,  282000000U),
PLLVIDEO0(94,   7,  285000000U),
PLLVIDEO0(11,   0,  288000000U),
PLLVIDEO0(96,   7,  291000000U),
PLLVIDEO0(48,   3,  294000000U),
PLLVIDEO0(36,   2,  296000000U),
PLLVIDEO0(98,   7,  297000000U),
PLLVIDEO0(24,   1,  300000000U),
PLLVIDEO0(100,  7,  303000000U),
PLLVIDEO0(37,   2,  304000000U),
PLLVIDEO0(101,  7,  306000000U),
PLLVIDEO0(102,  7,  309000000U),
PLLVIDEO0(12,   0,  312000000U),
PLLVIDEO0(104,  7,  315000000U),
PLLVIDEO0(52,   3,  318000000U),
PLLVIDEO0(39,   2,  320000000U),
PLLVIDEO0(106,  7,  321000000U),
PLLVIDEO0(26,   1,  324000000U),
PLLVIDEO0(108,  7,  327000000U),
PLLVIDEO0(40,   2,  328000000U),
PLLVIDEO0(109,  7,  330000000U),
PLLVIDEO0(110,  7,  333000000U),
PLLVIDEO0(27,   1,  336000000U),
PLLVIDEO0(112,  7,  339000000U),
PLLVIDEO0(56,   3,  342000000U),
PLLVIDEO0(114,  7,  345000000U),
PLLVIDEO0(28,   1,  348000000U),
PLLVIDEO0(116,  7,  351000000U),
PLLVIDEO0(58,   3,  354000000U),
PLLVIDEO0(118,  7,  357000000U),
PLLVIDEO0(29,   1,  360000000U),
PLLVIDEO0(120,  7,  363000000U),
PLLVIDEO0(121,  7,  366000000U),
PLLVIDEO0(122,  7,  369000000U),
PLLVIDEO0(61,   3,  372000000U),
PLLVIDEO0(124,  7,  375000000U),
PLLVIDEO0(125,  7,  378000000U),
PLLVIDEO0(126,  7,  381000000U),
PLLVIDEO0(15,   0,  384000000U),
PLLVIDEO0(64,   3,  390000000U),
PLLVIDEO0(32,   1,  396000000U),
PLLVIDEO0(66,   3,  402000000U),
PLLVIDEO0(16,   0,  408000000U),
PLLVIDEO0(68,   3,  414000000U),
PLLVIDEO0(69,   3,  420000000U),
PLLVIDEO0(70,   3,  426000000U),
PLLVIDEO0(71,   3,  432000000U),
PLLVIDEO0(72,   3,  438000000U),
PLLVIDEO0(36,   1,  444000000U),
PLLVIDEO0(74,   3,  450000000U),
PLLVIDEO0(18,   0,  456000000U),
PLLVIDEO0(76,   3,  462000000U),
PLLVIDEO0(38,   1,  468000000U),
PLLVIDEO0(78,   3,  474000000U),
PLLVIDEO0(79,   3,  480000000U),
PLLVIDEO0(80,   3,  486000000U),
PLLVIDEO0(81,   3,  492000000U),
PLLVIDEO0(82,   3,  498000000U),
PLLVIDEO0(20,   0,  504000000U),
PLLVIDEO0(84,   3,  510000000U),
PLLVIDEO0(85,   3,  516000000U),
PLLVIDEO0(86,   3,  522000000U),
PLLVIDEO0(21,   0,  528000000U),
PLLVIDEO0(88,   3,  534000000U),
PLLVIDEO0(89,   3,  540000000U),
PLLVIDEO0(90,   3,  546000000U),
PLLVIDEO0(91,   3,  552000000U),
PLLVIDEO0(92,   3,  558000000U),
PLLVIDEO0(93,   3,  564000000U),
PLLVIDEO0(94,   3,  570000000U),
PLLVIDEO0(95,   3,  576000000U),
PLLVIDEO0(96,   3,  582000000U),
PLLVIDEO0(97,   3,  588000000U),
PLLVIDEO0(98,   3,  594000000U),
PLLVIDEO0(99,   3,  600000000U),
PLLVIDEO0(100,  3,  606000000U),
PLLVIDEO0(101,  3,  612000000U),
PLLVIDEO0(102,  3,  618000000U),
PLLVIDEO0(103,  3,  624000000U),
PLLVIDEO0(104,  3,  630000000U),
PLLVIDEO0(105,  3,  636000000U),
PLLVIDEO0(106,  3,  642000000U),
PLLVIDEO0(107,  3,  648000000U),
PLLVIDEO0(108,  3,  654000000U),
PLLVIDEO0(109,  3,  660000000U),
PLLVIDEO0(110,  3,  666000000U),
PLLVIDEO0(27,   0,  672000000U),
PLLVIDEO0(112,  3,  678000000U),
PLLVIDEO0(113,  3,  684000000U),
PLLVIDEO0(114,  3,  690000000U),
PLLVIDEO0(28,   0,  696000000U),
PLLVIDEO0(116,  3,  702000000U),
PLLVIDEO0(117,  3,  708000000U),
PLLVIDEO0(118,  3,  714000000U),
PLLVIDEO0(119,  3,  720000000U),
PLLVIDEO0(120,  3,  726000000U),
PLLVIDEO0(121,  3,  732000000U),
PLLVIDEO0(122,  3,  738000000U),
PLLVIDEO0(123,  3,  744000000U),
PLLVIDEO0(124,  3,  750000000U),
PLLVIDEO0(125,  3,  756000000U),
PLLVIDEO0(126,  3,  762000000U),
PLLVIDEO0(127,  3,  768000000U),
PLLVIDEO0(32,   0,  792000000U),
PLLVIDEO0(33,   0,  816000000U),
PLLVIDEO0(34,   0,  840000000U),
PLLVIDEO0(35,   0,  864000000U),
PLLVIDEO0(36,   0,  888000000U),
PLLVIDEO0(37,   0,  912000000U),
PLLVIDEO0(38,   0,  936000000U),
PLLVIDEO0(39,   0,  960000000U),
PLLVIDEO0(40,   0,  984000000U),
PLLVIDEO0(41,   0,  1008000000U),
};

static unsigned int pllvideo0_max;
#define PLL_MAX_ASSIGN(name)    pll##name##_max=factor_pll##name##_tbl[ARRAY_SIZE(factor_pll##name##_tbl)-1].freq

static int sunxi_clk_freq_search(struct sunxi_clk_factor_freq tbl[],
                unsigned long freq, int low, int high)
{
    int mid;
    unsigned long checkfreq;
    if(low > high)
        return (high==-1)? 0: high;

    mid = (low + high)/2;
    checkfreq = tbl[mid].freq/1000000;
    if( checkfreq == freq)
        return mid;
    else if(checkfreq > freq)
        return sunxi_clk_freq_search(tbl, freq, low, mid -1);
    else
        return sunxi_clk_freq_search(tbl, freq, mid + 1,high);
}

static int sunxi_clk_freq_find(struct sunxi_clk_factor_freq tbl[],
                unsigned long n, unsigned long freq)
{
    int delta1, delta2;
    int i = sunxi_clk_freq_search(tbl, freq, 0, n-1);

    if (i != n-1) {

        delta1 = (freq > tbl[i].freq / 1000000)
            ? (freq - tbl[i].freq / 1000000)
            : (tbl[i].freq / 1000000 - freq);

        delta2 = (freq > tbl[i+1].freq / 1000000)
            ? (freq - tbl[i+1].freq / 1000000)
            : (tbl[i+1].freq / 1000000 - freq);

        if(delta2 < delta1)
            i++;
    }

    return i;
}

int sunxi_clk_com_ftr_sr(struct sunxi_clk_factors_config *f_config,
                struct clk_factors_value *factor,
                struct sunxi_clk_factor_freq table[],
                unsigned long index, unsigned long tbl_count)
{
    int i = sunxi_clk_freq_find(table, tbl_count, index);

    if(i >= tbl_count)
        return -1;

    factor->factorn = (table[i].factor >> f_config->nshift)&((1<<(f_config->nwidth))-1);
    factor->factork = (table[i].factor>>f_config->kshift)&((1<<(f_config->kwidth))-1);
    factor->factorm = (table[i].factor>>f_config->mshift)&((1<<(f_config->mwidth))-1);
    factor->factorp = (table[i].factor>>f_config->pshift)&((1<<(f_config->pwidth))-1);
    factor->factord1 = (table[i].factor>>f_config->d1shift)&((1<<(f_config->d1width))-1);
    factor->factord2 = (table[i].factor>>f_config->d2shift)&((1<<(f_config->d2width))-1);

    if (f_config->frac) {
        factor->frac_mode = (table[i].factor>>f_config->modeshift)&1;
        factor->frac_freq = (table[i].factor>>f_config->outshift)&1;
    }

    return 0;
}

static int get_factors_pll_video0(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{
    u64 tmp_rate;
    int index;

    if (!factor)
        return -1;

	PLL_MAX_ASSIGN(video0);
    tmp_rate = rate>pllvideo0_max ? pllvideo0_max : rate;
    do_div(tmp_rate, 1000000);
    index = tmp_rate;

    if (sunxi_clk_com_ftr_sr(&sunxi_clk_factor_pll_video0, factor,
                factor_pllvideo0_tbl, index,
                sizeof(factor_pllvideo0_tbl)
                / sizeof(struct sunxi_clk_factor_freq)))
        return -1;

    if (rate == 297000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 1;
        factor->factorm = 0;
    } else if (rate == 270000000) {
        factor->frac_mode = 0;
        factor->frac_freq = 0;
        factor->factorm = 0;
    } else {
        factor->frac_mode = 1;
        factor->frac_freq = 0;
    }

    return 0;
}

static int get_factors_pll_mipi(u32 rate, u32 parent_rate, struct clk_factors_value *factor)
{

    u64 tmp_rate;
    u32 delta1, delta2, want_rate, new_rate, save_rate = 0;
    int n, k, m;

    if(!factor)
        return -1;

    tmp_rate = (rate > 1440000000) ? 1440000000 : rate;
    do_div(tmp_rate, 1000000);
    want_rate = tmp_rate;

    for(m=1; m <=16; m++) {
        for(k=2; k <=4; k++) {
            for(n=1; n <=16; n++) {
                new_rate = (parent_rate / 1000000)*k*n/m;

                delta1 = (new_rate > want_rate)
                    ? (new_rate - want_rate)
                    : (want_rate - new_rate);

                delta2 = (save_rate > want_rate)
                    ? (save_rate - want_rate)
                    : (want_rate - save_rate);
                if(delta1 < delta2) {
                    factor->factorn = n-1;
                    factor->factork = k-1;
                    factor->factorm = m-1;
                    save_rate = new_rate;
                }
            }
        }
    }

    return 0;
}

static unsigned long calc_rate_media(u32 parent_rate, struct clk_factors_value *factor)
{
    u64 tmp_rate = (parent_rate ? parent_rate : 24000000);

    if (factor->frac_mode == 0) {
        if (factor->frac_freq == 1)
            return 297000000;
        else
            return 270000000;
    } else {
        tmp_rate = tmp_rate * (factor->factorn+1);
        do_div(tmp_rate, factor->factorm+1);
        return (unsigned long)tmp_rate;
    }
}

static void set_video_reg(u32 reg, struct clk_factors_value *factor)
{
	void __iomem *addr;
	u32 val;

	addr = ioremap(reg, 0x4);
	val = readl(addr);
	val &= ~(0xff<<8);
	val &= ~0xf;
	val |= factor->factorn << 8;
	val |= factor->factorm;
	writel(val, addr);
}

static void set_mipi_reg(u32 reg, struct clk_factors_value *factor)
{
	void __iomem *addr;
	u32 val;

	addr = ioremap(reg, 0x4);
	val = readl(addr);
	val &= ~(0xf<<8);
	val &= ~0x3f;
	val |= factor->factorn << 8;
	val |= factor->factork << 4;
	val |= factor->factorm;
	writel(val, addr);
}

void __f_set_mipi_pll(u32 rate, u32 prate)
{
	struct clk_factors_value *factor;
	u32 parent_rate;

	factor = (struct clk_factors_value *)kzalloc(GFP_KERNEL, sizeof(struct clk_factors_value));

	get_factors_pll_video0(prate, 24000000, factor);
	parent_rate = calc_rate_media(24000000, factor);
	set_video_reg(0x1c20010, factor);

	get_factors_pll_mipi(rate, parent_rate, factor);
	set_mipi_reg(0x1c20040, factor);
}
