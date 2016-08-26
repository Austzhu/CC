
COBJS :=meter_crc.o meter_cmdrdwr.o
OBJS += $(COBJS)

all:$(COBJS)

%.o:%.c
	@$(CC) -c $<  $(CFLAGS) $(DFLAGS) -o $(Rootdir)/$(output)/$@ 
$(COBJS):