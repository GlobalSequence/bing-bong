#include <AGTimerR4.h>
#include "ra4m1_dac_setup.h"
#include "analog_control.h"
#include "bing_bong_math.h"
#include "envelope.h"
#include "noise.h"
#include "pm_osc.h"
#include "svf.h"        

// buffer defines
#define BUF_LEN     (128)
#define NUM_BUF     (2)

// pin defines
#define NUMBER_OF_ANALOG_INPUTS 4
#define ADC_PIN1 A1
#define ADC_PIN2 A2
#define ADC_PIN3 A3
#define ADC_PIN4 A5_SCL
#define PUSH_PIN D7
#define TRIG_PIN D8
#define BING_BONG_PIN D9

// SVF Defines
#define NUMBER_OF_FILTERS 7
#define SPREAD_PARAM_MAX (12 * (ANALOG_TRUNCATED_BITS_TO_27B << 2))

// PM Oscillator Defines
#define NUMBER_OF_PM_OSCS 3

// output buffer
static volatile uint32_t out_buf[NUM_BUF][BUF_LEN] __attribute__((aligned(256)));

// SVF Variables
struct SVF_Variables svf_var[NUMBER_OF_FILTERS];
int8_t partial_spread_multiplier[NUMBER_OF_FILTERS] = {-3, -2, -1, 0, 1, 2, 3};

// PM Oscillator Variables
struct PM_Osc_Variables pm_var[NUMBER_OF_PM_OSCS];
uint32_t pm_env_amt_pitch;
uint32_t pm_env_amt_pm;

// envelope Params
struct Env_Params noise_env;
struct Env_Params pm_env;

// struct for noise variables
struct xorShiftVariables xor_s;

// DAC timer callback
void dacUpdate();
volatile bool buf_index;
bool last_buf_index;

// trigger
bool last_push_state;
volatile bool trig;
void trigIRQ() {trig = true;}

// analog input lock
LockPotParams l_pot[NUMBER_OF_ANALOG_INPUTS];

void setup() {
  // Serial.begin(115200);

  bingBongMathInit();
  analogReadResolution(12);

  buf_index = 0;
  last_buf_index = 1;

  // pinMode(4, OUTPUT); // speed test of buffering loop
  // pinMode(9, OUTPUT); // speed test of DAC ISR
  pinMode(PUSH_PIN, INPUT_PULLUP);
  pinMode(TRIG_PIN, INPUT_PULLUP);
  pinMode(BING_BONG_PIN, INPUT_PULLUP);
  attachInterrupt(TRIG_PIN, trigIRQ, FALLING);

  AGTimer.init(SAMPLE_RATE, dacUpdate);
  AGTimer.start();
  dacSetup();

  EnvInit(&noise_env);
  EnvInit(&pm_env);
  xorShift128Init(&xor_s);
}

void loop() {

  if(buf_index != last_buf_index) {
    last_buf_index = buf_index;
    // digitalWrite(4, HIGH);

  bool push = false;
  bool this_push_state = digitalRead(PUSH_PIN);
  if(this_push_state != last_push_state) {
      last_push_state = this_push_state;
      if(!this_push_state) {
          push = true;
      } 
  }
  
  if(push || trig) {
    trig = false;
    noise_env.value_ = 4095;
    EnvTrigger(RELEASE, &noise_env);
    EnvTrigger(ATTACK, &pm_env);
  }

    int a_read_1 = lockPotGetValue(analogRead(ADC_PIN1), millis(), &l_pot[0]);
    int a_read_2 = lockPotGetValue(analogRead(ADC_PIN2), millis(), &l_pot[1]);
    int a_read_3 = lockPotGetValue(analogRead(ADC_PIN3), millis(), &l_pot[2]);
    int a_read_4 = lockPotGetValue(analogRead(ADC_PIN4), millis(), &l_pot[3]);

    EnvUpdate(0, 0, 0, a_read_4 >> 5, &noise_env);
    EnvUpdate(4, 0, 0, a_read_4 >> 5, &pm_env);

    uint32_t noise_env_level = EnvRender(&noise_env) << 16;
    uint32_t pm_env_level = EnvRender(&pm_env) << 15;

    bool bing = digitalRead(BING_BONG_PIN);
    bool bong = !bing;

    if(bing) {
      pmDrum(a_read_1, a_read_2, a_read_3, pm_env_level);
    } else /* bong */{
      modalDrum(a_read_1, a_read_2, a_read_3, noise_env_level);
    }    
    // digitalWrite(4, LOW);
  }

}

