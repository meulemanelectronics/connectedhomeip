/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file
 * Defines compile-time configuration overrides for the chip Device Layer.
 */

#pragma once

// -------------------- General Configuration --------------------
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 0x00001600

// -------------------- WiFi Station Configuration --------------------

// -------------------- WiFi AP Configuration --------------------
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0

// -------------------- Service Discovery Configuration -----------------------
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1

// -------------------- Thread Configuration --------------------
#ifndef CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 0
#define CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE 0x00001000
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 1
#endif

// -------------------- Event Logging Configuration --------------------
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE 256

