#include "../include/simulator.h"
#include <string.h>
#include <vector>
#include <iostream>
using namespace std;
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

static int base;
static int nextseqnum;
static int message_seq = 0;
static int expectedseqnum;
static int N;
static int buffer_size = 0;
static struct pkt current_pkt;
static float RTT;

vector<pkt> buffer;

int checksum(struct pkt packet)
{
	int check_s = 0;
	for(int i = 0; i < 20; i++)
	{	
		check_s += packet.payload[i];	
	}
	check_s += packet.seqnum;
	check_s += packet.acknum;
	return check_s;
} 

struct pkt *make_packet(struct msg message)
{
	struct pkt *new_pkt = new pkt();
	new_pkt->seqnum = message_seq;
	new_pkt->acknum = -1;
	strcpy(new_pkt->payload, message.data);
	new_pkt->checksum = checksum(*new_pkt);
	return new_pkt;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
	if(nextseqnum < base + N)
	{
        buffer.push_back(*make_packet(message));
		tolayer3(0, buffer[message_seq]);
		if(base == nextseqnum)
			starttimer(0, RTT);
        nextseqnum++;
	}
    else
    {
        buffer.push_back(*make_packet(message));
        buffer_size++;
    }
    message_seq++;  
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	if(checksum(packet) == packet.checksum && packet.acknum >= base)
	{
        base = packet.acknum + 1;
        stoptimer(0);
        if(base < nextseqnum)
            starttimer(0, RTT); 
        for(int i = nextseqnum; i < base + N && buffer_size != 0; i++)
        {
            tolayer3(0, buffer[i]);
            if(base == nextseqnum)
                starttimer(0, RTT);
            nextseqnum++;
            buffer_size--;  
        }
	}
                                                                    

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	starttimer(0, RTT);
	for(int i = base; i < nextseqnum; i++)
	{
		tolayer3(0, buffer[i]);
	}
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	base = 0;
	nextseqnum = 0;
	N = getwinsize();
	RTT = 20.0;
}
/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	cout << "My expected seq is " << expectedseqnum << endl;
	if(packet.checksum == checksum(packet) && expectedseqnum == packet.seqnum)
	{
        struct pkt *ack = new pkt();
        ack->acknum = expectedseqnum;
        ack->seqnum = expectedseqnum;
        ack->checksum = checksum(*ack);
		tolayer5(1, packet.payload);
		tolayer3(1, *ack);
        expectedseqnum++;
	}
	else if(packet.checksum == checksum(packet))
	{
        struct pkt *ack = new pkt();
        ack->acknum = expectedseqnum-1;
        ack->seqnum = expectedseqnum-1;
        ack->checksum = checksum(*ack);
		tolayer3(1, *ack);
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	expectedseqnum = 0;
}

