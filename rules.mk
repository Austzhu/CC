%.o:%.c
	$(CC)  $(CFLAGS) $(DFLAGS) -c -o $(ROOTDIR)/$(output)/$@  	$<
$(COBJS-y):
