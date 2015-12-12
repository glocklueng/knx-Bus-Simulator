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
 * mylib.c
 *
 * some useful functions
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-11-20	PA1	userId	inception;
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<strings.h>
#include	<time.h>
#include	<math.h>
#include	<sys/types.h>
#include	<sys/ipc.h> 
#include	<sys/shm.h> 

#include	"nodeinfo.h"

void	dumpData( node *data, int lastDataIndex, int mask, void *knxData) {
		int	i ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
		int	*ints	=	(int *) knxData ;
		float	*floats	=	(float *) knxData ;
	/**
	 *
	 */
	for ( i=0 ; i < lastDataIndex ; i++) {
		if ( data[i].monitor & mask) {
			actTime	=	time( NULL) ;
			myTime	=	gmtime( &actTime) ;
			strftime( timeBuffer,  sizeof( timeBuffer), "%F %T", myTime) ;
			switch ( data[i].type) {
			case	dtBit	:
				printf( "Time: %s, Id: %3d/%3d, Name: '%-25s', HW: %04x, KNX: %5d, bit:   %6d ",
					 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAdr, data[i].knxGroupAdr, data[i].val.i) ;
				if ( data[i].knxGroupAdr != 0) {
					printf( "KNX...: %d", ints[data[i].knxGroupAdr]) ;
				}
				break ;
			case	dtInt1	:
			case	dtUInt1	:
			case	dtInt2	:
			case	dtUInt2	:
			case	dtInt4	:
			case	dtUInt4	:
				printf( "Time: %s, Id: %3d/%3d, Name: '%-25s', HW: %04x, KNX: %5d, int:   %6d ",
					 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAdr, data[i].knxGroupAdr, data[i].val.i) ;
				if ( data[i].knxGroupAdr != 0) {
					printf( "KNX...: %d", ints[data[i].knxGroupAdr]) ;
				}
				break ;
			case	dtFloat2	:
			case	dtFloat4	:
				printf( "Time: %s, Id: %3d/%3d, Name: '%-25s', HW: %04x, KNX: %5d, float: %6.2f ",
					 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAdr, data[i].knxGroupAdr, data[i].val.f) ;
				if ( data[i].knxGroupAdr != 0) {
					printf( "KNX...: %5.2f", floats[data[i].knxGroupAdr]) ;
				}
				break ;
			case	dtString	:
				break ;
			case	dtDate	:
				break ;
			case	dtTime	:
				break ;
			case	dtDateTime	:
				break ;
			}
			printf( "\n") ;
		}
	}
}

void	dumpDataAll( node *data, int lastDataIndex, void *knxData) {
		int	i ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
		int	*ints	=	(int *) knxData ;
		float	*floats	=	(float *) knxData ;
	/**
	 *
	 */
	for ( i=0 ; i < lastDataIndex ; i++) {
		actTime	=	time( NULL) ;
		myTime	=	gmtime( &actTime) ;
		strftime( timeBuffer,  sizeof( timeBuffer), "%F %T", myTime) ;
		switch ( data[i].type) {
		case	dtBit	:
			printf( "Time: %s, Id: %3d/%3d, Name: '%-25s', HW: %04x, KNX: %5d, bit:   %6d ",
				 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAdr, data[i].knxGroupAdr, data[i].val.i) ;
			if ( data[i].knxGroupAdr != 0) {
				printf( "KNX...: %d", ints[data[i].knxGroupAdr]) ;
			}
			break ;
		case	dtInt1	:
		case	dtUInt1	:
		case	dtInt2	:
		case	dtUInt2	:
		case	dtInt4	:
		case	dtUInt4	:
			printf( "Time: %s, Id: %3d/%3d Name: '%-25s', HW: %04x, KNX: %5d, int:   %6d ",
				 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAdr, data[i].knxGroupAdr, data[i].val.i) ;
			if ( data[i].knxGroupAdr != 0) {
				printf( "KNX...: %d", ints[data[i].knxGroupAdr]) ;
			}
			break ;
		case	dtFloat2	:
		case	dtFloat4	:
			printf( "Time: %s, Id: %3d/%3d, Name: '%-25s', HW: %04x, KNX: %5d, float: %6.2f ",
				 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAdr, data[i].knxGroupAdr, data[i].val.f) ;
			if ( data[i].knxGroupAdr != 0) {
				printf( "KNX...: %5.2f", floats[data[i].knxGroupAdr]) ;
			}
			break ;
		case	dtString	:
			break ;
		case	dtDate	:
			printf( "................dtDate") ;
			break ;
		case	dtTime	:
			printf( "................dtTime") ;
			break ;
		case	dtDateTime	:
			printf( "................dtDateTime") ;
			break ;
		}
		printf( "\n") ;
	}
}

