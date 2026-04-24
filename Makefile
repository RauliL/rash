default :
	$(CXX) -g -Wall -Werror rash.cpp -o rash

clean :
	rm -f ./rash
