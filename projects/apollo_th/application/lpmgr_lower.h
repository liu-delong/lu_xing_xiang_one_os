#ifndef __LPMGR_LOWER_H
#define __LPMGR_LOWER_H

void lpmgr_low_set(void);

void lpmgr_low_resume(void);

void ADCTurnOn(void);

void ADCTurnOFF(void);
void lpmgr_hard_init(void);
int nbModuleHardwareReset(int opt);

typedef struct _tag_LPMGR_TIME_S
{
    int run_time;
    int sleep_time;
}LPMGR_TIME_S;

LPMGR_TIME_S *lpmgr_time_get(void);


#endif /* __LPMGR_LOWER_H */

