//***********************************************************************
// Plik: keyb.c 
//
// Zaawansowana obs³uga przycisków i klawiatur 
// Wersja:    1.0
// Licencja:  GPL v2
// Autor:     Deucalion  
// Email:     deucalion#wp.pl
// Szczegó³y: http://mikrokontrolery.blogspot.com/2011/04/jezyk-c-biblioteka-obsluga-klawiatury.html
//
//***********************************************************************


#include "keyb.h"

#define PERIOD_1S      100
#define PERIOD_750MS   75
#define PERIOD_500MS   50
#define PERIOD_100MS   10
#define PERIOD_30MS    3

// ***********************************************************************
static unsigned int keys;
// Wewnêtrzna zmienna w której przechowywany jest stan klawiszy
// Zawartoœæ tej zmiennej pobierana jest przez program za pomoca funkcji
//  GetKeys(); jeœli zmienna keycnt jest wiêksza od zera.
// ***********************************************************************

// ***********************************************************************
static unsigned int keycnt;
// Wewnêtrzna zmienna w której przechowywana jest licznik autorepetycji
// Zwiêkszany okresowo w momencie gdy s¹ naciœniête klawisze i ich stan
// nie zmienia siê przez d³u¿szy czas. Zmniejszana podczas ka¿dego
// wywo³ania funkcji GetKeys();
// ***********************************************************************

// ***********************************************************************
static unsigned int keytime;
// Wewnêtrzna zmienna reprezentuj¹ca czas od ostatniego naciœniêcia
// klawiszy. Zwracana do programu za pomoc¹ funkcji KeysTime();
// Jeœli nie ma wciœniêtego ¿adnego klawisza, zmienna utrzymuje wartoœæ 0;
// ***********************************************************************

// ***********************************************************************
static unsigned int prevKeybSt;
// Wewnêtrzna zmienna pamiêtaj¹ca stan klawiszy z poprzedniego wywo³ania
// funkcji KeybProc(). S³u¿y do wykrycia zmiany stanu klawiatury
// ***********************************************************************

// ***********************************************************************
static unsigned int arTime;
// Wewnêtrzna zmienna reprezentuj¹ca czas w którym ma nast¹piæ zwiekszenie
// licznika klawiatury. Zwiêkszana o odpowiedni czas zale¿ny o czasów
// autorepetycji po ka¿dym zwiekszeniu licznika klawiatury
// ***********************************************************************

// ***********************************************************************
static unsigned char arIndex ;
// Wewnêtrzna zmienna indeksuj¹ca tablicê z kolejnymi czasami autorepetycji
// ***********************************************************************

// ***********************************************************************
static unsigned char keyblock;
// Wewnêtrzna zmienna ustawiana na 1 funkcj¹ KeybLock() lub ClrKeyb()
// z parametrem KBD_LOCK. Jeœli zmienna ma wartoœæ 1 obs³uga klawiatury
// zostaje zablokowana do momentu zwolnienia wszystkich klawiszy.   
// ***********************************************************************

// ***********************************************************************
// Domyœlna tablica z kolejnymi czasami autorepetycji. Ostatnia pozycja ró¿na
// od zera jest czasem autorepetycji nieskoñczonej. Iloœæ pozycji dowolna,
// ostatnia pozycja musi byæ równa 0;
static const unsigned short DefaultAutoRepeatTab[] =
{
   PERIOD_30MS,
   PERIOD_1S,
   PERIOD_500MS,
   PERIOD_500MS,
   PERIOD_500MS,
   PERIOD_500MS,
   PERIOD_100MS,
   0
};

static unsigned short
    * volatile _art = (unsigned short *)DefaultAutoRepeatTab,
    * volatile art = (unsigned short *)DefaultAutoRepeatTab;

// ***********************************************************************
// Funkcja dostarczaj¹ca surowy stan klawiatury.
// Stan wysoki dla wcisniêtych klawiszy. Jeœli funkcja KeybProc()
// bêdzie wywo³ywana z przerwania to funkcja GetKeybSample() musi
// trwaæ jak najkrócej
// ***********************************************************************
unsigned int
GetKeybSample( void )
{
    //Tu nale¿y dopisaæ cia³o funkcji, która bêdzie zwracaæ stan klawiszy,
  //  stan niski oznacza klawisz nienaciœniêty, stan wysoki  klawisz naciœniêty
  //  Przyk³ad:
    // zak³adamy, ¿e wciœniêcie klawisza zwiera pin do masy
    return ( ~PINC ) & ( KEY0 | KEY1);
}


