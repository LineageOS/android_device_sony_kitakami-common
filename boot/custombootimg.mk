LOCAL_PATH := $(call my-dir)

uncompressed_ramdisk := $(PRODUCT_OUT)/ramdisk.cpio
$(uncompressed_ramdisk): $(INSTALLED_RAMDISK_TARGET)
	zcat $< > $@

recovery_uncompressed_ramdisk := $(PRODUCT_OUT)/ramdisk-recovery.cpio
recovery_uncompressed_device_ramdisk := $(PRODUCT_OUT)/ramdisk-recovery-device.cpio
$(recovery_uncompressed_device_ramdisk): $(MKBOOTFS) \
               $(INTERNAL_RECOVERYIMAGE_FILES) \
               $(recovery_initrc) $(recovery_sepolicy) $(recovery_kernel) \
               $(INSTALLED_2NDBOOTLOADER_TARGET) \
               $(recovery_build_prop) $(recovery_resource_deps) $(recovery_root_deps) \
               $(recovery_fstab) \
               $(RECOVERY_INSTALL_OTA_KEYS) \
               $(INTERNAL_BOOTIMAGE_FILES)
	$(call build-recoveryramdisk)
	@echo "----- Making uncompressed recovery ramdisk ------"
	$(hide) $(MKBOOTFS) $(TARGET_RECOVERY_ROOT_OUT) > $@
	$(hide) rm -f $(recovery_uncompressed_ramdisk)
	$(hide) cp $(recovery_uncompressed_device_ramdisk) $(recovery_uncompressed_ramdisk)

recovery_ramdisk := $(PRODUCT_OUT)/ramdisk-recovery.img
$(recovery_ramdisk): $(MINIGZIP) \
		$(recovery_uncompressed_device_ramdisk)
	@echo "----- Making compressed recovery ramdisk ------"
	$(hide) $(MINIGZIP) < $(recovery_uncompressed_ramdisk) > $@

INIT_SONY := $(PRODUCT_OUT)/utilities/init_sony

$(INSTALLED_BOOTIMAGE_TARGET): \
    $(PRODUCT_OUT)/kernel \
    $(uncompressed_ramdisk) \
    $(recovery_uncompressed_device_ramdisk) \
    $(INSTALLED_RAMDISK_TARGET) \
    $(INIT_SONY) \
    $(PRODUCT_OUT)/utilities/toybox \
    $(PRODUCT_OUT)/utilities/keycheck \
    $(MKBOOTIMG) $(MINIGZIP) \
    $(INTERNAL_BOOTIMAGE_FILES)

	@echo -e ${CL_CYN}"----- Making boot image ------"${CL_RST}
	$(hide) rm -fr $(PRODUCT_OUT)/combinedroot
	$(hide) cp -a $(PRODUCT_OUT)/root $(PRODUCT_OUT)/combinedroot
	$(hide) mkdir -p $(PRODUCT_OUT)/combinedroot/sbin

	$(hide) cp -n $(uncompressed_ramdisk) $(recovery_uncompressed_ramdisk)
	$(hide) cp $(recovery_uncompressed_ramdisk) $(PRODUCT_OUT)/combinedroot/sbin/
	$(hide) cp $(PRODUCT_OUT)/utilities/toybox $(PRODUCT_OUT)/combinedroot/sbin/toybox_init
	$(hide) cp $(PRODUCT_OUT)/utilities/keycheck $(PRODUCT_OUT)/combinedroot/sbin/

	$(hide) cp $(INIT_SONY) $(PRODUCT_OUT)/combinedroot/sbin/init_sony
	$(hide) chmod 755 $(PRODUCT_OUT)/combinedroot/sbin/init_sony
	$(hide) mv $(PRODUCT_OUT)/combinedroot/init $(PRODUCT_OUT)/combinedroot/init.real
	$(hide) ln -s sbin/init_sony $(PRODUCT_OUT)/combinedroot/init

	$(hide) $(MKBOOTFS) $(PRODUCT_OUT)/combinedroot/ > $(PRODUCT_OUT)/combinedroot.cpio
	$(hide) cat $(PRODUCT_OUT)/combinedroot.cpio | $(MINIGZIP) > $(PRODUCT_OUT)/combinedroot.fs

	$(hide) $(MKBOOTIMG) \
        --kernel $(PRODUCT_OUT)/kernel \
        --ramdisk $(PRODUCT_OUT)/combinedroot.fs \
        --cmdline "$(BOARD_KERNEL_CMDLINE)" \
        --base $(BOARD_KERNEL_BASE) \
        --pagesize $(BOARD_KERNEL_PAGESIZE) \
	$(BOARD_MKBOOTIMG_ARGS) \
        -o $(INSTALLED_BOOTIMAGE_TARGET)
	@echo -e ${CL_CYN}"Made boot image: $@"${CL_RST}

INSTALLED_RECOVERYIMAGE_TARGET := $(PRODUCT_OUT)/recovery.img
$(INSTALLED_RECOVERYIMAGE_TARGET): $(MKBOOTIMG) $(recovery_ramdisk) $(recovery_kernel)
	@echo -e ${CL_CYN}"----- Making recovery image ------"${CL_RST}
	$(call build-recoveryimage-target, $@)
	$(hide) $(MKBOOTIMG) \
        --kernel $(PRODUCT_OUT)/kernel \
        --ramdisk $(PRODUCT_OUT)/ramdisk-recovery.img \
        --cmdline "$(BOARD_KERNEL_CMDLINE)" \
        --base $(BOARD_KERNEL_BASE) \
        --pagesize $(BOARD_KERNEL_PAGESIZE) \
	$(BOARD_MKBOOTIMG_ARGS) \
        -o $(INSTALLED_RECOVERYIMAGE_TARGET)
	@echo -e ${CL_CYN}"Made recovery image: $@"${CL_RST}
