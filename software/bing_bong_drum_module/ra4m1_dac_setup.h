#ifndef RA4M1_DAC_SETUP_H
#define RA4M1_DAC_SETUP_H

// // 12-Bit D/A Converter.The reference source for the DAC settings is below.
// // https://github.com/Grumpy-Mike/Game_of_Life_with_sound
// // Credit to Hagiwo for finding this code: https://note.com/solder_state
#define DACBASE 0x40050000                                                      // DAC Base - DAC output on A0 (P014 AN09 DAC)
#define DAC12_DADR0 ((volatile unsigned short *)(DACBASE + 0xE000))             // D/A Data Register 0
#define DAC12_DACR ((volatile unsigned char *)(DACBASE + 0xE004))               // D/A Control Register
#define DAC12_DADPR ((volatile unsigned char *)(DACBASE + 0xE005))              // DADR0 Format Select Register
#define DAC12_DAADSCR ((volatile unsigned char *)(DACBASE + 0xE006))            // D/A A/D Synchronous Start Control Register
#define DAC12_DAVREFCR ((volatile unsigned char *)(DACBASE + 0xE007))           // D/A VREF Control Register
#define MSTP_MSTPCRD ((volatile unsigned int *)(MSTP + 0x7008))                 // Module Stop Control Register D
#define MSTPD20 20                                                              // DAC12  - 12-Bit D/A Converter Module
#define MSTP 0x40040000                                                         // Module Registers
#define MSTP_MSTPCRB ((volatile unsigned int *)(MSTP + 0x7000))                 // Module Stop Control Register B
#define PFS_P014PFS ((volatile unsigned int *)(PORTBASE + P000PFS + (14 * 4)))  // A0 / DAC12
#define PORTBASE 0x40040000                                                     /* Port Base */
#define P000PFS 0x0800                                                          // Port 0 Pin Function Select Register

static void dacSetup() {
    //DAC setting
  *MSTP_MSTPCRD &= ~(0x01 << MSTPD20);  // Enable DAC12 module
  *DAC12_DADPR = 0x00;                  // DADR0 Format Select Register - Set right-justified format
                                        //  *DAC12_DAADSCR  = 0x80;             // D/A A/D Synchronous Start Control Register - Enable
  *DAC12_DAADSCR = 0x00;                // D/A A/D Synchronous Start Control Register - Default
                                        // 36.3.2 Notes on Using the Internal Reference Voltage as the Reference Voltage
  *DAC12_DAVREFCR = 0x00;               // D/A VREF Control Register - Write 0x00 first - see 36.2.5
  *DAC12_DADR0 = 0x0000;                // D/A Data Register 0
  delayMicroseconds(10);                // Needed delay - see data sheet
  *DAC12_DAVREFCR = 0x01;               // D/A VREF Control Register - Select AVCC0/AVSS0 for Vref
  *DAC12_DACR = 0x5F;                   // D/A Control Register -
  delayMicroseconds(5);                 // Needed delay - see data sheet
  *DAC12_DADR0 = 2048;                  // D/A Data Register 0 - value of mid range bias
  *PFS_P014PFS = 0x00000000;            // Port Mode Control - Make sure all bits cleared
  *PFS_P014PFS |= (0x1 << 15);          // ... use as an analog pin
}

#endif