%.o:%.c
	$(CC) $(CFLAGS) $(DFLAGS) -c -o $(Rootdir)/$(output)/$@  	$<
$(COBJS-y):
