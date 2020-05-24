/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Integrator AP board.
 *.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "../../board/mv_feroceon/mv_kw/mvSysHwConfig.h"

/************/
/* VERSIONS */
/************/
#ifdef MV_TINY_IMAGE
#define CONFIG_IDENT_STRING	" Marvell version: 3.5.3-TINY"
#else
#define CONFIG_IDENT_STRING	" Marvell version: 3.5.3"
#endif /* MV_TINY_IMAGE */
/* version number passing when loading Kernel */
#define VER_NUM 0x03050300           /* 3.5.3 */

/********************/
/* MV DEV SUPPORTS  */
/********************/	
#define CONFIG_PCI           /* pci support               */
#undef CONFIG_PCI_1         /* sec pci interface support */

/********************/
/* Environment variables */
/********************/	


#define CFG_ENV_IS_IN_FLASH		1

/**********************************/
/* Marvell Monitor Extension      */
/**********************************/
#define enaMonExt()( /*(!getenv("enaMonExt")) ||\*/\
		     ( getenv("enaMonExt") && \
                       ((!strcmp(getenv("enaMonExt"),"yes")) ||\
		       (!strcmp(getenv("enaMonExt"),"Yes"))) \
		     )\
		    )

/********/
/* CLKs */
/********/
#ifndef __ASSEMBLY__
extern unsigned int mvSysClkGet(void);
extern unsigned int mvTclkGet(void);
#define UBOOT_CNTR		0		/* counter to use for uboot timer */
#define MV_TCLK_CNTR		1		/* counter to use for uboot timer */
#define MV_REF_CLK_DEV_BIT	1000		/* Number of cycle to eanble timer */
#define MV_REF_CLK_BIT_RATE	100000		/* Ref clock frequency */
#define MV_REF_CLK_INPUT_GPP	6		/* Ref clock frequency input */

#define CFG_HZ			1000
#define CFG_TCLK                mvTclkGet()
#define CFG_BUS_HZ              mvSysClkGet()
#define CFG_BUS_CLK             CFG_BUS_HZ
#endif

/********************/
/* Dink PT settings */
/********************/
#define CFG_MV_PT

#ifdef CFG_MV_PT
#define CFG_PT_BASE  (CFG_MALLOC_BASE - 0x20000)
#endif /* #ifdef CFG_MV_PT */


/*************************************/
/* High Level Configuration Options  */
/* (easy to change)		     */
/*************************************/
#define CONFIG_MARVELL		1
#define CONFIG_ARM926EJS	1		/* CPU */


/* commands */

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT | \
				 CONFIG_BOOTP_BOOTFILESIZE)


/* Default U-Boot supported features */
#if defined(MV_TINY_IMAGE)
#define CONFIG_CMD_BASIC (CFG_CMD_PCI \
			 | CFG_CMD_FLASH\
			 | CFG_CMD_ENV	\
			 | CFG_CMD_NET	\
			 | CFG_CMD_IDE	\
			 | CFG_CMD_EXT2	\
			 | CFG_CMD_MEMORY\
            | CFG_CMD_DATE \
			 | CFG_CMD_BOOTD)

#elif defined(DB_FPGA)
#define CONFIG_CMD_BASIC ( CFG_CMD_DHCP	\
			 | CFG_CMD_ELF	\
                         | CFG_CMD_I2C \
                         | CFG_CMD_EEPROM \
                         | CFG_CMD_PCI \
                         | CFG_CMD_JFFS2 \
                         | CFG_CMD_DATE \
			 | CFG_CMD_LOADS	\
			 | CFG_CMD_FLASH	\
			 | CFG_CMD_MEMORY	\
			 | CFG_CMD_ENV	\
			 | CFG_CMD_BOOTD	\
			 | CFG_CMD_CONSOLE	\
			 | CFG_CMD_RUN	\
			 | CFG_CMD_MISC	\
			 )
