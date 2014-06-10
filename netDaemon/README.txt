Instrucciones de instalacion:
	Dependencias requeridas:
		ZeroMQ: libzmq

		ZeroMQ es la biblioteca usada para manejar las funciones de red.

	Para compilar:
		Reemplazar client con el nombre deseado y en -L/XXX reemplazar por la ubicacion de la biblioteca ZeroMQ:
		Utilizando Clang:
			clang -Wall netDaemon.cpp -o netDaemon -L/usr/local/lib -lzmq

		Si desea usar g++ como alternativa a Clang para compilar:
			g++ -Wall netDaemon.cpp -o netDaemon -L/usr/local/lib -lzmq

Instrucciones de uso:

./client ARG1

ARG1:
	Debe contener el protocolo, la interfaz y el puerto que se utilizaran en la prueba, ej:
		tcp://wlan0:5555

Ejemplo de uso:
	./netDaemon tcp://wlan0:5555

Limitaciones:
	El demo ha sido probado solo con TCP en su totalidad, al parecer UDP no es soportado 
	por la funcion que utilizamos para el binding de los puertos, zmq_bind, trabajaremos 
	para solucionar esto de ser posible.

	Tama;o de mensaje para mensajes peque;os:
		Max: 29 bytes (esta definido dentro del codigo fuente de ZeroMQ)
