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
 * <filename>.c
 *
 * some high level functional description
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-12-04	PA1	khw	inception;
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
/**
 *
 */
#include "knxbridge.h"
#include "eib.h"
#include "nodeinfo.h"
#include "mylib.h"
/**
 *
 */
unsigned	char	buffer[16] ;
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
/**
 *
 */
void	eibOpen() {
	/**
	 * general setup
	 */
	/**
	 * attach the received messages queue
	 */
	printf( "trying to attach received message queue ... \n") ;
	if (( msgRcvQueue = msgget( shmRcvQueueKey, 0666)) < 0) {
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
	if (( msgSndQueue = msgget( shmSndQueueKey, 0666)) < 0) {
		printf( "could not attach message queue ... \n") ;
		exit( -1) ;
	}
	msgctl( msgSndQueue, IPC_STAT, &msgSndQueueInfo) ;
	printf( "  Key......: %8lx\n", (unsigned long) shmSndQueueKey) ;
	printf( "  Bytes....: %8lx\n", (unsigned long) msgSndQueueInfo.msg_qbytes) ;
	printf( "  Messages.: %8lx\n", ((unsigned long) msgSndQueueInfo.msg_qbytes) /  sizeof( knxMsg)) ;
	/**
	 *
	 */
}
/**
 *
 */
void	eibClose() {
}
/**
 *
 */
void	eibSend( knxMsg *msg) {
	msgsnd( msgSndQueue, msg, KNX_MSG_SIZE, IPC_NOWAIT) ;
}
/**
 *
 */
knxMsg	*eibReceive( knxMsg *msgBuf) {
	knxMsg	*msgRcvd ;
	/**
	 * we do this in plain priority sequence
	 * if there's something in the internal queue waiting to be sent to the real bus
	 *	send it immediately back to the receive queue
	 * else
	 *	increment a sleep timer up to a max value of 3 seconds
	 *	and goto sleep
	 */
	msgRcvd	=	NULL ;
	if ( msgrcv( msgRcvQueue, msgBuf, KNX_MSG_SIZE, 1, IPC_NOWAIT) >= 0) {
		msgRcvd	=	msgBuf ;
	} else if ( msgrcv( msgSndQueue, msgBuf, KNX_MSG_SIZE, 2, IPC_NOWAIT) >= 0) {
		msgRcvd	=	msgBuf ;
	} else if ( msgrcv( msgSndQueue, msgBuf, KNX_MSG_SIZE, 3, IPC_NOWAIT) >= 0) {
		msgRcvd	=	msgBuf ;
	}
	if ( msgRcvd != NULL) {
		msgRcvd->tlc	=	 msgRcvd->mtext[0] >> 6 ;
		msgRcvd->apci	=	(( msgRcvd->mtext[0] & 0x03) << 2) | (( msgRcvd->mtext[1] & 0xc0) >> 6) ;
	}
	return( msgRcvd) ;
}
/**
 *
 */
void	dumpMsg( char *header, knxMsg *msg) {
	eibDump( header, msg) ;
}
void	eibDump( char *header, knxMsg *msg) {
	int	length ;
	int	apci ;
	int	il0 ;
	/**
	 *
	 */
	msg->repeat	=	(( msg->control & 0x20) >> 5) ;
	msg->prio	=	(( msg->control & 0x0c) >> 2) ;
	msg->tlc	=	msg->mtext[0] >> 6 ;
	/**
	 *
	 */
	printf( "%s", header) ;
	printf( "  mtype....................:  %3ld \n", msg->mtype) ;
	printf( "  control byte.............:  %02x \n", msg->control) ;
	printf( "    repeat.................:  %02x \n", msg->repeat) ;
	printf( "    priority...............:  %02x \n", msg->prio) ;
	printf( "  physical address.........:  %04x \n", msg->sndAdr) ;
	printf( "  for address..............:  %5d \n", msg->rcvAdr) ;
	printf( "  Info.....................:  %02x \n", msg->info) ;
	length	=	( msg->info & 0x0f) + 1 ;
	printf( "    Length.................:  %02x \n", length) ;
	printf( "    Data...................:  ") ;
	for ( il0=0 ; il0<length ; il0++) {
		printf( "%02x ", msg->mtext[il0]) ;
	}
	printf( "\n") ;
	switch ( msg->tlc) {
	case	0x00	:	// UDP
		msg->apci	=	(( msg->mtext[0] & 0x03) << 2) | (( msg->mtext[1] & 0xc0) >> 6) ;
		printf( "    Type...................:  UDP \n") ;
		printf( "      APCI.................:  %04x \n", msg->apci) ;
		switch ( msg->apci) {
		case	0x00	:
			printf( "        Group Value Read \n") ;
			break ;
		case	0x01	:
			printf( "        Group Value Response \n") ;
			break ;
		case	0x02	:
			printf( "        Group Value Write \n") ;
			break ;
		case	0x03	:
			printf( "        Individual Address Write \n") ;
			break ;
		case	0x04	:
			printf( "        Individual Address Request \n") ;
			break ;
		case	0x05	:
			printf( "        Individual Address Reply \n") ;
			break ;
		case	0x06	:
			printf( "        Adc Read \n") ;
			break ;
		case	0x07	:
			printf( "        Adc Response \n") ;
			break ;
		case	0x08	:
			printf( "        Memory Read \n") ;
			break ;
		case	0x09	:
			printf( "        Memory Response \n") ;
			break ;
		case	0x0a	:
			printf( "        Memory Write \n") ;
			break ;
		case	0x0b	:
			printf( "        User Message \n") ;
			break ;
		case	0x0c	:
			printf( "        Mask Version Read \n") ;
			break ;
		case	0x0d	:
			printf( "        Mask Version Response \n") ;
			break ;
		case	0x0e	:
			printf( "        Restart \n") ;
			break ;
		case	0x0f	:
			printf( "        Escape \n") ;
			break ;
		}
		break ;
	case	0x01	:	// NDP
		msg->seqNo	=	(( msg->mtext[0] & 0x3c) >> 2) ;
		msg->apci	=	(( msg->mtext[0] & 0x03) << 2) | (( msg->mtext[1] & 0xc0) >> 6) ;
		printf( "    Type...................:  NDP \n") ;
		printf( "      Sequence no..........:  %02x \n", msg->seqNo) ;
		printf( "      APCI.................:  %04x \n", msg->apci) ;
		break ;
	case	0x02	:	// UCD
		msg->ppCmd	=	msg->mtext[0] & 0x03 ;
		printf( "    Type...................:  UCD \n") ;
		printf( "      Command..............:  %02x \n", msg->ppCmd) ;
		break ;
	case	0x03	:	// NCD
		msg->seqNo	=	(( msg->mtext[0] & 0x3c) >> 2) ;
		msg->ppConf	=	msg->mtext[0] & 0x03 ;
		printf( "    Type...................:  NCD \n") ;
		printf( "      Sequence no..........:  %02x \n", msg->seqNo) ;
		printf( "      Confirm..............:  %02x \n", msg->ppConf) ;
		break ;
	}
	printf( "    Checksum...............:  %02x \n", msg->checksum) ;
	printf( "    own Checksum...........:  %02x \n", msg->ownChecksum) ;
}
/**
 *
 */
void	eibWriteBit( unsigned int sender, unsigned int receiver, unsigned char value, unsigned char repeat) {
	knxMsg	myMsg ;
	myMsg.mtype	=	1 ;
	myMsg.control	=	0x9c | ( repeat << 5);
	myMsg.sndAdr	=	sender ;
	myMsg.rcvAdr	=	receiver ;
	myMsg.info	=	0xe0 | 0x01 ;
	myMsg.mtext[0]	=	0x00 ;
	myMsg.mtext[1]	=	0x80 | ( value & 0x01) ;
	eibSend( &myMsg) ;
}
/**
 *
 */
void	eibWriteHalfFloat( unsigned int sender, unsigned int receiver, float value, unsigned char repeat) {
	knxMsg	myMsg ;
	myMsg.mtype	=	1 ;
	myMsg.control	=	0x9c | ( repeat << 5);
	myMsg.sndAdr	=	sender ;
	myMsg.rcvAdr	=	receiver ;
	myMsg.info	=	0xe0 | 0x03 ;
	myMsg.mtext[0]	=	0x00 ;
	myMsg.mtext[1]	=	0x80 ;
	fthfb( value, &myMsg.mtext[2]) ;
	eibSend( &myMsg) ;
}
/**
 *
 *	data[0]=	weekday
 *	data[1]=	hour
 *	data[2]=	minute
 *	data[3]=	second
 */
void	eibWriteTime( unsigned int sender, unsigned int receiver, int *data, unsigned char repeat) {
	knxMsg	myMsg ;
	myMsg.mtype	=	1 ;
	myMsg.control	=	0x9c | ( repeat << 5);
	myMsg.sndAdr	=	sender ;
	myMsg.rcvAdr	=	receiver ;
	myMsg.info	=	0xe0 | 0x04 ;
	myMsg.mtext[ 0]	=	0x00 ;
	myMsg.mtext[ 1]	=	0x80 ;
	myMsg.mtext[ 2]	=	(( data[0] & 0x07) << 5) | ( data[1] & 0x1f) ;
	myMsg.mtext[ 3]	=	data[2] & 0x3f ;
	myMsg.mtext[ 4]	=	data[3] & 0x3f ;
	eibSend( &myMsg) ;
}
