# Bluetooth configurations
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/bluetooth/bt_vendor.conf:system/etc/bluetooth/bt_vendor.conf

# Bluetooth properties
PRODUCT_PROPERTY_OVERRIDES += \
    bluetooth.enable_timeout_ms=12000 \
    ro.bluetooth.hfp.ver=1.6 \
    ro.bt.bdaddr_path=/data/etc/bluetooth_bdaddr
