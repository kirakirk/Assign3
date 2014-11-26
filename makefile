.PHONY all:
all:
	gcc -Wall -DPART1 3_2.c -o diskinfo
	gcc -Wall -DPART2 3_2.c -o disklist
	gcc -Wall -DPART3 3_2.c -o diskget
	gcc -Wall -DPART4 3_2.c -o diskput

.PHONY clean:
clean:
	-rm diskinfo disklist diskget diskput