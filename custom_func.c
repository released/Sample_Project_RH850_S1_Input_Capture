/*_____ I N C L U D E S ____________________________________________________*/
// #include <stdio.h>
#include <string.h>
#include "r_smc_entry.h"

#include "misc_config.h"
#include "custom_func.h"
#include "retarget.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

volatile struct flag_32bit flag_PROJ_CTL;
#define FLAG_PROJ_TIMER_PERIOD_1000MS                 	(flag_PROJ_CTL.bit0)
#define FLAG_PROJ_TIMER_PERIOD_100MS                    (flag_PROJ_CTL.bit1)
#define FLAG_PROJ_TIMER_PERIOD_SPECIFIC                 (flag_PROJ_CTL.bit2)
#define FLAG_PROJ_REVERSE3                              (flag_PROJ_CTL.bit3)
#define FLAG_PROJ_REVERSE4                              (flag_PROJ_CTL.bit4)
#define FLAG_PROJ_REVERSE5                              (flag_PROJ_CTL.bit5)
#define FLAG_PROJ_TRIG_BTN1                             (flag_PROJ_CTL.bit6)
#define FLAG_PROJ_TRIG_BTN2                             (flag_PROJ_CTL.bit7)

#define FLAG_PROJ_TRIG_1                                (flag_PROJ_CTL.bit8)
#define FLAG_PROJ_TRIG_2                                (flag_PROJ_CTL.bit9)
#define FLAG_PROJ_TRIG_3                                (flag_PROJ_CTL.bit10)
#define FLAG_PROJ_TRIG_4                                (flag_PROJ_CTL.bit11)
#define FLAG_PROJ_TRIG_5                                (flag_PROJ_CTL.bit12)
#define FLAG_PROJ_TRIG_6                                (flag_PROJ_CTL.bit13)
#define FLAG_PROJ_REVERSE14                             (flag_PROJ_CTL.bit14)
#define FLAG_PROJ_REVERSE15                             (flag_PROJ_CTL.bit15)

#define FLAG_PROJ_GET_WIDTH_MEASUREMENT                 (flag_PROJ_CTL.bit16)
#define FLAG_PROJ_GET_PERIOD_MEASUREMENT                (flag_PROJ_CTL.bit17)
#define FLAG_PROJ_WIDTH_HIGH_TRIG                       (flag_PROJ_CTL.bit18)
#define FLAG_PROJ_WIDTH_LOW_TRIG                        (flag_PROJ_CTL.bit19)
#define FLAG_PROJ_REVERSE20                             (flag_PROJ_CTL.bit20)
#define FLAG_PROJ_REVERSE21                             (flag_PROJ_CTL.bit21)
#define FLAG_PROJ_PWM_DUTY_INC                          (flag_PROJ_CTL.bit22)
#define FLAG_PROJ_PWM_DUTY_DEC                          (flag_PROJ_CTL.bit23)

/*_____ D E F I N I T I O N S ______________________________________________*/

volatile unsigned short counter_tick = 0U;
volatile unsigned long ostmr_tick = 0U;
volatile unsigned long btn_counter_tick = 0U;

#define BTN_PRESSED_LONG                                (2500U)

#pragma section privateData

const unsigned char dummy_3 = 0x5AU;

volatile unsigned char dummy_2 = 0xFFU;

volatile unsigned char dummy_1;

#pragma section default

volatile unsigned long g_u32_counter = 0U;

unsigned char g_uart0rxbuf = 0U;                                 /* UART0 receive buffer */
unsigned char g_uart0rxerr = 0U;                                 /* UART0 receive error status */


#define PWM_RESOLUTION                                  (100U)
volatile unsigned short pwm_clk_src = 0U;
volatile unsigned short pwm_duty[16] = {0U};
volatile unsigned char pwm_ch_idx = 1U;

unsigned char g_capturelogflag = 1U;
// #define ENABLE_AVG
#define INPUT_PULSE_CAPTURE_PORT                        (GPIO_GETLEVEL(9,0))
#define INPUT_PULSE_MEASURE_START(void)                 (R_Config_TAUD0_0_Start())
#define INPUT_PULSE_MEASURE_STOP(void)                  (R_Config_TAUD0_0_Stop())
#define INPUT_PULSE_IRQ_FLAG                            (INTC1.ICTAUD0I0.BIT.RFTAUD0I0)