#else
#define CONFIG_CMD_BASIC ( CFG_CMD_DHCP	\
                         | CFG_CMD_I2C \
                         | CFG_CMD_EEPROM \
                         | CFG_CMD_PCI \
                         | CFG_CMD_NET \
                         | CFG_CMD_PING \
                         | CFG_CMD_JFFS2 \
                         | CFG_CMD_DATE \
                         | CFG_CMD_BSP	\
			 | CFG_CMD_FLASH	\
			 | CFG_CMD_MEMORY	\
			 | CFG_CMD_IDE	\
             | CFG_CMD_IMI \
             | CFG_CMD_EXT2	\
			 | CFG_CMD_ENV	\
			 | CFG_CMD_BOOTD	\
			 | CFG_CMD_RUN	\
			 | CFG_CMD_RCVR	\
             | CFG_CMD_FAT	\
             | CFG_CMD_USB	\
			 )
						 
#endif

#ifndef MV_MMC
#define MV_MMC
#endif

#if defined(MV_NAND) || defined(MV_NAND_BOOT)
#define CONFIG_CMD_BASIC1 (CONFIG_CMD_BASIC | CFG_CMD_NAND)
#define CONFIG_MTD_NAND_VERIFY_WRITE
#else
#define CONFIG_CMD_BASIC1 CONFIG_CMD_BASIC
#endif

#if (defined(MV_INCLUDE_PEX) | defined(MV_INCLUDE_PCI))
#define CONFIG_CMD_BASIC2 CONFIG_CMD_BASIC1
#else
#undef CONFIG_PCI
#define CONFIG_CMD_BASIC2 (CONFIG_CMD_BASIC1 & ~CFG_CMD_PCI)
#endif


#if (!defined (MV_INC_BOARD_NOR_FLASH) && \
     !defined(MV_INC_BOARD_SPI_FLASH) && \
     !defined(MV_INCLUDE_INTEG_MFLASH))
#undef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_IS_NOWHERE
#define CFG_NO_FLASH
#define CONFIG_CMD_BASIC3 (CONFIG_CMD_BASIC2 & ~CFG_CMD_FLASH 	\
				& ~CFG_CMD_ENV	\
				& ~CFG_CMD_JFFS2)
#else
#define CONFIG_CMD_BASIC3 CONFIG_CMD_BASIC2
#endif

#if !defined (MV_INC_BOARD_NAND_FLASH)
#define CONFIG_CMD_BASIC4 (CONFIG_CMD_BASIC3 & ~CFG_CMD_NAND)
#else
#define CONFIG_CMD_BASIC4 CONFIG_CMD_BASIC3
#endif

#if !defined (MV_INC_BOARD_PCI_SATA) && !defined (MV_INCLUDE_INTEG_SATA)
#define CONFIG_CMD_BASIC5 (CONFIG_CMD_BASIC4 & ~CFG_CMD_IDE	\
				& ~CFG_CMD_EXT2)                
#else
#define CONFIG_CMD_BASIC5 CONFIG_CMD_BASIC4
#endif

#if defined(MV_MMC)
#define CONFIG_CMD_BASIC6 (CONFIG_CMD_BASIC5 | CFG_CMD_MMC | CFG_CMD_FAT)
#else
#define CONFIG_CMD_BASIC6 CONFIG_CMD_BASIC5
#endif
#define CONFIG_COMMANDS CONFIG_CMD_BASIC6



/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define	CFG_MAXARGS	32		/* max number of command args	*/

/*-----------------------------------------------------------------------
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

#undef	CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CFG_IDE_MAXBUS		4	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	CFG_IDE_MAXBUS * 8	/* max. 1 drive per IDE bus	*/

#define CFG_ATA_IDE0_OFFSET	0x0000

#undef CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

/* For 6121/6145 support */
#define CONFIG_SCSI_AHCI
#ifdef CONFIG_SCSI_AHCI
#define CONFIG_SATA_6121
#define CFG_SCSI_MAX_SCSI_ID	4
#define CFG_SCSI_MAX_LUN	1
#define CFG_SCSI_MAX_DEVICE 	(CFG_SCSI_MAX_SCSI_ID * CFG_SCSI_MAX_LUN)
#define CFG_SCSI_MAXDEVICE	CFG_SCSI_MAX_DEVICE
#endif

#ifndef CONFIG_MMC
#define CONFIG_LBA48
#endif

