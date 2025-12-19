#include <assert.h>
#include "debug.h"
#include "render.h"

// Model Container

CModelContainer::CModelContainer()
{
	m_Extension[0] = 0;
	m_Description[0] = 0;
}

CModelContainer::~CModelContainer()
{
}

void CModelContainer::Init()
{
}

void CModelContainer::Shutdown()
{
}

IModel* CModelContainer::CreateModel( const char* pFilename )
{
	return NULL;
}

void CModelContainer::ReleaseModel( IModel* pModel )
{
}

// Render

void CRender::RegisterModelContainer( CModelContainer* pContainer )
{
	assert( pContainer );
	m_pContainers[ m_dwNumContainers++ ] = pContainer;

	pContainer->Init();

	//DPrintf( "CRender::RegisterModelContainer: registered container \"%s\" \"%s\"\n",
	//	pContainer->m_Extension, pContainer->m_Description );
}