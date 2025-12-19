#ifndef INPUT_H
#define INPUT_H

#define MAX_KEYS 260

class CInput
{
private:
	bool m_bKeys[ MAX_KEYS ];

public:
	void Init();

	void KeyAction( int key, bool bAction );
	void ResetKeys();

	bool IsKeyPressed( int key ) { return m_bKeys[ key ]; }
};

extern CInput* g_pInput;

#endif // !INPUT_H