/* which initialization functions to call for this board */
#define CONFIG_MISC_INIT_R	1      /* after relloc initialization*/
#undef CONFIG_DISPLAY_MEMMAP    /* at the end of the bootprocess show the memory map*/

#define CONFIG_ENV_OVERWRITE    /* allow to change env parameters */

#undef	CONFIG_WATCHDOG		/* watchdog disabled		*/

/* Cache */
#define CFG_CACHELINE_SIZE	32	


/* global definetions. */
#define	CFG_SDRAM_BASE		0x00000000


#define CFG_RESET_ADDRESS	0xffff0000

#define CFG_MALLOC_BASE		(TEXT_BASE + (1 << 20)) /* TEXT_BASE + 1M */

/*
 * When locking data in cache you should point the CFG_INIT_RAM_ADDRESS
 * To an unused memory region. The stack will remain in cache until RAM
 * is initialized 
*/
#define	CFG_MALLOC_LEN		(1 << 20)	/* (default) Reserve 1MB for malloc*/

#define CFG_GBL_DATA_SIZE	128  /* size in bytes reserved for init data */

#undef CONFIG_INIT_CRITICAL		/* critical code in start.S */

#define MASTER_CPU	0

/********/
/* DRAM */
/********/

#define CFG_DRAM_BANKS		4

/* this defines whether we want to use the lowest CAL or the highest CAL available,*/
/* we also check for the env parameter CASset.					  */
#define MV_MIN_CAL

#define CFG_MEMTEST_START     0x00400000
#define CFG_MEMTEST_END       0x007fffff

/********/
/* RTC  */
/********/
#if (CONFIG_COMMANDS & CFG_CMD_DATE)
#define CFG_NVRAM_SIZE  0x00 /* dummy */
#define CFG_NVRAM_BASE_ADDR DEVICE_CS1_BASE /* dummy */
#endif /* #if (CONFIG_COMMANDS & CFG_CMD_DATE) */

/********************/
/* Serial + parser  */
/********************/
/*
 * The following defines let you select what serial you want to use
 * for your console driver.
 */
#define CONFIG_BAUDRATE         115200   /* console baudrate = 115200    */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600 }
 
#ifdef DB_88F6280A
#define CFG_DUART_CHAN		1		/* channel to use for console */
#else
#define CFG_DUART_CHAN		0		/* channel to use for console */
#endif

#define CFG_INIT_CHAN1
#define CFG_INIT_CHAN2


#define CONFIG_LOADS_ECHO       0       /* echo off for serial download */
#define CFG_LOADS_BAUD_CHANGE           /* allow baudrate changes       */

#define CFG_CONSOLE_INFO_QUIET  /* don't print In/Out/Err console assignment. */
#undef CONFIG_SILENT_CONSOLE  /* define for Pex complince */

/* parser */
/* don't chang the parser if you want to load Linux(if you cahnge it to HUSH the cmdline will
	not pass to the kernel correctlly???) */
/*#define CFG_HUSH_PARSER */
#undef CFG_HUSH_PARSER 
#define CONFIG_AUTO_COMPLETE

#define CFG_PROMPT_HUSH_PS2	"> "

#define	CFG_LONGHELP			/* undef to save memory		*/
#define	CFG_PROMPT	"Marvell>> "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */

/************/
/* ETHERNET */
/************/
/* to change the default ethernet port, use this define (options: 0, 1, 2) */
#if (CONFIG_COMMANDS & CFG_CMD_NET)
#define CONFIG_NET_MULTI
#define CONFIG_IPADDR		192.168.0.50
#define CONFIG_SERVERIP		192.168.0.1
#define CONFIG_NETMASK		255.255.255.0
#endif

#define ETHADDR          	"00:00:00:00:51:81"
#define ETH1ADDR          	"00:00:00:00:51:82"
#define ENV_ETH_PRIME		"egiga0"

/************/
/* PCI	    */
/************/
#if defined(MV_INCLUDE_PCI)

#if defined(DB_PEX_PCI)
#define ENV_PCI_MODE	"device"	/* PCI */
#else
#define ENV_PCI_MODE	"host"		/* PCI */
#endif

#endif

/************/
/* LCD	    */
/************/

