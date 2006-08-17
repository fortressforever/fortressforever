////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-08-11 09:08:58 -0700 (Fri, 11 Aug 2006) $
// $LastChangedRevision: 1243 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __USERFLAGS_H__
#define __USERFLAGS_H__

#include "Omni-Bot_BasicTypes.h"

class BitFlag32
{
public:
	bool CheckFlag(obint32 _flag) const
	{
		return (m_Flags & ((obint32)1<<_flag)) != 0;
	}
	void SetFlag(obint32 _flag)
	{
		m_Flags |= ((obint32)1<<_flag);
	}
	void ClearFlag(obint32 _flag)
	{
		m_Flags &= ~((obint32)1<<_flag);
	}
	void ClearAll()
	{
		m_Flags = 0;
	}
	explicit BitFlag32(obint32 flgs = 0) : m_Flags(flgs) {}
private:
	obint32	m_Flags;
	//obint32	m_Persistant;
};

class BitFlag64
{
public:
	bool CheckFlag(obint32 _flag) const
	{
		obint64 flg = (obint64)1<<_flag;
		return (m_Flags & flg) != 0;
	}
	void SetFlag(obint32 _flag)
	{
		m_Flags |= ((obint64)1<<_flag);
	}
	void ClearFlag(obint32 _flag)
	{
		m_Flags &= ~((obint64)1<<_flag);
	}
	void ClearAll()
	{
		m_Flags = 0;
	}
	explicit BitFlag64(obint64 flgs = 0) : m_Flags(flgs) {}
private:
	obint64	m_Flags;
	//obint64	m_Persistant;
};


#endif
