#include "cbase.h"
#include "ff_quantityhelper.h"
#include "keyvalues.h"

namespace FFQuantityHelper
{
	vgui::HFont GetFont(
		vgui::HFont* hfFamily,
		int iSize,
		bool bUseModifier)
	{
		int iModifier
			= bUseModifier ? 1 : 0;

		int iFontIndex
			= iSize * 3 + iModifier;

		return hfFamily[iFontIndex];
	}

	int GetInt(
		const char *keyName,
		KeyValues *kvStyleData,
		int iDefaultValue)
	{
		return kvStyleData->GetInt(keyName, iDefaultValue);
	}

	int GetInt(
		const char *keyName,
		KeyValues *kvStyleData,
		KeyValues *kvDefaultStyleData,
		int iDefaultValue)
	{
		iDefaultValue = kvDefaultStyleData->GetInt(keyName, iDefaultValue);
		return kvStyleData->GetInt(keyName, iDefaultValue);
	}

	KeyValues* GetData(
		const char *keyName,
		KeyValues *kvStyleData,
		KeyValues *kvDefaultStyleData)
	{
		kvStyleData = kvStyleData->FindKey(keyName);
		
		if(kvStyleData)
		{
			return kvStyleData;
		}

		return kvDefaultStyleData->FindKey(keyName);
	}

	void CombineHash(int &iHash, int iValue) 
	{
		iHash = iHash * 23 + iValue;
	}
	
	void CalculateAnchorOffset(
		int iAnchorPosition,
		int &iAnchorPositionX,
		int &iAnchorPositionY,
		int iAnchorWidth,
		int iAnchorHeight)
	{
		int iAlignHorizontally, iAlignVertically;
		
		FFQuantityHelper
			::ConvertToAlignment(
				iAnchorPosition,
				iAlignHorizontally,
				iAlignVertically);
		
		FFQuantityHelper
			::CalculatePositionOffset(
				iAnchorPositionX,
				iAnchorPositionY,
				-iAnchorWidth,
				-iAnchorHeight,
				iAlignHorizontally,
				iAlignVertically);
	}

	void ConvertToAlignment(
		int iAnchorPosition,
		int &iAlignHoriz, 
		int &iAlignVert)
	{
		switch(iAnchorPosition)
		{
		case ANCHORPOS_TOPLEFT:
			iAlignVert = ALIGN_TOP;
			iAlignHoriz = ALIGN_LEFT;
			break;
		case ANCHORPOS_TOPCENTER:
			iAlignVert = ALIGN_TOP;
			iAlignHoriz = ALIGN_CENTER;
			break;
		case ANCHORPOS_TOPRIGHT:
			iAlignVert = ALIGN_TOP;
			iAlignHoriz = ALIGN_RIGHT;
			break;
		case ANCHORPOS_MIDDLELEFT:
			iAlignVert = ALIGN_MIDDLE;
			iAlignHoriz = ALIGN_LEFT;
			break;
		case ANCHORPOS_MIDDLECENTER:
			iAlignVert = ALIGN_MIDDLE;
			iAlignHoriz = ALIGN_CENTER;
			break;
		case ANCHORPOS_MIDDLERIGHT:
			iAlignVert = ALIGN_MIDDLE;
			iAlignHoriz = ALIGN_RIGHT;
			break;
		case ANCHORPOS_BOTTOMLEFT:
			iAlignVert = ALIGN_BOTTOM;
			iAlignHoriz = ALIGN_LEFT;
			break;
		case ANCHORPOS_BOTTOMCENTER:
			iAlignVert = ALIGN_BOTTOM;
			iAlignHoriz = ALIGN_CENTER;
			break;
		case ANCHORPOS_BOTTOMRIGHT:
			iAlignVert = ALIGN_BOTTOM;
			iAlignHoriz = ALIGN_RIGHT;
			break;
		}
	}

	void CalculatePositionOffset(
		int &iX,
		int &iY,
		int iWide,
		int iTall,
		int iAlignHoriz,
		int iAlignVert)
	{
		switch(iAlignHoriz)
		{
		case ALIGN_CENTER:
			iX = iWide / 2;
			break;
		case ALIGN_RIGHT:
			iX = iWide;
			break;
		case ALIGN_LEFT:
		default:
			iX = 0;
			break;
		}

		switch(iAlignVert)
		{
		case ALIGN_MIDDLE:
			iY = iTall / 2;
			break;
		case ALIGN_BOTTOM:
			iY = iTall;
			break;
		case ALIGN_TOP:
		default:
			iY = 0;
			break;
		}
	}
}