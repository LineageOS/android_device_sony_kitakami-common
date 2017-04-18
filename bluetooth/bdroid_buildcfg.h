/*
 * Copyright (C) 2016 The CyanogenMod Project
 * Copyright (C) 2017 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _BDROID_BUILDCFG_H
#define _BDROID_BUILDCFG_H

#if !defined(OS_GENERIC)
#include <cutils/properties.h>
#include <string.h>

static inline const char* getBTDefaultName()
{
    char device[PROPERTY_VALUE_MAX];
    property_get("ro.product.device", device, "");

    if ((!strcmp("ivy", device))||(!strcmp("ivy_dsds", device))) {
        return "Xperia Z3+";
    } else if ((!strcmp("karin", device))||(!strcmp("karin_windy", device))) {
        return "Xperia Z4 Tablet";
    } else if ((!strcmp("sumire", device))||(!strcmp("sumire_dsds", device))) {
        return "Xperia Z5";
    } else if (!strcmp("suzuran", device)) {
        return "Xperia Z5 Compact";
    } else if ((!strcmp("satsuki", device))||(!strcmp("satsuki_dsds", device))) {
        return "Xperia Z5 Premium";
    }

    return "Xperia";
}

#define BTM_DEF_LOCAL_NAME getBTDefaultName()
#endif // OS_GENERIC

#define BTM_WBS_INCLUDED     TRUE

#endif
