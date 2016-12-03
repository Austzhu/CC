-include $(Rootdir)/autoconf.mk
COBJS-y :=ether.o
COBJS-${Config_TCP_Server} += ether_server.o
OBJS += $(COBJS-y)
all:$(COBJS-y)
include $(Rootdir)/rules.mk

