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
 * 201x-mm-dd	PA1	userId	inception;
 *
 */
#ifndef eib_INCLUDED
#define eib_INCLUDED

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<termios.h>
#include	<sys/ioctl.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<sys/types.h>
#include	<sys/ipc.h> 
#include	<sys/shm.h> 
#include	<sys/msg.h> 

extern	void	eibOpen() ;
extern	void	eibClose() ;
extern	void	eibSend( knxMsg *) ;
extern	void	eibWriteBit( unsigned int, unsigned int, unsigned char, unsigned char) ;
extern	void	eibWriteHalfFloat( unsigned int, unsigned int, float, unsigned char) ;
extern	void	eibWriteTime( unsigned int, unsigned int, int *, unsigned char) ;
extern	knxMsg	*eibReceive() ;
extern	void	dumpMsg( char *, knxMsg *) ;
extern	void	eibDump( char *, knxMsg *) ;

#endif
