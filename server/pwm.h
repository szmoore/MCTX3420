#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sched.h>
#include <stdint.h>

#define START    	0x44e00000    // see datasheet of AM335x
#define LENGTH   	1024
#define OFFSET_1 	0xcc          // offset of PWM1 clock (see datasheet of AM335x p.1018)
#define FREQ	 	50			   //50Hz pwm frequency for pressure regulator
#define PWMMuxPath	"/sys/kernel/debug/omap_mux/gpmc_a2"
#define PWMRunPath	"/sys/class/pwm/ehrpwm.1:0/run"
#define PWMDutyPath	"/sys/class/pwm/ehrpwm.1:0/duty_percent"
#define PWMFreqPath	"/sys/class/pwm/ehrpwm.1:0/period_freq"

void pwm_init(void);
void pwm_start(void);
void pwm_stop(void);
void pwm_set_duty(int duty_percent);
void pwm_set_period(int freq);