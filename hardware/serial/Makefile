-include $(ROOTDIR)/autoconf.mk
COBJS-${Config_serial} +=serial.o
COBJS-${Config_UART} += UART.o
OBJS += $(COBJS-y)
all:$(COBJS-y)
include $(ROOTDIR)/rules.mk
