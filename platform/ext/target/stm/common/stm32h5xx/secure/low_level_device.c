/**
  ******************************************************************************
  * @file    low_level_device.c
  * @author  MCD Application Team
  * @brief   This file contains device definition for low_level_device
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  * <h2><center>&copy; Copyright (c) 2021 Arm Limited.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

#include "flash_layout.h"
#include "low_level_flash.h"

struct flash_range nvm_psa_its_vect[] = {
	{ FLASH_OTP_NV_COUNTERS_AREA_OFFSET, FLASH_ITS_AREA_OFFSET + FLASH_ITS_AREA_SIZE - 1},
	{ FLASH_AREA_2_OFFSET, FLASH_AREA_3_OFFSET + FLASH_AREA_3_SIZE - 1},
};

struct flash_range nvm_psa_secure_range[] = {
	{ FLASH_AREA_BL2_OFFSET, FLASH_AREA_2_OFFSET - 1  }
};

struct low_level_device FLASH0_DEV =  {
	.erase = { .nb =sizeof(nvm_psa_its_vect)/sizeof(struct flash_range), .range = nvm_psa_its_vect},
	.write = { .nb =sizeof(nvm_psa_its_vect)/sizeof(struct flash_range), .range = nvm_psa_its_vect},
	.secure = { .nb =sizeof(nvm_psa_secure_range)/sizeof(struct flash_range), .range = nvm_psa_secure_range},
	.read_error = 1
};
