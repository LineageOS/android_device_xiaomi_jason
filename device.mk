#
# Copyright (C) 2017-2020 The LineageOS Project
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
# This file sets variables that control the way modules are built
# thorughout the system. It should not be used to conditionally
# disable makefiles (the proper mechanism to control what gets
# included in a build is to use PRODUCT_PACKAGES in a product
# definition file).
#

# Inherit from sdm660-common
$(call inherit-product, device/xiaomi/sdm660-common/sdm660.mk)

$(call inherit-product, vendor/xiaomi/jason/jason-vendor.mk)

# Setup dalvik vm configs
$(call inherit-product, frameworks/native/build/phone-xhdpi-6144-dalvik-heap.mk)

# Overlays
DEVICE_PACKAGE_OVERLAYS += \
    $(LOCAL_PATH)/overlay \
    $(LOCAL_PATH)/overlay-lineage

PRODUCT_ENFORCE_RRO_EXCLUDED_OVERLAYS += \
    $(LOCAL_PATH)/overlay-lineage/lineage-sdk

# Boot animation
TARGET_SCREEN_HEIGHT := 1920
TARGET_SCREEN_WIDTH := 1080

# Haters gonna hate..
PRODUCT_CHARACTERISTICS := nosdcard

# Treble
PRODUCT_COMPATIBLE_PROPERTY_OVERRIDE := true

# Audio
PRODUCT_PACKAGES += \
    libspkrprot

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/audio_platform_info_intcodec.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info_intcodec.xml \
    $(LOCAL_PATH)/audio/mixer_paths.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths.xml \
    $(LOCAL_PATH)/audio/sound_trigger_mixer_paths.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sound_trigger_mixer_paths.xml \
    $(LOCAL_PATH)/audio/sound_trigger_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/sound_trigger_platform_info.xml

PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.audio.spkr.cal.duration=100 \
    persist.vendor.audio.speaker.prot.enable=true \
    persist.vendor.audio.spv3.enable=true \
    persist.vendor.audio.avs.afe_api_version=2

# Camera
PRODUCT_PACKAGES += \
    camera.sdm660 \
    libmm-qcamera

PRODUCT_PROPERTY_OVERRIDES += \
    persist.camera.hist.high=20 \
    persist.camera.hist.drc=1.2 \
    persist.vendor.camera.expose.aux=1 \
    persist.vendor.camera.sat.enable=1 \
    persist.vendor.camera.instant.aec=1 \
    persist.vendor.camera.ae.instant.bound=20 \
    persist.vendor.camera.set.afd=4 \
    persist.vendor.camera.feature.cac=1 \
    persist.vendor.camera.fovc.enable=1 \
    persist.vendor.dualcam.lpm.enable=1 \
    persist.vendor.dualcam.defer.enable=1

PRODUCT_SYSTEM_DEFAULT_PROPERTIES += \
    vendor.camera.aux.packageblacklist=com.tencent.mm \
    vendor.camera.hal1.packagelist=com.whatsapp,com.intsig.camscanner,com.instagram.android

# ConfigPanel
PRODUCT_PACKAGES += \
    ConfigPanel

# Display
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=420

PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.display.ad=1 \
    ro.vendor.display.ad.sdr_calib_data=/system/vendor/etc/sdr_config.cfg \
    ro.vendor.display.ad.hdr_calib_data=/system/vendor/etc/hdr_config.cfg \
    ro.vendor.display.sensortype=2

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayout/gpio-keys.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/gpio-keys.kl \
    $(LOCAL_PATH)/keylayout/synaptics_dsx.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/synaptics_dsx.kl

# Gatekeeper HAL
PRODUCT_PACKAGES += \
    android.hardware.gatekeeper@1.0-impl \
    android.hardware.gatekeeper@1.0-service

# Keymaster HAL
PRODUCT_PACKAGES += \
    android.hardware.keymaster@3.0-impl \
    android.hardware.keymaster@3.0-service

# Lineage hardware
PRODUCT_PACKAGES += \
    vendor.lineage.touch@1.0-service.jason

# LiveDisplay
PRODUCT_PACKAGES += \
    vendor.lineage.livedisplay@2.0-service-sysfs

# NFC
PRODUCT_PACKAGES += \
    NQNfcNci \
    libnqnfc-nci \
    libnqnfc_nci_jni \
    nfc_nci.nqx.default \
    libp61-jcop-kit \
    com.nxp.nfc.nq \
    com.nxp.nfc.nq.xml \
    nqnfcee_access.xml \
    nqnfcse_access.xml \
    Tag \
    com.android.nfc_extras \
    vendor.nxp.hardware.nfc@1.2-service \
    nfc_nci.nqx.default.hw

PRODUCT_SOONG_NAMESPACES += vendor/nxp/opensource/pn5xx

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/nfc/libnfc-nci.conf:$(TARGET_COPY_OUT_SYSTEM)/etc/libnfc-nci.conf \
    $(LOCAL_PATH)/configs/nfc/libnfc-nxp.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-nxp.conf

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.hcef.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.hcef.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.xml \
    frameworks/native/data/etc/com.android.nfc_extras.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/com.android.nfc_extras.xml \
    frameworks/native/data/etc/com.nxp.mifare.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/com.nxp.mifare.xml

PRODUCT_PROPERTY_OVERRIDES += \
    ro.hardware.nfc_nci=nqx.default

# Ramdisk
PRODUCT_PACKAGES += \
    init.panel_info.sh \
    init.device.rc \
    fstab.qcom

# Recovery
PRODUCT_PACKAGES += \
    librecovery_updater_jason

# Telephony
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.radio.force_on_dc=true

# USB
PRODUCT_PACKAGES += \
    android.hardware.usb@1.0-service

# VNDK
PRODUCT_PACKAGES += \
    libstdc++.vendor

# Wifi
PRODUCT_PACKAGES += \
    readmac
