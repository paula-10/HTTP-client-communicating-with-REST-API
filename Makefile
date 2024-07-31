client: client.cpp requests.cpp helpers.cpp buffer.cpp
	g++ -o client client.cpp requests.cpp helpers.cpp buffer.cpp -Wall

run: client
	./client

clean:
	rm -f client
