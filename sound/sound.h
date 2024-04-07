
#define SOUNDBUFFMAX 1024 // Size of sound buffers
#define SOUNDPACKET 100 //Size of a sound packet, must be less than SoundBuffMax-1
uint8_t fring(uint8_t s_in);
uint8_t ftext(uint8_t s_in);
void SetPWM(void);
void SetA2D(void);
void ZeroMikeBuffer(void);
void ZeroSpkBuffer(void);
void AddToSpkBuffer(uint8_t v);
void AddToMikeBuffer(uint8_t v);
uint8_t GetFromSpkBuffer(void);
bool Sound_Timer_Callback(struct repeating_timer *t);
void init_sound(void);
void send_sound(void);
void StartSoundTimer(void);

