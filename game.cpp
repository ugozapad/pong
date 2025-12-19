// game.cpp: implementation of the game.
//
//////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "game.h"
#include "debug.h"
#include "input.h"
#include "render.h"
#include "sound.h"
#include "FileSystem.h"

#define WIDTH 640
#define HEIGHT 480

#define PADDLE_W 120
#define PADDLE_H 14
#define MARGIN 16
#define SCORE_PANEL_H 48

#define BALL_SIZE 10
#define BALL_VY 5.0f
#define BALL_VX 3.5f
#define MAX_VX 7.0f

#define PLAYER_KEY_SPEED 220
#define AI_SPEED 3.0f

float g_fPlayerX = 0.0f;
float g_fPlayerY = 0.0f;

float g_fEnemyX = 0.0f;
float g_fEnemyY = 0.0f;

float g_fBallX = 0.0f;
float g_fBallY = 0.0f;
float g_fBallVelocityX = 0.0f;
float g_fBallVelocityY = 0.0f;

int g_iPlayerScore = 0;
int g_iEnemyScore = 0;

HCFONT g_hFont = INVALID_RENDER_HANDLE;

HSOUND g_hHitSound = INVALID_SOUND_HANDLE;
HSOUND g_hClearSound = INVALID_SOUND_HANDLE;
HSOUND g_hSuccessSound = INVALID_SOUND_HANDLE;

inline float Clamp(float n, float lower, float upper)
{
  return max(lower, min(n, upper));
}

inline bool Intersects(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh)
{
    return (ax < bx + bw) && (ax + aw > bx) && (ay < by + bh) && (ay + ah > by);
}

//-----------------------------------------------------------------------------
// Name: Game_ResetBall()
// Desc:
//-----------------------------------------------------------------------------
void Game_ResetBall( int dirY )
{
	g_fBallX = (float)(WIDTH/2 - BALL_SIZE/2);
    g_fBallY = (float)(HEIGHT/2 - BALL_SIZE/2);

	int sx = (int)Sys_RandomRange(0, 1) ? 1 : -1;
	g_fBallVelocityX = sx * BALL_VX;
    g_fBallVelocityY = (dirY >= 0 ? 1.0f : -1.0f) * BALL_VY;
}

//-----------------------------------------------------------------------------
// Name: Game_Init()
// Desc:
//-----------------------------------------------------------------------------
void Game_Init()
{
	// Check for files
	if ( !g_pFS->IsExist( "data/textures/fonts/fixed_20.tga" ) || 
		 !g_pFS->IsExist( "data/sounds/move.wav" ) || 
		 !g_pFS->IsExist( "data/sounds/clear.wav" ) || 
		 !g_pFS->IsExist( "data/sounds/success.wav" ) )
		DError( "Can't find game resources, wrong working directory!" );

	// Game initialization
	g_fPlayerX = WIDTH / 2 - PADDLE_W / 2;
	g_fPlayerY = HEIGHT - MARGIN - PADDLE_H;

	g_fEnemyX  = WIDTH/2 - PADDLE_W/2;
    g_fEnemyY  = SCORE_PANEL_H + MARGIN;

	Game_ResetBall( (int)Sys_RandomRange(0, 1) ? 1 : -1 );

	// Resource loading

	g_hFont = g_pRender->RegisterFont( "data/textures/fonts/fixed_20.tga" );

	g_hHitSound = g_pSoundSystem->LoadSound( "data/sounds/move.wav" );
	g_hClearSound = g_pSoundSystem->LoadSound( "data/sounds/clear.wav" );
	g_hSuccessSound = g_pSoundSystem->LoadSound( "data/sounds/success.wav" );
}

//-----------------------------------------------------------------------------
// Name: Game_UpdatePlayer()
// Desc:
//-----------------------------------------------------------------------------
void Game_UpdatePlayer()
{
	// Left key
	if ( g_pInput->IsKeyPressed( 0x25 ) || g_pInput->IsKeyPressed( 'A' ) )
	{
		g_fPlayerX -= (float)PLAYER_KEY_SPEED * Sys_GetDelta();
	}

	// Right key
	if ( g_pInput->IsKeyPressed( 0x27 ) || g_pInput->IsKeyPressed( 'D' ) )
	{
		g_fPlayerX += (float)PLAYER_KEY_SPEED * Sys_GetDelta();
	}

	g_fPlayerX = Clamp(g_fPlayerX, 0.0f, (float)(WIDTH - PADDLE_W));
}

