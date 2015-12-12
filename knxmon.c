/**
 *
 * knxmon.c
 *
 * KNX bus monitor with buffer
 *
 * knxmon maintains three shared memory segments and a message buffer
 * to communicate with otehr processes.
 *
 * Shared Memory Segment #0:	COM Table with sizes of the following three
 *				shared memory segments
 *				-> COMtable, -> int	*sizeTable
 * Shared Memory Segment #1:	OPC Table with value buffer
 *				-> OPCtable, -> node	*data (copy from _data)
 * Shared Memory Segment #2:	KNX Group Address value buffer
 *				-> KNXtable, -> int	*ints AND float	*floats
 * Shared Memory Segment #3:	Fixes size buffer of 256 bytes to communicate
 *				buffer sizes for Shared Memory Segment #1 and #2.
 *				->CRFtable, int		*crf
 *
 * Revision history
 *
 * Date		Rev.	Who	what
 * -----------------------------------------------------------------------------
 * 2015-11-20	PA1	khw	inception;
 * 2015-11-24	PA2	khw	added semaphores for shared memory
 *				and message queue;
 * 2015-11-26	PA3	khw	added mylib function for half-float
 *				conversions;
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<strings.h>
#include	<time.h>
#include	<math.h>
#include	<sys/types.h>
#include	<sys/ipc.h> 
#include	<sys/shm.h> 
#include	<sys/msg.h> 

#include	"nodeinfo.h"
#include	"nodedata.h"
#include	"knxprot.h"
#include	"knxbridge.h"

#include	"eib.h"		// rs232.c will differentiate:
				// ifdef  __MAC__
				// 	simulation
				// else
				// 	real life
#include	"mylib.h"

extern	void	help() ;

char	progName[64] ;
int	debugLevel	=	0 ;

#ifdef	__MACH__
#else
#endif
int	main( int argc, char *argv[]) {
		int	opt ;
		int	status		=	0 ;
		int	sleepTimer	=	0 ;
		int	i ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
	/**
	 * define shared memory segment #0: COM Table
	 *	this segment holds information about the sizes of the other tables
 	 */
		key_t	shmCOMKey	=	SHM_COM_KEY ;
		int	shmCOMFlg	=	IPC_CREAT | 0666 ;
		int	shmCOMId ;
		int	shmCOMSize	=	256 ;
		int	*sizeTable ;
	/**
	 * define shared memory segment #1: OPC Table with buffer
	 *	this segment holds the structure defined in nodedata.h
 	 */
		key_t	shmOPCKey	=	SHM_OPC_KEY ;
		int	shmOPCFlg	=	IPC_CREAT | 0666 ;
		int	shmOPCId ;
		int	shmOPCSize ;
		node	*data ;
	/**
	 * define shared memory segment #2: KNX Table with buffer
	 *	this segment holds the KNX table defined in nodedata.h
 	 */
		key_t	shmKNXKey	=	SHM_KNX_KEY ;
		int	shmKNXFlg	=	IPC_CREAT | 0666 ;
		int	shmKNXId ;
		int	shmKNXSize	=	65536 * sizeof( float) ;
		float	*floats ;
		int	*ints ;
	/**
	 * define shared memory segment #3: CRF Table with buffer
	 *	this segment holds the cross-reference-table
 	 */
		key_t	shmCRFKey	=	SHM_CRF_KEY ;
		int	shmCRFFlg	=	IPC_CREAT | 0666 ;
		int	shmCRFId ;
		int	shmCRFSize	=	65536 * sizeof( int) ;
		int	*crf ;
	/**
	 * define message queue #1: OPC Table with buffer
 	 */
		key_t	shmMSGKey	=	SHM_MSG_KEY ;
		int	msgQueue ;
	struct	msqid_ds	msgQueueInfo ;
		msgBuf	msgBuffer ;
	/**
	 * variables needed for the reception of EIB message
	 */
			FILE	*file ;
	unsigned	char	buf, myBuf[64] ;
			node	*actData ;
			int	rcvdBytes ;
			int	checksumError ;
			int	cport_nr	=	22 ;	// /dev/ttyS0 (COM1 on windows) */
			int	bdrate		=	19200 ;	// 9600 baud */
			int	adrBytes ;
			int	n; 				// holds number of received characters
	unsigned        int     control ;
	unsigned        int     addressType ;
	unsigned        int     routingCount ;
		int     expectedLength ;
			float	value ;
	unsigned        int     checkSum ;
	unsigned        char    checkS1 ;
			char    *ptr ;
		msgBuf  buffer ;
			knxMsg	myMsgBuf ;
			knxMsg	*myMsg ;
	/**
	 *
	 */
	strcpy( progName, *argv) ;
	printf( "%s: starting up ... \n", progName) ;
	/**
	 * get command line options
	 */
	while (( opt = getopt( argc, argv, "d:?")) != -1) {
		switch ( opt) {
		case	'd'	:
			debugLevel	=	atoi( optarg) ;
			break ;
		case	'?'	:
			help() ;
			exit(0) ;
			break ;
		default	:
			help() ;
			exit( -1) ;
			break ;
		}
	}
	/**
	 * setup the shared memory for COMtable
	 */
	if ( debugLevel > 0) {
		printf( "trying to obtain shared memory COMtable ... \n") ;
	}
	if (( shmCOMId = shmget( shmCOMKey, shmCOMSize, IPC_CREAT | 0666)) < 0) {
		perror( "knxmon: shmget failed for COMtable");
		exit( -1) ;
	}
	if ( debugLevel > 0) {
		printf( "trying to attach shared memory COMtable \n") ;
	}
	if (( sizeTable = (int *) shmat( shmCOMId, NULL, 0)) == (int *) -1) {
		perror( "knxmon: shmat failed for COMtable");
		exit( -1) ;
	}
	if ( debugLevel > 10) {
		printf( "shmCOMKey........: %d\n", shmCOMKey) ;
		printf( "  shmCOMSize.....: %d\n", shmCOMSize) ;
		printf( "  shmCOMId.......: %d\n", shmCOMId) ;
		printf( "  Address..: %8lx\n", (unsigned long) data) ;
	}
	sizeTable[ SIZE_COM_TABLE]	=	256 ;
	sizeTable[ SIZE_KNX_TABLE]	=	shmKNXSize ;
	sizeTable[ SIZE_CRF_TABLE]	=	shmCRFSize ;
	/**
	 * setup the shared memory for OPCtable
	 */
	if ( debugLevel > 0) {
		printf( "trying to obtain shared memory OPCtable ... \n") ;
	}
	shmOPCSize	=	sizeof( _data) ;
	sizeTable[ SIZE_OPC_TABLE]	=	shmOPCSize ;
	if (( shmOPCId = shmget (shmOPCKey, shmOPCSize, shmOPCFlg)) < 0) {
		perror( "knxmon: shmget failed for OPCtable");
		exit(1);
	}
	if ( debugLevel > 0) {
		printf( "trying to attach shared memory OPCtable \n") ;
	}
	if (( data = (node *) shmat(shmOPCId, NULL, 0)) == (node *) -1) {
		perror( "knxmon: shmat failed for OPCtable");
		exit(1);
	}
	if ( debugLevel > 10) {
		printf( "shmOPCKey........: %d\n", shmOPCKey) ;
		printf( "  shmOPCSize.....: %d\n", shmOPCSize) ;
		printf( "  shmOPCId.......: %d\n", shmOPCId) ;
		printf( "  Address..: %8lx\n", (unsigned long) data) ;
	}
	memcpy( data, _data, sizeof( _data)) ;
	/**
	 * setup the shared memory for KNXtable
	 */
	if ( debugLevel > 0) {
		printf( "trying to obtain shared memory KNXtable ... \n") ;
	}
	if (( shmKNXId = shmget( shmKNXKey, shmKNXSize, IPC_CREAT | 0666)) < 0) {
		perror( "knxmon: shmget failed for KNXtable");
		exit( -1) ;
	}
	if ( debugLevel > 0) {
		printf( "trying to attach shared memory KNX table \n") ;
	}
	if (( floats = (float *) shmat( shmKNXId, NULL, 0)) == (float *) -1) {
		perror( "knxmon: shmat failed for KNXtable");
		exit( -1) ;
	}
	if ( debugLevel > 10) {
		printf( "shmKNXKey........: %d\n", shmKNXKey) ;
		printf( "  shmKNXSize.....: %d\n", shmKNXSize) ;
		printf( "  shmKNXId.......: %d\n", shmKNXId) ;
	}
	ints	=	(int *) floats ;
	/**
	 * setup the shared memory for CRFtable
	 */
	if ( debugLevel > 0) {
		printf( "trying to obtain shared memory CRFtable ... \n") ;
	}
	if (( shmCRFId = shmget( shmCRFKey, shmCRFSize, IPC_CREAT | 0666)) < 0) {
		perror( "knxmon: shmget failed for CRFtable");
		exit( -1) ;
	}
	if ( debugLevel > 0) {
		printf( "trying to attach shared memory CRFtable... \n") ;
	}
	if (( crf = (int *) shmat( shmCRFId, NULL, 0)) == (int *) -1) {
		perror( "knxmon: shmat failed for CRFtable");
		exit( -1) ;
	}
	if ( debugLevel > 10) {
		printf( "shmCRFKey........: %d\n", shmCRFKey) ;
		printf( "  shmCRFSize.....: %d\n", shmCRFSize) ;
		printf( "  shmCRFId.......: %d\n", shmCRFId) ;
	}
	/**
	 * build the cross-reference table for the KNX group numbers
	 */
	createCRF( data, lastDataIndex, crf, (void *) floats) ;
	for ( i=0 ; i < 65536 ; i++) {
		if ( crf[i] != 0) {
			printf( "KNX Group Address %d is assigned to %d \n", i, crf[i]) ;
		}
	}
	/**
	 *
	 */
	printf( "trying to attach message queue ... \n") ;
	if (( msgQueue = msgget( shmMSGKey, IPC_CREAT | 0666)) < 0) {
		printf( "could not attach message queue ... \n") ;
		exit( -1) ;
	}
	msgctl( msgQueue, IPC_STAT, &msgQueueInfo) ;
	printf( "  Bytes....: %8lx\n", (unsigned long) msgQueueInfo.msg_qbytes) ;
	printf( "  Messages.: %8lx\n", ((unsigned long) msgQueueInfo.msg_qbytes) /  sizeof( msgBuf)) ;
	/**
	 *
	 */
	dumpDataAll( data, lastDataIndex, (void *) floats) ;
	eibOpen() ;
	sleepTimer	=	0 ;
	while ( 1) {
		myMsg	=	eibReceive( &myMsgBuf) ;
#ifdef	__MACH__
#else
#endif
		if ( myMsg != NULL) {
			sleepTimer	=	0 ;
			eibDump( "Monitor received ...:\n", myMsg) ;
			if ( crf[ myMsg->rcvAdr] != 0) {
				if ( debugLevel >= 3) {
					printf( "received a known group address ... \n") ;
				}
				actData	=	&data[ crf[ myMsg->rcvAdr]] ;
				switch ( myMsg->tlc) {
				case	0x00	:		// UDP
				case	0x01	:		// NDP
					switch ( myMsg->apci) {
					case	0x02	:	// groupValueWrite
						printf( "WRITING ... \n") ;
						printf( ".......%s \n", actData->name) ;
						actData->knxHWAdr	=	myMsg->sndAdr ;
						switch ( actData->type) {
						case	dtBit	:
							printf( "Assigning BIT \n") ;
							value	=	myMsg->mtext[1] & 0x01 ;
							actData->val.i	=	value ;
							ints[ myMsg->rcvAdr]	=	value ;
							break ;
						case	dtFloat2	:
							printf( "Assigning HALF-FLOAT \n") ;
							value	=	hfbtf( &myMsg->mtext[2]) ;
							actData->val.f	=	value ;
							floats[ myMsg->rcvAdr]	=	value ;
							break ;
						default	:
							break ;
						}
						break ;
					}
					break ;
				case	0x02	:		// UCD
					break ;
				case	0x03	:		// NCD
					break ;
				}
			} else {
				printf( "received an un-known group address [%5d]... \n", myMsg->rcvAdr) ;
			}
		} else if ( msgrcv( msgQueue, &msgBuffer, MSG_SIZE, 1, IPC_NOWAIT) >= 0) {
				printf( "Message received, priority...........: 1\n") ;
				printf( "Type.......: %ld \n", msgBuffer.mtype) ;
				printf( "Message....: \n") ;
				printf( "  Group....: %ld \n", msgBuffer.group) ;
				printf( "  Value....: %d \n", msgBuffer.val.i) ;
				eibOpen() ;
				eibWriteBit( MY_HW_ADDRESS_DEC, msgBuffer.group, msgBuffer.val.i, 0) ;
#ifdef	__MACH__
#else
#endif
		} else if ( msgrcv( msgQueue, &msgBuffer, MSG_SIZE, 2, IPC_NOWAIT) >= 0) {
				printf( "Message received, priority...........: 2\n") ;
				printf( "Type.......: %ld \n", msgBuffer.mtype) ;
				printf( "Message....: \n") ;
				printf( "  Group....: %ld \n", msgBuffer.group) ;
				printf( "  Value....: %d \n", msgBuffer.val.i) ;
				eibWriteBit( MY_HW_ADDRESS_DEC, msgBuffer.group, msgBuffer.val.i, 0) ;
#ifdef	__MACH__
#else
#endif
		} else if ( msgrcv( msgQueue, &msgBuffer, MSG_SIZE, 3, IPC_NOWAIT) >= 0) {
				printf( "Message received, priority...........: 3\n") ;
				printf( "Type.......: %ld \n", msgBuffer.mtype) ;
				printf( "Message....: \n") ;
				printf( "  Group....: %ld \n", msgBuffer.group) ;
				printf( "  Value....: %d \n", msgBuffer.val.i) ;
				eibWriteBit( MY_HW_ADDRESS_DEC, msgBuffer.group, msgBuffer.val.i, 0) ;
#ifdef	__MACH__
#else
#endif
		} else {
			sleepTimer++ ;
			if ( sleepTimer > 1)
				sleepTimer	=	1 ;
			printf( "will go to sleep ... for %d seconds\n", sleepTimer) ;
			sleep( sleepTimer) ;
		}
	}
	exit( status) ;
}

void	help() {
}
