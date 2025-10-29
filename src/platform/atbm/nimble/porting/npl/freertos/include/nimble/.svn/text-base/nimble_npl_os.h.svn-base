/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef _NIMBLE_NPL_OS_H_
#define _NIMBLE_NPL_OS_H_

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"


#ifdef __cplusplus
extern "C" {
#endif

#define BLE_NPL_OS_ALIGNMENT    4

#define BLE_NPL_TIME_FOREVER    portMAX_DELAY

/* This should be compatible with TickType_t */
typedef uint32_t ble_npl_time_t;
typedef int32_t ble_npl_stime_t;

struct ble_npl_event {
    bool queued;
    ble_npl_event_fn *fn;
    void *arg;
};

struct ble_npl_eventq {
    QueueHandle_t q;
};

struct ble_npl_callout {
    TimerHandle_t handle;
    struct ble_npl_eventq *evq;
    struct ble_npl_event ev;
};

struct ble_npl_mutex {
    SemaphoreHandle_t handle;
};

struct ble_npl_sem {
    SemaphoreHandle_t handle;
};

/*
 * Simple APIs are just defined as static inline below, but some are a bit more
 * complex or require some global state variables and thus are defined in .c
 * file instead and static inline wrapper just calls proper implementation.
 * We need declarations of these functions and they are defined in header below.
 */
#include "npl_freertos.h"
#include "nimble_npl_os_function.h"


#ifdef __cplusplus
}
#endif

#endif  /* _NPL_H_ */