#define ENV_LCD0_ENABLE	"1"
#define ENV_LCD0_PARAMS "640x480-16@60"
#define ENV_LCD_BOOTARGS "video-dovefb:lcd0:$(lcd0_params) clcd.lcd0_enable=$(lcd0_enable)"

/************/
/* USB	    */
/************/

#if defined(MV_INCLUDE_USB)
#define ENV_USB0_MODE	"host"
#define CONFIG_USB_EHCI
#if defined(CONFIG_CMD_USB)
#define CONFIG_USB_STORAGE
#endif
#endif

/************/
/* SDIO/MMC */
/************/

#if defined(MV_88F6183) || defined(MV_88F6183L) || defined(MV88F6281) || defined(MV88F6282) || \
    defined(MV88F6192) || defined(MV88F6180) || defined(MV88F6280)
#define CONFIG_MMC
#define CFG_MMC_BASE 0xF0000000
#endif

/***************************************/
/* LINUX BOOT and other ENV PARAMETERS */
/***************************************/
#define CFG_BOOTARGS_END ":::DB88FXX81:eth0:none"
#define CFG_BOOTARGS_END_SWITCH ":::RD88FXX81:eth0:none"
#define RCVR_IP_ADDR "169.254.100.100"
#define	RCVR_LOAD_ADDR	"0x02000000"

#define CONFIG_ZERO_BOOTDELAY_CHECK

#define	CFG_LOAD_ADDR		0x02000000	/* default load address	*/

#undef	CONFIG_BOOTARGS

/* auto boot*/
#define CONFIG_BOOTDELAY	3 		/* by default no autoboot */

#if (CONFIG_BOOTDELAY >= 0)
#define CONFIG_ROOTPATH	/mnt/ARM_FS/
#endif /* #if (CONFIG_BOOTDELAY >= 0) */

#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CFG_BOOTMAPSZ		(8<<20)	/* Initial Memory map for Linux */

#define BRIDGE_REG_BASE_BOOTM 0xfbe00000 /* this paramaters are used when booting the linux kernel */ 

#define CONFIG_CMDLINE_TAG              1       /* enable passing of ATAGs  */
#define CONFIG_INITRD_TAG		1	/* enable INITRD tag for ramdisk data */
#define CONFIG_SETUP_MEMORY_TAGS        1
#define CONFIG_MARVELL_TAG              1
#define ATAG_MARVELL                    0x41000403

/********/
/* I2C  */
/********/
#define CFG_I2C_EEPROM_ADDR_LEN 	1
#define CFG_I2C_MULTI_EEPROMS
#define CFG_I2C_SPEED   		100000		/* I2C speed default */

/* I2C addresses for the two DIMM SPD chips */
#define DIMM0_I2C_ADDR			0x56
#define DIMM1_I2C_ADDR			0x54

/* CPU I2C settings */
#define CPU_I2C  
#define I2C_CPU0_EEPROM_ADDR    	0x51


/********/
/* PCI  */
/********/
#ifdef CONFIG_PCI
 #define CONFIG_PCI_HOST PCI_HOST_FORCE /* select pci host function     */
 #define CONFIG_PCI_PNP          	/* do pci plug-and-play         */

/* To reduce image size... */
#if defined (MV_INCLUDE_USB) || (MV_TINY_IMAGE) || defined(__BE)
 #undef CONFIG_SK98			/* yukon */
 #undef CONFIG_DRIVER_RTL8029
 #undef CONFIG_EEPRO100	
#endif

/* To reduce image size... */
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#if !defined (MV_NAND_BOOT)
 #undef CONFIG_DRIVER_RTL8029
 #undef CONFIG_EEPRO100
 #undef CONFIG_SK98
#endif
#endif

/* DB_PRPMC support only Yukon */
#if defined (DB_PRPMC) || defined (DB_MNG)
 #undef CONFIG_EEPRO100	
 #undef CONFIG_DRIVER_RTL8029 
#endif

#endif /* #ifdef CONFIG_PCI */

#define PCI_HOST_ADAPTER 0              /* configure ar pci adapter     */
#define PCI_HOST_FORCE   1              /* configure as pci host        */
#define PCI_HOST_AUTO    2              /* detected via arbiter enable  */

