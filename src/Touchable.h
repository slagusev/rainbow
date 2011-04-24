/// Interface for touchable objects.

/// Copyright 2010 Bifrost Games. All rights reserved.
/// \author Tommy Nguyen

#ifndef TOUCHABLE_H_
#define TOUCHABLE_H_

#include "Input/Touch.h"

class Touchable
{
public:
	virtual void touch_began(const Touches &touches) = 0;
	virtual void touch_canceled() = 0;
	virtual void touch_ended(const Touches &touches) = 0;
	virtual void touch_moved(const Touches &touches) = 0;
};

#endif