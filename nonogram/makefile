all:
	g++ -o nonogram nonogram.cc inference.cc -O3

clean:
	rm -rf *.o nonogram

debug:
	g++ -o nonogram nonogram.cc inference.cc -g -D _DEBUG

test:
	g++ -o nonogramtest nonogramtest.cc inference.cc -g

