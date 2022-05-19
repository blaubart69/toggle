
// cl /nologo /Fm /Fatoggle_%_BUILDARCH%.asm /FAscu /W4 /DWIN /D_WINNT /D_WIN32_WINNT=0x0400 /Zl /Zp8 /Gy /Gm- /Gd /EHs-c- /GR- /GF /GS- /Oxt /Ob2 /Oy- /MT /Fetoggle_%_BUILDARCH% toggle.c kernel32.lib user32.lib winmm.lib /link /OPT:NOWIN98 /NODEFAULTLIB /SUBSYSTEM:CONSOLE /RELEASE /ENTRY:rawmain

#define  UNICODE
#define _UNICODE
#define STRICT
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used
#include <windows.h>
#include <mmsystem.h>

#define ACTION_FREE  -1
#define ACTION_SLEEP -2
#define ACTION_EXIT  -3

#define ACTION_MIN   -3
#define ACTION_MAX    4

static volatile BYTE _Free[ 5 ] = { 1, 1, 1, 1, 1 };
static volatile LONG _Data[ 5 ];
static volatile WCHAR _Usage[] = L" ---- ";
static volatile LONG _Camera2Detect = ACTION_FREE;
static volatile LONG _Detect2Encode = ACTION_FREE;

//-------------------------------------------------------------------------------------------------
static LONG SleepRandom( LONG Rand ) {
//-------------------------------------------------------------------------------------------------

    Sleep( Rand & 0x3F );
    return (( Rand * 214013L + 2531011L ) >> 16 ) & 0x7fff;
}
 
//-------------------------------------------------------------------------------------------------
static VOID Message( WCHAR *p, ... ) {
//-------------------------------------------------------------------------------------------------
 
    WCHAR msg[ 128 ];

    va_list vaList;
    va_start( vaList, p );
 
    wvsprintf( msg, p, vaList );
    OutputDebugString( msg );
 
    va_end( vaList );
}

//-------------------------------------------------------------------------------------------------
static LONG GetFree( VOID ) {
//-------------------------------------------------------------------------------------------------

    for (;;) {
        if ( _Free[ 0 ] == 1 ) return 0;
        if ( _Free[ 1 ] == 1 ) return 1;
        if ( _Free[ 2 ] == 1 ) return 2;
        if ( _Free[ 3 ] == 1 ) return 3;
        if ( _Free[ 4 ] == 1 ) return 4; // we need 5!

        Message( L"CAMERA: ERROR 1\n" );
        Sleep( 10 );
    } /* endfor */
}
 
//=================================================================================================
DWORD WINAPI CameraThreadProc( PVOID Param ) {
//=================================================================================================
 
    HANDLE *hEvents = (HANDLE*)Param;
    LONG action, work, rand, n = 0, m = 0, i;

    rand = GetTickCount();

    for ( i = 0; i < 100; i++ ) {

        // --- read ---

        action = GetFree();

        _Free[ action ] = 0;

        // --- process ---

        work = i;
        _Data[ action ] = 0xDEADBEEF;

//        Message( L"CAMERA: Processing %d\n", action );
        rand = SleepRandom( rand );

        n += work;
        _Data[ action ] = work;

        // --- write ---

        action = _InterlockedExchange( &_Camera2Detect, action );

        if ( action == ACTION_FREE ) {

            ;

        } else if ( action == ACTION_SLEEP ) {

            SetEvent( hEvents[ 0 ] );

        } else if ( action == ACTION_EXIT ) {

            Message( L"CAMERA: ERROR 2\n" );

        } else if ( action < ACTION_MIN || action > ACTION_MAX ) {

            Message( L"CAMERA: ERROR 3\n" );

        } else {

            m += _Data[ action ];
            _Free[ action ] = 1;

        } /* endif */
 
    } /* endfor */

    Message( L"CAMERA: %d (skipped %d) = %d\n", n, m, n - m );

    Sleep( 200 );
    _Camera2Detect = ACTION_EXIT;
    SetEvent( hEvents[ 0 ] );
 
    return 0;
}
 
