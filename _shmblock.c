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
 * _shmblock.c
 *
 * complete code-block to establish the shared memory
 * must NOT be included by knxmon.c since this include file
 * does not include the IPC_CREAT mask!
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-11-20	PA1	userId	inception;
 *
 */
/**
 *
 */
	/**
	 * setup the shared memory for OPCtable
	 */
	printf( "trying to obtain shared memory OPCtable ... \n") ;
	if (( shmOPCId = shmget (shmOPCKey, shmOPCSize, shmOPCFlg)) < 0) {
		perror( "knxmon: shmget failed for OPCtable");
		exit(1);
	}
	printf( "trying to attach shared memory OPCtable \n") ;
	if (( data = (node *) shmat(shmOPCId, NULL, 0)) == (node *) -1) {
		perror( "knxmon: shmat failed for OPCtable");
		exit(1);
	}
	printf( "shmOPCKey........: %d\n", shmOPCKey) ;
	printf( "  shmOPCSize.....: %d\n", shmOPCSize) ;
	printf( "  shmOPCId.......: %d\n", shmOPCId) ;
	printf( "  Address..: %8lx\n", (unsigned long) data) ;
	/**
	 * setup the shared memory for KNXtable
	 */
	printf( "trying to obtain shared memory KNXtable ... \n") ;
	if (( shmKNXId = shmget( shmKNXKey, shmKNXSize, IPC_CREAT | 0666)) < 0) {
		perror( "knxmon: shmget failed for KNXtable");
		exit( -1) ;
	}
	printf( "trying to attach shared memory KNX table \n") ;
	if (( floats = (float *) shmat( shmKNXId, NULL, 0)) == (float *) -1) {
		perror( "knxmon: shmat failed for KNXtable");
		exit( -1) ;
	}
	printf( "shmKNXKey........: %d\n", shmKNXKey) ;
	printf( "  shmKNXSize.....: %d\n", shmKNXSize) ;
	printf( "  shmKNXId.......: %d\n", shmKNXId) ;
	ints	=	(int *) floats ;
	/**
	 * setup the shared memory for CRFtable
	 */
	printf( "trying to obtain shared memory CRFtable ... \n") ;
	if (( shmCRFId = shmget( shmCRFKey, shmCRFSize, IPC_CREAT | 0666)) < 0) {
		perror( "knxmon: shmget failed for CRFtable");
		exit( -1) ;
	}
	printf( "trying to attach shared memory CRFtable... \n") ;
	if (( crf = (int *) shmat( shmCRFId, NULL, 0)) == (int *) -1) {
		perror( "knxmon: shmat failed for CRFtable");
		exit( -1) ;
	}
	printf( "shmCRFKey........: %d\n", shmCRFKey) ;
	printf( "  shmCRFSize.....: %d\n", shmCRFSize) ;
	printf( "  shmCRFId.......: %d\n", shmCRFId) ;
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
