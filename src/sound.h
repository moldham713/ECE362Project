void setupDAC();
void setupDMA();
void setupTimer15();
void play_song();
void init_now(int);
void micro_wait(unsigned int);



int nothing[] = {
        0
};

int nothing_duration[] = {
        6
};


int win_noise[] = {
  2000, 3100
};

int win_duration[] = {
   6, 7
};





int main_noise[] = {
  3945, 0, 3562,  0,
  3562, 0, 3100,  0,
  3600, 0, 3300,  0,
  3562, 0, 3100,  0,

};

int main_duration[] = {
   6, 6, 6, 6,
   6, 6, 6, 6,
   6, 6, 6, 6,
   6, 6, 6, 6

};

int lose_noise[] = {
  3562, 0, 3200,  0,
  3945, 3300, 3400, 3300
};

int lose_duration[] = {
   6, 6, 8, 6,
   6, 8, 6, 6
};


int16_t wavetable[16];
int offset;
int counter;

int * currentSong = nothing;
int * currentDuration = nothing_duration;
int songLength = 1;
