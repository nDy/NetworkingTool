Instrucciones de instalacion:
	Dependencias requeridas:
		ZeroMQ: libzmq

		ZeroMQ es la biblioteca usada para manejar las funciones de red.

	Para compilar:
		Reemplazar client con el nombre deseado y en -L/XXX reemplazar por la ubicacion de la biblioteca ZeroMQ:
		Utilizando Clang:
			clang -Wall ProyectoRedes.cpp -o client -L/usr/local/lib -lzmq

		Si desea usar g++ como alternativa a Clang para compilar:
			g++ -Wall ProyectoRedes.cpp -o client -L/usr/local/lib -lzmq

Instrucciones de uso:

./client ARG1 ARG2 ARG3 ARG4

ARG1:
	Para locale (ARG4==1||ARG4==3):
		Debe contener el protocolo, la interfaz y el puerto que se utilizaran en la prueba, ej:
			tcp://wlan0:5555

	Para remote (ARG4==2||ARG4==4):
		Debe contener el protocolo, la direccion IP en donde se ejecuta el cliente en modo local
		y el puerto que se utilizaran en la prueba, ej:
			tcp://192.168.0.108:5555

ARG2:
	Tamaño de los paquetes a enviar/recibir en Bytes
ARG3:
	Numero de paquetes a enviar/recibir
ARG4:
	Bandera para indicar que operacion se desea realizar con el programa, los valores que recibe son:
		1: Abre el programa en modo local (recibe datos) para medicion de latencia.
		2: Abre el programa en modo remoto (envia datos) para medicion de latencia
		3: Abre el programa en modo local (recibe datos) para medicion de Throughput.
		4: Abre el programa en modo remoto (envia datos) para medicion de Throughput.
		5: Abre el programa en modo local (recibe datos) para medicion de Perdida de paquetes.
		6: Abre el programa en modo remoto (envia datos) para medicion de Perdida de paquetes.

Ejemplo de uso (para medicion de latencia):
	Para modo local:
		./client tcp://wlan0:5555 1 1000 1
	Para modo remoto:
		./client tcp://192.168.0.108:5555 1 1000 2

Limitaciones:
	El demo ha sido probado solo con TCP en su totalidad, al parecer UDP no es soportado 
	por la funcion que utilizamos para el binding de los puertos, zmq_bind, trabajaremos 
	para solucionar esto de ser posible.
