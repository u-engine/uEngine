// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_D3D8_MATERIAL_RENDERER_H_INCLUDED__
#define __C_D3D8_MATERIAL_RENDERER_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_WINDOWS_API_

#ifdef _IRR_COMPILE_WITH_DIRECT3D_8_
#include <d3d8.h>

#include "IMaterialRenderer.h"
#include "CD3D8Driver.h"

namespace ue
{
namespace video
{

namespace
{
D3DMATRIX UnitMatrixD3D8;
D3DMATRIX SphereMapMatrixD3D8;
inline void setTextureColorStage(IDirect3DDevice8* dev, DWORD i,
		DWORD arg1, DWORD op, DWORD arg2)
{
	dev->SetTextureStageState(i, D3DTSS_COLOROP, op);
	dev->SetTextureStageState(i, D3DTSS_COLORARG1, arg1);
	dev->SetTextureStageState(i, D3DTSS_COLORARG2, arg2);
}
inline void setTextureColorStage(IDirect3DDevice8* dev, DWORD i, DWORD arg1)
{
	dev->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(i, D3DTSS_COLORARG1, arg1);
}

inline void setTextureAlphaStage(IDirect3DDevice8* dev, DWORD i,
		DWORD arg1, DWORD op, DWORD arg2)
{
	dev->SetTextureStageState(i, D3DTSS_ALPHAOP, op);
	dev->SetTextureStageState(i, D3DTSS_ALPHAARG1, arg1);
	dev->SetTextureStageState(i, D3DTSS_ALPHAARG2, arg2);
}
inline void setTextureAlphaStage(IDirect3DDevice8* dev, DWORD i, DWORD arg1)
{
	dev->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(i, D3DTSS_ALPHAARG1, arg1);
}
} // anonymous namespace

//! Base class for all internal D3D8 material renderers
class CD3D8MaterialRenderer : public IMaterialRenderer
{
public:

	//! Constructor
	CD3D8MaterialRenderer(IDirect3DDevice8* d3ddev, CD3D8Driver* driver)
		: pID3DDevice(d3ddev), Driver(driver)
	{
	}

protected:

	IDirect3DDevice8* pID3DDevice;
	CD3D8Driver* Driver;
};


//! Solid material renderer
class CD3D8MaterialRenderer_SOLID : public CD3D8MaterialRenderer
{
public:

	CD3D8MaterialRenderer_SOLID(IDirect3DDevice8* p, CD3D8Driver* d)
		: CD3D8MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) _IRR_OVERRIDE_
	{
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			setTextureColorStage(pID3DDevice, 0,
				D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
		}

		pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	}
};

//! Generic Texture Blend
class CD3D8MaterialRenderer_ONETEXTURE_BLEND : public CD3D8MaterialRenderer
{
public:

	CD3D8MaterialRenderer_ONETEXTURE_BLEND(IDirect3DDevice8* p, CD3D8Driver* d)
		: CD3D8MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) _IRR_OVERRIDE_
	{
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

//		if (material.MaterialType != lastMaterial.MaterialType ||
//			material.MaterialTypeParam != lastMaterial.MaterialTypeParam ||
//			resetAllRenderstates)
		{

			E_BLEND_FACTOR srcFact,dstFact;
			E_MODULATE_FUNC modulate;
			u32 alphaSource;
			unpack_textureBlendFunc ( srcFact, dstFact, modulate, alphaSource, material.MaterialTypeParam );

            Driver->getBridgeCalls()->setBlend(true);
            Driver->getBridgeCalls()->setBlendFunc(Driver->getD3DBlend(srcFact), Driver->getD3DBlend(dstFact));

			setTextureColorStage(pID3DDevice, 0,
				D3DTA_TEXTURE, Driver->getD3DModulate(modulate), D3DTA_DIFFUSE);

			if ( alphaSource && (textureBlendFunc_hasAlpha ( srcFact ) || textureBlendFunc_hasAlpha ( dstFact ) ))
			{
				if (alphaSource==EAS_VERTEX_COLOR)
				{
					setTextureAlphaStage(pID3DDevice, 0, D3DTA_DIFFUSE);
				}
				else if (alphaSource==EAS_TEXTURE)
				{
					setTextureAlphaStage(pID3DDevice, 0, D3DTA_TEXTURE);
				}
				else
				{
					setTextureAlphaStage(pID3DDevice, 0,
						D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
				}
			}

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

		}
	}

    virtual void OnUnsetMaterial() _IRR_OVERRIDE_
	{
		Driver->getBridgeCalls()->setBlend(false);
	}

	//! Returns if the material is transparent.
	/** The scene management needs to know this for being able to sort the
	materials by opaque and transparent.
	The return value could be optimized, but we'd need to know the
	MaterialTypeParam for it. */
	virtual bool isTransparent() const _IRR_OVERRIDE_
	{
		return true;
	}
};


//! Solid 2 layer material renderer
class CD3D8MaterialRenderer_SOLID_2_LAYER : public CD3D8MaterialRenderer
{
public:

