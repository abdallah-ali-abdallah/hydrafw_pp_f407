/*
 * HydraBus/HydraNFC
 *
 * Copyright (C) 2012-2014 Benjamin VERNOUX
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HYDRABUS_MODE_SPI_H_
#define _HYDRABUS_MODE_SPI_H_

#include "hydrabus_mode.h"

enum {
	SPI_MODE_MASTER = 1,
	SPI_MODE_SLAVE,
};

enum {
	SPI_MSB_FIRST,
	SPI_LSB_FIRST,
};

int mode_cmd_spi_init(t_hydra_console *con, t_tokenline_parsed *p);
int mode_cmd_spi_exec(t_hydra_console *con, t_tokenline_parsed *p,
		int token_pos);

#endif /* _HYDRABUS_MODE_SPI_H_ */

