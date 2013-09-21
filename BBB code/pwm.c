#include "pwm.h"

/* Initialize PWM : 
1/ mmap /dev/mem to have write access to system clock 
2/ enable system clock (0x0 = disabled, 0x02 = enabled)
3/ set correct pin MUX 

can modify pwm variables through virtual filesystem:
"/sys/class/pwm/ehrpwm.1:0/..."

pwm drivers reference:
http://processors.wiki.ti.com/index.php/AM335x_PWM_Driver%27s_Guide */

static int pwminit = 0;
static int pwmstart = 0;

void pwm_init(void) {
    FILE *pwm_mux;
    int i;
    volatile uint32_t *epwmss1;
    
    int fd = open("/dev/mem", O_RDWR);
    
    if(fd < 0)
        {
        printf("Can't open /dev/mem\n");
        exit(1);
        }

    epwmss1 = (volatile uint32_t *) mmap(NULL, LENGTH, PROT_READ|PROT_WRITE, MAP_SHARED, fd, START);
    if(epwmss1 == NULL)
        {
        printf("Can't mmap\n");
        exit(1);
        }
    else
    	{
		epwmss1[OFFSET_1 / sizeof(uint32_t)] = 0x2;
		}
    close(fd);
    pwminit = 1;
    pwm_mux = fopen(PWMMuxPath, "w");
    fprintf(pwm_mux, "6");                                                      // pwm is mux mode 6
    fclose(pwm_mux);
}

//can change filepath of pwm module "/ehrpwm.%d:0/" by passing %d as argument
//depends how many pwm modules we have to run
//TODO:

void pwm_start(void) {
    FILE *pwm_run;
    pwm_run = fopen(PWMRunPath, "w");
    fprintf(pwm_run, "1");
    fclose(pwm_run);
	pwmstart = 1;
}

void pwm_stop(void) {
    FILE *pwm_run;
    pwm_run = fopen(PWMRunPath, "w");
    fprintf(pwm_run, "0");
    fclose(pwm_run);
	pwmstart = 0;
}

//duty_percent is just a regular percentage (i.e. 50 = 50%)

void pwm_set_duty(int duty_percent) {
    FILE *pwm_duty;
    pwm_duty = fopen(PWMDutyPath, "w"); 
    fprintf(pwm_duty, "%d", duty_percent);
    fclose(pwm_duty);
}

//freq is just normal frequency (i.e. 100 = 100Hz)

void pwm_set_period(int freq) {
    FILE *pwm_period;
    pwm_period = fopen(PWMFreqPath, "w");
    fprintf(pwm_period, "%d", freq);
    fclose(pwm_period);
}