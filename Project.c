/*
 * Project.c
 *
 *  Created on: Sep 19, 2021
 *      Author: Mostafa Mahmoud
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//global variables
unsigned char counter[6]={0},flag=0;
void timer1_init(void)
{
	/*CTC mode
	 * WGM12=1
	 * WGM10=0
	 * WGM11=0
	 */
	TCCR1A=(1<<FOC1A); //compare match mode(CTC)
	TCCR1B=(1<<CS12)|(1<<CS10)|(1<<WGM12); //pre-scaler 1024
	TCNT1=0; //initial value =0
	OCR1A=1000; // top value
	TIMSK|=(1<<OCIE1A); //enable interrupt
}
void INT0_init(void)
{
	SREG  &= ~(1<<7); // disable interrupt by clearing the I-bit
	DDRD  &= (~(1<<PD2)); //configure pin PD2 as input pin
	PORTD |=(1<<PD2);//Activate the internal pull up resistor
	MCUCR|=(1<<ISC01); //Trigger INT0 with falling edge
	GICR |=(1<<INT0); //Enable the external interrupt request
	SREG|=(1<<7); //enable interrupts by setting the I-bit
}
void INT1_init(void)
{
	SREG  &= ~(1<<7);// disable interrupt by clearing the I-bit
	DDRD  &= (~(1<<PD3));//configure pin PD3 as input pin
	MCUCR|=(1<<ISC11)|(1<<ISC10);//Trigger INT1 with rising edge
	GICR |=(1<<INT1);//Enable the external interrupt request
	SREG|=(1<<7); //enable interrupts by setting the I-bit
}
void INT2_init(void)
{
	SREG  &= ~(1<<7);// disable interrupt by clearing the I-bit
	DDRB &=(1<<PB2);//configure pin PB2 as input pin
	PORTB |=(1<<PB2);//Activate the internal pull up resistor
	GICR|=(1<<INT2);//Enable the external interrupt request
	MCUCSR &=~(1<<ISC2); //detect the falling edge
	SREG|=(1<<7);//enable interrupts by setting the I-bit
}
ISR (TIMER1_COMPA_vect)
{
	flag=1; //enable the flag to increment the counter
}
ISR (INT0_vect)
{
	//reset the clock
	for(unsigned char j=0;j<6;j++)
	{
		counter[j]=0;
	}
}
ISR(INT1_vect)
{
	//turn timer1 interrupt off to pause the clock
	TIMSK&=~(1<<OCIE1A);
}
ISR(INT2_vect)
{
	//turn on timer1 interrupt to resume the clock
	TIMSK|=(1<<OCIE1A);
}
void count(void)
{
	if(flag==1)
	{
	//check if a segment reached its maximum increment the next one
		if(counter[0]==9)
		{
		//check if the first segment reached (09) , reset the first and increment the second segment (10 seconds)
			counter[0]=0;
			counter[1]++;
			if(counter[1]==6)
			{
			//check if the the counter reached 60 seconds , increment the minutes (00:01:00)
				counter[1]=0;
				counter[2]++;
				if(counter[2]==10)
				{
			//check if the counter reached 09 minutes
					counter[2]=0;
					counter[3]++;
					if(counter[3]==6)
					{
					//check if the counter reached 60 minutes , increment the hours
						counter[3]=0;
						counter[4]++;
						if(counter[4]==10)
						{
							counter[4]=0;
							counter[5]++;
						}
						else if(counter[5]==2 && counter[4]==4)
						{
						//if the counter reached 24:00:00 reset
							counter[0]=0,counter[1]=0,counter[2]=0,counter[3]=0,counter[4]=0,counter[5]=0;
						}
					}
				}
			}
		}
	else
	{
		counter[0]++; //increment the seconds
	}
	flag=0; //disable the flag to start it again in the ISR
	}
}
int main()
{
	timer1_init(); //initialize timer1
	INT0_init(); //initialize INT0
	INT1_init(); //initialize INT1
	INT2_init(); //initialize INT2
	SREG|=(1<<7); //enable interrupts by setting the I-bit
	DDRA =0xff; //configure PORTA as output port
	DDRC=0xff;//configure PORTC as output port
	while(1)
	{
		PORTA=(1<<PA0); //write 1 in Pin PA0
		//display all the segments
		for(unsigned char i=0;i<6;i++)
		{
			PORTC = (PORTC & 0xF0) | (counter[i] & 0x0F);
			_delay_ms(5);
			PORTA=PORTA<<1;
		}
		count();
	}
}