// ***********************************************************************
// Funkcja wywo³ywana z jakiegos przerwania w tym przypadku co 10ms
// ***********************************************************************
void
KeybProc( void )
{
   unsigned int realKeybSt;

   // Pobranie stanu klawiszy
   realKeybSt = GetKeybSample();
   
   // Sprawdzenie czy stan sie zmieni³
   if( prevKeybSt != realKeybSt )
   {
      // Stan sie zmieni³ wiêc resetowanie klawiatury i wyjœcie z funkcji
      ClrKeyb( KBD_NOLOCK );     
      return;
   }

   // Sprawdzenie czy brak naciœniêtych klawiszy lub klawiatura zablokowana
   if( !realKeybSt || keyblock )
   {
      // Ponowne sprawdzenie czy brak nacisniêtych klawiszy
      // Jeœli tak to odblokowanie klawiatury
      if( !realKeybSt ) keyblock = 0;
      return;
   }
   // Zwiekszenie licznika czasu klawiatury
   keytime++;
   // Zachowanie stanu klawiszy
   keys = realKeybSt;
     
      // Obs³uga autorepetycji
      // Sprawdzenie czy licznik czsu klawiatury osi¹gn¹³ czas nastêpnej
      // autorepetycji
   if( keytime >= arTime )
   {
      // Zwiêkszenie licznika autorepetycji
      keycnt++;
      
      // Obliczenie kolejnego czasu autorepetycji
      _art = art;
      if( _art[ arIndex + 1 ]) arIndex++;
      arTime += _art[ arIndex ];     
   }
}



// ***********************************************************************
// Funkcja zwraca stan klawiszy do programu jeœli licznik autorepetycji
// rózny od zera
// ***********************************************************************
unsigned int
GetKeys( void )
{
   if( keycnt )
   {
      keycnt--;     // Jeœli funkcja KeybProc() bêdzie wywo³ywana z przerwania
                    // to ta instrukcja musi byæ wykonana atomowo.
      return keys ;
   }
   return 0;
}


// ***********************************************************************
// Funkcja zwraca czas wciskania aktualnej kombinacji klawiszy
// ***********************************************************************
unsigned int
KeysTime( void )
{
   return keytime;
}

// ***********************************************************************
// Funkcja zwraca stan klawiszy zgodnie z podana mask¹ jako argument funkcji
// ***********************************************************************
unsigned int
IsKeyPressed( unsigned int mask )
{
   return keys & mask;
}

// ***********************************************************************
// Funkcja zwraca stan klawiszy zgodnie z ustawiona mask¹ podana jako
// argument funkcji, jest brany pod uwage licznik autorepetycji
// Ale pobranie stanu klawiwszy nie zminiejsza licznika autorepetycji
// ***********************************************************************
unsigned int
IsKey( unsigned int mask )
{
   if(keycnt)
   {
      return keys & mask ;
   }
   return 0;
}


// ***********************************************************************
// Funkcja resetuje stan klawiatury. Jako parametr nale¿y podaæ sta³¹
// KBD_LOCK lub KBD_NOLOCK, które odpowiednio blokuj¹ lub nie klawiaturê
// Jeœli funkcja KeybProc() bêdzie wywo³ywana z przerwania to funkcja
// ClrKeyb() musi byæ wykonana atomowo.
// ***********************************************************************

void
ClrKeyb( int lock )
{
   prevKeybSt = GetKeybSample();
   keys = 0;
   keytime = 0;
   keycnt = 0;
   arIndex = 0;
   arTime = _art[0];
   if( lock ) keyblock = 1;
}


// ***********************************************************************
// Funkcja blokuje klawiaturê. Odblokowanie nastêpuje po zwolnieniu
// wszystkich klawiszy.
// ***********************************************************************
void
KeybLock( void )
{
   keyblock = 1;
}


// ***********************************************************************
// Funkcja podmienia tablicê z miêdzyczasami autorepetycji. Nowa tablica
// powinna byæ zgodna z wczeœniej opisanym formatem
// ***********************************************************************
void
KeybSetAutoRepeatTimes( unsigned short * AutoRepeatTab )
{
   if( AutoRepeatTab == KBD_DEFAULT_ART )
      art = (unsigned short *)DefaultAutoRepeatTab;
   else
      art = AutoRepeatTab;
}
