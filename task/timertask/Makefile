
COBJS :=timertask.o Timetask.o
OBJS += $(COBJS)

all:$(COBJS)

%.o:%.c
	@$(CC) -c $<  $(CFLAGS) $(DFLAGS) -o $(Rootdir)/$(output)/$@ 
$(COBJS):