LITEOSTOPDIR := ../../../
LITEOSTOPDIR := $(realpath $(LITEOSTOPDIR))

# Common
C_SOURCES     += $(wildcard $(LITEOSTOPDIR)/kernel/src/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/kernel/src/mm/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/components/cpup/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/components/backtrace/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/components/bounds_checking_function/src/*.c) \
	             $(wildcard $(LITEOSTOPDIR)/components/fs/ramfs/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/components/fs/vfs/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/kal/posix/src/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/kal/posix/musl_src/string/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/utils/*.c) 

C_INCLUDES    += -I$(LITEOSTOPDIR)/utils \
                 -I$(LITEOSTOPDIR)/kernel/include \
                 -I$(LITEOSTOPDIR)/kal/posix/include \
                 -I$(LITEOSTOPDIR)/kal/cmsis \
                 -I$(LITEOSTOPDIR)/components/fs/vfs \
                 -I$(LITEOSTOPDIR)/components/fs/ramfs \
                 -I$(LITEOSTOPDIR)/components/fs/fatfs \
                 -I$(LITEOSTOPDIR)/components/cpup \
                 -I$(LITEOSTOPDIR)/components/backtrace \
                 -I$(LITEOSTOPDIR)/components/bounds_checking_function/include

#third party related
C_INCLUDES    += -I$(LITEOSTOPDIR)/third_party/bounds_checking_function/include \
                 -I$(LITEOSTOPDIR)/third_party/bounds_checking_function/src

C_SOURCES     += $(wildcard $(LITEOSTOPDIR)/third_party/bounds_checking_function/src/*.c)

# NMSIS related
C_INCLUDES    += -I$(LITEOSTOPDIR)/kernel/arch/risc-v/nuclei/gcc/nmsis/Core/Include \
                 -I$(LITEOSTOPDIR)/kernel/arch/risc-v/nuclei/gcc/nmsis/DSP/Include \
                 -I$(LITEOSTOPDIR)/kernel/arch/risc-v/nuclei/gcc/nmsis/NN/Include

ASM_SOURCES   += $(wildcard $(LITEOSTOPDIR)/kernel/arch/risc-v/nuclei/gcc/*.s)

ASMS_SOURCES  += $(wildcard $(LITEOSTOPDIR)/kernel/arch/risc-v/nuclei/gcc/*.S)

C_SOURCES     += $(wildcard $(LITEOSTOPDIR)/kernel/arch/risc-v/nuclei/gcc/*.c)

C_INCLUDES    += -I. \
                 -I$(LITEOSTOPDIR)/kernel/arch/include \
                 -I$(LITEOSTOPDIR)/kernel/arch/risc-v/nuclei/gcc

# list of ASM .S program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASMS_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASMS_SOURCES)))

$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@
