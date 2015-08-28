#ifndef POSITION__H__
#define POSITION__H__

void position_init();       // initializes the position module  and the necessary positions

unsigned int position_load();        // load a trajectory from the client and return its length

void position_reset();      // reset the current trajectory position

void position_gains_read(float gain1, float gain2, float gain3); // read the position gains from the client

void position_gains_send(); // send the position gains to the client

void hold_final_traj_pos(int additional_samples, int traj_len); // have PIC32 hold final trajectory position for N more samples

#endif