/* for Yukon */
#define __mem_pci(x) x
#define __io_pci(x) x
#define __arch_getw(a)			(*(volatile unsigned short *)(a))
#define __arch_putw(v,a)		(*(volatile unsigned short *)(a) = (v))



/***********************/
/* FLASH organization  */
/***********************/

/*
 * When CFG_MAX_FLASH_BANKS_DETECT is defined, the actual number of Flash
 * banks has to be determined at runtime and stored in a gloabl variable
 * mv_board_num_flash_banks. The value of CFG_MAX_FLASH_BANKS_DETECT is only
 * used instead of CFG_MAX_FLASH_BANKS to allocate the array flash_info, and
 * should be made sufficiently large to accomodate the number of banks that
 * might actually be detected.  Since most (all?) Flash related functions use
 * CFG_MAX_FLASH_BANKS as the number of actual banks on the board, it is
 * defined as mv_board_num_flash_banks.
 */
#define CFG_MAX_FLASH_BANKS_DETECT	5
#ifndef __ASSEMBLY__
extern int mv_board_num_flash_banks;
#endif
#define CFG_MAX_FLASH_BANKS (mv_board_num_flash_banks)

#define CFG_MAX_FLASH_SECT	300	/* max number of sectors on one chip */
#define CFG_FLASH_PROTECTION    1

#define CFG_EXTRA_FLASH_DEVICE	DEVICE3	/* extra flash at device 3 */
#define CFG_EXTRA_FLASH_WIDTH	4	/* 32 bit */
#define CFG_BOOT_FLASH_WIDTH	1	/* 8 bit */

#define CFG_FLASH_ERASE_TOUT	120000/1000	/* 120000 - Timeout for Flash Erase (in ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* 500 - Timeout for Flash Write (in ms) */
#define CFG_FLASH_LOCK_TOUT	500	/* 500- Timeout for Flash Lock (in ms) */


/***************************/
/* CFI FLASH organization  */
/***************************/
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI		1
#define CFG_FLASH_USE_BUFFER_WRITE
#define CFG_FLASH_QUIET_TEST
#define CFG_FLASH_BANKS_LIST	{BOOTDEV_CS_BASE}
#if defined(__BE)
#define CFG_WRITE_SWAPPED_DATA
#endif

#ifdef MV_BOOTROM
#if defined (MV_FLASH_BOOT)
	#define CFG_FLASH_BASE		DEVICE_MFLASH_BASE
	#define CFG_FLASH_SIZE      	(256 << 10)
#elif defined (MV_SPI_BOOT)
	#define CFG_FLASH_BASE		DEVICE_SPI_BASE
	#define CFG_FLASH_SIZE      	_16M	
#endif
#else
	#define CFG_FLASH_BASE		BOOTDEV_CS_BASE
	#define CFG_FLASH_SIZE      	BOOTDEV_CS_SIZE
#endif /* MV_BOOTROM */

#define CFG_ENV_SIZE		0x1000
#define MONITOR_HEADER_LEN	0x200
#if defined(MV_SEC_128K)

 #if defined(MV_TINY_IMAGE)
  	#define CFG_MONITOR_LEN			(256 << 10)	/* Reserve 252 kB for Monitor */
 #else
  	#define CFG_MONITOR_LEN			(512 << 10)	/* Reserve 512 kB for Monitor */
 #endif								 

 #define CFG_ENV_SECT_SIZE			0x20000
//modfiy by QNAP: Support SPANSION SPI flash sector size for 64K
#elif defined(MV_SEC_64K)
 #if defined(MV_TINY_IMAGE)
  	#define CFG_MONITOR_LEN			(256 << 10)	/* Reserve 252 kB for Monitor */
 #else
 #if defined(MV_BOOTROM)
  	#define CFG_MONITOR_LEN			(512 << 10)	/* Reserve 512 kB for Monitor */
 #else
  	#define CFG_MONITOR_LEN			(448 << 10)	/* Reserve 448 kB for Monitor */
 #endif								 
 #endif								 
 #define CFG_ENV_SECT_SIZE			0x10000
