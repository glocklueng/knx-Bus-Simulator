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
 * hdlheating.c
 *
 * handler for "our" heating system.
 * this file is not yet
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

#define	CYCLE_TIME	10		// run in 10-seconds cycles

extern	void	setOff( node *) ;
extern	void	setOn( node *) ;

extern	void	dumpData( node *, int, int, void *) ;
floorValve	valves[]	=	{
						{	0, "UG_TECH"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "UG_OFCL"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "UG_OFCR"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "UG_STO1"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "UG_STO2"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "UG_HALL"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "EG_HWR"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "EG_GUEST"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "EG_LIV"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "EG_DIN"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "EG_KITCH"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "EG_BATH"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "EG_HALL"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "OG_MBATH"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "OG_MBR"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "OG_BATH"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "OG_BRR"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "OG_BRF"	,0	,0	,0	,0	,0	,0	}
					,	{	0, "OG_HALL"	,0	,0	,0	,0	,0	,0	}
					} ;

int	main( int argc, char *argv[]) {
		int	status		=	0 ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
	/**
	 *
	 */
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
	 *
	 */
	while ( 1) {
		/**
		 * dump all input data for this "MES"
		 */
		dumpData( data, lastDataIndexC, MASK_PELLET, (void *) floats) ;
		sleep( CYCLE_TIME) ;
	}
	exit( status) ;
}

void	setOn( node *data) {
	printf( "  ... will switch off pellet stove ...\n") ;
	printf( "  ... and switch valve to WW ...\n") ;
}

void	setOff( node *data) {
	printf( "  ... will switch on pellet stove ...\n") ;
	printf( "  ... and switch valve to WW ...\n") ;
}
