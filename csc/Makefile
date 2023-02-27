.PHONY: out clean

out:
	cd ./code && make
	cd ..
	gcc ./code/fibo.o ./code/main.o -o fibo
	
clean:
	cd ./code && make clean