#define PERIOD_MEASURE_TIME                             (8U)
unsigned long g_pulse_width[PERIOD_MEASURE_TIME] = {0U};    /* Store pulse width */
unsigned char g_times = 0U;                                 /* Measurement times counter */
unsigned long  period_total = 0U;

#define WIDTH_MEASURE_TIME                              (4U)
unsigned char g_times_low = 0U;                             /* Measurement times counter (low level width) */
unsigned char g_times_high = 0U;                            /* Measurement times counter (high level width) */
unsigned char g_count = WIDTH_MEASURE_TIME;                 /* Number of measurement */
volatile unsigned char g_edge_flag = 0xFFU;                 /* Edge flag */ 
unsigned char g_port_data[2] = {0U, 0U};                    /* Store PULSE level */
unsigned long g_width_low[WIDTH_MEASURE_TIME] = {0U};       /* Store low level width */
unsigned long g_width_high[WIDTH_MEASURE_TIME] = {0U};      /* Store high level width */
unsigned char g_times_invalid = 0x00U;                      /* Invalid edge counter */

unsigned long pulse_high = 0U;
unsigned long pulse_low = 0U;
unsigned long pulse_total = 0U;

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
extern volatile uint32_t  g_taud0_ch0_width;

unsigned long btn_get_tick(void)
{
	return (btn_counter_tick);
}

void btn_set_tick(unsigned long t)
{
	btn_counter_tick = t;
}

void btn_tick_counter(void)
{
	btn_counter_tick++;
    if (btn_get_tick() >= 60000U)
    {
        btn_set_tick(0U);
    }
}

void ostmr_tick_counter(void)
{
	ostmr_tick++;
}

void ostmr_1ms_IRQ(void)
{
	ostmr_tick_counter();
}

void ostimer_dealyms(unsigned long ms)
{
    R_Config_OSTM0_Start();
    ostmr_tick = 0U;

    while(ostmr_tick < ms);

    R_Config_OSTM0_Stop();

}

unsigned short get_tick(void)
{
	return (counter_tick);
}

void set_tick(unsigned short t)
{
	counter_tick = t;
}

void tick_counter(void)
{
	counter_tick++;
    if (get_tick() >= 60000U)
    {
        set_tick(0U);
    }
}

void delay_ms(unsigned long ms)
{
    unsigned long tickstart = get_tick();
    unsigned long wait = ms;
	unsigned long tmp = 0U;
	
    while (1)
    {
		if (get_tick() > tickstart)	// tickstart = 59000 , tick_counter = 60000
		{
			tmp = get_tick() - tickstart;
		}
		else // tickstart = 59000 , tick_counter = 2048
		{
			tmp = 60000U -  tickstart + get_tick();
		}		
		
		if (tmp > wait)
			break;
    }
}


/*
    Set the capture pin input valid edge to “Both edge”.
*/
void PulseWidthCapture_Process_in_IRQ(void)    
{
    g_port_data[0] = INPUT_PULSE_CAPTURE_PORT;
    // NOP();
    // NOP();
    // NOP();
    // NOP();
    g_port_data[1] = INPUT_PULSE_CAPTURE_PORT;

    if ((0U == INPUT_PULSE_IRQ_FLAG) && ((0x01U & g_port_data[0]) == (0x01U & g_port_data[1])))
    {
        g_edge_flag = (g_port_data[0] & 0x01U);    /* Set edge flag */

        if (0U == g_edge_flag)    /* Store high level width */
        {
            #if defined (ENABLE_AVG)
            if (0U < g_times_high)    /* 4 times measurement finished ? */
            {
                g_width_high[g_count - g_times_high] = g_taud0_ch0_width;    /* Store capture value */
                g_times_high--;
            }
            else
            {
                FLAG_PROJ_WIDTH_HIGH_TRIG = 1;
            }
            #else
            pulse_high = g_taud0_ch0_width;
            FLAG_PROJ_WIDTH_HIGH_TRIG = 1;

            #endif
        }
        else    /* Store low level width */
        {
            #if defined (ENABLE_AVG)
            if (0U < g_times_low)     /* 4 times measurement finished ? */
            {
                g_width_low[g_count-g_times_low] = g_taud0_ch0_width;    /* Store capture value */
                g_times_low--;
            }
            else
            {
                FLAG_PROJ_WIDTH_LOW_TRIG = 1;
            }
            #else
            pulse_low = g_taud0_ch0_width;
            FLAG_PROJ_WIDTH_LOW_TRIG = 1;
            #endif
         }
    }
    else     /* Interrupt request is generated */
    {
        g_times_invalid++;      /* Increment counter of invalid edge */
        INPUT_PULSE_IRQ_FLAG = 0U;            /* Clear interrupt request */
    }
}

