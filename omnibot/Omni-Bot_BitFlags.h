////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-08-26 20:09:40 -0700 (Sat, 26 Aug 2006) $
// $LastChangedRevision: 1258 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __USERFLAGS_H__
#define __USERFLAGS_H__

#include "Omni-Bot_BasicTypes.h"

class BitFlag32
{
public:
	bool AnyFlagSet() const
	{
		return m_Flags != 0;
	}
	bool CheckFlag(obint32 _flag) const
	{
		return (m_Flags & ((obint32)1<<_flag)) != 0;
	}
	void SetFlag(obint32 _flag)
	{
		m_Flags |= ((obint32)1<<_flag);
	}
	void SetFlag(obint32 _flag, bool _set)
	{
		if(_set)
			SetFlag(_flag);
		else
			ClearFlag(_flag);
	}
	void ClearFlag(obint32 _flag)
	{
		m_Flags &= ~((obint32)1<<_flag);
	}
	void ClearAll()
	{
		m_Flags = 0;
	}
	BitFlag32& operator&=(const BitFlag32& rhs)
	{
		m_Flags &= rhs.m_Flags;
		return *this;
	}
	BitFlag32 operator&(const BitFlag32& rhs) const
	{
		BitFlag32 bf(*this);
		bf &= rhs;
		return bf;
	}
	explicit BitFlag32(obint32 flgs = 0) : m_Flags(flgs) {}
private:
	obint32	m_Flags;
	//obint32	m_Persistant;
};

class BitFlag64
{
public:
	bool AnyFlagSet() const
	{
		return m_Flags != 0;
	}
	bool CheckFlag(obint32 _flag) const
	{
		obint64 flg = (obint64)1<<_flag;
		return (m_Flags & flg) != 0;
	}
	void SetFlag(obint32 _flag)
	{
		m_Flags |= ((obint64)1<<_flag);
	}
	void SetFlag(obint32 _flag, bool _set)
	{
		if(_set)
			SetFlag(_flag);
		else
			ClearFlag(_flag);
	}
	void ClearFlag(obint32 _flag)
	{
		m_Flags &= ~((obint64)1<<_flag);
	}
	void ClearAll()
	{
		m_Flags = 0;
	}
	BitFlag64& operator&=(const BitFlag64& rhs)
	{
		m_Flags &= rhs.m_Flags;
		return *this;
	}
	BitFlag64 operator&(const BitFlag64& rhs) const
	{
		BitFlag64 bf(*this);
		bf &= rhs;
		return bf;
	}
	explicit BitFlag64(obint64 flgs = 0) : m_Flags(flgs) {}
private:
	obint64	m_Flags;
	//obint64	m_Persistant;
};


#endif
