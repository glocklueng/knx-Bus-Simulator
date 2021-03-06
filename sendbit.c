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
 * sendbit.c
 *
 * send a bit value (DPT 1.001) to a given group address
 *
 * Revision history
 *
 * Date		Rev.	Who	what
 * ----------------------------------------------------------------------------
 * 2015-11-26	PA1	khw	inception; derived from sendfloat.c
 * 2015-12-10	PA2	khw	adopted to new structure
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>

#include	"knxbridge.h"
#include	"eib.h"
#include	"nodeinfo.h"
#include	"mylib.h"
#include	"knxbridge.h"

extern	void	help() ;

char	progName[64] ;
int	debugLevel	=	0 ;

int main( int argc, char *argv[]) {
			short		group ;
	unsigned	char		buf[16] ;
			int		msgLen ;
			int		opt ;
			int		sender	=	0 ;
			int		receiver	=	0 ;
			int		value ;
			int		repeat	=	0x20 ;
			knxMsg		myMsg ;
	/**
	 *
	 */
	strcpy( progName, *argv) ;
	printf( "%s: starting up ... \n", progName) ;
	/**
	 * get command line options
	 */
	while (( opt = getopt( argc, argv, "d:s:r:v:n?")) != -1) {
		switch ( opt) {
		case	'd'	:
			debugLevel	=	atoi( optarg) ;
			break ;
		case	's'	:
			sender	=	atoi( optarg) ;
			break ;
		case	'r'	:
			receiver	=	atoi( optarg) ;
			break ;
		case	'n'	:
			repeat	=	0x00 ;
			break ;
		case	'v'	:
			value	=	atoi( optarg) ;
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
	 *
	 */
	if ( sender != 0 && receiver != 0) {
		/**
		 *
		 */
		if ( debugLevel > 0) {
			fprintf( stderr, "%s: calling eibOpen() \n", progName) ;
		}
		eibOpen() ;
		eibWriteBit( sender, receiver, ( value & 0x01), 1) ;
		eibClose() ;
	} else {
		printf( "%s: invalid sender and/or receiver address\n", progName) ;
		help() ;
	}
	/**
	 *
	 */
	exit( 0);
}
void	help() {
	printf( "%s: %s -s=<senderAdr> -r=<receiverAdr> -v=<value> \n\n", progName, progName) ;
	printf( "sends an arbitrary bit <value> from node <senderAdr> to <receiverAdr>h \n") ;
	printf( "receiver address is basically the group address \n") ;
}