void	createCRF( node *data, int lastDataIndex, int *crf, void *knxData) {
		int	i ;
		int	*ints ;
		float	*floats ;
	/**
	 *
	 */
	ints	=	(int *) knxData ;
	floats	=	(float *) knxData ;
	/**
	 *
	 */
	for ( i=0 ; i < 65536 ; i++) {
		crf[i]	=	0 ;
	}
	for ( i=0 ; i < lastDataIndex ; i++) {
		if ( data[i].knxGroupAdr != 0) {
			crf[ data[i].knxGroupAdr]	=	i ;
			switch ( data[i].type) {
			case	dtBit	:
				data[i].val.i	=	ints[data[i].knxGroupAdr] ;
				break ;
			case	dtInt1	:
			case	dtUInt1	:
			case	dtInt2	:
			case	dtUInt2	:
			case	dtInt4	:
			case	dtUInt4	:
				data[i].val.i	=	ints[data[i].knxGroupAdr] ;
				break ;
			case	dtFloat2	:
			case	dtFloat4	:
				data[i].val.f	=	floats[data[i].knxGroupAdr] ;
				break ;
			case	dtString	:
				break ;
			case	dtDate	:
				break ;
			case	dtTime	:
				break ;
			case	dtDateTime	:
				break ;
			}
		}
	}
}

int	getEntry( node *data, int lastDataIndex, char *name) {
	int	i, j ;
	j	=	-1 ;
	for ( i=0 ; i<lastDataIndex && j == -1 ; i++) {
		if ( strcmp( data[i].name, name) == 0) {
			j	=	i ;
		}
	}
	return j ;
}
/**
 * hfbtf	- half-float binary to float
 */
float	hfbtf( const unsigned char buf[]) {
	float	val ;
	int	mant ;
	int	exp ;
	if ( buf[0] & 0x80) {
		mant	=	( 0xfffff800 | ((buf[0] & 0x07) << 8) | buf[1]) ;
		exp	=	(( buf[0] & 0x78) >> 3) ;
		val	=	mant * 0.01 * pow( 2, exp) ;
	} else {
		mant	=	( buf[1] | ((buf[0] & 0x07) << 8)) ;
		exp	=	(( buf[0] & 0x78) >> 3) ;
		val	=	mant * 0.01 * pow( 2, exp) ;
	}
	return( val) ;
}
/**
 * hfbtf	- float to half-float binary
 */
void	fthfb( float val, unsigned char buf[]) {
	float	rem ;
	int	mant ;
	int	exp ;
	/**
	 *
	 */
	rem	=	val * 100.0 ;
	exp	=	0 ;
	while ( fabs( rem) > 2047) {
		rem	=	rem / 2 ;
		exp++ ;
//		printf( "rem := %f, exp := %d \n", rem, exp) ;
	}
	mant	=	round( rem) ;

//	printf( "mant := %d \n", mant) ;

	buf[ 0]	=	((( mant & 0x80000000) >> 24) | (( mant & 0x0700) >> 8) | (( exp & 0x0f) << 3)) ;
	buf[ 1]	=	mant & 0x00ff ;
}