//////////////////////////////////////////////////////////////
#elif defined(MV_SEC_256K)
 #if defined(MV_TINY_IMAGE)
  	#define CFG_MONITOR_LEN			(256 << 10)	/* Reserve 252 kB for Monitor */
 #else
  	#define CFG_MONITOR_LEN			(512 << 10)	/* Reserve 512 kB for Monitor */
 #endif								 

 #define CFG_ENV_SECT_SIZE			0x40000
#endif

#ifdef MV_BOOTROM
 #define CFG_MONITOR_IMAGE_OFFSET	0	/* offset of the monitor from the u-boot image */
 #define CFG_MONITOR_BASE		(CFG_FLASH_BASE)
 #define CFG_ENV_ADDR    		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#else
 #define CFG_MONITOR_IMAGE_OFFSET	CFG_ENV_SECT_SIZE	/* offset of the monitor from the u-boot image */
 #define CFG_MONITOR_BASE		(CFG_FLASH_BASE + CFG_FLASH_SIZE 	\
							- CFG_MONITOR_LEN)
 #define CFG_ENV_ADDR    		(CFG_MONITOR_BASE - CFG_ENV_SECT_SIZE)
#endif

/*-----------------------------------------------------------------------
 * NAND-FLASH stuff
 *-----------------------------------------------------------------------*/
/* Use the new NAND code. */

#undef CFG_NAND_LEGACY
#define CFG_MAX_NAND_DEVICE     1       /* Max number of NAND devices */
#define NAND_MAX_CHIPS          CFG_MAX_NAND_DEVICE

/* JFFS2 over NAND define */ 
#define CONFIG_JFFS2_NAND 1

/* Boot from NAND settings */
/* Duplicate defines from nBootstrap.h */
#ifdef MV_NAND_BOOT
#define CFG_NAND_BOOT
#define CFG_ENV_IS_IN_NAND	1

#if defined(MV_BOOTROM)
#if defined(MV_LARGE_PAGE)
#define CFG_ENV_OFFSET				(640 << 10)	/* environment starts here  */
#undef CFG_ENV_SECT_SIZE
#define CFG_ENV_SECT_SIZE			(128 << 10)	/* environment take 1 block */
#undef CFG_ENV_SIZE		
#define CFG_ENV_SIZE				CFG_ENV_SECT_SIZE /* environment take 1 block */	
#define CFG_NBOOT_BASE				0	
#define CFG_NBOOT_LEN				(4 << 10)	/* Reserved 4KB for boot strap */
#undef	CFG_MONITOR_LEN
#define	CFG_MONITOR_LEN				(640 << 10)	/* Reserve 4 * 128KB + ENV = 640KB for Monitor */
#undef CFG_MONITOR_BASE	
#define CFG_MONITOR_BASE			0
#undef	CFG_ENV_IS_IN_FLASH		
#undef CFG_MONITOR_IMAGE_OFFSET		
#define CFG_MONITOR_IMAGE_OFFSET		0 /* offset of the monitor from the u-boot image */
#define CFG_MONITOR_IMAGE_DST			TEXT_BASE 	/* Load NUB to this addr	*/
#undef CFG_ENV_ADDR
#define CFG_ENV_ADDR    			CFG_ENV_OFFSET

#else /* ! LARGE PAGE NAND */

#define CFG_ENV_OFFSET				0x84000	/* environment starts here  */
#define CFG_NBOOT_BASE				0	
#define CFG_NBOOT_LEN				(16 << 10)	/* Reserved 16kB for boot strap */
#undef	CFG_MONITOR_LEN
#define	CFG_MONITOR_LEN				(512 << 10)	/* Reserve 512 kB for Monitor */
#undef CFG_MONITOR_BASE	
#define CFG_MONITOR_BASE			0
#undef	CFG_ENV_IS_IN_FLASH		
#undef CFG_ENV_SECT_SIZE
#define CFG_ENV_SECT_SIZE			0x4000
#undef CFG_ENV_SIZE		
#define CFG_ENV_SIZE				CFG_ENV_SECT_SIZE /* environment take 1 block */	
#undef CFG_MONITOR_IMAGE_OFFSET		
#define CFG_MONITOR_IMAGE_OFFSET		0 /* offset of the monitor from the u-boot image */
#define CFG_MONITOR_IMAGE_DST			TEXT_BASE 	/* Load NUB to this addr	*/
#undef CFG_ENV_ADDR
#define CFG_ENV_ADDR    			CFG_ENV_OFFSET /* UBOOT_IMAGE_DEST - UBOOT_IMAGE_OFFS */