	CD3D8MaterialRenderer_SOLID_2_LAYER(IDirect3DDevice8* p, CD3D8Driver* d)
		: CD3D8MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) _IRR_OVERRIDE_
	{
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			setTextureColorStage(pID3DDevice, 0, D3DTA_TEXTURE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);
		}
	}
};


//! Transparent add color material renderer
class CD3D8MaterialRenderer_TRANSPARENT_ADD_COLOR : public CD3D8MaterialRenderer
{
public:

	CD3D8MaterialRenderer_TRANSPARENT_ADD_COLOR(IDirect3DDevice8* p, CD3D8Driver* d)
		: CD3D8MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) _IRR_OVERRIDE_
	{
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

        Driver->getBridgeCalls()->setBlend(true);
        Driver->getBridgeCalls()->setBlendFunc(D3DBLEND_ONE, D3DBLEND_INVSRCCOLOR);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			setTextureColorStage(pID3DDevice, 0,
				D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		}
	}

	virtual void OnUnsetMaterial() _IRR_OVERRIDE_
	{
		Driver->getBridgeCalls()->setBlend(false);
	}

	//! Returns if the material is transparent. The scene management needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() const _IRR_OVERRIDE_
	{
		return true;
	}
};


//! Transparent vertex alpha material renderer
class CD3D8MaterialRenderer_TRANSPARENT_VERTEX_ALPHA : public CD3D8MaterialRenderer
{
public:

	CD3D8MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(IDirect3DDevice8* p, CD3D8Driver* d)
		: CD3D8MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) _IRR_OVERRIDE_
	{
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		Driver->getBridgeCalls()->setBlend(true);
		Driver->getBridgeCalls()->setBlendFunc(D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			setTextureColorStage(pID3DDevice, 0,
				D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
			setTextureAlphaStage(pID3DDevice, 0, D3DTA_DIFFUSE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		}
	}

	virtual void OnUnsetMaterial() _IRR_OVERRIDE_
	{
		Driver->getBridgeCalls()->setBlend(false);
	}

	//! Returns if the material is transparent. The scene managment needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() const _IRR_OVERRIDE_
	{
		return true;
	}
};


//! Transparent alpha channel material renderer
class CD3D8MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL : public CD3D8MaterialRenderer
{
public:

	CD3D8MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(IDirect3DDevice8* p, CD3D8Driver* d)
		: CD3D8MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) _IRR_OVERRIDE_
	{
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		Driver->getBridgeCalls()->setBlend(true);
		Driver->getBridgeCalls()->setBlendFunc(D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates
			|| material.MaterialTypeParam != lastMaterial.MaterialTypeParam )
		{
			setTextureColorStage(pID3DDevice, 0,
				D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_CURRENT);
			setTextureAlphaStage(pID3DDevice, 0, D3DTA_TEXTURE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

			pID3DDevice->SetRenderState(D3DRS_ALPHAREF, core::floor32(material.MaterialTypeParam * 255.f));
			pID3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		}
	}

	virtual void OnUnsetMaterial() _IRR_OVERRIDE_
	{
		pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		Driver->getBridgeCalls()->setBlend(false);
	}

	//! Returns if the material is transparent. The scene managment needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() const _IRR_OVERRIDE_
	{
		return true;
	}
};


//! Transparent alpha channel material renderer
class CD3D8MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF : public CD3D8MaterialRenderer
{
public:

	CD3D8MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF(IDirect3DDevice8* p, CD3D8Driver* d)
		: CD3D8MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) _IRR_OVERRIDE_
	{
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			setTextureColorStage(pID3DDevice, 0,
				D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_CURRENT);
			setTextureAlphaStage(pID3DDevice, 0, D3DTA_TEXTURE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

			// 127 is required by EMT_TRANSPARENT_ALPHA_CHANNEL_REF
			pID3DDevice->SetRenderState(D3DRS_ALPHAREF, 127);
			pID3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		}
	}

	virtual void OnUnsetMaterial() _IRR_OVERRIDE_
	{
		pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	}

	//! Returns if the material is transparent. The scene managment needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() const _IRR_OVERRIDE_
	{
		return false; // this material is not really transparent because it does no blending.
	}
};


//! material renderer for all kinds of lightmaps
class CD3D8MaterialRenderer_LIGHTMAP : public CD3D8MaterialRenderer
{
public:

