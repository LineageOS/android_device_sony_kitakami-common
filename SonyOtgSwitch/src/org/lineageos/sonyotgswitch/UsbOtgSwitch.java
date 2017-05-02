/*
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
package org.lineageos.sonyotgswitch;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.drawable.Icon;
import android.provider.Settings;
import android.service.quicksettings.Tile;
import android.service.quicksettings.TileService;

import cyanogenmod.providers.CMSettings;

import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;

import android.util.Log;

public class UsbOtgSwitch extends TileService {
    protected static final String OTG_SWITCH_ENABLED = "service.usb.otg.switch";
    private static boolean mUsbOtg = false;

    protected static void setSystemProperty(String key, String value) {
	    try {
		    Class.forName("android.os.SystemProperties").getMethod("set", String.class, String.class).invoke(null, key, value);
	    } catch (Exception e) {
		    e.printStackTrace();
	    }
    }


    @Override
    public void onStartListening() {
        super.onStartListening();
        refresh();
    }

    @Override
    public void onStopListening() {
        super.onStopListening();
    }

    @Override
    public void onClick() {
        super.onClick();
	if(mUsbOtg)
	{
		mUsbOtg = false;
		setSystemProperty(OTG_SWITCH_ENABLED, "false");
	        Log.d("OtgSwitch", "disable");
	}
	else 
	{
		mUsbOtg = true;
		setSystemProperty(OTG_SWITCH_ENABLED, "true");
	        Log.d("OtgSwitch", "enable");
	}
        refresh();
    }


    private void refresh() {
        if (mUsbOtg) {
            getQsTile().setIcon(Icon.createWithResource(this, R.drawable.ic_usb_otg_on));
            getQsTile().setState(Tile.STATE_ACTIVE);
        } else {
            getQsTile().setIcon(Icon.createWithResource(this, R.drawable.ic_usb_otg_off));
            getQsTile().setState(Tile.STATE_INACTIVE);
        }
        getQsTile().updateTile();
    }

}
