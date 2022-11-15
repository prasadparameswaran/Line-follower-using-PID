//O0,1.1Mhz
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.c"

#define		Th		185

unsigned char ADC_Conversion(unsigned char);
unsigned char ADC_Value;
unsigned char l = 0;
unsigned char c = 0;
unsigned char r = 0;
float kp,kd,ki,e,pe,P,I,D,O;
unsigned char PortBRestore = 0;

//Function to configure LCD port
void lcd_port_config (void)
{
 DDRC = DDRC | 0xF7;    //all the LCD pin's direction set as output
 PORTC = PORTC & 0x80;  // all the LCD pins are set to logic 0 except PORTC 7
}

//ADC pin configuration
void adc_pin_config (void)
{
 DDRA = 0x00;   //set PORTF direction as input
 PORTA = 0x00;  //set PORTF pins floating
}

//Function to configure ports to enable robot's motion
void motion_pin_config (void)
{
 DDRB = DDRB | 0x0F;    //set direction of the PORTB3 to PORTB0 pins as output
 PORTB = PORTB & 0xF0;  //set initial value of the PORTB3 to PORTB0 pins to logic 0
 DDRD = DDRD | 0x30;    //Setting PD5 and PD4 pins as output for PWM generation
 PORTD = PORTD | 0x30;  //PD5 and PD4 pins are for velocity control using PWM
}

//Function to Initialize PORTS
void port_init()
{
 lcd_port_config();
 adc_pin_config();
 motion_pin_config();
}

//TIMER1 initialize - prescale:64
// WGM: 5) PWM 8bit fast, TOP=0x00FF
// desired value: 450Hz
// actual value: 450.000Hz (0.0%)
void timer1_init(void)
{
 TCCR1B = 0x00; //stop
 TCNT1H = 0xFF; //setup
 TCNT1L = 0x01;
 OCR1AH = 0x00;
 OCR1AL = 0xFF;
 OCR1BH = 0x00;
 OCR1BL = 0xFF;
 ICR1H  = 0x00;
 ICR1L  = 0xFF;
 TCCR1A = 0xA1;
 TCCR1B = 0x0D; //start Timer
}


//Function to Initialize ADC
void adc_init()
{
 ADCSRA = 0x00;
 ADMUX = 0x20;		//Vref=5V external --- ADLAR=1 --- MUX4:0 = 0000
 ACSR = 0x80;
 ADCSRA = 0x86;		//ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0
}

//This Function accepts the Channel Number and returns the corresponding Analog Value
unsigned char ADC_Conversion(unsigned char Ch)
{
 unsigned char a;
 Ch = Ch & 0x07;
 ADMUX= 0x20| Ch;
 ADCSRA = ADCSRA | 0x40;	//Set start conversion bit
 while((ADCSRA&0x10)==0);	//Wait for ADC conversion to complete
 a=ADCH;
 ADCSRA = ADCSRA|0x10;      //clear ADIF (ADC Interrupt Flag) by writing 1 to it
 return a;
}

// This Function prints the Analog Value Of Corresponding Channel No. at required Row
// and Coloumn Location.
//lcd_print(row, coloumn, ADC_Value, 3);


//Function used for setting motor's direction
/*void motion_set (unsigned char Direction)
{
 unsigned char PortBRestore = 0;

 Direction &= 0x0F; 			// removing upper nibbel as it is not needed
 PortBRestore = PORTB; 			// reading the PORTB's original status
 PortBRestore &= 0xF0; 			// setting lower direction nibbel to 0
 PortBRestore |= Direction; 	// adding lower nibbel for direction command and restoring the PORTB status
 PORTB = PortBRestore; 			// setting the command to the port
}

//Function for velocity control
void velocity (unsigned char left_motor, unsigned char right_motor)
{
 OCR1AH = 0x00;
 OCR1AL = left_motor;
 OCR1BH = 0x00;
 OCR1BL = right_motor;
}*/

void init_devices (void)
{
 cli();          //Clears the global interrupts
 port_init();
 timer1_init();
 adc_init();
 sei();          //Enables the global interrupts
}

//Main Function
int main(void)
{
 init_devices();

 lcd_set_4bit();
 lcd_init();

 kp=100;
 kd=0.00001;
 ki=1/10000;

 while(1)
 {
	l=ADC_Conversion(3);
	c=ADC_Conversion(4);
	r=ADC_Conversion(5);
	lcd_print(1, 1, l, 3);
	lcd_print(1, 5, c, 3);
	lcd_print(1, 9, r, 3);

	if(c>Th)
		{
		e=0;
		}
	else if(l>Th)
		{
		e=-1;
		}

	else if(r>Th)
		{
		e=1;
		}

	P=kp*e;
	I=I+e;
	I=I*ki;
	D=(e-pe)*kd;
	O=P+I+D;
	pe=e;

	if(O>510)
		{
		O=510;
		}
	if(O<-510)
		{
		O=-510;
		}

	if (O==0)
			{
			PORTB = 0x06; 			// setting the command to the port

			OCR1AH = 0x00;
			OCR1AL = 255;
			}
	
	if (O>0.0)
		{
		if (O>255)
	 		{

	//		PORTB = 0x05; 			// setting the command to the port

	//		OCR1AH = 0x00;
	//		OCR1AL = O-255;
			}
		else
			{

			PORTB = 0x0A; 			// setting the command to the port

			OCR1AH = 0x00;
			OCR1AL = 255-O;
			}
		}
	else if (O<0.0)
			{
		if (O<-255)
			{

	//		PORTB = 0x0A; 			// setting the command to the port

	//		OCR1BH = 0x00;
	//		OCR1BL = -(O+255);
			}
		else
			{

			PORTB = 0x05; 			// setting the command to the port

			OCR1BH = 0x00;
			OCR1BL = 255+O;
			}
	
		}
	_delay_ms(10);

 }
}
