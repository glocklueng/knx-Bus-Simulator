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
 * hdlpellet.c
 *
 * handling for "our" pellet stove
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-11-20	PA1	userId	inception;
 *
 */
#include	<stdio.h>
#include	<unistd.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<strings.h>
#include	<time.h>
#include	<sys/types.h>
#include	<sys/ipc.h> 
#include	<sys/shm.h> 
#include	<sys/msg.h> 

#include	"nodeinfo.h"
#include	"knxbridge.h"
#include	"eib.h"

#define	MODE_INVALID	-1
#define	MODE_STOPPED	0
#define	MODE_WATER	1
#define	MODE_BUFFER	2

#define	GAP_OFFON	600		// may not be switched on before this amount of seconds has elapsed
#define	GAP_ONOFF	600		// may not be switched off before this amount of seconds has elapsed

#define	TEMP_WW_ON	50
#define	TEMP_WW_OFF	55
#define	TEMP_HB_ON	30
#define	TEMP_HB_OFF	35

extern	void	setModeStopped( int, node *) ;
extern	void	setModeWater( int, node *) ;
extern	void	setModeBuffer( int, node *) ;

extern	void	dumpData( node *, int, int, void *) ;
extern	int	getEntry( node *, int, char *) ;

int	pelletStove ;
int	valvePelletStove ;

int	main( int argc, char *argv[]) {
		int	status		=	0 ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
	/**
	 *
	 */
		float	tempWWOn	=	TEMP_WW_ON ;	// low temp. when water heating
								// needs to start
		float	tempWWOff	=	TEMP_WW_OFF ;	// high temp. when water heating
								// can stop
		float	tempHBOn	=	TEMP_HB_ON ;	// low temp. when buffer heating
								// needs to start
		float	tempHBOff	=	TEMP_HB_OFF ;	// high temp. when buffer heating
								// can stop
		float	tempWW ;
		float	tempHB ;
		int	lastMode	=	MODE_INVALID ;
		int	mode	=	MODE_STOPPED ;
		int	changeMode ;
		int	tempWWu ;				// point to node["TempWWu"],
								// WarmWater
		int	tempHBu ;				// point to node["TempHBu"],
								// HeatingBuffer
		time_t	lastOffTime	=	0L ;
		time_t	lastOnTime	=	0L ;
	/**
	 * define shared memory segment #0: COM Table
 	 */
		key_t	shmCOMKey	=	SHM_COM_KEY ;
		int	shmCOMFlg	=	IPC_CREAT | 0666 ;
		int	shmCOMId ;
		int	shmCOMSize	=	256 ;
		int	*sizeTable ;
	/**
	 * define shared memory segment #1: OPC Table with buffer
 	 */
		key_t	shmOPCKey	=	SHM_OPC_KEY ;
		int	shmOPCFlg	=	IPC_CREAT | 0666 ;
		int	shmOPCId ;
		int	shmOPCSize ;
		node	*data ;
	/**
	 * define shared memory segment #2: KNX Table with buffer
 	 */
		key_t	shmKNXKey	=	SHM_KNX_KEY ;
		int	shmKNXFlg	=	IPC_CREAT | 0666 ;
		int	shmKNXId ;
		int	shmKNXSize	=	65536 * sizeof( float) ;
		float	*floats ;
		int	*ints ;
	/**
	 * define shared memory segment #2: KNX Table with buffer
 	 */
		key_t	shmCRFKey	=	SHM_CRF_KEY ;
		int	shmCRFFlg	=	IPC_CREAT | 0666 ;
		int	shmCRFId ;
		int	shmCRFSize	=	65536 * sizeof( int) ;
		int	*crf ;
	/**
	 * define shared memory segment #1: OPC Table with buffer
 	 */
		key_t	shmMSGKey	=	SHM_MSG_KEY ;
		int	msgQueue ;
	struct	msqid_ds	msgQueueInfo ;
	/**
	 * setup the shared memory for COMtable
	 */
	printf( "trying to obtain shared memory COMtable ... \n") ;
	if (( shmCOMId = shmget( shmCOMKey, shmCOMSize, 0666)) < 0) {
		perror( "knxmon: shmget failed for COMtable");
		exit( -1) ;
	}
	printf( "trying to attach shared memory for COMtable \n") ;
	if (( sizeTable = (int *) shmat( shmCOMId, NULL, 0)) == (int *) -1) {
		perror( "knxmon: shmat failed for COMtable");
		exit( -1) ;
	}
	shmCOMSize	=	sizeTable[ SIZE_COM_TABLE] ;
	shmOPCSize	=	sizeTable[ SIZE_OPC_TABLE] ;
	shmKNXSize	=	sizeTable[ SIZE_KNX_TABLE] ;
	shmCRFSize	=	sizeTable[ SIZE_CRF_TABLE] ;
	/**
	 *
	 */
#include	"_shmblock.c"
	/**
	 * get some indices from the
	 */
	pelletStove		=	getEntry( data, lastDataIndexC, "PelletStove") ;
	valvePelletStove	=	getEntry( data, lastDataIndexC, "ValvePelletStove") ;
	printf( "pelletStove at index ...........: %d\n", pelletStove) ;
	printf( "valvePelletStove at index ......: %d\n", valvePelletStove) ;
	tempWWu		=	getEntry( data, lastDataIndexC, "TempWWu") ;
	tempHBu		=	getEntry( data, lastDataIndexC, "TempPSu") ;
	/**
	 * try to determine the current mode of the pellet-module
	 */
	printf( "trying to setermine current status\n") ;
	if ( data[pelletStove].val.i == 1) {
		if ( data[valvePelletStove].val.i == VALVE_PS_WW) {
			mode	=	MODE_WATER ;
		} else {
			mode	=	MODE_BUFFER ;
		}
	} else {
		mode	=	MODE_STOPPED ;
	}
	/**
	 *
	 */
	while ( 1) {
		/**
		 * dump all input data for this "MES"
		 */
		dumpData( data, lastDataIndexC, MASK_PELLET, (void *) floats) ;
		/**
		 *
		 */
		changeMode	=	1 ;
		lastMode	=	mode ;
		tempWW	=	data[tempWWu].val.f ;
		tempHB	=	data[tempHBu].val.f ;
		while ( changeMode) {
			changeMode	=	0 ;
			switch( mode) {
			case	MODE_STOPPED	:
				printf( "Pellet stove is not working ... \n") ;
				if ( tempWW <= tempWWOn) {
					mode	=	MODE_WATER ;
				} else if ( tempHB <= tempHBOn) {
					mode	=	MODE_BUFFER ;
				} else {
					mode	=	MODE_STOPPED ;
				}
				break ;
			case	MODE_WATER	:
				printf( "Pellet stove is heating WATER ... \n") ;
printf( "WW ...... %f \n", tempWW) ;
printf( "WWOff ... %f \n", tempWWOff) ;
				if ( tempWW >= tempWWOff) {
					changeMode	=	1 ;
					mode	=	MODE_STOPPED ;
				}
				break ;
			case	MODE_BUFFER	:
				printf( "Pellet stove is heating BUFFER ... \n") ;
				if ( tempWW <= tempWWOn) {
					mode	=	MODE_STOPPED ;
					changeMode	=	1 ;
				} else if ( tempHB >= tempHBOff) {
					mode	=	MODE_STOPPED ;
					changeMode	=	1 ;
				}
				break ;
			}
		}
		if ( mode != lastMode) {
			printf( "changing mode ... \n") ;
			lastMode	=	mode ;
			switch ( mode) {
			case	MODE_STOPPED	:
				setModeStopped( msgQueue, data) ;
				break ;
			case	MODE_WATER	:
				setModeWater( msgQueue, data) ;
				break ;
			case	MODE_BUFFER	:
				setModeBuffer( msgQueue, data) ;
				break ;
			}
		}
		printf( "\n") ;
		sleep( 5) ;
	}
	exit( status) ;
}

