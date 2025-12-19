#include <string.h>
#include "common.h"
#include "input.h"

static CInput s_Input;
CInput* g_pInput = &s_Input;

void CInput::Init()
{
	ResetKeys();
}

void CInput::KeyAction( int key, bool bAction )
{
	m_bKeys[ key ] = bAction;
}

void CInput::ResetKeys()
{
	memset( m_bKeys, 0, sizeof( m_bKeys ) );
}
