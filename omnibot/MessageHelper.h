////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: drevil $
// $LastChangedDate: 2007-10-16 21:53:00 -0700 (Tue, 16 Oct 2007) $
// $LastChangedRevision: 2149 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __MESSAGEHELPER_H__
#define __MESSAGEHELPER_H__

#include <assert.h>

//////////////////////////////////////////////////////////////////////////

struct SubscriberHandle
{
	union 
	{
		struct
		{
			short m_MessageId;
			short m_SerialNum;
		} split;		
		int	m_Int;
	} u;
};

//////////////////////////////////////////////////////////////////////////

class MessageHelper
{
public:
	friend class MessageRouter;

	template<class Type>
	Type *Get() const
	{
		assert(sizeof(Type) == m_BlockSize && "Memory Block Doesn't match!");
		return static_cast<Type*>(m_pVoid);
	}

	template<class Type>
	void Get2(Type *&_p) const
	{
		assert(sizeof(Type) == m_BlockSize && "Memory Block Doesn't match!");
		_p = static_cast<Type*>(m_pVoid);
	}

	int GetMessageId() const { return m_MessageId; }

	operator bool() const
	{
		return (m_MessageId != 0);
	}
	
	MessageHelper(int _msgId, void *_void = 0, obuint32 _size = 0) :
		m_MessageId	(_msgId),
		m_pVoid		(_void),
		m_BlockSize	(_size)
	{
	}
	~MessageHelper() {};
private:
	mutable int m_MessageId;
	void		*m_pVoid;
	obuint32	m_BlockSize;

	MessageHelper();
};

//////////////////////////////////////////////////////////////////////////

typedef void (*pfnMessageFunction)(const MessageHelper &_helper);

//////////////////////////////////////////////////////////////////////////

#endif
