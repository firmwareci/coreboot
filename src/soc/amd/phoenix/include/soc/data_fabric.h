/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef AMD_PHOENIX_DATA_FABRIC_H
#define AMD_PHOENIX_DATA_FABRIC_H

#include <amdblocks/data_fabric_defs.h>
#include <types.h>

#define IOMS0_FABRIC_ID			0x13

#define DF_MMIO_BASE0			DF_REG_ID(0, 0xD80)
#define DF_MMIO_LIMIT0			DF_REG_ID(0, 0xD84)
#define   DF_MMIO_SHIFT			16
#define DF_MMIO_CTRL0			DF_REG_ID(0, 0xD88)

#define DF_MMIO_REG_SET_SIZE		4
#define DF_MMIO_REG_SET_COUNT		8

union df_mmio_control {
	struct {
		uint32_t re		:  1; /* [ 0.. 0] */
		uint32_t we		:  1; /* [ 1.. 1] */
		uint32_t		:  1; /* [ 2.. 2] */
		uint32_t np		:  1; /* [ 3.. 3] */
		uint32_t		: 12; /* [15.. 4] */
		uint32_t dst_fabric_id	:  6; /* [21..16] */
		uint32_t		: 10; /* [31..22] */
	};
	uint32_t raw;
};

#define DF_FICAA_BIOS			DF_REG_ID(4, 0x8C)
#define DF_FICAD_LO			DF_REG_ID(4, 0xB8)
#define DF_FICAD_HI			DF_REG_ID(4, 0xBC)

union df_ficaa {
	struct {
		uint32_t cfg_inst_acc_en :  1; /* [ 0.. 0] */
		uint32_t reg_num         : 10; /* [10.. 1] */
		uint32_t func_num        :  3; /* [13..11] */
		uint32_t b64_en          :  1; /* [14..14] */
		uint32_t                 :  1; /* [15..15] */
		uint32_t inst_id         :  8; /* [23..16] */
		uint32_t                 :  8; /* [31..24] */
	};
	uint32_t raw;
};

#endif /* AMD_PHOENIX_DATA_FABRIC_H */
