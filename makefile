test:
	gcc tests.c ./libs/CuTest.c -o test

repl:
	gcc sme_repl.c -o repl