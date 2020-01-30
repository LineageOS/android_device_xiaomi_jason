#!/bin/bash
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

set -e

export DEVICE=jason
export DEVICE_COMMON=sdm660-common
export VENDOR=xiaomi

export DEVICE_BRINGUP_YEAR=2017

./../../$VENDOR/$DEVICE_COMMON/extract-files.sh "$@"

MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "$MY_DIR" ]]; then MY_DIR="$PWD"; fi

DEVICE_BLOB_ROOT="$MY_DIR"/../../../vendor/"$VENDOR"/"$DEVICE"/proprietary

sed -i 's/\x1e\x40\x9a\x99\x99\x99\x99\x99\x3b\x40\x10/\x1e\x40\x9a\x99\x99\x99\x99\x99\x3b\x40\x01/' \
    "$DEVICE_BLOB_ROOT"/vendor/lib/libmmcamera_jason_s5k3p8sp_sunny.so

patchelf --remove-needed libandroid.so "$DEVICE_BLOB_ROOT"/vendor/lib/libmmcamera2_stats_modules.so
patchelf --remove-needed libandroid.so "$DEVICE_BLOB_ROOT"/vendor/lib/libmpbase.so
patchelf --remove-needed libgui.so "$DEVICE_BLOB_ROOT"/vendor/lib/libmmcamera_ppeiscore.so
patchelf --remove-needed libgui.so "$DEVICE_BLOB_ROOT"/vendor/lib/libmmcamera2_stats_modules.so
