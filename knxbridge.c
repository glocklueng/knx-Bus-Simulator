/**
 * Copyright (c) 2015 wimtecc, Karl-Heinz Welter
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/**
 *
 * knxbridge.c
 *
 * KNX bridge process
 *
 * knxbridge bridges a "simulated" knx-bus to a "real-world" knx-bus 
 * through a TPUART or, in
 * a purely simulated mode, e.g. on Mac OS, acts as a virtual TPUART which copies
 * everything supposed to be transmitted to the real-orld to the incoming side.
 * the communication towards other modules happens through two separate message queue.
 * the message queues contain a complete message (see: eib.h)
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
 * 2015-12-03	PA4	khw	derived from knx R1
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

#include	"rs232.h"
#include	"knxbridge.h"
#include	"eib.h"

extern	void	help() ;

char	progName[64] ;
int	debugLevel ;

int	main( int argc, char *argv[]) {
	int	sendingByte ;
	int	opt ;
	int	sleepTimer	=	0 ;
	int	incompleteMsg ;
	int	rcvdLength ;
	int	sentLength ;
	int	expLength ;
	bridgeModeRcv	rcvMode ;
	bridgeModeSnd	sndMode ;
	char	mode[]={'8','e','1',0};
	int	cport_nr	=	22 ;	// /dev/ttyS0 (COM1 on windows) */
	int	bdrate		=	19200 ;	// 9600 baud */
	int	rcvdBytes ;
	unsigned	char	buf, bufp ;
	unsigned	char	*rcvData ;
	unsigned	char	*sndData ;
	/**
	 * define receive message queue #1: queue for incoming (from the real bus) messages
 	 */
		key_t	shmRcvQueueKey	=	SHM_RCVMSG_KEY ;
		int	msgRcvQueue ;
	struct	msqid_ds	msgRcvQueueInfo ;
		knxMsg	msgRcv ;
	/**
	 * define receive message queue #2: queue for outgoing (to the real bus) messages
 	 */
		key_t	shmSndQueueKey	=	SHM_SNDMSG_KEY ;
		int	msgSndQueue ;
	struct	msqid_ds	msgSndQueueInfo ;
		knxMsg	msgSnd ;
		knxMsg	*msgToSnd ;
	/**
	 * setup the shared memory for EIB Receiving Buffer
	 */
	strcpy( progName, *argv) ;
	printf( "Sizeof( int) := %d\n", ( int) sizeof( int)) ;
	/**
	 * get command line options
	 */
	while (( opt = getopt( argc, argv, "d:s:r:v:?")) != -1) {
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
	 * attach the received messages queue
	 */
	printf( "trying to attach received message queue ... \n") ;
	if (( msgRcvQueue = msgget( shmRcvQueueKey, IPC_CREAT | 0666)) < 0) {
		printf( "could not attach message queue ... \n") ;
		exit( -1) ;
	}
	msgctl( msgRcvQueue, IPC_STAT, &msgRcvQueueInfo) ;
	printf( "  Key......: %8lx\n", (unsigned long) shmRcvQueueKey) ;
	printf( "  Bytes....: %8lx\n", (unsigned long) msgRcvQueueInfo.msg_qbytes) ;
	printf( "  Messages.: %8lx\n", ((unsigned long) msgRcvQueueInfo.msg_qbytes) /  sizeof( knxMsg)) ;
	/**
	 * attach the send messages queue
	 */
	printf( "trying to attach send message queue ... \n") ;
	if (( msgSndQueue = msgget( shmSndQueueKey, IPC_CREAT | 0666)) < 0) {
		printf( "could not attach message queue ... \n") ;
		exit( -1) ;
	}
	msgctl( msgSndQueue, IPC_STAT, &msgSndQueueInfo) ;
	printf( "  Key......: %8lx\n", (unsigned long) shmSndQueueKey) ;
	printf( "  Bytes....: %8lx\n", (unsigned long) msgSndQueueInfo.msg_qbytes) ;
	printf( "  Messages.: %8lx\n", ((unsigned long) msgSndQueueInfo.msg_qbytes) /  sizeof( knxMsg)) ;
	/**
	 * open communication port
	 */
	RS232_PrepComport(cport_nr, bdrate, mode) ;
	if ( RS232_OpenComport( cport_nr, bdrate, mode)) {
		printf("Can not open comport\n");
		return(0);
	}
	RS232_SendByte( cport_nr, 0x01);
	rcvdBytes	=	RS232_PollComport(cport_nr, (unsigned char *) &buf, 1);
	printf( "-->%02x\n", (int) buf) ;
#ifdef	__MACH__
	/**
	 * running in plain simulation mode
	 * the only thing we do here is to copy every byte from the transmit buffer
	 * to the receive buffer. this is actually what happens if we are in TPUART mode,
	 * when the transmitted bytes are immediately received back
	 */
	printf( "%s: running in SIMULATION mode ... (on MacOS?)\n", progName) ;
	while ( 1) {
		/**
		 * we do this in plain priority sequence
		 * if there's something in the internal queue waiting to be sent to the real bus
		 *	send it immediately back to the receive queue
		 * else
		 *	increment a sleep timer up to a max value of 3 seconds
		 *	and goto sleep
		 */
		msgToSnd	=	NULL ;
		if ( msgrcv( msgSndQueue, &msgSnd, KNX_MSG_SIZE, 1, IPC_NOWAIT) >= 0) {
			dumpMsg( "Looping message...:\n", &msgSnd) ;
			msgToSnd	=	&msgSnd ;
		} else if ( msgrcv( msgSndQueue, &msgSnd, KNX_MSG_SIZE, 2, IPC_NOWAIT) >= 0) {
			dumpMsg( "Looping message...:\n", &msgSnd) ;
			msgToSnd	=	&msgSnd ;
		} else if ( msgrcv( msgSndQueue, &msgSnd, KNX_MSG_SIZE, 3, IPC_NOWAIT) >= 0) {
			dumpMsg( "Looping message...:\n", &msgSnd) ;
			msgToSnd	=	&msgSnd ;
		} else {
			sleepTimer++ ;
			if ( sleepTimer > 1)
				sleepTimer	=	1 ;
			printf( "will go to sleep ... for %d seconds\n", sleepTimer) ;
			sleep( sleepTimer) ;
		}
		if ( msgToSnd != NULL) {
			if ( debugLevel >= 1) {
				fprintf( stderr, "%s: got message to send\n", progName) ;
			}
			if (( msgToSnd->control & 0x20) == 0x20) {
				if ( debugLevel >= 2) {
					fprintf( stderr, "%s: to be repeated 3 times\n", progName) ;
				}
				msgsnd( msgRcvQueue, msgToSnd, KNX_MSG_SIZE, 3) ;
				msgToSnd->control	&=	0xdf ;
				msgsnd( msgRcvQueue, msgToSnd, KNX_MSG_SIZE, 3) ;
				msgsnd( msgRcvQueue, msgToSnd, KNX_MSG_SIZE, 3) ;
				msgsnd( msgRcvQueue, msgToSnd, KNX_MSG_SIZE, 3) ;
			} else {
				if ( debugLevel >= 2) {
					fprintf( stderr, "%s: not to be repeated\n", progName) ;
				}
				msgToSnd->control	|=	0x20 ;
				msgsnd( msgRcvQueue, msgToSnd, KNX_MSG_SIZE, 3) ;
			}
		}
	}
#else
	printf( "%s: running in real bridging mode ... (on Raspberry?)\n", progName) ;
	rcvMode	=	waiting_for_control ;
	incompleteMsg	=	0 ;
	while ( 1) {
		/**
		 * as long as there's something coming from the serial port or an incomplete message
		 *	put it into the receive buffer
		 */
		rcvMode	=	waiting_for_control ;
		rcvdBytes	=	RS232_PollComport(cport_nr, (unsigned char *) &buf, 1) ;
		while ( rcvdBytes > 0 || incompleteMsg) {
			if ( rcvdBytes > 0) {
			switch ( rcvMode) {
				case	waiting_for_control	:
					if (( buf & 0xd3) == 0x90) {
						msgRcv.control	=	buf ;
						rcvMode	=	waiting_for_hw_adr ;
						rcvdLength	=	0 ;
						incompleteMsg	=	1 ;
						msgRcv.sndAdr	=	0x0000 ;
						msgRcv.ownChecksum	=	0x00 ;
					} else {
			printf( "SPURIOUS BYTE ... %02x \n", buf) ;
					}
					break ;
				case	waiting_for_hw_adr	:
					rcvdLength++ ;
					if ( rcvdLength >= 2) {
						rcvMode	=	waiting_for_group_adr ;
						rcvdLength	=	0 ;
						msgRcv.rcvAdr	=	0x0000 ;
					}
					msgRcv.sndAdr	<<=	8 ;
					msgRcv.sndAdr	|=	buf ; ;
					break ;
				case	waiting_for_group_adr	:
					rcvdLength++ ;
					if ( rcvdLength >= 2) {
						rcvMode	=	waiting_for_info ;
					}
					msgRcv.rcvAdr	<<=	8 ;
					msgRcv.rcvAdr	|=	buf ; ;
					break ;
				case	waiting_for_info	:
					msgRcv.info	=	buf ;
					expLength	=	(int) (buf & 0x07) + 1 ;
					rcvdLength	=	0 ;
					rcvMode	=	waiting_for_data ;
					rcvData	=	msgRcv.mtext ;
					break ;
				case	waiting_for_data	:
					*rcvData++	=	buf ;
					rcvdLength++ ;
					if ( rcvdLength >= expLength) {
						rcvMode	=	waiting_for_checksum ;
					}
					break ;
				case	waiting_for_checksum	:
					incompleteMsg	=	0 ;
					rcvMode	=	waiting_for_control ;
					msgRcv.checksum	=	buf ;
					msgRcv.mtype	=	1 ;
					if (( msgRcv.checksum + msgRcv.ownChecksum) == 0xff) {
						printf( "Checksum ok ... \n") ;
						msgsnd( msgRcvQueue, &msgRcv, KNX_MSG_SIZE, 0) ;
					} else {
						printf( "Checksum crappy ... \n") ;
						printf( "will not bridge this message \n") ;
					}
					dumpMsg( "Received Message...:\n", &msgRcv) ;
					break ;
				}
				msgRcv.ownChecksum	^=	buf ;
			}
			rcvdBytes	=	RS232_PollComport(cport_nr, (unsigned char *) &buf, 1) ;
		}
		/**
		 * new we check the send queue and put on the real bus what's needed
		 */
		if ( msgrcv( msgSndQueue, &msgSnd, KNX_MSG_SIZE, 1, IPC_NOWAIT) >= 0) {
			msgToSnd	=	&msgSnd ;
		} else if ( msgrcv( msgSndQueue, &msgSnd, KNX_MSG_SIZE, 2, IPC_NOWAIT) >= 0) {
			msgToSnd	=	&msgSnd ;
		} else if ( msgrcv( msgSndQueue, &msgSnd, KNX_MSG_SIZE, 3, IPC_NOWAIT) >= 0) {
			msgToSnd	=	&msgSnd ;
		} else {
			msgToSnd	=	NULL ;
		}
		if ( msgToSnd != NULL) {
			sndMode	=	sending_control ;
		 	msgToSnd->checksum	=	0x00 ;
			sendingByte	=	0 ;
			while ( sndMode != sending_idle) {
				switch ( sndMode) {
				case	sending_control	:
					if ( debugLevel > 1) {
						printf( "sending control\n") ;
					}
					bufp	=	0x80 + sendingByte ;
					buf	=	msgToSnd->control ;
		 			msgToSnd->checksum	^=	buf ;
					sndMode	=	sending_hw_adr ;
					sentLength	=	0 ;
					break ;
				case	sending_hw_adr	:
					if ( debugLevel > 1) {
						printf( "sending hw address\n") ;
					}
					bufp	=	0x80 + sendingByte ;
					if ( sentLength == 0)
						buf	=	msgToSnd->sndAdr >> 8 ;
					else if ( sentLength == 1)
						buf	=	msgToSnd->sndAdr & 0xff ;
		 			msgToSnd->checksum	^=	buf ;
					sentLength++ ;
					if ( sentLength >= 2) {
						sndMode	=	sending_group_adr ;
						sentLength	=	0 ;
					}
					break ;
				case	sending_group_adr	:
					if ( debugLevel > 1) {
						printf( "sending group address\n") ;
					}
					bufp	=	0x80 + sendingByte ;
					if ( sentLength == 0)
						buf	=	msgToSnd->rcvAdr >> 8 ;
					else if ( sentLength == 1)
						buf	=	msgToSnd->rcvAdr & 0xff ;
		 			msgToSnd->checksum	^=	buf ;
					sentLength++ ;
					if ( sentLength >= 2) {
						sndMode	=	sending_info ;
					}
					break ;
				case	sending_info	:
					if ( debugLevel > 1) {
						printf( "sending info\n") ;
					}
					bufp	=	0x80 + sendingByte ;
					buf	=	msgToSnd->info ;
		 			msgToSnd->checksum	^=	buf ;
					expLength	=	(int) ( msgToSnd->info & 0x07)  + 1 ;
					sentLength	=	0 ;
					sndMode	=	sending_data ;
					break ;
				case	sending_data	:
					if ( debugLevel > 1) {
						printf( "sending data\n") ;
					}
					bufp	=	0x80 + sendingByte ;
					buf	=	msgToSnd->mtext[sentLength] ;
		 			msgToSnd->checksum	^=	buf ;
					sentLength++ ;
					if ( sentLength >= expLength) {
						sndMode	=	sending_checksum ;
					}
					break ;
				case	sending_checksum	:
					if ( debugLevel > 1) {
						printf( "sending checksum %2x\n", msgToSnd->checksum) ;
					}
					bufp	=	0x40 + sendingByte ;
		 			msgToSnd->checksum	^=	0xff ;
					buf	=	msgToSnd->checksum ;
					sndMode	=	sending_idle ;
					break ;
				}
				RS232_SendBytes( cport_nr, bufp, buf) ;
				sendingByte++ ;
			}
			dumpMsg( "Sending Message...:\n", msgToSnd) ;
		}
	}
#endif
	exit( 0) ;
}

void	help() {
}