//-----------------------------------------------------------------------------
// Name: Game_UpdateEnemy()
// Desc:
//-----------------------------------------------------------------------------
void Game_UpdateEnemy()
{
	float fTarget = ( g_fBallX + BALL_SIZE * 0.5f ) - PADDLE_W * 0.5f;
	fTarget = Clamp( fTarget, 0.0f, (float)( WIDTH - PADDLE_W ) );

	float fDelta = fTarget - g_fEnemyX;
	if (fDelta > AI_SPEED) fDelta = AI_SPEED;
	else if (fDelta < -AI_SPEED) fDelta = -AI_SPEED;

	g_fEnemyX = Clamp( g_fEnemyX + fDelta, 0.0f, ( float )( WIDTH - PADDLE_W ) );
}

//-----------------------------------------------------------------------------
// Name: Game_UpdateBall()
// Desc:
//-----------------------------------------------------------------------------
void Game_UpdateBall()
{
	g_fBallX += g_fBallVelocityX;
	g_fBallY += g_fBallVelocityY;

	// Intersect with player
	if ( Intersects( g_fBallX, g_fBallY, BALL_SIZE, BALL_SIZE,
					 g_fPlayerX, g_fPlayerY, PADDLE_W, PADDLE_H ) )
	{
		g_fBallVelocityY = -g_fBallVelocityY;
		g_pSoundSystem->Play( g_hHitSound, false );
	}

	// Intersect with enemy
	if ( Intersects( g_fBallX, g_fBallY, BALL_SIZE, BALL_SIZE,
					 g_fEnemyX, g_fEnemyY, PADDLE_W, PADDLE_H ) )
	{
		g_fBallVelocityY = -g_fBallVelocityY;
		g_pSoundSystem->Play( g_hHitSound, false );
	}


	// Hit left or right
	if ( g_fBallX <= 0.0f )
	{
		g_fBallX = 0.0f;
		g_fBallVelocityX = -g_fBallVelocityX;
		g_pSoundSystem->Play( g_hHitSound, false );
	}
	else if ( g_fBallX + BALL_SIZE >= WIDTH )
	{
		g_fBallX = WIDTH - BALL_SIZE;
		g_fBallVelocityX = -g_fBallVelocityX;
		g_pSoundSystem->Play( g_hHitSound, false );
	}

	// Hit top or bottom 
	if ( g_fBallY + BALL_SIZE >= HEIGHT )
	{
		g_iEnemyScore++;
		g_pSoundSystem->Play( g_hClearSound, false );
		Game_ResetBall( -1 );
	}
	else if ( g_fBallY <= 0.0f )
	{
		g_iPlayerScore++;
		g_pSoundSystem->Play( g_hClearSound, false );
		Game_ResetBall( 1 );
	}
}

//-----------------------------------------------------------------------------
// Name: Game_Update()
// Desc:
//-----------------------------------------------------------------------------
void Game_Update()
{
	Game_UpdatePlayer();
	Game_UpdateEnemy();
	Game_UpdateBall();
}

//-----------------------------------------------------------------------------
// Name: Game_PreRender()
// Desc:
//-----------------------------------------------------------------------------
void Game_PreRender()
{
	SVIEWPORT viewport;
	viewport.Width = WIDTH;
	viewport.Height = HEIGHT;
	g_pRender->SetViewport( &viewport );

	Game_Update();
}

//-----------------------------------------------------------------------------
// Name: Game_PostRender()
// Desc:
//-----------------------------------------------------------------------------
void Game_PostRender()
{
	// Player
	g_pRender->Draw2DRect( INVALID_RENDER_HANDLE, g_fPlayerX, g_fPlayerY,
		PADDLE_W, PADDLE_H, 0xffffffff );

	// Enemy
	g_pRender->Draw2DRect( INVALID_RENDER_HANDLE, g_fEnemyX, g_fEnemyY,
		PADDLE_W, PADDLE_H, 0xffffffff );

	// Ball
	g_pRender->Draw2DRect( INVALID_RENDER_HANDLE, g_fBallX, g_fBallY,
		BALL_SIZE, BALL_SIZE, 0xffffffff );

	// Score
	char scoreText[32];
	sprintf( scoreText, "Player: %d Enemy: %d", g_iPlayerScore, g_iEnemyScore );
	g_pRender->Draw2DText( g_hFont, scoreText, 0.0f, 0.0f, 0xffffffff );
}

//-----------------------------------------------------------------------------
// Name: Game_OnEvent( EventType_t type, int userNum, const char* userString )
// Desc:
//-----------------------------------------------------------------------------
void Game_OnEvent( EventType_t type, int userNum, const char* userString )
{
	if ( type == EVENT_ON_GAME_INIT )
	{
		Game_Init();
	}
}