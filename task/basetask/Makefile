
COBJS :=task.o  task_list.o
OBJS += $(COBJS)

all:$(COBJS)

%.o:%.c
	@$(CC) -c $<  $(CFLAGS) $(DFLAGS) -o $(Rootdir)/$(output)/$@ 
$(COBJS):