void	setModeStopped( int msgQueue, node *data) {
	printf( "  ... will switch off pellet stove ...\n") ;
	printf( "  ... and switch valve to WW ...\n") ;
	eibOpen() ;
	eibWriteBit( 0x1111, data[pelletStove].knxGroupAdr, 0, 1) ;
	sleep( 1) ;
	eibWriteBit( 0x1111, data[valvePelletStove].knxGroupAdr, VALVE_PS_WW, 1) ;
	eibClose() ;
}

void	setModeWater( int msgQueue, node *data) {
	printf( "  ... will switch on pellet stove ...\n") ;
	printf( "  ... and switch valve to WW ...\n") ;
	eibOpen() ;
	eibWriteBit( 0x1111, data[pelletStove].knxGroupAdr, 1, 1) ;
	sleep( 1) ;
	eibWriteBit( 0x1111, data[valvePelletStove].knxGroupAdr, VALVE_PS_WW, 1) ;
	eibClose() ;
}

void	setModeBuffer( int msgQueue, node *data) {
	printf( "  ... will switch on pellet stove ...\n") ;
	printf( "  ... and switch valve to PS ...\n") ;
	eibOpen() ;
	eibWriteBit( 0x1111, data[pelletStove].knxGroupAdr, 1, 1) ;
	sleep( 1) ;
	eibWriteBit( 0x1111, data[valvePelletStove].knxGroupAdr, VALVE_PS_HB, 1) ;
	eibClose() ;
}
