# Sample_Project_RH850_S1_Input_Capture
Sample_Project_RH850_S1_Input_Capture

udpate @ 2025/04/30

1. base on EVM : Y-BLDC-SK-RH850F1KM-S1-V2 , initial below function

- TAUJ0_0 : timer interval for 1ms interrupt

- UART : RLIN3 (TX > P10_10 , RX > P10_9) , for printf and receive from keyboard

- LED : LED18 > P0_14 , LED17 > P8_5 , toggle per 1000ms

- PWM OUTPUT : 

	- P10_11/TAUB0O1 
	
	- P0_8/TAUB0O2
	
	- P10_12/TAUB0O3
	
	- P0_9/TAUB0O4

- PWM capture : 

	- P9_0/TAUD0I0
	
2. by connect capture pin (P9_0/TAUD0I0) to PWM pin (P10_11/TAUB0O1)

3. default will output capture log (press digit 6 , to turn off/on capture message)

```c
volatile unsigned char pwm_ch_idx = 1U;
unsigned char g_capturelogflag = 1U;
```
	
4. use terminal to control PWM channel duty

press digit A/a , to increase the target PWM channel DUTY ( ex :+5%)

press digit D/d , to decrease the target PWM channel DUTY ( ex :-5%)


below is log message :

![image](https://github.com/released/Sample_Project_RH850_S1_Input_Capture/blob/main/log_capt.jpg)


![image](https://github.com/released/Sample_Project_RH850_S1_Input_Capture/blob/main/log_cap2.jpg)


5. below is capture pin , smart config setting

![image](https://github.com/released/Sample_Project_RH850_S1_Input_Capture/blob/main/smc_capt.jpg)


