CC         := gcc
CFLAGS     := -O3 -Wall -fomit-frame-pointer
OBJS       := ramlat rambw ramwalk

all: $(OBJS)

%: %.o
	$(CC) $(LDFLAGS) -o $@ $^

rambw: rambw.o
	$(CC) $(LDFLAGS) -o $@ $^ -pthread

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	rm -f $(OBJS) *.o *~
