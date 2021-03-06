CC := gcc
SRC := main.c
EXE := simple-corr
OBJS:=$(SRC:.c=.o)

OPT += -O2
CFLAGS += -g -std=c99 -Wall -Wextra
LDFLAGS += -lm

all: Makefile $(EXE)

.PHONY: clean celan clena celna

$(EXE): $(OBJS)
	$(CC) $(OBJS) $(OPT) $(LDFLAGS) -o $@

%.o: %.c Makefile
	$(CC) $(CFLAGS) $(OPT) -c $< -o $@

tests: $(EXE)
	./$(EXE) gals_Mr19_10k.txt bins > output_10k.txt && echo "success!" || { echo "failed to run with 10k!"; exit 1; }
	diff -q output_10k.txt output_from_DD_10k_corrfunc_time_0p09s && echo  "success!" || { echo "results do not match with 10k!"; exit 1; }

	./$(EXE) gals_Mr19_10k.txt bins 1 > output_10k.txt && echo "success (logbins) !" || { echo "failed to run with 10k and logbins!"; exit 1; }
	diff -q output_10k.txt output_from_DD_10k_corrfunc_time_0p09s && echo  "success (logbins) !" || { echo "results do not match with 10k and logbins!"; exit 1; }

	./$(EXE) gals_Mr19_100k.txt bins > output_100k.txt && echo "success!" || { echo "failed to run with 100k!"; exit 1; }
	diff -q output_100k.txt output_from_DD_100k_corrfunc_time_0p51s && echo "success!" || { echo "results do not match with 100k!"; exit 1; }

	./$(EXE) gals_Mr19_100k.txt bins 1 > output_100k.txt && echo "success (logbins) !" || { echo "failed to run with 100k and logbins!"; exit 1; }
	diff -q output_100k.txt output_from_DD_100k_corrfunc_time_0p51s && echo "success (logbins)!" || { echo "results do not match with 100k and logbins!"; exit 1; }

celan celna clena:clean

clean:
	$(RM) $(OBJS) $(EXE)