void PulseWidthCapture_Process_logging(void)
{
    if (FLAG_PROJ_GET_WIDTH_MEASUREMENT)
    {
        FLAG_PROJ_GET_WIDTH_MEASUREMENT = 0U;
        pulse_total = pulse_high + pulse_low;

        if (g_capturelogflag)
        {
            tiny_printf("H:%4d(0x%04X),L:%4d(0x%04X),total:%4d,duty:%f\r\n" , 
            pulse_high,pulse_high,
            pulse_low,pulse_low,
            pulse_total,(float) pulse_high/pulse_total*100); 

            // tiny_printf("H:%4d(0x%04X),L:%4d(0x%04X)\r\n" ,pulse_high,pulse_high,pulse_low,pulse_low); 

            tiny_printf("\r\n");
        }

        pulse_high = 0U;
        pulse_low = 0U;
        pulse_total = 0U;

    }
}

/*
    refer to 
    RL78/G23 
    Timer Array Unit (Pulse Interval Measurement: Width) 
*/
void PulseWidthCapture_Process_in_polling(void) //duty
{
    #if 1
    // unsigned short  i = 0U;

    if (INPUT_PULSE_IRQ_FLAG)
    {
        INPUT_PULSE_IRQ_FLAG = 0U;

        g_times_low = g_count;     /* Set number of measurement */
        g_times_high = g_count;

        #if 1
        do
        {
            /* code */
        } while ( !FLAG_PROJ_WIDTH_HIGH_TRIG && !FLAG_PROJ_WIDTH_LOW_TRIG );

        INPUT_PULSE_MEASURE_STOP();    /* Stop timer counter */
        FLAG_PROJ_WIDTH_HIGH_TRIG = 0U;
        FLAG_PROJ_WIDTH_LOW_TRIG = 0U;
        
        #else
        while(1)
        {
            if ((0U == g_times_low) && (0U == g_times_high))
            {
                INPUT_PULSE_MEASURE_STOP();    /* Stop timer counter */
                break;
            }
        }
        #endif

        #if defined (ENABLE_AVG)
        // sum
        for (i = 0U ; i < WIDTH_MEASURE_TIME ; i++)
        {
            pulse_high += g_width_high[i];
            pulse_low += g_width_low[i];          
        }

        // avg
        pulse_high = pulse_high / WIDTH_MEASURE_TIME;
        pulse_low = pulse_low / WIDTH_MEASURE_TIME;
        #endif

        // calculate
        // cal_duty = (float) pulse_high/pulse_total;
        // cal_duty *= 100;

        FLAG_PROJ_GET_WIDTH_MEASUREMENT = 1;

        INPUT_PULSE_MEASURE_START(); 
    }


    #else
    while (0U == INPUT_PULSE_IRQ_FLAG)
    {
        HALT();    /* Wait first edge detection */
    }
    INPUT_PULSE_IRQ_FLAG = 0U;    /* Clear interrupt flag */
    
    g_times_low = g_count;     /* Set number of measurement */
    g_times_high = g_count;
    
    EI();         /* Enable interrupt */

    /* Wait for the measurement to finish */
    while (1U)
    {
        HALT();
        if ((0U == g_times_low) && (0U == g_times_high))
        {
            INPUT_PULSE_MEASURE_STOP();    /* Stop timer counter */
        }
    }
    #endif
}

/*
    Set the capture pin input valid edge to “Rising edge”. 
*/
void PulsePeriodCapture_Process_in_IRQ(void)    
{
    g_pulse_width[PERIOD_MEASURE_TIME - g_times] = g_taud0_ch0_width;    /* Store capture value */

    if (0U == ( --g_times))        /* 8 times measurement finished ? */
    {
        INPUT_PULSE_MEASURE_STOP();    /* Stops timer counter */
    }
}


void PulsePeriodCapture_Process_logging(void)
{
    if (FLAG_PROJ_GET_PERIOD_MEASUREMENT)
    {
        FLAG_PROJ_GET_PERIOD_MEASUREMENT = 0U;

        if (g_capturelogflag)
        {
            tiny_printf("PERIOD:%4d(0x%04X)\r\n" , period_total,period_total);    
            tiny_printf("\r\n");            
        }
        
        period_total = 0U;
    }
}

