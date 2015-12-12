all:	knxbridge sendbit knxmon hdlsolar hdlpellet hdlheating setval sendval sendbit sendint sendfloat sendtime readbit tf2

knxbridge:	knxbridge.o eib.o rs232.o
	gcc knxbridge.o eib.o mylib.o rs232.o -lm -o knxbridge

knxmon:	knxmon.o mylib.o rs232.o eib.o
	gcc knxmon.o mylib.o rs232.o eib.o -lm -o knxmon

hdlsolar:	hdlsolar.o	 mylib.o
	gcc hdlsolar.o mylib.o -lm -o hdlsolar

hdlpellet:	hdlpellet.o eib.o mylib.o
	gcc hdlpellet.o eib.o mylib.o -lm -o hdlpellet

hdlheating:	hdlheating.o	 mylib.o
	gcc hdlheating.o mylib.o -lm -o hdlheating

setval:	setval.o	
	gcc setval.o -o setval

sendval:	sendval.o	
	gcc sendval.o -o sendval

son:	son.o rs232.o
	gcc son.o rs232.o -o son

sendbit:	sendbit.o eib.o mylib.o
	gcc sendbit.o eib.o mylib.o -lm -o sendbit

sendint:	sendint.o eib.o mylib.o
	gcc sendint.o eib.o mylib.o -lm -o sendint

sendfloat:	sendfloat.o eib.o mylib.o
	gcc sendfloat.o eib.o mylib.o -lm -o sendfloat

sendtime:	sendtime.o eib.o mylib.o
	gcc sendtime.o eib.o mylib.o -lm -o sendtime

readbit:	readbit.o eib.o mylib.o
	gcc readbit.o eib.o mylib.o -lm -o readbit

tf2:	tf2.o mylib.o
	gcc tf2.o mylib.o -lm -o tf2

knxbridge.o:	knxbridge.c rs232.h eib.h knxbridge.h
	gcc -c knxbridge.c

knxmon.o:	knxmon.c nodeinfo.h nodedata.h knxprot.h
	gcc -c knxmon.c

hdlsolar.o:	hdlsolar.c nodeinfo.h
	gcc -c hdlsolar.c

hdlpellet.o:	hdlpellet.c nodeinfo.h
	gcc -c hdlpellet.c

hdlheating.o:	hdlheating.c nodeinfo.h
	gcc -c hdlheating.c

setval.o:	setval.c nodeinfo.h
	gcc -c setval.c

sendval.o:	sendval.c nodeinfo.h
	gcc -c sendval.c

son.o:	son.c rs232.h
	gcc -c son.c

sendbit.o:	sendbit.c eib.h knxprot.h mylib.h
	gcc -c sendbit.c

sendint.o:	sendint.c rs232.h mylib.h
	gcc -c sendint.c

sendfloat.o:	sendfloat.c eib.h knxprot.h mylib.h
	gcc -c sendfloat.c

sendtime.o:	sendtime.c eib.h knxprot.h mylib.h
	gcc -c sendtime.c

readbit.o:	readbit.c rs232.h mylib.h
	gcc -c readbit.c

tf2.o:	tf2.c mylib.h
	gcc -c tf2.c

mylib.o:	mylib.c nodeinfo.h
	gcc -c mylib.c

rs232.o:	rs232.c
	gcc -c rs232.c

eib.o:	eib.c
	gcc -c eib.c
