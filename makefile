test:
	gcc tests.c ./libs/CuTest.c -o test
	./test.exe
	rm ./test.exe

demo:
	gcc sme_repl.c -o repl