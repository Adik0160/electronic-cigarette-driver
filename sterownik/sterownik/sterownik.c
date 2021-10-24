/*
 * sterownik.c
 *
 * Created: 2015-04-05 13:26:37
 *  Author: PAN NAJLEPSZY ADRIAN KATULSKI
 */ 

//F_CPU 1000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>



char blokada_chwilowa = 0;
char flaga_5kliku = 0;
int ogranicznik_czasu = 6000;
char flaga_wlaczwylacz = 0;
unsigned char liczba[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
/*	*///tabela cyfr i hex liter///////0/////1/////2/////3/////4/////5/////6/////7/////8/////9//////a////b//////c/////d////e//////f
volatile uint8_t wyswietlacz[3];
volatile uint16_t licznik_czasu_wyswietlacza;
uint64_t wypelnienie = 50;


void timerinit(void) //inicjalizacja timera1 16 Bit//
{
	TCCR1A = (1<<WGM11);
	TCCR1B = (1<<WGM12)|(1<<WGM13); //prescaller 8, konfiguracja 14 - fast PWM z "TOPem - ICRn" 
	
	ICR1 = 1249; //top = 621 100%
	OCR1A = 624;
	
}


void adcinit(void) //konfiguracvja przetwornika analogowo cyfrowego 
{
 	ADMUX |= (1<<REFS0)|(1<<REFS1)|(1<<MUX0)|(1<<MUX1); //wybieranie wej�� i napi�cia referencyjnego
 	ADCSRA |= (1<<ADEN)|(1<<ADIE); //w�aczenie przetwornika oraz przerwania 
}

void timerint0(void) //inicjalizacja timera0 do multiplexu wyswietlaczy 
{
	
	TCCR0 = (1<<CS00)|(1<<CS01);
	TIMSK = (1<<TOIE0);
	sei();
	
}

void konwertujliczbe(uint32_t x, uint8_t przecinek)//konwersja na wyswietlacz + przecinek i wyswietlaj
{
	if(x<100) 
	{
		wyswietlacz[0] = 0;
	} else {		
		wyswietlacz[0] = liczba[x/100];		
	}								//obliczenia aby wyciagn��...
									//...pojedyncze cyfry z liczby	
	if(x<10)
	{
		wyswietlacz[0] = 0;			//i operacje por�wnania zapobiegaj�ce wyswietlaniu bezs�sownego zera 
		wyswietlacz[1] = 0;
	} else {
		wyswietlacz[1] = liczba[(x/10)%10];	
	}
	wyswietlacz[2] = liczba[x%10];
		
	switch (przecinek) //wybieranie na kt�rym wyswietlaczu ma by� kropka, zmienna "przecinek"
	{
		case 1:
		wyswietlacz[0] |= (1<<7);
		break;
		case 2:
		wyswietlacz[1] |= (1<<7);
		break;
		case 3:
		wyswietlacz[2] |= (1<<7);
		break;
		default:
		//nie ustawia nigdzie przecinka
		break;
	}	
} 


void configio(void) //konfiguracja IO
{
		
		DDRB = 0x02; //grza�ka z przyciskiem fire
		DDRB |= (1<<6)|(1<<7);
		PORTB = 0x01;
		
		DDRC = 0x07; //multiplex i dwa przyciski konfiguracyjne + wej�cie ADC
		PORTC = (1<<0)|(1<<1)|(1<<2)|(1<<4)|(1<<5);
		
		
		DDRD = 0xff; //wyj�cie segment�w 
		PORTD = 0;
		
		
}

void blokada_faji(char tak) //dost�p do rejestr�w konfiguracyjnych w czasie blokady i po
{
	if(!(tak))
	{
		TCCR1A = (1<<COM1A1)|(1<<WGM11);  //konfiguracja timer�w do pwma
		TCCR1B = (1<<WGM12)|(1<<WGM13)|(1<<CS11); //---/
	}	
}

void faja_zablokowana(void)//ustawienia dla zablokowanej baterii
{
	flaga_wlaczwylacz = 1;
	for(uint8_t x = 0;x<6;x++)
	{
		_delay_ms(100);
		PORTB ^= (1<<6);
	}
} 

void faja_odblokowana(void)//ustawienia dla odblokowanej baterii
{
	flaga_wlaczwylacz = 0; //faja w��czona
	for(uint8_t x = 0;x<6;x++)
	{
		_delay_ms(100);
		PORTB ^= (1<<7);
	}
}


void fire(void) //funkcja generuj�ca PWM na wyjsciu
{
	unsigned long int czas = 0;
	TCNT1 = 0; //reset rejestru zliczaj�cego 
	_delay_ms(20);
	
	blokada_faji(flaga_wlaczwylacz);
	
	while(!(PINB & 0x01))
	{
		czas++;
		if(czas > 500000)
		{
			//je�eli przycisk b�dzie d�ugo trzymany w��czy sie blokada chwilowa
			blokada_chwilowa = 1;
			break;
		}
	}
	
	if (czas > 10000)
		{
			flaga_5kliku = 0; //je�eli okres czasu trzymania przycisku b�dzie powy�ej 10000 5klik si� zresetuje

		} else {
			flaga_5kliku = flaga_5kliku + 1; //szybkie klikanie powoduje dodawanie 1 do flagi 5kliku
			if(flaga_5kliku == 5)
			{ 			
				if (flaga_wlaczwylacz)
				{
					faja_odblokowana(); //faja w��czona

				} else {
					TCCR1B &= ~(1<<CS11);
					TCCR1A &= ~(1<<COM1A1);
					TCNT1 = 0;
					PORTB = 0x01;
					faja_zablokowana(); //faja wy��czona
				}	
				flaga_5kliku = 0;	// po wszystkim flaga 5kliku jest resetowana
			}		
		}
	
	TCCR1B &= ~(1<<CS11);
	TCCR1A &= ~(1<<COM1A1);
	TCNT1 = 0;
	PORTB = 0x01;
	
	ogranicznik_czasu = 6000;

}

void dodaj(uint8_t ilosc, unsigned int ms) //funkcja dodaj�ca do wype�nienia
{
	uint64_t lokalna;
	licznik_czasu_wyswietlacza = 0;	
	wypelnienie += ilosc; //dodaje o tyle ile jest w parametrze ilosc
	if (wypelnienie>100) wypelnienie = 1; //if zabezpiecznajcy
	lokalna = (wypelnienie*1249)/100; //wyliczanie ilczby dla ocr1a z procenta
	OCR1A = lokalna; //wpisywaie do rejstru
	konwertujliczbe(wypelnienie,0); //wyswietlanie procenta
	for(unsigned int x = 0;x<ms;x++) //tworzy op�nienie z parametru ms
	{
		_delay_ms(1);
	}
}

void odejmij(uint8_t ilosc, unsigned int ms)//funkcja odejmuj�ca z wype�nienia
{
	uint64_t lokalna;
	licznik_czasu_wyswietlacza = 0;	
	wypelnienie -= ilosc; //odejmuje o tyle ile jest w parametrze ilosc
	if(wypelnienie<1) wypelnienie = 100; //tak samo jak na dole
	if (wypelnienie>100) wypelnienie = 1; //ify zabezpieczaj�ce przed przepe�nieniem itp
	lokalna = (wypelnienie*1249)/100; //wyliczanie ilczby dla ocr1a z procenta
	OCR1A = lokalna; //wpisywaie do rejstru
	konwertujliczbe(wypelnienie,0); //wyswietlanie procenta
	for(unsigned int x = 0;x<ms;x++) //tworzy op�nienie z parametru ms
	{
		_delay_ms(1);
	}
}


void wyswietlaczoff(void) //wyswietlacz jest czyszczony
{
	wyswietlacz[0] = 0;
	wyswietlacz[1] = 0;
	wyswietlacz[2] = 0;
	
} 

void startadc(void) //rozpoczyna wy��czanie multiplexu i przechodzenie w tryb konwersji napi�cia z przetwornika analogowo cyfrowego
{

	TCCR0 &= ~((1<<CS00)|(1<<CS01)); //wy��cza multiplex �eby nie by�o szum�w na przetworniku
	TIMSK &= ~(1<<TOIE0); //wy�acza multiplex
	wyswietlaczoff();  //wy��cza wyswietlacz �eby go nie spali�
	_delay_ms(500);
	ADCSRA |= (1<<ADSC); //wpisuje jedynk� do bitu ADSC - rozpoczyna proces konwersji - wywo�uje przerwanie	
}

int main(void) //g��wna funkcja programu
{
	unsigned int indexczasu = 0;

	timerinit();
	adcinit();
	timerint0();
	configio();

    while(1) //g��wna p�tla
    {
		wyswietlaczoff();
		_delay_ms(200);
		startadc();
		_delay_ms(1000);
		
		if (blokada_chwilowa == 1 )
		{
			if (PINB & 0X01)
			{
				blokada_chwilowa = 0;
			}
		}
				
		if (!(PINB & 0X01))
		{
			if ((blokada_chwilowa == 0))
			{				
				fire();
				if(!flaga_wlaczwylacz)
				{
					startadc();
				}
			}
		}
		
		if(!(PINC & 0x10) & !(flaga_wlaczwylacz)) //przycisk musi by� w��czony oraz bateria musi by� odblokwana - dodawanie
		{
			_delay_ms(100);
			if(!(PINC & 0x30)) //sprawdzenie czy nie s� przypadkiem nacisni�te dwa przyciski
			{
				startadc(); //procedura wy�aczajaca timery i przeliczaj�ca
				while(!(PINC & 0x30)){} //p�tla czeka na zwolnienie przycisku
				_delay_ms(200);
			} else {
				indexczasu = 0;
				dodaj(1,20);
				while(!(PINC & 0x10))
				{
					_delay_ms(1);
					indexczasu++;
					if(indexczasu >= 400) break;
				}
				while(!(PINC & 0x10))
				{
					dodaj(2,100);
				} 
			}
		}
		
		if(!(PINC & 0x20) & !(flaga_wlaczwylacz)) //to co wy�sza operacja tylko z odejmowaniem
		{
			_delay_ms(100);
			if(!(PINC & 0x30)) //sprawdzenie czy przypadkiem nie s� nacisni�te dwa przyciski
			{
				startadc(); //procedura wy�aczajaca timery i przeliczaj�ca
				while(!(PINC & 0x30)){} //p�tla czeka na zwolnienie przycisku
				_delay_ms(200);
			}	else {
				indexczasu = 0;
				odejmij(1,20);			
				while(!(PINC & 0x20))
				{
					_delay_ms(1);
					indexczasu++;
					if(indexczasu >= 400) break;
				}
				while(!(PINC & 0x20))
				{
					odejmij(2,100);
				}
			}
		}
		
		if (ogranicznik_czasu == 0) //je�eli czas odst�pu od przycisniecia b�dzie za d�ugi flaga 5kliku si� resetuje 
		{
			flaga_5kliku = 0; 
			
		} else {
					
			ogranicznik_czasu--;		//je�eli nie odejmuje czas (trzeba szybko klika�)
					
		}	  
    }
}

ISR(TIMER0_OVF_vect) //przerwanie multiplexu
{
	static uint8_t i = 0; //zmienna statyczna - nie resetuje si� po skonczonym wektorze	
	switch (i)
	{
		case 0:
		PORTC = ~0x09;
		PORTD = ~wyswietlacz[0];
		i++;
		break;
		
		case 1:
		PORTC = ~0x0a;
		PORTD = ~wyswietlacz[1];
		i++; 
		break;
		
		case 2:
		PORTC = ~0x0c;
		PORTD = ~wyswietlacz[2];
		i = 0;
		break;		
	}	
	TCNT0 = 169; //ustawia pe�n� warto�� licznika = 256-169 = 87	
	if (licznik_czasu_wyswietlacza>=720)//operacja powoduje wygasni�cie wyswietlacza po 4s = 720 * (1/180)
	{
		wyswietlaczoff();
	} else{
		licznik_czasu_wyswietlacza++;
	}
}

ISR(ADC_vect) //przerwanie przelicza napiecie z ADC po konwersji i wyswietla na wyswietlaczu, w��cza te� multiplex i zeruje licznik czasu wyswietlacza
{
	uint32_t x;
	x = ADC;
	x = (x*500)/1024;
	konwertujliczbe(x,1);
	licznik_czasu_wyswietlacza = 0;
	timerint0();
}