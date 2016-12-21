-include $(ROOTDIR)/autoconf.mk
COBJS-${Config_ether} :=ether.o
COBJS-${Config_TCP_Server} += ether_server.o
OBJS += $(COBJS-y)
all:$(COBJS-y)
include $(ROOTDIR)/rules.mk

