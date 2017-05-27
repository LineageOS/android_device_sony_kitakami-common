# Copyright (C) 2017, The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifeq ($(WITH_TWRP), true)
     LOCAL_PATH := $(call my-dir)

     # Copy vendor files needed for touchscreen if device is karin (SGP771) or karin_windy (SGP712)
     ifneq ($(filter karin karin_windy,$(TARGET_DEVICE)),)
           $(shell mkdir -p "$(TARGET_RECOVERY_ROOT_OUT)/sbin";)
           $(shell cp -f "$(LOCAL_PATH)/rootdir/sbin/postrecoveryboot.sh" "$(TARGET_RECOVERY_ROOT_OUT)/sbin/";)
           $(shell mkdir -p "$(TARGET_RECOVERY_ROOT_OUT)/vendor/bin";)
           $(shell cp -f "vendor/sony/$(TARGET_DEVICE)/proprietary/vendor/bin/touch_fusion" "$(TARGET_RECOVERY_ROOT_OUT)/vendor/bin/";)
           $(shell mkdir -p "$(TARGET_RECOVERY_ROOT_OUT)/vendor/firmware";)
           $(shell cp -rf "vendor/sony/$(TARGET_DEVICE)/proprietary/vendor/firmware/max11945.bin" "$(TARGET_RECOVERY_ROOT_OUT)/vendor/firmware/";)
           $(shell mkdir -p "$(TARGET_RECOVERY_ROOT_OUT)/vendor/etc";)
           $(shell cp -f "vendor/sony/$(TARGET_DEVICE)/proprietary/vendor/etc/touch_fusion_panel_id_"*".cfg" "$(TARGET_RECOVERY_ROOT_OUT)/vendor/etc/";)
     endif

endif
