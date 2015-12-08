cexpr_printf: Makefile cexpr_printf.cpp
	clang++ -std=c++14 -Wall -Werror -ggdb3 -O2 -o cexpr_printf cexpr_printf.cpp

ro_data_view: cexpr_printf
	objdump -s -j .rodata cexpr_printf

clean:
	rm -f cexpr_printf