/*
    refer to 
    RL78/G23 
    Timer Array Unit (Pulse Interval Measurement: Period) 
*/
void PulsePeriodCapture_Process_in_polling(void)
{
    #if 1
    unsigned short  i = 0U;

    if (INPUT_PULSE_IRQ_FLAG)
    {
        INPUT_PULSE_IRQ_FLAG = 0U;

        g_times = PERIOD_MEASURE_TIME;

        while (0U != g_times);
        
        // sum
        for (i = 0U ; i < PERIOD_MEASURE_TIME ; i++)
        {
            period_total += g_pulse_width[i];          
        }

        // avg
        period_total = period_total / PERIOD_MEASURE_TIME;

        FLAG_PROJ_GET_PERIOD_MEASUREMENT = 1;
    }

    #else
	while (!INPUT_PULSE_IRQ_FLAG)
	{
	    HALT();     /* Wait first edge detection */
	}
	INPUT_PULSE_IRQ_FLAG = 0U;    /* Clear interrupt flag */

	g_times = 8U;   /* Set number of measurement */

	EI();           /* Enable interrupt */

	/* Wait for the measurement to finish */
	while (0U != g_times)
	{
	    HALT();
	}	
    #endif
}

unsigned short get_pwm_ch_clk_src(void)
{
	return (pwm_clk_src);
}

void set_pwm_ch_clk_src(unsigned short src)
{
	pwm_clk_src = src;
}

unsigned short get_pwm_ch_duty(unsigned char ch)
{
	return (pwm_duty[ch]);
}

void set_pwm_ch_duty(unsigned char ch ,unsigned short duty)
{
    pwm_duty[ch] = duty;
}


void reload_pwm_ch_idx(unsigned char ch)
{
    // Reload Data Enable Register
    TAUB0.RDE = 1U << ch;    
    // Reload Data Trigger Register
    TAUB0.RDT = 1U << ch;   // enable bit correspond to pwm ch
}

void pwm_ch_Duty_Decrease(unsigned char idx)
{
    unsigned short tmp = 0U;
    unsigned short pwm_master = 0U;
    unsigned short duty_hex = 0U;

    tmp = get_pwm_ch_duty(idx);
    pwm_master = get_pwm_ch_clk_src()+1U;

    duty_hex = (pwm_master) / PWM_RESOLUTION * 5U;   // - 5 %
    tmp = (tmp <= 0U) ? (0U) : (tmp - duty_hex ) ;  
    set_pwm_ch_duty(idx,tmp);

    #if 1   // debug
    tiny_printf("[%d]-duty:0x%02X,",idx,pwm_master);
    tiny_printf("0x%02X,",duty_hex );
    tiny_printf("0x%02X,",tmp );
    tiny_printf("(%f)\r\n",(float) tmp*PWM_RESOLUTION/(pwm_master) );
    #endif

    switch(idx)
    {
        case 1U: 
            TAUB0.CDR1 = get_pwm_ch_duty(idx);
            break;
        case 2U:
            TAUB0.CDR2 = get_pwm_ch_duty(idx);
            break;
        case 3U:
            TAUB0.CDR3 = get_pwm_ch_duty(idx);
            break;
        case 4U:
            TAUB0.CDR4 = get_pwm_ch_duty(idx);
            break;
    }
    reload_pwm_ch_idx(idx);
}

void pwm_ch_Duty_Increase(unsigned char idx)
{
    unsigned short tmp = 0U;
    unsigned short pwm_master = 0U;
    unsigned short duty_hex = 0U;

    tmp = get_pwm_ch_duty(idx);
    pwm_master = get_pwm_ch_clk_src()+1U;

    duty_hex = (pwm_master) / PWM_RESOLUTION * 5U;   // + 5 %
    tmp = (tmp >= (pwm_master)) ? ((pwm_master)) : (tmp + duty_hex );   
    set_pwm_ch_duty(idx,tmp);

    #if 1   // debug
    tiny_printf("[%d]+duty:0x%02X,",idx,pwm_master);
    tiny_printf("0x%02X,",duty_hex );
    tiny_printf("0x%02X,",tmp );
    tiny_printf("(%f)\r\n",(float) tmp*PWM_RESOLUTION/(pwm_master) );
    #endif

    switch(idx)
    {
        case 1U:
            TAUB0.CDR1 = get_pwm_ch_duty(idx);
            break;
        case 2U: 
            TAUB0.CDR2 = get_pwm_ch_duty(idx);
            break;
        case 3U: 
            TAUB0.CDR3 = get_pwm_ch_duty(idx);
            break;
        case 4U: 
            TAUB0.CDR4 = get_pwm_ch_duty(idx);
            break;
    }
    reload_pwm_ch_idx(idx);
}

