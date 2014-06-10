Instrucciones de uso:

./client ARG1

ARG1:
	Para locale (ARG4==1||ARG4==3):
		Debe contener el protocolo, la interfaz y el puerto que se utilizaran en la prueba, ej:
			tcp://wlan0:5555

Ejemplo de uso (para medicion de latencia):
	Para modo local:
		./netDaemon tcp://wlan0:5555

Limitaciones:
	El demo ha sido probado solo con TCP en su totalidad, al parecer UDP no es soportado 
	por la funcion que utilizamos para el binding de los puertos, zmq_bind, trabajaremos 
	para solucionar esto de ser posible.

	Tama;o de mensaje para mensajes peque;os:
		Max: 29 bytes (esta definido dentro del codigo fuente de ZeroMQ)
