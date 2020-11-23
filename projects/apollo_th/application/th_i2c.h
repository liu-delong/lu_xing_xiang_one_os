#ifndef __TH_I2C_H_
#define __TH_I2C_H_

// Error codes
typedef enum{
  NO_ERROR       = 0x00, // no error
  ACK_ERROR      = 0x01, // no acknowledgment error
  CHECKSUM_ERROR = 0x02, // checksum mismatch error
  TIMEOUT_ERROR  = 0x04, // timeout error
  PARM_ERROR     = 0x80, // parameter out of range error
}etError;

//-- Enumerations -------------------------------------------------------------
// Sensor Commands
typedef enum
{
  CMD_READ_SERIALNBR  = 0x3780, // read serial number
  CMD_READ_STATUS     = 0xF32D, // read status register
  CMD_CLEAR_STATUS    = 0x3041, // clear status register
  CMD_HEATER_ENABLE   = 0x306D, // enabled heater
  CMD_HEATER_DISABLE  = 0x3066, // disable heater
  CMD_SOFT_RESET      = 0x30A2, // soft reset
  CMD_MEAS_CLOCKSTR_H = 0x2C06, // measurement: clock stretching, high repeatability
  CMD_MEAS_CLOCKSTR_M = 0x2C0D, // measurement: clock stretching, medium repeatability
  CMD_MEAS_CLOCKSTR_L = 0x2C10, // measurement: clock stretching, low repeatability
  CMD_MEAS_POLLING_H  = 0x2400, // measurement: polling, high repeatability
  CMD_MEAS_POLLING_M  = 0x240B, // measurement: polling, medium repeatability
  CMD_MEAS_POLLING_L  = 0x2416, // measurement: polling, low repeatability
  CMD_MEAS_PERI_05_H  = 0x2032, // measurement: periodic 0.5 mps, high repeatability
  CMD_MEAS_PERI_05_M  = 0x2024, // measurement: periodic 0.5 mps, medium repeatability
  CMD_MEAS_PERI_05_L  = 0x202F, // measurement: periodic 0.5 mps, low repeatability
  CMD_MEAS_PERI_1_H   = 0x2130, // measurement: periodic 1 mps, high repeatability
  CMD_MEAS_PERI_1_M   = 0x2126, // measurement: periodic 1 mps, medium repeatability
  CMD_MEAS_PERI_1_L   = 0x212D, // measurement: periodic 1 mps, low repeatability
  CMD_MEAS_PERI_2_H   = 0x2236, // measurement: periodic 2 mps, high repeatability
  CMD_MEAS_PERI_2_M   = 0x2220, // measurement: periodic 2 mps, medium repeatability
  CMD_MEAS_PERI_2_L   = 0x222B, // measurement: periodic 2 mps, low repeatability
  CMD_MEAS_PERI_4_H   = 0x2334, // measurement: periodic 4 mps, high repeatability
  CMD_MEAS_PERI_4_M   = 0x2322, // measurement: periodic 4 mps, medium repeatability
  CMD_MEAS_PERI_4_L   = 0x2329, // measurement: periodic 4 mps, low repeatability
  CMD_MEAS_PERI_10_H  = 0x2737, // measurement: periodic 10 mps, high repeatability
  CMD_MEAS_PERI_10_M  = 0x2721, // measurement: periodic 10 mps, medium repeatability
  CMD_MEAS_PERI_10_L  = 0x272A, // measurement: periodic 10 mps, low repeatability
  CMD_FETCH_DATA      = 0xE000, // readout measurements for periodic mode
  CMD_R_AL_LIM_LS     = 0xE102, // read alert limits, low set
  CMD_R_AL_LIM_LC     = 0xE109, // read alert limits, low clear
  CMD_R_AL_LIM_HS     = 0xE11F, // read alert limits, high set
  CMD_R_AL_LIM_HC     = 0xE114, // read alert limits, high clear
  CMD_W_AL_LIM_HS     = 0x611D, // write alert limits, high set
  CMD_W_AL_LIM_HC     = 0x6116, // write alert limits, high clear
  CMD_W_AL_LIM_LC     = 0x610B, // write alert limits, low clear
  CMD_W_AL_LIM_LS     = 0x6100, // write alert limits, low set
  CMD_NO_SLEEP        = 0x303E,
}etCommands;

// Measurement Repeatability
typedef enum{
  REPEATAB_HIGH,   // high repeatability
  REPEATAB_MEDIUM, // medium repeatability
  REPEATAB_LOW,    // low repeatability
}etRepeatability;

int SHT3X_I2C_init(void);

etError SHT3X_GetTempAndHumiClkStretch(float* temperature, float* humidity,
                                       etRepeatability repeatability,
                                       os_uint8_t timeout);


#endif /* __TH_I2C_H_ */

