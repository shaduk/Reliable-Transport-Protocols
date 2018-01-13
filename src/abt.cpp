
#include "../include/simulator.h"
#include <string.h>
#include <queue>
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

/* called from layer 5, passed the data to be sent to other side */
using namespace std;
static int global_seq;
static int global_ack = 1;
static int rec_seq;
static float timeOut = 5.0;
static struct pkt current_pkt;
queue<msg> buffer;

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
	new_pkt->seqnum = global_seq;
	new_pkt->acknum = -1;
	strcpy(new_pkt->payload, message.data);
	new_pkt->checksum = checksum(*new_pkt);
	return new_pkt;
}

void A_output(struct msg message)
{
	if(global_ack == 1)
	{
		if(buffer.empty())
		{
			global_ack = 0;
			current_pkt = *make_packet(message);
			tolayer3(0, current_pkt);
			starttimer(0, timeOut);
		}
		else
		{
			global_ack = 0;
			current_pkt = *make_packet(buffer.front());
			buffer.pop();	
			buffer.push(message);
			tolayer3(0, current_pkt);
			starttimer(0, timeOut);
		}
	}
	else
	{
		buffer.push(message);
	}
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	if(packet.acknum == global_seq && packet.checksum == checksum(packet))
	{
		global_ack = 1;
		global_seq = 1 - global_seq;	//flip sequence
		stoptimer(0);			
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{

	tolayer3(0, current_pkt); 	
	starttimer(0, timeOut);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	global_ack = 1;
	global_seq = 0;
}


/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	struct pkt *ack = new pkt();
	ack->acknum = packet.seqnum;
	ack->seqnum = packet.seqnum;
	ack->checksum = checksum(*ack);

	if(rec_seq == packet.seqnum && checksum(packet) == packet.checksum)
	{
		tolayer5(1, packet.payload);
		tolayer3(1, *ack);
		rec_seq = 1 - rec_seq; 		 //flip sequence
	}	
	else if(rec_seq != packet.seqnum && checksum(packet) == packet.checksum)
	{

		tolayer3(1, *ack);	
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	rec_seq = 0;
}
