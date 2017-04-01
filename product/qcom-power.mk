# Power HAL package
PRODUCT_PACKAGES += \
    power.msm8994

# QCOM-perf properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.min_freq_0=384000 \
    ro.min_freq_4=384000 \
    ro.vendor.extension_library=libqti-perfd-client.so
