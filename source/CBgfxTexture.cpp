// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_BGFX_

#include "CBgfxTexture.h"
#include "os.h"

namespace ue
{
namespace video
{

//! constructor
CBgfxTexture::CBgfxTexture(IImage* image, const io::path& name)
: ITexture(name), IsRenderTarget(false), LockImage(NULL)
{
	#ifdef _DEBUG
	setDebugName("CBgfxTexture");
	#endif

    Format = image->getColorFormat();
	OriginalSize = image->getDimension();
	core::dimension2d<u32> optSize=OriginalSize.getOptimalSize();

    auto bgfxFormat = toBgfx(Format);

    uint32_t flags = BGFX_TEXTURE_NONE;
    Texture = bgfx_create_texture_2d(uint16_t(OriginalSize.Width), uint16_t(OriginalSize.Height), 1
        , bgfxFormat
        , flags
        , bgfx_copy(image->lock(), OriginalSize.Width*OriginalSize.Height * image->getBytesPerPixel())
        );
    image->unlock();

    bgfx_calc_texture_size(&Info
        , uint16_t(OriginalSize.Width)
        , uint16_t(OriginalSize.Height)
        , 0
        , false
        , 1
        , bgfxFormat
        );
}

CBgfxTexture::CBgfxTexture(const io::path& name)
    : ITexture(name), IsRenderTarget(false), LockImage(NULL)
{

}


//! destructor
CBgfxTexture::~CBgfxTexture()
{
	if (LockImage)
        LockImage->drop();

    //bgfx_destroy_texture(Texture);
}



//! lock function
void* CBgfxTexture::lock(E_TEXTURE_LOCK_MODE mode, u32 mipmapLevel)
{
	return LockImage->lock();
}



//! unlock function
void CBgfxTexture::unlock()
{
	//if (LockImage != Texture)
	//{
	//	os::Printer::log("Performance warning, slow unlock of non power of 2 texture.", ELL_WARNING);
	//	Image->copyToScaling(Texture);
	//}

    LockImage->unlock();
}


//! Returns original size of the texture.
const core::dimension2d<u32>& CBgfxTexture::getOriginalSize() const
{
	return OriginalSize;
}


//! Returns (=size) of the texture.
const core::dimension2d<u32>& CBgfxTexture::getSize() const
{
    return{ Info.width, Info.height };
}


//! returns driver type of texture (=the driver, who created the texture)
E_DRIVER_TYPE CBgfxTexture::getDriverType() const
{
	return EDT_BGFX_OPENGL;
}



//! returns color format of texture
ECOLOR_FORMAT CBgfxTexture::getColorFormat() const
{
	return Format;
}



//! returns pitch of texture (in bytes)
u32 CBgfxTexture::getPitch() const
{
	return Info.width * Info.bitsPerPixel;
}


//! Regenerates the mip map levels of the texture. Useful after locking and
//! modifying the texture
void CBgfxTexture::regenerateMipMapLevels(void* mipmapData)
{
	// our software textures don't have mip maps
}

bool CBgfxTexture::isRenderTarget() const
{
	return IsRenderTarget;
}

bgfx_texture_format CBgfxTexture::toBgfx(ECOLOR_FORMAT format)
{
    static bgfx_texture_format texFormats[] =
    {
        BGFX_TEXTURE_FORMAT_RGB5A1,//ECF_A1R5G5B5 = 0,
        BGFX_TEXTURE_FORMAT_R5G6B5,//ECF_R5G6B5,
        BGFX_TEXTURE_FORMAT_UNKNOWN,//ECF_R8G8B8,
        BGFX_TEXTURE_FORMAT_RGBA8,//ECF_A8R8G8B8,

        /** Compressed image formats. **/
        BGFX_TEXTURE_FORMAT_BC1,//ECF_DXT1,
        BGFX_TEXTURE_FORMAT_BC1,//ECF_DXT2,
        BGFX_TEXTURE_FORMAT_BC2,//ECF_DXT3,
        BGFX_TEXTURE_FORMAT_BC2,//ECF_DXT4,
        BGFX_TEXTURE_FORMAT_BC3,//ECF_DXT5,
        BGFX_TEXTURE_FORMAT_PTC12,//ECF_PVRTC_RGB2,
        BGFX_TEXTURE_FORMAT_PTC12A,//ECF_PVRTC_ARGB2,
        BGFX_TEXTURE_FORMAT_PTC14,//ECF_PVRTC_RGB4,
        BGFX_TEXTURE_FORMAT_PTC14A,//ECF_PVRTC_ARGB4,
        BGFX_TEXTURE_FORMAT_PTC22,//ECF_PVRTC2_ARGB2,
        BGFX_TEXTURE_FORMAT_PTC24,//ECF_PVRTC2_ARGB4,
        BGFX_TEXTURE_FORMAT_ETC1,//ECF_ETC1,
        BGFX_TEXTURE_FORMAT_ETC2,//ECF_ETC2_RGB,
        BGFX_TEXTURE_FORMAT_ETC2A,//ECF_ETC2_ARGB,

        /** Floating Point formats. The following formats may only be used for render target textures. */
        BGFX_TEXTURE_FORMAT_R16F,//ECF_R16F,
        BGFX_TEXTURE_FORMAT_RG16F,//ECF_G16R16F,
        BGFX_TEXTURE_FORMAT_RGBA16F,//ECF_A16B16G16R16F,
        BGFX_TEXTURE_FORMAT_R32F,//ECF_R32F,
        BGFX_TEXTURE_FORMAT_RG32F,//ECF_G32R32F,
        BGFX_TEXTURE_FORMAT_RGBA32F,//ECF_A32B32G32R32F,
        BGFX_TEXTURE_FORMAT_UNKNOWN,//ECF_UNKNOWN
    };

    return texFormats[format];
}

CBgfxFBOTexture::CBgfxFBOTexture(const core::dimension2du& size,
    const io::path& name, const ECOLOR_FORMAT format)
    :CBgfxTexture(name)
{
    OriginalSize = size;
    IsRenderTarget = true;
    Format = format;

    FrameBuffer = bgfx_create_frame_buffer(OriginalSize.Width, OriginalSize.Height, toBgfx(format), 0);
}

CBgfxFBOTexture::~CBgfxFBOTexture()
{
    bgfx_destroy_frame_buffer(FrameBuffer);
}

} // end namespace video
} // end namespace ue

#endif // _IRR_COMPILE_WITH_SOFTWARE_

