# Recovery configurations
TARGET_RECOVERY_FSTAB := $(COMMON_PATH)/rootdir/fstab.qcom
TARGET_USERIMAGES_USE_EXT4 := true

# Releasetools
TARGET_RECOVERY_UPDATER_LIBS := librecovery_updater_kitakami
TARGET_RELEASETOOLS_EXTENSIONS := $(COMMON_PATH)
