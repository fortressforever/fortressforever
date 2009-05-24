
#include "cbase.h"
#include "ProxyEntity.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/ITexture.h"
#include "vtf/vtf.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialProxy.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CTeamColorProxy; // forward definition

class CTeamColorRegenerator : public ITextureRegenerator
{
public:
	CTeamColorRegenerator( CTeamColorProxy *pProxy ) : m_pProxy(pProxy) {}
	virtual void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect );
	virtual void Release( void );
private:
	CTeamColorProxy *m_pProxy;
};

class CTeamColorProxy: public IMaterialProxy
{
public:
	CTeamColorProxy();
	virtual ~CTeamColorProxy();
	virtual bool Init( IMaterial* pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pEntity );
	virtual void Release( void ) { delete this; }
	void GenerateTeamTexture( ITexture *pTexture, IVTFTexture *pVTFTexture );

private:
	IMaterialVar		*m_pTextureVar;   // The material variable
	ITexture		*m_pTexture;      // The texture
	ITextureRegenerator	*m_pTextureRegen; // The regenerator
	C_BaseEntity *m_pEnt;
};

void CTeamColorRegenerator::RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect )
{
	m_pProxy->GenerateTeamTexture(pTexture, pVTFTexture);
}

void CTeamColorProxy::GenerateTeamTexture( ITexture *pTexture, IVTFTexture *pVTFTexture )
{
	if (!m_pEnt)
		return;

	// Gets a pointer to the start of the texture memory.
	unsigned char *imageData = pVTFTexture->ImageData();

	//NOTE: the format of any texture using the IVTFTexture interface appears to be in BGRX8888 everytime
	//      despite what you set it at in the VTF, so keep that in mind.

	// Lets place our texture width and height in local vars.
	int height = pVTFTexture->Height();
	int width = pVTFTexture->Width();

	int m = height*width;

	// Now, lets have some fun with our image.
	// This could be optimized...but I'm going for readability
	for( int i = 0; i < m; i++ )
	{
		int texture_offset = i<<2; // 4*i
		
		/*
		imageData[texture_offset]     = 255;        // Blue Channel
		imageData[texture_offset + 1] = 0;        // Green Channel
		imageData[texture_offset + 2] = 0;  // Red Channel
		imageData[texture_offset + 3] = 255;      // 100% Alpha (Opaque)
		continue;
		*/

		//if (imageData[texture_offset + 3] <= 127)
		{
			// it's mostly transparent, go ahead and hue shift

			// generate the hsl values
			float h,s,l;
			float r = imageData[texture_offset+2], g = imageData[texture_offset+1], b = imageData[texture_offset];

			float maxColor = max(r, max(g, b)); 
			float minColor = min(r, min(g, b));

			if (imageData[texture_offset] == imageData[texture_offset+1] && imageData[texture_offset+1] == imageData[texture_offset+2])
			{   
				h = 0.0;
				s = 0.0;       
				l = r;
			}
			else
			{   
				l = (minColor + maxColor) / 2;     

				if(l < 0.5) s = (maxColor - minColor) / (maxColor + minColor);
				else s = (maxColor - minColor) / (2.0 - maxColor - minColor);

				if(r == maxColor) h = (g - b) / (maxColor - minColor);
				else if(g == maxColor) h = 2.0 + (b - r) / (maxColor - minColor);
				else h = 4.0 + (r - g) / (maxColor - minColor);

				h /= 6; //to bring it to a number between 0 and 1
				if(h < 0) h ++;
			}

			// go ahead and shift it by 1/3
			h = fmod(h + 2.0/6.0, 1.0);

			// convert back to rgb
			if (s == 0)
			{
				r = g = b = l;
			}
			else
			{
				float temp1, temp2, tempr, tempg, tempb;

        //Set the temporary values      
        if(l < 0.5) temp2 = l * (1 + s);      
        else temp2 = (l + s) - (l * s);     
        temp1 = 2 * l - temp2;    
        tempr = h + 1.0 / 3.0;    
        if(tempr > 1) tempr--;
        tempg = h;     
        tempb = h - 1.0 / 3.0;
        if(tempb < 0) tempb++; 
        
        //Red     
        if(tempr < 1.0 / 6.0) r = temp1 + (temp2 - temp1) * 6.0 * tempr;      
        else if(tempr < 0.5) r = temp2;   
        else if(tempr < 2.0 / 3.0) r = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempr) * 6.0;
        else r = temp1; 
        
        //Green       
        if(tempg < 1.0 / 6.0) g = temp1 + (temp2 - temp1) * 6.0 * tempg;    
        else if(tempg < 0.5) g = temp2;
        else if(tempg < 2.0 / 3.0) g = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempg) * 6.0;
        else g = temp1; 
        
        //Blue    
        if(tempb < 1.0 / 6.0) b = temp1 + (temp2 - temp1) * 6.0 * tempb;   
        else if(tempb < 0.5) b = temp2; 
        else if(tempb < 2.0 / 3.0) b = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempb) * 6.0;    
        else b = temp1;
			}


			imageData[texture_offset]     = int(b*255);        // Blue Channel
			imageData[texture_offset + 1] = int(g*255);        // Green Channel
			imageData[texture_offset + 2] = int(r*255);  // Red Channel
			imageData[texture_offset + 3] = 255;      // 100% Alpha (Opaque)
		}
	}

}

void CTeamColorRegenerator::Release()
{
	//delete stuff
}

CTeamColorProxy::CTeamColorProxy()
{
	m_pTextureVar = NULL;
	m_pTexture = NULL;
	m_pTextureRegen = NULL;

	m_pEnt = NULL;
}

CTeamColorProxy::~CTeamColorProxy()
{
	if (m_pTexture != NULL)
	{
		m_pTexture->SetTextureRegenerator( NULL );
	}
}

bool CTeamColorProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool found;
	
	m_pTextureVar = pMaterial->FindVar("$basetexture", &found, false);  // Get a reference to our base texture variable
	if( !found )
	{
		m_pTextureVar = NULL;
		return false;
	}

	m_pTexture = m_pTextureVar->GetTextureValue();  // Now grab a ref to the actual texture
	if (m_pTexture != NULL)
	{
		m_pTextureRegen = new CTeamColorRegenerator( this );  // Here we create our regenerator
		m_pTexture->SetTextureRegenerator(m_pTextureRegen); // And here we attach it to the texture.
	}

	return true;
}

void CTeamColorProxy::OnBind( void *pEntity )
{
	if( !m_pTexture )  // Make sure we have a texture
		return;

	if(!m_pTextureVar->IsTexture())  // Make sure it is a texture
		return;

	m_pEnt = (C_BaseEntity *)pEntity;
	m_pTexture->Download(); // Force the regenerator to redraw

	m_pEnt = NULL;
}

EXPOSE_INTERFACE( CTeamColorProxy, IMaterialProxy, "TeamColorProxy" IMATERIAL_PROXY_INTERFACE_VERSION );
