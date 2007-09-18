
#ifndef _G15_H_
#define _G15_H_

namespace G15
{
	bool IsEnabled();

	void Initialize();
	void Shutdown();
	void Update();

	//////////////////////////////////////////////////////////////////////////

    void UpdateHealth(int _current, int _max);
	void UpdateArmor(int _current, int _max);

	void AddFortPoints(const char *_msg, const char *_score);
};

#endif
