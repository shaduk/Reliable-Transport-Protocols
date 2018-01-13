#ifndef SIMULATOR_H_
#define SIMULATOR_H_

#define BIDIRECTIONAL 0

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
  char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
   int seqnum;
   int acknum;
   int checksum;
   char payload[20];
};

/* Implementation framework interface */
void A_output(struct msg message);
void B_output(struct msg message);
void A_input(struct pkt packet);
void A_timerinterrupt();
void A_init();

void B_input(struct pkt packet);
void B_init();

/* Simulator API */
void starttimer(int AorB, float increment);
void stoptimer(int AorB);
void tolayer3(int AorB, struct pkt packet);
void tolayer5(int AorB, char datasent[]);
int getwinsize();
float get_sim_time();

#endif