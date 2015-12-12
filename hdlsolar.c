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
 * hdlsolar.c
 *
 * handle "our" solar collector
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

#define	MODE_INVALID	-1
#define	MODE_STOPPED	0
#define	MODE_WATER	1
#define	MODE_BUFFER	2

#define	TEMP_WW_ON	50
#define	TEMP_WW_OFF	58
#define	TEMP_HB_ON	30
#define	TEMP_HB_OFF	35

extern	void	setModeStopped( int, node *) ;
extern	void	setModeWater( int, node *) ;
extern	void	setModeBuffer( int, node *) ;

extern	void	dumpData( node *, int, int, void *) ;
extern	int	getEntry( node *, int, char *) ;

int	pumpSolar ;
int	valveSolar ;
int	main( int argc, char *argv[]) {
		int	status		=	0 ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
	/**
	 * define application specific variables
	 */
		float	tempWWOn	=	TEMP_WW_ON ;	// low temp. when water heating needs to start
		float	tempWWOff	=	TEMP_WW_OFF ;	// high temp. when water heating can stop
		float	tempHBOn	=	TEMP_HB_ON ;	// low temp. when buffer heating needs to start
		float	tempHBOff	=	TEMP_HB_OFF ;	// high temp. when buffer heating can stop
		float	diffTempCollHB ;
		float	diffTempCollWW ;
		float	tempWW ;
		float	tempHB ;
		int	lastMode	=	MODE_INVALID ;
		int	mode	=	MODE_STOPPED ;
		int	changeMode ;
		int	tempWWu ;				// point to node["TempWWu"], WarmWater
		int	tempHBu ;				// point to node["TempHBu"], HeatingBuffer
		int	tempCol1 ;				// point to node["TempCol1"], SolarCollector
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
	pumpSolar	=	getEntry( data, lastDataIndexC, "PumpSolar") ;
	valveSolar	=	getEntry( data, lastDataIndexC, "ValveSolar") ;
	printf( "pumpSolar at index .......: %d\n", pumpSolar) ;
	printf( "valveSolar at index ......: %d\n", valveSolar) ;
	tempWWu		=	getEntry( data, lastDataIndexC, "TempWWu") ;
	tempHBu		=	getEntry( data, lastDataIndexC, "TempPSu") ;
	tempCol1		=	getEntry( data, lastDataIndexC, "TempCol1") ;
	/**
	 * try to determine the current mode of the solar-module
	 */
	printf( "trying to setermine current status\n") ;
	if ( data[pumpSolar].val.i == 1) {
		if ( data[valveSolar].val.i == VALVE_SOLAR_WW) {
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
	printf( "entering processing node\n") ;
	while ( 1) {
		/**
		 * dump all input data for this "MES"
		 */
		dumpData( data, lastDataIndexC, MASK_SOLAR, (void *) floats) ;
		/**
		 *
		 */
		diffTempCollWW	=	data[tempCol1].val.f - data[tempWWu].val.f ;
		diffTempCollHB	=	data[tempCol1].val.f - data[tempHBu].val.f ;
		tempWW	=	data[tempWWu].val.f ;
		tempHB	=	data[tempHBu].val.f ;
		changeMode	=	1 ;
		lastMode	=	mode ;
		while ( changeMode) {
			changeMode	=	0 ;
			switch( mode) {
			case	MODE_STOPPED	:
				printf( "Solar pump is not running ... \n") ;
				if ( diffTempCollWW >= 10.0 && tempWW < tempWWOff) {
					mode	=	MODE_WATER ;
				} else if ( diffTempCollHB >= 10.0) {
					mode	=	MODE_BUFFER ;
				} else {
					mode	=	MODE_STOPPED ;
				}
				break ;
			case	MODE_WATER	:
				printf( "Solar pump is heating WATER ... \n") ;
				if ( diffTempCollWW <= 5.0 || tempWW >= tempWWOff) {
					changeMode	=	1 ;
					mode	=	MODE_STOPPED ;
				} else {
				}
				break ;
			case	MODE_BUFFER	:
				printf( "Solar pump is heating BUFFER ... \n") ;
				if ( diffTempCollWW >= 10.0 && tempWW < tempWWOn) {
					mode	=	MODE_STOPPED ;
					changeMode	=	1 ;
				} else if ( data[valveSolar].val.i == VALVE_SOLAR_HB && diffTempCollHB <= 5.0) {
					mode	=	MODE_STOPPED ;
					changeMode	=	1 ;
				}
				break ;
			}
		}
		if ( mode != lastMode || mode == MODE_STOPPED) {
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
		sleep( 5) ;
	}
	exit( status) ;
}

void	setModeStopped( int msgQueue, node *data) {
	printf( "  ... will switch off solar pump ...\n") ;
	printf( "  ... and switch valve to WW ...\n") ;
	msgBuf	myMsg ;
	myMsg.mtype	=	1 ;
	myMsg.group	=	data[pumpSolar].knxGroupAdr ;
	myMsg.val.i	=	0 ;
	msgsnd( msgQueue, &myMsg, MSG_SIZE, 0) ;
	sleep( 1) ;
	myMsg.mtype	=	3 ;
	myMsg.group	=	data[valveSolar].knxGroupAdr ;
	myMsg.val.i	=	VALVE_SOLAR_WW ;
	msgsnd( msgQueue, &myMsg, MSG_SIZE, 0) ;
}

void	setModeWater( int msgQueue, node *data) {
	printf( "  ... will switch on solar pump ...\n") ;
	printf( "  ... and switch valve to WW ...\n") ;
	msgBuf	myMsg ;
	myMsg.mtype	=	1 ;
	myMsg.group	=	data[pumpSolar].knxGroupAdr ;
	myMsg.val.i	=	1 ;
	msgsnd( msgQueue, &myMsg, MSG_SIZE, 0) ;
	sleep( 1) ;
	myMsg.mtype	=	3 ;
	myMsg.group	=	data[valveSolar].knxGroupAdr ;
	myMsg.val.i	=	VALVE_SOLAR_WW ;
	msgsnd( msgQueue, &myMsg, MSG_SIZE, 0) ;
}

void	setModeBuffer( int msgQueue, node *data) {
	printf( "  ... will switch on solar pump ...\n") ;
	printf( "  ... and switch valve to HB ...\n") ;
	msgBuf	myMsg ;
	myMsg.mtype	=	1 ;
	myMsg.group	=	data[pumpSolar].knxGroupAdr ;
	myMsg.val.i	=	1 ;
	msgsnd( msgQueue, &myMsg, MSG_SIZE, 0) ;
	sleep( 1) ;
	myMsg.mtype	=	3 ;
	myMsg.group	=	data[valveSolar].knxGroupAdr ;
	myMsg.val.i	=	VALVE_SOLAR_HB ;
	msgsnd( msgQueue, &myMsg, MSG_SIZE, 0) ;
}
