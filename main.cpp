#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <wingdi.h>
#include "debug.h"
#include "FileSystem.h"
#include "render.h"
#include "input.h"
#include "sound.h"
#include "game.h"

#include <mmsystem.h>

DWORD		m_dwWindowStyle		= 0;
HWND		m_hWnd				= NULL;
HINSTANCE	m_hInstance			= NULL;
RECT		m_rcWindowBounds;
RECT		m_rcWindowClient;
SVIEWPORT	m_Viewport;

// Game forward declaration
void Game_OnEvent( EventType_t type, int userNum, const char* userString );

//-----------------------------------------------------------------------------
// Name: WndProc()
// Desc:
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_KEYDOWN:
		g_pInput->KeyAction( (int)wParam, true );
		break;

	case WM_KEYUP:
		g_pInput->KeyAction( (int)wParam, false );
		break;

	case WM_ACTIVATE:
		if ( LOWORD( wParam ) == WA_INACTIVE )
			g_pInput->ResetKeys();

		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Name: InitWindow()
// Desc:
//-----------------------------------------------------------------------------
void InitWindow()
{
	// Unless a substitute hWnd has been specified, create a window to
    // render into
    if( m_hWnd == NULL)
    {
        // Register the windows class
        WNDCLASS wndClass = { 0, WndProc, 0, 0, m_hInstance,
                              0,
                              LoadCursor( NULL, IDC_ARROW ),
                              (HBRUSH)GetStockObject(WHITE_BRUSH),
                              NULL, "GameClass" };
        RegisterClass( &wndClass );

        // Set the window's initial style
        m_dwWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                          WS_MINIMIZEBOX  | WS_VISIBLE;

        // Set the window's initial width
        RECT rc;
        SetRect( &rc, 0, 0, 640, 480 );
        AdjustWindowRect( &rc, m_dwWindowStyle, true );

        // Create the render window
        m_hWnd = CreateWindow( "GameClass", "Game", m_dwWindowStyle,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               (rc.right-rc.left), (rc.bottom-rc.top), 0,
                               0, m_hInstance, 0 );
    }

    // Save window properties
    m_dwWindowStyle = GetWindowLong( m_hWnd, GWL_STYLE );
    GetWindowRect( m_hWnd, &m_rcWindowBounds );
    GetClientRect( m_hWnd, &m_rcWindowClient );

	m_Viewport.Width = m_rcWindowClient.right - m_rcWindowClient.left;
	m_Viewport.Height = m_rcWindowClient.bottom - m_rcWindowClient.top;
}

//-----------------------------------------------------------------------------
// Name: Cmd_Quit()
// Desc:
//-----------------------------------------------------------------------------
void Cmd_Quit()
{
	// Kill engine
	SendMessage( m_hWnd, WM_DESTROY, 0, 0 );
}

//-----------------------------------------------------------------------------
// Name: Rend_DrawLoadingScreen()
// Desc:
//-----------------------------------------------------------------------------
void Rend_DrawLoadingScreen( const char* pFilename )
{
	// Setup viewport
	g_pRender->SetViewport( &m_Viewport );

	// Load texture
	HTEXTURE hTexture = g_pRender->RegisterTexture( pFilename );

	// rendering
	g_pRender->BeginScene();

	// Draw full screen rectangle.
	g_pRender->Draw2DRect( hTexture, 0.0f, 0.0f, ( float )m_Viewport.Width, ( float )m_Viewport.Height, 0xffffffff );

	g_pRender->EndScene();

	// Unregister texture for memory save
	g_pRender->UnregisterTexture( hTexture );
}


//-----------------------------------------------------------------------------
// Name: InitGame()
// Desc: Initializing the game.
//-----------------------------------------------------------------------------
void InitGame()
{
	// Initialize timer.
	Sys_InitTimer();

	// Send event for game initialization
	Sys_SendEvent( EVENT_ON_GAME_INIT, 0, NULL );
}

//-----------------------------------------------------------------------------
// Name: ShutdownGame()
// Desc: Shutdown the game.
//-----------------------------------------------------------------------------
void ShutdownGame()
{
}

//-----------------------------------------------------------------------------
// Name: RenderGame()
// Desc:
//-----------------------------------------------------------------------------
void RenderGame()
{
	Sys_UpdateTimer();

	float fDeltaTime = Sys_GetDelta();

	// DPrintf( "%f\n", fDeltaTime );

	Game_PreRender();

	Game_PostRender();
}

//-----------------------------------------------------------------------------
// Name: Engine_OnEvent( EventType_t type, int userNum, const char* userString )
// Desc:
//-----------------------------------------------------------------------------
void Engine_OnEvent( EventType_t type, int userNum, const char* userString )
{
	if ( type == EVENT_INIT_GAME )
	{
		InitGame();
	}
	else if ( type == EVENT_SHUTDOWN_GAME )
	{
		ShutdownGame();
	}
	else if ( type > EVENT_SHUTDOWN_GAME && type < EVENT_COUNT )
	{
		// Game events
		Game_OnEvent( type, userNum, userString );
	}
	else
	{
		DPrintf( "Engine_OnEvent( %d, %d, %s ): unknown event passed, skip...\n", 
			type, userNum, userString ? userString : "NULL" );
	}
}

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point of the application.
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
	m_hInstance = hInst;

#ifdef MEMORY_DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Sys_InitRandom();

	Debug_Init();

	g_pFS = NEW CFileSystem();

	InitWindow();

	g_pRender = NEW CRender();
	g_pRender->Init( m_hWnd );
	
	// Render loading screen
	Rend_DrawLoadingScreen("data/textures/loading_screen.tga");

	// Initialize sound system
	g_pSoundSystem->Init();

	// Initialize the game
	InitGame();

	// Enter the message loop
	MSG msg;
	ZeroMemory( &msg, sizeof(msg) );
	while ( msg.message != WM_QUIT )
	{
		if ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else   
		{
			// Event process

			Sys_ProcessEvents( Engine_OnEvent );

			// Rendering

			g_pRender->BeginScene();

			RenderGame();

			g_pRender->EndScene();
		}
	}

	// Destroy game
	ShutdownGame();

	// Shutdown sound system
	g_pSoundSystem->Shutdown();

	g_pRender->Shutdown();
	delete g_pRender;

	UnregisterClass("GameClass", NULL);

	delete g_pFS;

	Debug_Shutdown();

#ifdef MEMORY_DEBUG
//	_CrtDumpMemoryLeaks();
#endif
	return 0;
}