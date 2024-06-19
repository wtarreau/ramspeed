CC         := gcc
CFLAGS     := -O3 -Wall -fomit-frame-pointer
OBJS       := ramlat rambw ramwalk

all: $(OBJS)

%: %.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	rm -f $(OBJS) *.o *~