//=================================================================================================
DWORD WINAPI DetectThreadProc( PVOID Param ) {
//=================================================================================================
 
    HANDLE *hEvents = (HANDLE*)Param;
    LONG action, work, rand, n = 0, m = 0, running = 1;

    rand = GetTickCount() ^ 0x5555;

    do {

        // --- read ---
 
        action = _InterlockedExchange( &_Camera2Detect, ACTION_FREE );

        if ( action == ACTION_FREE ) {

            action= InterlockedCompareExchange( &_Camera2Detect, ACTION_SLEEP, ACTION_FREE );

            if ( action == ACTION_FREE ) {

                WaitForSingleObject( hEvents[ 0 ], INFINITE );
                continue;

            } /* endif */

        } /* endif */

        if ( action == ACTION_SLEEP ) {

            Message( L"DETECT: ERROR 1\n" );
            
        } else if ( action == ACTION_EXIT ) {

            running = 0;

        } else if ( action < ACTION_MIN || action > ACTION_MAX ) {

            Message( L"DETECT: ERROR 2\n" );

        } else {

           // --- process ---

           work = _Data[ action ];
           _Data[ action ] = 0xDEADBEEF;

//           Message( L"DETECT: Processing %d\n", action );
           rand = SleepRandom( rand );

           n += work;
           _Data[ action ] = work;

        } /* endif */

        // --- write ---

        action = _InterlockedExchange( &_Detect2Encode, action );

        if ( action == ACTION_FREE ) {

            ;

        } else if ( action == ACTION_SLEEP ) {

            SetEvent( hEvents[ 1 ] );

        } else if ( action == ACTION_EXIT ) {

            Message( L"DETECT: ERROR 3\n" );

        } else if ( action < ACTION_MIN || action > ACTION_MAX ) {

            Message( L"DETECT: ERROR 4\n" );

        } else {

            m += _Data[ action ];
            _Free[ action ] = 1;

        } /* endif */

    } while ( running );  /* enddo */

    Message( L"DETECT: %d (skipped %d) = %d\n", n, m, n - m );

    return 0;
}
 
//=================================================================================================
DWORD WINAPI EncodeThreadProc( PVOID Param ) {
//=================================================================================================
 
    HANDLE *hEvents = (HANDLE*)Param;
    LONG action, work, rand, n = 0, running = 1;

    rand = GetTickCount() ^ 0x3BCD;

    do {

        // --- read ---
 
        action = _InterlockedExchange( &_Detect2Encode, ACTION_FREE );

        if ( action == ACTION_FREE ) {

            action= InterlockedCompareExchange( &_Detect2Encode, ACTION_SLEEP, ACTION_FREE );

            if ( action == ACTION_FREE ) {

                WaitForSingleObject( hEvents[ 1 ], INFINITE );
                continue;

            } /* endif */

        } /* endif */

        if ( action == ACTION_SLEEP ) {

            Message( L"ENCODE: ERROR 1\n" );
            
        } else if ( action == ACTION_EXIT ) {

            running = 0;

        } else if ( action < ACTION_MIN || action > ACTION_MAX ) {

            Message( L"ENCODE: ERROR 2\n" );

        } else {

            // --- process ---

            work = _Data[ action ];
            _Data[ action ] = 0xDEADBEEF;

//            Message( L"ENCODE: Processing %d\n", action );
            rand = SleepRandom( rand );

            n += work;
            _Data[ action ] = work;

            // --- write ---

            _Free[ action ] = 1;

        } /* endif */

    } while ( running ); /* enddo */

    Message( L"ENCODE: %d\n", n );

    return 0;
}
 
//=================================================================================================
VOID rawmain( VOID ) {
//=================================================================================================
 
    HANDLE hThreads[ 3 ];
    HANDLE hEvents[ 2 ];
    TIMECAPS tc;

    timeGetDevCaps( &tc, sizeof( tc ));
    timeBeginPeriod( tc.wPeriodMin ); // make things more accurate

    hEvents[ 0 ] = CreateEvent( NULL, FALSE, 0, NULL );
    hEvents[ 1 ] = CreateEvent( NULL, FALSE, 0, NULL );

    hThreads[ 0 ] = CreateThread( NULL, 0, CameraThreadProc, hEvents, 0, NULL );
    hThreads[ 1 ] = CreateThread( NULL, 0, DetectThreadProc, hEvents, 0, NULL );
    hThreads[ 2 ] = CreateThread( NULL, 0, EncodeThreadProc, hEvents, 0, NULL );

    WaitForMultipleObjects( 3, hThreads, TRUE, INFINITE );

    Message( L"EXIT\n" );

    ExitProcess( 0 );
}
 
// -=EOF=-
