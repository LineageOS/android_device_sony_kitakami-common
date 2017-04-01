#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017 The LineageOS Project
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
#

# Common path
COMMON_PATH := device/sony/kitakami-common

# Common specific overlays
DEVICE_PACKAGE_OVERLAYS += $(COMMON_PATH)/overlay

# Common radio specific elements
ifneq ($(BOARD_HAVE_RADIO),false)
    DEVICE_PACKAGE_OVERLAYS += $(COMMON_PATH)/radio/overlay
    $(call inherit-product, $(COMMON_PATH)/radio/radio.mk)
endif

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:system/etc/permissions/android.hardware.opengles.aep.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.vulkan.level-0.xml:system/etc/permissions/android.hardware.vulkan.level.xml \
    frameworks/native/data/etc/android.hardware.vulkan.version-1_0_3.xml:system/etc/permissions/android.hardware.vulkan.version.xml

# ANT+
PRODUCT_PACKAGES += \
    AntHalService \
    com.dsi.ant.antradio_library \
    libantradio

# IRQ
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/msm_irqbalance.conf:system/vendor/etc/msm_irqbalance.conf

# Recovery
PRODUCT_PACKAGES += \
    librecovery_updater_kitakami

# TimeKeep
PRODUCT_PACKAGES += \
    timekeep \
    TimeKeep

# Common product elements
include $(COMMON_PATH)/product/*.mk

$(call inherit-product, hardware/broadcom/wlan/bcmdhd/config/config-bcm.mk)

# Get non-open-source common files
$(call inherit-product, vendor/sony/kitakami-common/kitakami-common-vendor.mk)
