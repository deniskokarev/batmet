//
// Created by Denis Kokarev on 6/12/23.
//

#ifndef BATMET_MAIN_H
#define BATMET_MAIN_H

#include <zephyr/logging/log.h>
#include "soc.h"

#ifndef LOG_REGISTERED
LOG_MODULE_DECLARE(batmet, LOG_LEVEL_DBG);
#endif

// CubeMX-compatible error handler
void Error_Handler();

#endif //BATMET_MAIN_H