	CD3D8MaterialRenderer_LIGHTMAP(IDirect3DDevice8* p, CD3D8Driver* d)
		: CD3D8MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) _IRR_OVERRIDE_
	{
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			if (material.MaterialType >= EMT_LIGHTMAP_LIGHTING)
			{
				// with lighting
				setTextureColorStage(pID3DDevice, 0,
					D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
			}
			else
			{
				setTextureColorStage(pID3DDevice, 0, D3DTA_TEXTURE);
			}

			pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

			setTextureColorStage(pID3DDevice, 1,
				D3DTA_TEXTURE,
				(material.MaterialType == EMT_LIGHTMAP_ADD)?
				D3DTOP_ADD:
				(material.MaterialType == EMT_LIGHTMAP_M4 || material.MaterialType == EMT_LIGHTMAP_LIGHTING_M4)?
				D3DTOP_MODULATE4X:
				(material.MaterialType == EMT_LIGHTMAP_M2 || material.MaterialType == EMT_LIGHTMAP_LIGHTING_M2)?
				D3DTOP_MODULATE2X:
				D3DTOP_MODULATE,
				D3DTA_CURRENT);
		}
	}
};


//! material renderer for detail maps
class CD3D8MaterialRenderer_DETAIL_MAP : public CD3D8MaterialRenderer
{
public:

	CD3D8MaterialRenderer_DETAIL_MAP(IDirect3DDevice8* p, CD3D8Driver* d)
		: CD3D8MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) _IRR_OVERRIDE_
	{
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			setTextureColorStage(pID3DDevice, 0,
				D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
			setTextureColorStage(pID3DDevice, 1,
				D3DTA_TEXTURE, D3DTOP_ADDSIGNED, D3DTA_CURRENT);
			pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
		}
	}
};


//! sphere map material renderer
class CD3D8MaterialRenderer_SPHERE_MAP : public CD3D8MaterialRenderer
{
public:

	CD3D8MaterialRenderer_SPHERE_MAP(IDirect3DDevice8* p, CD3D8Driver* d)
		: CD3D8MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) _IRR_OVERRIDE_
	{
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			setTextureColorStage(pID3DDevice, 0,
				D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);

			pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

			pID3DDevice->SetTransform( D3DTS_TEXTURE0, &SphereMapMatrixD3D8 );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
			pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
		}
	}

	virtual void OnUnsetMaterial() _IRR_OVERRIDE_
	{
		pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		pID3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);
		pID3DDevice->SetTransform( D3DTS_TEXTURE0, &UnitMatrixD3D8 );
	}
};


//! reflection 2 layer material renderer
class CD3D8MaterialRenderer_REFLECTION_2_LAYER : public CD3D8MaterialRenderer
{
public:

	CD3D8MaterialRenderer_REFLECTION_2_LAYER(IDirect3DDevice8* p, CD3D8Driver* d)
		: CD3D8MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) _IRR_OVERRIDE_
	{
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			setTextureColorStage(pID3DDevice, 0,
				D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);

			setTextureColorStage(pID3DDevice, 1,
				D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_CURRENT);

			pID3DDevice->SetTransform( D3DTS_TEXTURE1, &SphereMapMatrixD3D8 );
			pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
			pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
		}
	}

	virtual void OnUnsetMaterial() _IRR_OVERRIDE_
	{
		pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		pID3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1);
		pID3DDevice->SetTransform( D3DTS_TEXTURE1, &UnitMatrixD3D8 );
	}
};


//! reflection 2 layer material renderer
class CD3D8MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER : public CD3D8MaterialRenderer
{
public:

	CD3D8MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER(IDirect3DDevice8* p, CD3D8Driver* d)
		: CD3D8MaterialRenderer(p, d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) _IRR_OVERRIDE_
	{
		services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		Driver->getBridgeCalls()->setBlend(true);
		Driver->getBridgeCalls()->setBlendFunc(D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			setTextureColorStage(pID3DDevice, 0,
				D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
			setTextureAlphaStage(pID3DDevice, 0, D3DTA_DIFFUSE);
			setTextureColorStage(pID3DDevice, 1,
				D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_CURRENT);
			setTextureAlphaStage(pID3DDevice, 1, D3DTA_CURRENT);

			pID3DDevice->SetTransform(D3DTS_TEXTURE1, &SphereMapMatrixD3D8 );
			pID3DDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
			pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
		}
	}

	virtual void OnUnsetMaterial() _IRR_OVERRIDE_
	{
		pID3DDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
		pID3DDevice->SetTransform(D3DTS_TEXTURE1, &UnitMatrixD3D8);
		Driver->getBridgeCalls()->setBlend(false);
	}

	//! Returns if the material is transparent. The scene managment needs to know this
	//! for being able to sort the materials by opaque and transparent.
	virtual bool isTransparent() const _IRR_OVERRIDE_
	{
		return true;
	}
};

} // end namespace video
} // end namespace ue

#endif
#endif
#endif