void PWM_Process_Adjust(void)
{


    /*
        duty * 16
        --------  = target hex 
            10
    */

    if (FLAG_PROJ_PWM_DUTY_INC)
    {
        FLAG_PROJ_PWM_DUTY_INC = 0U;
        pwm_ch_Duty_Increase(pwm_ch_idx);
    }

    if (FLAG_PROJ_PWM_DUTY_DEC)
    {
        FLAG_PROJ_PWM_DUTY_DEC = 0U;
        pwm_ch_Duty_Decrease(pwm_ch_idx);
    }
}

void tmr_1ms_IRQ(void)
{
    tick_counter();

    if ((get_tick() % 1000U) == 0U)
    {
        FLAG_PROJ_TIMER_PERIOD_1000MS = 1U;
    }

    if ((get_tick() % 100U) == 0U)
    {
        FLAG_PROJ_TIMER_PERIOD_100MS = 1U;
    }

    if ((get_tick() % 50U) == 0U)
    {
        FLAG_PROJ_TIMER_PERIOD_SPECIFIC = 1U;
    }	

    Button_Process_long_counter();
}

void LED_Toggle(void)
{
    static unsigned char flag_gpio = 0U;
		
    GPIO_TOGGLE(0,14);//PORT.PNOT0 |= 1u<<14;
	
	if (!flag_gpio)
	{
		flag_gpio = 1U;
        GPIO_HIGH(P8,5);//PORT.P8 |= 1u<<5;
	}
	else
	{
		flag_gpio = 0U;
		GPIO_LOW(P8,5);//PORT.P8 &= ~(1u<<5);
	}	
}

void loop(void)
{
	// static unsigned long LOG1 = 0U;

    Button_Process_in_polling();

    if (FLAG_PROJ_TIMER_PERIOD_1000MS)
    {
        FLAG_PROJ_TIMER_PERIOD_1000MS = 0U;

        g_u32_counter++;
        LED_Toggle();   
        // tiny_printf("timer:%4d\r\n",LOG1++);
    }

    if (FLAG_PROJ_TIMER_PERIOD_100MS)
    {        
        FLAG_PROJ_TIMER_PERIOD_100MS = 0U;

        PulseWidthCapture_Process_logging();     
        // PulsePeriodCapture_Process_logging();
    }

    if (FLAG_PROJ_TIMER_PERIOD_SPECIFIC)
    {
        FLAG_PROJ_TIMER_PERIOD_SPECIFIC = 0U;
        
    }

    if (FLAG_PROJ_TRIG_5)
    {
        FLAG_PROJ_TRIG_5 = 0U;

    }
    if (FLAG_PROJ_TRIG_6)
    {
        FLAG_PROJ_TRIG_6 = 0U;
        g_capturelogflag ^= 1;
    }

    PulseWidthCapture_Process_in_polling();
    // PulsePeriodCapture_Process_in_polling();

    PWM_Process_Adjust();
}

// F1KM S1 EVB , P8_2/INTP6 , set both edge 
void Button_Process_long_counter(void)
{
    if (FLAG_PROJ_TRIG_BTN2)
    {
        btn_tick_counter();
    }
    else
    {
        btn_set_tick(0U);
    }
}

void Button_Process_in_polling(void)
{
    static unsigned char cnt = 0U;

    if (FLAG_PROJ_TRIG_BTN1)
    {
        FLAG_PROJ_TRIG_BTN1 = 0U;
        tiny_printf("BTN pressed(%d)\r\n",cnt);

        if (cnt == 0U)   //set both edge  , BTN pressed
        {
            FLAG_PROJ_TRIG_BTN2 = 1U;
        }
        else if (cnt == 1U)  //set both edge  , BTN released
        {
            FLAG_PROJ_TRIG_BTN2 = 0U;
        }

        cnt = (cnt >= 1U) ? (0U) : (cnt+1U) ;
    }

    if ((FLAG_PROJ_TRIG_BTN2 == 1U) && 
        (btn_get_tick() > BTN_PRESSED_LONG))
    {         
        tiny_printf("BTN pressed LONG\r\n");
        btn_set_tick(0U);
        FLAG_PROJ_TRIG_BTN2 = 0U;
    }
}

