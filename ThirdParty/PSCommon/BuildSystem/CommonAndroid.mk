# Check if we're building from OS or NDK
ifdef TARGET_BUILD_VARIANT
	PS_OS_BUILD := true
else
	PS_NDK_BUILD := true
endif

# Setup arm flags
LOCAL_CFLAGS += -O3 -ftree-vectorize -ffast-math -funroll-loops -fPIC -fvisibility=hidden

ifeq ($(ARCH_ARM_HAVE_NEON),true)
	LOCAL_CFLAGS += -DHAVE_NEON=1 -DXN_NEON -flax-vector-conversions
	LOCAL_ARM_NEON := true
endif
