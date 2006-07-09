////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-04-14 21:53:05 -0400 (Fri, 14 Apr 2006) $
// $LastChangedRevision: 1171 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __MESSAGEHELPER_H__
#define __MESSAGEHELPER_H__

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

	/*template<class Type>
	const Type *Get() const
	{
		assert(sizeof(Type) <= m_BlockSize && "Memory Block Too Small!");
		return static_cast<const Type*>(m_pVoid);
	}*/

	int GetMessageId() const { return m_MessageId; }

	operator bool() const
	{
		return (m_MessageId != 0);
	}
	
	MessageHelper(int _msgId, void *_void, obuint32 _size) :
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
