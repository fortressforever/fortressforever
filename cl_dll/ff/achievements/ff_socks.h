/********************************************************************
	created:	2005/12/27
	created:	27:12:2005   20:49
	filename: 	f:\cvs\code\stats\ff_socks.h
	file path:	f:\cvs\code\stats
	file base:	ff_socks
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#ifndef FF_SOCKS_H
#define FF_SOCKS_H

/**
* Sockets
*
* @author Gavin "Mirvin_Monkey" Bramhill
* @version 1.0.0
*/
class Socks
{
public:
	Socks();
	virtual ~Socks();

	bool	Open(int type, int protocol);
	bool	Connect(const char *hostname, unsigned short port);
	bool	Send(const char *buffer);
	bool	CheckBuffer();
	int		RecvNoCheck(void *buffer, int bufferlen);
	int		Recv(void *buffer, int bufferlen);
	bool	Close();

private:
	int		m_iSocket;
};

#endif /* FF_SOCKS_H */
