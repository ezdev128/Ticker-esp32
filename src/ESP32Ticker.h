/* 
  Ticker.h - esp32 library that calls functions periodically

  Copyright (c) 2017 Bert Melis. all rights reserved
  Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
  This file is part of the esp32 core for Arduino environment.
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef TICKER_H
#define TICKER_H

#include <esp32-hal-log.h>

extern "C" {
	#include "esp_timer.h"
}

class Ticker
{
public:
	Ticker();
	~Ticker();
	typedef void (*callback_t)();
	typedef void (*callback_with_arg_t)(void*);

    __attribute__((unused)) void attach(float seconds, callback_t callback);
    __attribute__((unused)) void attach_ms(uint32_t milliseconds, callback_t callback);
	template<typename TArg>
    __attribute__((unused)) void attach(float seconds, void (*callback)(TArg), TArg arg);
	template<typename TArg>
    __attribute__((unused)) void attach_ms(uint32_t milliseconds, void (*callback)(TArg), TArg arg);

    __attribute__((unused)) void once(float seconds, callback_t callback);
    template<typename TArg>
    __attribute__((unused)) void once(float seconds, void (*callback)(TArg), TArg arg);
    __attribute__((unused)) void once_ms(uint32_t milliseconds, callback_t callback);
	template<typename TArg>
    __attribute__((unused)) void once_ms(uint32_t milliseconds, void (*callback)(TArg), TArg arg);

    __attribute__((unused)) void detach();
    __attribute__((unused)) bool active();

protected:	
    void _attach_ms(uint32_t milliseconds, bool repeat, callback_with_arg_t callback, uintptr_t arg);
    static uint32_t _to_ms(float seconds);

protected:
	esp_timer_handle_t _timer;
};

#endif // TICKER_H
