# Audio configurations
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/audio/audio_effects.conf:system/vendor/etc/audio_effects.conf \
    $(COMMON_PATH)/audio/audio_output_policy.conf:system/vendor/etc/audio_output_policy.conf \
    $(COMMON_PATH)/audio/audio_platform_info.xml:system/etc/audio_platform_info.xml \
    $(COMMON_PATH)/audio/audio_policy.conf:system/etc/audio_policy.conf

# Audio properties
PRODUCT_PROPERTY_OVERRIDES += \
    tunnel.audio.encode=false \
    audio.deep_buffer.media=true \
    audio.offload.gapless.enabled=true \
    audio.offload.buffer.size.kb=32 \
    audio.offload.video=false \
    audio.offload.pcm.16bit.enable=true \
    audio.offload.passthrough=false \
    audio.offload.multiple.enabled=true \
    audio.offload.pcm.24bit.enable=true \
    audio.offload.track.enable=true \
    persist.audio.fluence.voicecall=true \
    persist.audio.fluence.voicerec=false \
    persist.audio.fluence.speaker=true \
    ro.qc.sdk.audio.fluencetype=none \
    ro.qc.sdk.audio.ssr=false \
    persist.audio.ssr.3mic=false \
    use.voice.path.for.pcm.voip=true \
    persist.speaker.prot.enable=false \
    persist.spkr.cal.duration=0 \
    qcom.hw.aac.encoder=true
