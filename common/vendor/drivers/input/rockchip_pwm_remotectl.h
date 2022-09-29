/* SPDX-License-Identifier: GPL-2.0 */

#ifndef H_ROCKCHIP_PWM_REMOTECTL_H
#define H_ROCKCHIP_PWM_REMOTECTL_H

#define MAX_NUM_KEYS 60
#define PWM_PWR_KEY_CAPURURE_MAX 10

/* PWM0 registers  */
#define PWM_REG_CNTR 0x00    /* Counter Register */
#define PWM_REG_HPR 0x04     /* Period Register */
#define PWM_REG_LPR 0x08     /* Duty Cycle Register */
#define PWM_REG_CTRL 0x0c    /* Control Register */
#define PWM3_REG_INTSTS 0x10 /* Interrupt Status Refister For Pwm3*/
#define PWM2_REG_INTSTS 0x20 /* Interrupt Status Refister For Pwm2*/
#define PWM1_REG_INTSTS 0x30 /* Interrupt Status Refister For Pwm1*/
#define PWM0_REG_INTSTS 0x40 /* Interrupt Status Refister For Pwm0*/
#define PWM3_REG_INT_EN 0x14 /* Interrupt Enable Refister For Pwm3*/
#define PWM2_REG_INT_EN 0x24 /* Interrupt Enable Refister For Pwm2*/
#define PWM1_REG_INT_EN 0x34 /* Interrupt Enable Refister For Pwm1*/
#define PWM0_REG_INT_EN 0x44 /* Interrupt Enable Refister For Pwm0*/

/*REG_CTRL bits definitions*/
#define PWM_ENABLE (1 << 0)
#define PWM_DISABLE (0 << 0)

/*operation mode*/
#define PWM_MODE_ONESHOT (0x00 << 1)
#define PWM_MODE_CONTINUMOUS (0x01 << 1)
#define PWM_MODE_CAPTURE (0x02 << 1)

/*duty cycle output polarity*/
#define PWM_DUTY_POSTIVE (0x01 << 3)
#define PWM_DUTY_NEGATIVE (0x00 << 3)

/*incative state output polarity*/
#define PWM_INACTIVE_POSTIVE (0x01 << 4)
#define PWM_INACTIVE_NEGATIVE (0x00 << 4)

/*clock source select*/
#define PWM_CLK_SCALE (1 << 9)
#define PWM_CLK_NON_SCALE (0 << 9)

#define PWM_CH0_INT (1 << 0)
#define PWM_CH1_INT (1 << 1)
#define PWM_CH2_INT (1 << 2)
#define PWM_CH3_INT (1 << 3)
#define PWM_PWR_KEY_INT (1 << 7)

#define PWM_CH0_POL (1 << 8)
#define PWM_CH1_POL (1 << 9)
#define PWM_CH2_POL (1 << 10)
#define PWM_CH3_POL (1 << 11)

#define PWM_CH0_INT_ENABLE (1 << 0)
#define PWM_CH0_INT_DISABLE (0 << 0)

#define PWM_CH1_INT_ENABLE (1 << 1)
#define PWM_CH1_INT_DISABLE (0 << 1)

#define PWM_CH2_INT_ENABLE (1 << 2)
#define PWM_CH2_INT_DISABLE (0 << 2)

#define PWM_CH3_INT_ENABLE (1 << 3)
#define PWM_CH3_INT_DISABLE (0 << 3)

#define PWM_INT_ENABLE 1
#define PWM_INT_DISABLE 0

/*prescale factor*/
#define PWMCR_MIN_PRESCALE 0x00
#define PWMCR_MAX_PRESCALE 0x07

#define PWMDCR_MIN_DUTY 0x0001
#define PWMDCR_MAX_DUTY 0xFFFF

#define PWMPCR_MIN_PERIOD 0x0001
#define PWMPCR_MAX_PERIOD 0xFFFF

#define PWMPCR_MIN_PERIOD 0x0001
#define PWMPCR_MAX_PERIOD 0xFFFF

enum pwm_div {
    PWM_DIV1 = (0x0 << 12),
    PWM_DIV2 = (0x1 << 12),
    PWM_DIV4 = (0x2 << 12),
    PWM_DIV8 = (0x3 << 12),
    PWM_DIV16 = (0x4 << 12),
    PWM_DIV32 = (0x5 << 12),
    PWM_DIV64 = (0x6 << 12),
    PWM_DIV128 = (0x7 << 12),
};

/*  NEC Protocol */
#define RK_PWM_TIME_PRE_MIN 4000
#define RK_PWM_TIME_PRE_MAX 5000

#define RK_PWM_TIME_PRE_MIN_LOW 8000
#define RK_PWM_TIME_PRE_MAX_LOW 10000

#define RK_PWM_TIME_BIT0_MIN 390
#define RK_PWM_TIME_BIT0_MAX 730

#define RK_PWM_TIME_BIT1_MIN 1300
#define RK_PWM_TIME_BIT1_MAX 2000

#define RK_PWM_TIME_BIT_MIN_LOW 390
#define RK_PWM_TIME_BIT_MAX_LOW 730

#define RK_PWM_TIME_RPT_MIN 2000
#define RK_PWM_TIME_RPT_MAX 2500

#define RK_PWM_TIME_SEQ1_MIN 95000
#define RK_PWM_TIME_SEQ1_MAX 98000

#define RK_PWM_TIME_SEQ2_MIN 30000
#define RK_PWM_TIME_SEQ2_MAX 55000

#define PWM_REG_INTSTS(n) ((3 - (n)) * 0x10 + 0x10)
#define PWM_REG_INT_EN(n) ((3 - (n)) * 0x10 + 0x14)
#define RK_PWM_VERSION_ID(n) ((3 - (n)) * 0x10 + 0x2c)
#define PWM_REG_PWRMATCH_CTRL(n) ((3 - (n)) * 0x10 + 0x50)
#define PWM_REG_PWRMATCH_LPRE(n) ((3 - (n)) * 0x10 + 0x54)
#define PWM_REG_PWRMATCH_HPRE(n) ((3 - (n)) * 0x10 + 0x58)
#define PWM_REG_PWRMATCH_LD(n) ((3 - (n)) * 0x10 + 0x5C)
#define PWM_REG_PWRMATCH_HD_ZERO(n) ((3 - (n)) * 0x10 + 0x60)
#define PWM_REG_PWRMATCH_HD_ONE(n) ((3 - (n)) * 0x10 + 0x64)
#define PWM_PWRMATCH_VALUE(n) ((3 - (n)) * 0x10 + 0x68)
#define PWM_PWRCAPTURE_VALUE(n) ((3 - (n)) * 0x10 + 0x9c)

#define PWM_CH_INT(n) BIT(n)
#define PWM_CH_POL(n) BIT((n) + 8)

#define PWM_CH_INT_ENABLE(n) BIT(n)
#define PWM_PWR_INT_ENABLE BIT(7)
#define CH3_PWRKEY_ENABLE BIT(3)

typedef enum _RMC_STATE {
    RMC_IDLE,
    RMC_PRELOAD,
    RMC_USERCODE,
    RMC_GETDATA,
    RMC_SEQUENCE,
} eRMC_STATE;

struct RKxx_remotectl_platform_data {
    int nbuttons;
    int rep;
    int timer;
    int wakeup;
};

#endif
