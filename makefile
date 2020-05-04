test: test.cc double.h linear.h bucket.h
	g++ -o test -fmax-errors=5 test.cc

debug: test.cc double.h linear.h bucket.h
	g++ -g -o debug -fmax-errors=5 test.cc

clean:
	rm *.exe test debug