// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/sama5d3_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <debug_uart.h>
#include <spl.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/at91_ddrsdrc.h>
#include <asm/arch/at91_wdt.h>

DECLARE_GLOBAL_DATA_PTR;

extern void at91_pda_detect(void);

#ifdef CONFIG_NAND_ATMEL
void scc_air_v2_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;

	at91_periph_clk_enable(ATMEL_ID_SMC);

	// /* Periph A, not B/C/D */
 	// at91_pio3_set_a_periph(AT91_PIO_PORTE, 21, 1);
 	// at91_pio3_set_a_periph(AT91_PIO_PORTE, 22, 1);
	// /* Periph, not PIO */
 	// at91_set_a_periph(AT91_PIO_PORTE, 21, 1);
 	// at91_set_a_periph(AT91_PIO_PORTE, 22, 1);
	// /* Disable pullup */
 	// at91_pio3_set_pio_pullup(AT91_PIO_PORTE, 21, 1);
	// at91_pio3_set_pio_pullup(AT91_PIO_PORTE, 22, 1);
	// /* Disable pulldown */
	// at91_pio3_set_pio_pulldown(AT91_PIO_PORTE, 21, 1);
	// at91_pio3_set_pio_pulldown(AT91_PIO_PORTE, 22, 1);

	 /* Configure timings for NAND on chip select NCS3 */
	writel(AT91_SMC_SETUP_NWE(2)
		| AT91_SMC_SETUP_NCS_WR(0)
		| AT91_SMC_SETUP_NRD(2)
		| AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(2)
		| AT91_SMC_PULSE_NCS_WR(5)
		| AT91_SMC_PULSE_NRD(4)
		| AT91_SMC_PULSE_NCS_RD(7),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(5)
		| AT91_SMC_CYCLE_NRD(7),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_TIMINGS_TCLR(3)
		| AT91_SMC_TIMINGS_TADL(10)
		| AT91_SMC_TIMINGS_TAR(3)
		| AT91_SMC_TIMINGS_TRR(4)
		| AT91_SMC_TIMINGS_TWB(5)
		| AT91_SMC_TIMINGS_RBNSEL(3)
		| AT91_SMC_TIMINGS_NFSEL(1),
			&smc->cs[3].timings);
	writel(AT91_SMC_MODE_RM_NRD
		| AT91_SMC_MODE_WM_NWE
		| AT91_SMC_MODE_EXNW_DISABLE
	    | AT91_SMC_MODE_DBW_8
	    | AT91_SMC_MODE_TDF_CYCLE(14),
	       &smc->cs[3].mode);
}
#endif

#ifdef CONFIG_CMD_USB
static void scc_air_v2_usb_hw_init(void)
{
	at91_set_pio_output(AT91_PIO_PORTE, 3, 0);
	at91_set_pio_output(AT91_PIO_PORTE, 4, 0);
}
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
static void scc_air_v2_mci0_hw_init(void)
{
	at91_set_pio_output(AT91_PIO_PORTE, 2, 0);	/* MCI0 Power */
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	at91_seriald_hw_init();
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	// at91_pda_detect();
	return 0;
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif
	return 0;
}
#endif

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_NAND_ATMEL
	scc_air_v2_nand_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	scc_air_v2_usb_hw_init();
#endif
#ifdef CONFIG_GENERIC_ATMEL_MCI
	scc_air_v2_mci0_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);

	return 0;
}

/* SPL */
#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
#ifdef CONFIG_SD_BOOT
#ifdef CONFIG_GENERIC_ATMEL_MCI
	scc_air_v2_mci0_hw_init();
#endif
#elif CONFIG_NAND_BOOT
	scc_air_v2_nand_hw_init();
#endif
}

static void ddr2_conf(struct atmel_mpddrc_config *ddr2)
{
	ddr2->md = (ATMEL_MPDDRC_MD_DBW_32_BITS | AT91C_DDRC2_MD_LP_DDR_SDRAM);

	ddr2->cr = (AT91C_DDRC2_NC_DDR10_SDR9 // 10 bits for DDR + 9-bit for low-power DDR1-SDRAM
				| AT91C_DDRC2_NR_14 // 14_ROWS_BITS (16384)
				| AT91C_DDRC2_CAS_3
				| AT91C_DDRC2_DISABLE_RESET_DLL
				| AT91C_DDRC2_NORMAL_STRENGTH_RZQ6
				| AT91C_DDRC2_ENABLE_DLL
				| AT91C_DDRC2_ENRDM_ENABLE
				| AT91C_DDRC2_ZQ_INIT
				| AT91C_DDRC2_OCD_EXIT
				| AT91C_DDRC2_DQMS_NOT_SHARED
				| AT91C_DDRC2_ENRDM_DISABLE
				| AT91C_DDRC2_NB_BANKS_4
				| AT91C_DDRC2_NDQS_ENABLED
				| AT91C_DDRC2_DECOD_SEQUENTIAL
				| AT91C_DDRC2_UNAL_SUPPORTED);
	/*
	 * As the DDR2-SDRAm device requires a refresh time is 7.8125us
	 * when DDR run at 133MHz, so it needs (7.8125us * 133MHz / 10^9) clocks
	 */
	ddr2->rtr = 0x407;

	ddr2->tpr0 = (6 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET |
		      3 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TWR_OFFSET |
		      8 << ATMEL_MPDDRC_TPR0_TRC_OFFSET |
		      3 << ATMEL_MPDDRC_TPR0_TRP_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRRD_OFFSET |
		      1 << ATMEL_MPDDRC_TPR0_TWTR_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TMRD_OFFSET);

	ddr2->tpr1 = (1 << ATMEL_MPDDRC_TPR1_TXP_OFFSET |
		      0 << ATMEL_MPDDRC_TPR1_TXSRD_OFFSET |
		      15 << ATMEL_MPDDRC_TPR1_TXSNR_OFFSET |
		      10 << ATMEL_MPDDRC_TPR1_TRFC_OFFSET);

	ddr2->tpr2 = (10 << ATMEL_MPDDRC_TPR2_TFAW_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET |
		      7 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET |
		      7 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET);
}

void mem_init(void)
{
	struct atmel_mpddrc_config ddr2;

	ddr2_conf(&ddr2);

	/* Enable MPDDR clock */
	at91_periph_clk_enable(ATMEL_ID_MPDDRC);
	at91_system_clk_enable(AT91_PMC_DDR);

	/* DDRAM2 Controller initialize */
	ddr2_init(ATMEL_BASE_MPDDRC, ATMEL_BASE_DDRCS, &ddr2);
}

void at91_pmc_init(void)
{
	u32 tmp;

	tmp = AT91_PMC_PLLAR_29 |
	      AT91_PMC_PLLXR_PLLCOUNT(0x3f) |
	      AT91_PMC_PLLXR_MUL(43) |
	      AT91_PMC_PLLXR_DIV(1);
	at91_plla_init(tmp);

	at91_pllicpr_init(AT91_PMC_IPLL_PLLA(0x3));

	tmp = AT91_PMC_MCKR_MDIV_4 |
	      AT91_PMC_MCKR_CSS_PLLA;
	at91_mck_init(tmp);
}
#endif
