#include "BaseVSShader.h"

#include "ff_post_colordrain_vs20.inc"
#include "ff_post_colordrain_ps20.inc"

BEGIN_VS_SHADER(ff_post_colordrain, "Help for ff_post_colordrain")
	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
	}

	SHADER_FALLBACK
	{
		return 0;
	}

	bool NeedsFullFrameBufferTexture(IMaterialVar **params) const
	{
		return true;
	}

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			s_pShaderShadow->EnableTexture(SHADER_TEXTURE_STAGE0, true);
			s_pShaderShadow->EnableDepthWrites(false);
			int format = VERTEX_POSITION;
			s_pShaderShadow->VertexShaderVertexFormat(format, 1, 0, 0, 0);
			s_pShaderShadow->SetVertexShader("ff_post_colordrain_vs20", 0);
			s_pShaderShadow->SetPixelShader("ff_post_colordrain_ps20", 0);

			DefaultFog();
		}

		DYNAMIC_STATE
		{
			//DevMsg("Drawing ff_post_colordrain overlay.\n");
			s_pShaderAPI->BindFBTexture(SHADER_TEXTURE_STAGE0, 0);
		}

		Draw();
	}
END_SHADER