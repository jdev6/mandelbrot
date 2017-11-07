OBJS = $(patsubst %.c,%.o,$(shell find . -name "*.c"))
LIBS = -lallegro -lallegro_font -lallegro_primitives -lm
OUT = ./mandelbrot
CFLAGS = -Ofast -funroll-loops -funsafe-math-optimizations -faggressive-loop-optimizations -Wall

# link
$(OUT): $(OBJS) $(MAPS)
	@echo "[INFO] linking"
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(OUT)

debug: CFLAGS += -ggdb
debug: $(OUT)

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

# compile and generate dependency info
%.o: %.c
	@echo "[INFO] generating deps for $*.o"
	gcc -MM $(CFLAGS) $*.c > $*.d
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	@echo

# remove compilation products
clean:
	$(RM) $(OBJS) $(DEPS)

