/**********************************************************************/
/*                                                                    */
/*         Lecture: Embedded Systems Architecture				      */
/*         Author:  Victor Rocha                                      */
/*																	  */
/*	This is program is a timer for PIC16F628A microcontroller used	  */
/*	to count hours and minutes or minutes and seconds.				  */
/*	It switches between modes depending on its input signal.		  */
/*	Also it counts incrementing or decrementing.                      */
/*                                                                    */
/**********************************************************************/

#include <16F628A.h>

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES INTRC                    
#FUSES NOPUT                    //No Power Up Timer
#FUSES NOPROTECT                //Code not protected from reading
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOMCLR                   //Master Clear pin used for I/O
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOCPD                    //No EE protection


#use delay(clock=4000000)

#DEFINE changeTime input(PIN_A4)
#DEFINE upDown input(PIN_B6)
#DEFINE ligaLED output_high(PIN_B5)
#DEFINE desligaLED output_low(PIN_B5)

/*
Define as inicial time 23:59:30 inside each independently controlled timer
Defines and initializes one global counter = 0
Defines and intializes the revert variable to increment or decrement counter
*/

signed int16 count=0,timerSU=0,timerSD=3,timerMU=9,timerMD=5,timerHU=3,timerHD=2,revert=0;
int1 status=0,statuspos=0;

/*
External interruption
Enables button combination to switch visualizatin between hour/minutes and minutes/seconds
Also enables combination to increment or decrement the clock
*/

#INT_EXT
void ext_isr()
{
if(changeTime && upDown)
 {
 statuspos=1;
 status=1;
 ligaLED;
 
 }
else if(changeTime && !upDown)
 {
 statuspos=1;
 status=0;
 desligaLED;
 
 }
else if(!changeTime && upDown)
 {
 statuspos=0;
 status=1;
 ligaLED;
 
 }
	else
	{
	 statuspos=0;
	 status=0;
	 desligaLED;
	}
}

/*
Timer interruption
*/
#int_TIMER0
void  TIMER0_isr(void){
   
	/*
	Sets first timer to initialize from the sith value + over pulse clocks, assuming a punctual clock behavior
	*/
   set_timer0 (6 + get_timer0());
   count++;
   if(count==500)
   {
	   count=0;

	if (status==1)
		revert=-1;
	else
		revert=1;
 
		timerSU=timerSU+revert;
		/*
		Initial condition when the first second unity is zero and it's shortly decremented by revert variable, assuming the -1 value
		*/
		if (timerHD==0 && timerHU==0 && timerMD==0 && timerMU==0 && timerSD==0 && timerSU<0)
		{
		timerSU=0;
		}
		else 
		{
		/*
		Every time one of the clocks reaches the ninth value, he will return to zero and its decimal value is incremented
		Same way, when it reaches the value -1, it's started over to 9 and its decimal value is decremented
		*/
		if(timerSU>9)
		{
			timerSU=0;
			timerSD=timerSD+revert;
		}
		else if (timerSU<0)
		{
			timerSU=9;
			timerSD=timerSD+revert;
		}

		/*
		The second decimal value bigger than 5 it means one minute was completed
		Its value is returned to zero and the minute variable is incremented
		The opposite applies to decrementation
		*/
		if (timerSD>5)
		{
		timerSD=0;
		timerMU=timerMU+revert;
		}
		else if (timerSD<0)
		{
		timerSD=5;
		timerMU=timerMU+revert;
		}
 
		/*
		The logic itself applies also to the other timers
		*/
		if (timerMU>9)
		{
		timerMU=0;
		timerMD=timerMD+revert;
		}
		else if (timerMU<0)
		{
		timerMU=9;
		timerMD=timerMD+revert;
		}
		if (timerMD>5)
		{
		timerMD=0;
		timerHU=timerHU+revert;
		}
		else if (timerMD<0)
		{
		timerMD=5;
		timerHU=timerHU+revert;
		}
		if(timerHU>9 && timerHD<2){
		timerHU=0;
		timerHD=timerHD+revert;
		}
		else if (timerHU>3 && timerHD==2){
		timerHU=0;
		timerHD=0;
		}
		else if (timerHU<0){
		timerHU=9;
		timerHD=timerHD+revert;
		}
	}
   
	}
}

/*
Releases ports signal, switching places to be illuminated or turned off
*/
void refresh()
{
 if (statuspos==0) 
 {
  output_low(PIN_B1); 
  output_a(timerSU);
  delay_ms(10);
  output_high(PIN_B1); 
  
  output_low(PIN_B2); 
  output_a(timerSD);
  delay_ms(10);
  output_high(PIN_B2);
  
  output_low(PIN_B3); 
  output_a(timerMU);
  delay_ms(10);
  output_high(PIN_B3);
  
  output_low(PIN_B4); 
  output_a(timerMD);
  delay_ms(10);
  output_high(PIN_B4); 
 } 
 else 
 {
  output_low(PIN_B1); 
  output_a(timerMU);
  delay_ms(10);
  output_high(PIN_B1); 
  
  output_low(PIN_B2); 
  output_a(timerMD);
  delay_ms(10);
  output_high(PIN_B2);
  
  output_low(PIN_B3); 
  output_a(timerHU);
  delay_ms(10);
  output_high(PIN_B3);
  
  output_low(PIN_B4); 
  output_a(timerHD);
  delay_ms(10);
  output_high(PIN_B4); 
 }
}

void main()
{   
   /*
   Defines timer0 with start at the sith value to blow up the buffer at 250 unities
   */
   set_timer0(6);
   /*
   Interruption by timer's buffer blown  has 3 timers
   We are only using timer0 and deactivating the rest of them
   Its also programmed to use internal counter with period multiplied by 8x
   */
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_8);
   setup_timer_1(T1_DISABLED);
   setup_timer_2(T2_DISABLED,0,1);
   setup_comparator(NC_NC_NC_NC);
   
   /*
   Deactivates the necessity to energize PIC at proteus (simulator)
   */
   setup_vref(FALSE);
   
   /*
   Activates interruption
   */
   enable_interrupts(INT_EXT);
   enable_interrupts(INT_TIMER0);
   enable_interrupts(GLOBAL);

   // TODO: USER CODE!!
   while(true)
   {
      refresh();  
   }
}
