# Camera packages
PRODUCT_PACKAGES += \
    Snap

# Camera SHIM packages
PRODUCT_PACKAGES += \
    camera.qcom_shim

# Camera flash LED config
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/configs/flashled_calc_parameters.cfg:system/etc/flashled_calc_parameters.cfg

# Camera properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.qc.sdk.gestures.camera=false \
    ro.qc.sdk.camera.facialproc=false \
    camera.disable_zsl_mode=1
