run : compile
	./main.out

compile:
	g++ main.cpp -o main.out

clean:
	rm main.out