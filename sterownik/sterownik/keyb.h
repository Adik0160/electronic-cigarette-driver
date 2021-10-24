//***********************************************************************
// Plik: keyb.h
//
// Zaawansowana obs³uga przycisków i klawiatur
// Wersja:    1.0
// Licencja:  GPL v2
// Autor:     Deucalion
// Email:     deucalion#wp.pl
// Szczegó³y: http://mikrokontrolery.blogspot.com/2011/04/jezyk-c-biblioteka-obsluga-klawiatury.html
//
//***********************************************************************


#define KEY_PORT   PINC

#define KEY0 ( 1 << PC4 )
#define KEY1 ( 1 << PC5 )


#define KEY_ENTER  KEY1

#define KEY_UP     KEY0

#define ANYKEY     (KEY0 | KEY1)
#define KEY_MASK   (KEY0 | KEY1)

#define KBD_LOCK     1
#define KBD_NOLOCK   0

#define KBD_DEFAULT_ART   ((void *)0)

void
ClrKeyb( int lock );

unsigned int
GetKeys( void );

unsigned int
KeysTime( void );

unsigned int
IsKeyPressed( unsigned int mask );

unsigned int
IsKey( unsigned int mask );

void
KeybLock( void );

void
KeybSetAutoRepeatTimes( unsigned short * AutoRepeatTab );
#endif