void Button_Process_in_IRQ(void)    
{
    FLAG_PROJ_TRIG_BTN1 = 1U;
}


void UARTx_ErrorCheckProcess(unsigned char err)
{
    if (err)          /* Check reception error */
    {   
        /* Reception error */
        switch(err)
        {
            case _UART_PARITY_ERROR_FLAG:   /* Parity error */
                tiny_printf("uart rx:Parity Error Flag\r\n");
                break;
            case _UART_FRAMING_ERROR_FLAG:  /* Framing error */
                tiny_printf("uart rx:Framing Error Flag\r\n");
                break;
            case _UART_OVERRUN_ERROR_FLAG:  /* Overrun error */
                tiny_printf("uart rx:Overrun Error Flag\r\n");
                break;
            case _UART_BIT_ERROR_FLAG:      /* Bit error */
                tiny_printf("uart rx:Bit Error Flag\r\n");
                break;
        }
        g_uart0rxerr = 0U;
    }
}

void UARTx_Process(unsigned char rxbuf)
{    
    if (rxbuf == 0x00U)
    {
        return;
    }

    if (rxbuf > 0x7FU)
    {
        tiny_printf("invalid command\r\n");
    }
    else
    {
        tiny_printf("press:%c(0x%02X)\r\n" , rxbuf,rxbuf);   // %c :  C99 libraries.
        switch(rxbuf)
        {
            case '1':
                FLAG_PROJ_TRIG_1 = 1U;
                pwm_ch_idx = 1U;
                break;
            case '2':
                FLAG_PROJ_TRIG_2 = 1U;
                pwm_ch_idx = 2U;
                break;
            case '3':
                FLAG_PROJ_TRIG_3 = 1U;
                pwm_ch_idx = 3U;
                break;
            case '4':
                FLAG_PROJ_TRIG_4 = 1U;
                pwm_ch_idx = 4U;
                break;
            case '5':
                FLAG_PROJ_TRIG_5 = 1U;
                break;
            case '6':
                FLAG_PROJ_TRIG_6 = 1U;
                break;

            case 'a':
            case 'A':
                FLAG_PROJ_PWM_DUTY_INC = 1U;
                break;
            case 'd':
            case 'D':
                FLAG_PROJ_PWM_DUTY_DEC = 1U;
                break;

            case 'X':
            case 'x':
            case 'Z':
            case 'z':
                RH850_software_reset();
                break;

            default:       
                // exception
                break;                
        }
    }
}

void RH850_software_reset(void)
{
    uint32_t  reg32_value;

    reg32_value = 0x00000001UL;
    WPROTR.PROTCMD0 = _WRITE_PROTECT_COMMAND;
    RESCTL.SWRESA = reg32_value;
    RESCTL.SWRESA = (uint32_t) ~reg32_value;
    RESCTL.SWRESA = reg32_value;
    while (WPROTR.PROTS0 != reg32_value)
    {
        NOP();
    }
}

void RLIN3_UART_SendChar(unsigned char c)
{
    /*
        UTS : 0 - transmission is not in progress    
    */
    while (((RLN30.LST & _UART_TRANSMISSION_OPERATED) != 0U));    
    RLN30.LUTDR.UINT16 = c;
    // RLN30.LUTDR.UINT8[L] = (unsigned char) c;  
}

void SendChar(unsigned char ch)
{
    RLIN3_UART_SendChar(ch);
}

void hardware_init(void)
{
    EI();

    R_Config_TAUJ0_3_Start();
    R_Config_OSTM0_Start();

    /*
        LED : 
            - LED18 > P0_14
            - LED17 > P8_5 
        UART : 
            - TX > P10_10
            - RX > P10_9    
    */
    R_Config_UART0_Receive(&g_uart0rxbuf, 1U);
    R_Config_UART0_Start();
    
    /*
        button :
            - P8_2/INTP6
    */
    R_Config_INTC_INTP6_Start();


    /*
        PWM OUTPUT :
            - P10_11/TAUB0O1
            - P0_8/TAUB0O2
            - P10_12/TAUB0O3
            - P0_9/TAUB0O4
    */
    R_Config_TAUB0_Start();


    /*
        PWM INPUT :
            - P9_0/TAUD0I0
    */
    R_Config_TAUD0_0_Start();

    
    // R_Config_RIIC0_Start();
    // R_Config_CSIG0_Start();
    // R_Config_CSIH0_Start();


    tiny_printf("\r\nhardware_init rdy\r\n");

}