void modalDrum(int c_param, int s_param, int q_param, uint32_t env_level) {

    int32_t cutoff = controlValBipolar(c_param);
    int32_t q = controlValPolar(q_param);
    int32_t spread = controlValPolar(s_param);

    for(int i = 0; i < NUMBER_OF_FILTERS; i++) {
      int32_t cutoff_spread_freq = ((spread >> 4) * partial_spread_multiplier[i]) + cutoff;
      SVFSetCutoff(cutoff_spread_freq, &svf_var[i]);
      SVFSetResonance(q, &svf_var[i]);
    }

    for(int i = 0; i < BUF_LEN; i++) {
        int32_t sample = 0;

        int32_t noise = ___SMMUL(xorShift128(&xor_s), env_level);
        for(int j = 0; j < NUMBER_OF_FILTERS; j++) {
            sample += SVFProcess(noise, BAND_PASS, &svf_var[j]) / NUMBER_OF_FILTERS;
            sample = __SSAT(sample, 31);
        } 
        out_buf[!buf_index][i] = __USAT((sample >> 16) + 2048, 12);
    }
}

void pmDrum(int c_param, int m_param, int p_param, uint32_t env_level) {

    int32_t pm_osc_carrier_freq = controlValBipolar(c_param);
    int32_t modulator_param_val = c_param + m_param - 1152;
    int32_t pm_osc_modulator_one_freq = controlValBipolar(modulator_param_val);
    // modulator_two base frequency is an octave up from modulator 1, 
    // which is 12 << 5 (7 bit freq table to 12 bit analog value);
    int32_t pm_osc_modulator_two_freq = controlValBipolar(modulator_param_val + 384);
    PMOscUpdateFreq(pm_osc_carrier_freq, &pm_var[0]);
    PMOscUpdateFreq(pm_osc_modulator_one_freq, &pm_var[1]);
    PMOscUpdateFreq(pm_osc_modulator_two_freq, &pm_var[2]);
    int32_t pm_env_amt_pitch = controlValPolar(p_param);

    const uint32_t pm_max_env_wave_val = ((1<<16) - 1) << 15;
    uint32_t pm_env_wave_modified = pm_env.stage_ == ATTACK? pm_max_env_wave_val : env_level;
    uint32_t pitch_mod_level = ___SMMUL(pm_env_wave_modified, pm_env_amt_pitch); 
    uint32_t pm_level = pm_env_wave_modified;
    uint32_t feedback_level = pm_level >> 1;

    for(int i = 0; i < NUMBER_OF_PM_OSCS; i++) {
        pm_var[i].freq += pitch_mod_level;
    }

    for(int i = 0; i < BUF_LEN; i++) {
        int32_t sample;
        static int32_t feedback;
        int32_t modulator_two = PMOscRender(___SMMUL(feedback, pm_level >> 1), &pm_var[2]);
        feedback = modulator_two;
        int32_t modulator_one = PMOscRender(___SMMUL(modulator_two, pm_level), &pm_var[1]);
        int32_t carrier = PMOscRender(___SMMUL(modulator_one, pm_level), &pm_var[0]);
        sample = ___SMMUL(carrier, env_level) << 1;
        out_buf[!buf_index][i] = (sample >> 16) + 2048;
    }
}

void dacUpdate() {
  static uint8_t buf_counter;
  uint8_t buf_sample_index;
  if(buf_counter < BUF_LEN) {
    buf_index = 0;
    buf_sample_index = buf_counter;
    // digitalWrite(9, HIGH);
  } else {
    buf_index = 1;
    buf_sample_index = buf_counter - BUF_LEN;
    // digitalWrite(9, LOW);
  }
  buf_counter++;
  *DAC12_DADR0 = out_buf[buf_index][buf_sample_index];
}