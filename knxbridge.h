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
 * knxbridge.h
 *
 * some high level functional description
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-12-07	PA1	khw	inception;
 *
 */

#ifndef	knxbridge_INCLUDED
#define	knxbridge_INCLUDED

/**
 * define the various states for the serial receiver
 */
typedef	enum	{
	waiting_for_control		,
		waiting_for_hw_adr	,
		waiting_for_group_adr	,
		waiting_for_info	,
		waiting_for_data	,
		waiting_for_checksum
}	bridgeModeRcv ;

/**
 * define the various states for the serial sender
 */
typedef	enum	{
	sending_idle		,
	sending_control		,
		sending_hw_adr	,
		sending_group_adr	,
		sending_info	,
		sending_data	,
		sending_checksum
}	bridgeModeSnd ;

/**
 * description of a knx bus message (as going though the internal message queues)
 *
 * byte #	signficance
 *      0	control byte must be of binary shape b10x1xx00,
 *		i.e. BYTE & 0xd3 == 0x90 matches a valid control byte
 *      1	sender address high byte
 *      2	sender address low byte+
 *      3	receiver address high byte
 *      4	receiver address low byte
 *      5	info, bits bxxxx1111 signfiy length of packet, 0= length 1, 1= length 2 etc,
 *      6	message[0]info, buts bxxxx1111 signfiy length if packe, 0= length 1, 1= length 2 etc,
 *  7..21	message[1]..[15], message
 *  8..22	checksum
 */

typedef struct {
                		long	mtype;		// message priority
							//	1= top priority,
							//	2= medium priority,
							//	3= low priority
		unsigned	char	control ;	// control byte
							//  b=..x.....     0= do not repeat
							//		   1= repeat
							//  b=....xx..    00= system function
							//		  01= alarm function
							//		  02= high priority data
							//		  03= low priority data
		unsigned	int	sndAdr ;	// sender address, here: H/W address
		unsigned	int	rcvAdr ;	// receiver address, here: group address
                unsigned	char	info ;		// N_PDU message information:
							//   b=x.......	   0= Individual address
							//		   1= Group address
							//   b=.xxx....	0..7=	Routing counter
							//   b=....xxxx 0..15= useful message length
                unsigned	char	mtext[16] ;	// useful message
                unsigned	char	checksum ;	// received checksum
                unsigned	char	ownChecksum ;	// own calculated checksum (during reception)
		/**
		 * the following values are crteated during disassembly
		 */
		unsigned	char	repeat ;	// repeate message
		unsigned	char	prio ;		// priority
		unsigned	char	tlc ;		// from mtext[0]:xx......
							//	b00	UDP
							//	b01	NDP
							//	b10	UCD
							//	b11	NCD
		unsigned	char	seqNo ;		// sequence number:
							// from mtext[0]:..xxxx..
		unsigned	char	apci ;		// command identification
							// from mtext[0]:......xx and
							//      mtext[1]:xx......
		unsigned	char	ppCmd ;		// p2p command
							//	b00	open P2P commuincation
							//	b01	close/break down p2p
							//		communication
		unsigned	char	ppConf ;	// p2p Comfirmation
							//	b10	confirm ok
							//	b11	confirm not ok
        } knxMsg ;

#define	KNX_MSG_SIZE	sizeof( knxMsg)

#define	SHM_RCVMSG_KEY		10020
#define	SHM_SNDMSG_KEY		10021

#endif

