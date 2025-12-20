#include "debug.h"
#include "filesystem.h"
#include "render.h"

// Vertex Buffer

CVertexBuffer::CVertexBuffer()
{
	memset( &m_Desc, 0, sizeof( m_Desc ) );

	m_pVB = NULL;
}

CVertexBuffer::~CVertexBuffer()
{
	if ( m_pVB != NULL )
		Destroy();
}

void CVertexBuffer::Create( const D3DVERTEXBUFFER_DESC& desc )
{
	// Copy whole desc
	memcpy( &m_Desc, &desc, sizeof( m_Desc ) );

	// Create vertex buffer
	HRESULT hr = g_pRender->Dev()->CreateVertexBuffer(m_Desc.Size,
		m_Desc.Usage, m_Desc.FVF, m_Desc.Pool, &m_pVB, NULL );
	if ( FAILED( hr ) )
	{
		DError( "Failed to create vertex buffer (length %d fvf %d pool %d)",
			m_Desc.Size, m_Desc.FVF, m_Desc.Pool );
	}
}

void CVertexBuffer::Destroy()
{
	if ( m_pVB != NULL )
	{
		m_pVB->Release();
		m_pVB = NULL;
	}
}

void* CVertexBuffer::Lock( int sizeToLock )
{
	DWORD dwFlags = D3DLOCK_NOSYSLOCK;
	if ( m_Desc.Usage & D3DUSAGE_DYNAMIC )
		dwFlags = D3DLOCK_DISCARD;

	void* pLockData = NULL;
	if( FAILED( m_pVB->Lock( 0, sizeToLock, (void**)&pLockData, dwFlags ) ) )
		DError( "Failed to lock vertex buffer." );

	return pLockData;
}

void CVertexBuffer::Unlock()
{
	m_pVB->Unlock();
}

IDirect3DVertexBuffer9* CVertexBuffer::GetBuffer()
{
	return m_pVB;
}