#endif /* defined(MV_LARGE_PAGE) */

#else /* ! MV_BOOTROM */

#if defined(MV_LARGE_PAGE)
#define CFG_ENV_OFFSET				(640 << 10)	/* environment starts here  */
#undef CFG_ENV_SECT_SIZE
#define CFG_ENV_SECT_SIZE			(128 << 10)	/* environment take 1 block */
#undef CFG_ENV_SIZE		
#define CFG_ENV_SIZE				CFG_ENV_SECT_SIZE /* environment take 1 block */	
#define CFG_NBOOT_BASE				0	
#define CFG_NBOOT_LEN				(4 << 10)	/* Reserved 4KB for boot strap */
#undef	CFG_MONITOR_LEN
#define	CFG_MONITOR_LEN				(640 << 10)	/* Reserve 4 * 128KB + ENV = 640KB for Monitor */
#undef CFG_MONITOR_BASE	
#define CFG_MONITOR_BASE			0
#undef	CFG_ENV_IS_IN_FLASH		
#undef CFG_MONITOR_IMAGE_OFFSET		
#define CFG_MONITOR_IMAGE_OFFSET		0 /* offset of the monitor from the u-boot image */
#define CFG_MONITOR_IMAGE_DST			TEXT_BASE	/* Load NUB to this addr	*/
#undef CFG_ENV_ADDR
#define CFG_ENV_ADDR    			CFG_ENV_OFFSET

#else /* ! LARGE PAGE NAND */

#define CFG_ENV_OFFSET				(16 << 10)	/* environment starts here  */
#undef CFG_ENV_SECT_SIZE
#define CFG_ENV_SECT_SIZE			(128 << 10)
#undef CFG_ENV_SIZE		
#define CFG_ENV_SIZE				CFG_ENV_SECT_SIZE /* environment take 1 block */	
#define CFG_NBOOT_BASE				0	
#define CFG_NBOOT_LEN				(4 << 10)	/* Reserved 16kB for boot strap */
#undef	CFG_MONITOR_LEN
#define	CFG_MONITOR_LEN				(640 << 10)	/* Reserve 4 * 128KB + ENV = 640KB for Monitor */
#undef CFG_MONITOR_BASE	
#define CFG_MONITOR_BASE			(CFG_ENV_OFFSET)
#undef	CFG_ENV_IS_IN_FLASH		
#undef CFG_MONITOR_IMAGE_OFFSET		
#define CFG_MONITOR_IMAGE_OFFSET		CFG_ENV_SECT_SIZE /* offset of the monitor from the u-boot image */
#define CFG_MONITOR_IMAGE_DST			TEXT_BASE - CFG_ENV_SECT_SIZE	/* Load NUB to this addr	*/
#undef CFG_ENV_ADDR
#define CFG_ENV_ADDR    			CFG_MONITOR_IMAGE_DST
#endif /* defined(MV_LARGE_PAGE) */
#endif /* defined(MV_BOOTROM) */

#endif /* MV_NAND_BOOT */


#if defined(RD_88F5181L_FE) || defined(RD_88F5181L_GE) || defined(RD_88F5181L_FXO_GE) || \
		defined(RD_88F5181_GTWGE) || defined (RD_88F5181_GTWFE)
 #define BOARD_LATE_INIT
#endif

#if (CONFIG_COMMANDS & CFG_CMD_JFFS2)
#define CONFIG_JFFS2_CMDLINE 
#endif /* #if (CONFIG_COMMANDS & CFG_CMD_JFFS2) */


/*****************/
/* others        */
/*****************/
#define CFG_DFL_MV_REGS		0xd0000000 	/* boot time MV_REGS */
#define CFG_MV_REGS		INTER_REGS_BASE /* MV Registers will be mapped here */

#undef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE	(1 << 20)	/* regular stack - up to 4M (in case of exception)*/
#define CONFIG_NR_DRAM_BANKS 	4 

#endif							/* __CONFIG_H */
