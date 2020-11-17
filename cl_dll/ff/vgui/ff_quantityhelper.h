#include "cbase.h"

namespace FFQuantityHelper
{		
	enum Position {
		ANCHORPOS_TOPLEFT=0,
		ANCHORPOS_TOPCENTER,
		ANCHORPOS_TOPRIGHT,
		ANCHORPOS_MIDDLELEFT,
		ANCHORPOS_MIDDLECENTER,
		ANCHORPOS_MIDDLERIGHT,
		ANCHORPOS_BOTTOMLEFT,
		ANCHORPOS_BOTTOMCENTER,
		ANCHORPOS_BOTTOMRIGHT
	};

	enum AlignmentHorizontal {
		ALIGN_LEFT=0,
		ALIGN_CENTER,
		ALIGN_RIGHT
	};

	enum AlignmentVertical {
		ALIGN_TOP=0,
		ALIGN_MIDDLE,
		ALIGN_BOTTOM
	};

	enum ColorMode {
		COLOR_MODE_CUSTOM=0,
		COLOR_MODE_STEPPED,
		COLOR_MODE_FADED,
		COLOR_MODE_TEAM
	};

	template <typename T>
	bool Change( T& existingThing, T newThing )
	{
		bool bHasChange = existingThing != newThing;

		if(bHasChange)
		{
			existingThing = newThing;
		}

		return bHasChange;
	}

	vgui::HFont GetFont(
		vgui::HFont* hfFamily,
		int iSize,
		bool bUseModifier);

	int GetInt(
		const char *keyName,
		KeyValues *kvStyleData,
		int iDefaultValue = -1);

	int GetInt(
		const char *keyName,
		KeyValues *kvStyleData,
		KeyValues *kvDefaultStyleData,
		int iDefaultValue = -1);

	KeyValues* GetData(
		const char *keyName,
		KeyValues *kvStyleData,
		KeyValues *kvDefaultStyleData);

	void CombineHash(
		int &iHash, 
		int iValue);
	
	void CalculateAnchorOffset(
		int iAnchorPosition,
		int &iAnchorPositionX,
		int &iAnchorPositionY,
		int iAnchorWidth,
		int iAnchorHeight);

	void ConvertToAlignment(
		int iAnchorPos,
		int &iAlignHoriz, 
		int &iAlignVert);
	
	void CalculatePositionOffset(
		int &iX,
		int &iY,
		int iWide,
		int iTall,
		int iAlignHoriz,
		int iAlignVert);
}