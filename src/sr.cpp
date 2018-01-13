#include "../include/simulator.h"
#include <vector>
#include <string.h>
#include <iostream>

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

/* called from layer 5, passed the  data to be sent to other side */

using namespace std;

static int A_Window;
static int send_base;
static int nextseqnum;

static int B_Window;
static int expectedseqnum;
static int lasttimer = -1;

static float RTT = 35.0;

static struct pkt current_pkt;

vector<pkt> send_buffer;
static struct pkt rcv_buffer[1010];
vector<int> timer;

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
  new_pkt->seqnum = nextseqnum;
  new_pkt->acknum = -1;
  strcpy(new_pkt->payload, message.data);
  new_pkt->checksum = checksum(*new_pkt);
  return new_pkt;
}


void A_output(struct msg message)
{
  send_buffer.push_back(*make_packet(message));
  timer.push_back(0);
  if(nextseqnum >= send_base && (nextseqnum < send_base + A_Window))
  {
   cout << "sending from A_output:" << send_buffer[nextseqnum].payload << endl; 
   timer[nextseqnum] = get_sim_time();
    tolayer3(0, send_buffer[nextseqnum]);

  }
  if(send_base == nextseqnum)
  {
      starttimer(0, RTT);
      lasttimer = send_base;
  }
  nextseqnum++;
}

/* called from layer 3, when a packet arrives for layer 4 */

void A_input(struct pkt packet)
{
  if((checksum(packet) == packet.checksum) && (packet.acknum >= send_base && (packet.acknum < send_base + A_Window)))
  {
    stoptimer(0);
    cout << "ack Received for: " << send_buffer[packet.seqnum].payload << endl;
    send_buffer[packet.seqnum].acknum = true;

    for(int i = send_base; i < nextseqnum; i++)
    {
      if(send_buffer[i].acknum == -1 && get_sim_time() - timer[i] < RTT)
      {
        int time_left = RTT - (get_sim_time() - timer[i]);
          starttimer(0, time_left);
          lasttimer = i;
          break;
      }

    }
    for(int i = send_base; i < nextseqnum; i++)
    {
     if(send_buffer[i].acknum == -1 && get_sim_time() - timer[i] > RTT)
      {
        cout << "sending from timeout:" << send_buffer[send_base + A_Window].payload << endl; 
        tolayer3(0, send_buffer[i]);
        timer[i] = get_sim_time();
      }

    }

    if(send_base == packet.acknum)
    {
      while(send_buffer[send_base].acknum == true)
      {
          send_base++;
          if(send_base + A_Window < nextseqnum && send_buffer[send_base + A_Window].acknum == -1)
          {
            tolayer3(0, send_buffer[send_base + A_Window]);
            cout << "sending from A_input:" << send_buffer[send_base + A_Window].payload << endl; 
            timer[send_base + A_Window] = get_sim_time();
          }
      }
        /*
        if(send_buffer[send_base].acknum == -1)
        {
            int time_left = RTT - (get_sim_time() - timer[send_base]);
            starttimer(0, time_left);
            lasttimer = send_base;
            break;
        } */
      }
    
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{ 
    tolayer3(0, send_buffer[lasttimer]);
    timer[lasttimer] = get_sim_time();
    for(int i = send_base; i < nextseqnum; i++)
    {
      if(send_buffer[i].acknum == -1 && get_sim_time() - timer[i] < RTT)
      {
        int time_left = RTT - (get_sim_time() - timer[i]);
          starttimer(0, time_left);
          lasttimer = i;
          break;
      }

    }

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  send_base = 0;
  nextseqnum = 0;
  A_Window = getwinsize();
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{

  
  struct pkt *ack = new pkt();
  ack->acknum = packet.seqnum;
  ack->seqnum = packet.seqnum;
  ack->checksum = checksum(*ack);
  
  if(packet.checksum == checksum(packet) && packet.seqnum >= expectedseqnum && packet.seqnum < expectedseqnum + B_Window)
  {
    cout << "Received in B_input: " << packet.payload << endl;
    tolayer3(1, *ack);
    rcv_buffer[packet.seqnum] = packet;
    rcv_buffer[packet.seqnum].acknum = 1;
    
    cout << "Expected Seq Num " <<  expectedseqnum << endl;
    if(expectedseqnum == packet.seqnum)
    {
      cout << rcv_buffer[expectedseqnum].acknum << endl;
      while(rcv_buffer[expectedseqnum].acknum == 1)
      {
        cout << "For sequence " << expectedseqnum << " delivered" << endl;
        cout << "Delivered to layer :" << rcv_buffer[expectedseqnum].payload << endl;
        /* for(int i = 0; i < expectedseqnum + B_Window; i++)
        {
          cout << "printing array seq " << i << " " << rcv_buffer[i].acknum << endl;
        } */
        tolayer5(1, rcv_buffer[expectedseqnum].payload);
        expectedseqnum++;
        cout << "Now check ack for " << expectedseqnum << " it is " << rcv_buffer[expectedseqnum].acknum << endl;
      }
    }
  }
  else if(packet.checksum == checksum(packet) && packet.seqnum >= 0 && packet.seqnum < expectedseqnum)
  {
    tolayer3(1, *ack);
  } 
  
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  expectedseqnum = 0;
  B_Window = getwinsize();
  for(int i = 0; i < 1010; i++)
  {
    rcv_buffer[i].acknum = 0;
    rcv_buffer[i].seqnum = 0;
    memset(rcv_buffer[i].payload,0, sizeof(rcv_buffer[i].payload)); 
  }
}
