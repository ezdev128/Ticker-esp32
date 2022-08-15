/* 

  2022-08.14 Added multiticker functionality

  -------------------------------------------------
  Code is based on:
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

#ifndef __ESP332_TICKER_H__
#define __ESP332_TICKER_H__

#include <vector>

#ifndef TARG
#define TARG template<typename arg_t>
#endif
#ifndef UN
#define UN __attribute__((unused))
#endif

extern "C" {
	#include "esp_timer.h"
}

class Ticker
{
public:
	typedef void (*callback_t)();
	typedef void (*callback_with_arg_t)(void*);
    struct ticker_task_object {
        callback_with_arg_t callback;
        bool repeatable;
        uint32_t milliseconds;
    };

    explicit Ticker(bool = false);
    ~Ticker();

    UN void enable_multiticker();

    UN void attach(float seconds, callback_t callback);
    TARG UN void attach(float seconds, void (*callback)(arg_t), arg_t arg);

    UN void attach_ms(uint32_t milliseconds, callback_t callback);
    TARG UN void attach_ms(uint32_t milliseconds, void (*callback)(arg_t), arg_t arg);

    UN void once(float seconds, callback_t callback);
    TARG UN void once(float seconds, void (*callback)(arg_t), arg_t arg);

    UN void once_ms(uint32_t milliseconds, callback_t callback);
    TARG UN void once_ms(uint32_t milliseconds, void (*callback)(arg_t), arg_t arg);

    UN bool detach();
    UN bool detach(Ticker::callback_t);

    UN bool active();
    UN bool active(Ticker::callback_t);

    UN int get_tickers_count();
    UN std::vector<ticker_task_object *> get_tickers();

protected:
    struct ticker_object_t {
        esp_timer_create_args_t timer_config;
        esp_timer_handle_t timer_handle;
        bool repeatable;
        uint32_t milliseconds;
    };

protected:
    esp_timer_handle_t _timer_handle = {nullptr};
    void _attach_ms(uint32_t milliseconds, bool repeat, callback_with_arg_t callback, uintptr_t arg);
    UN bool detach(Ticker::ticker_object_t);
    UN static bool detach(esp_timer_handle_t &);
    UN bool active(esp_timer_handle_t);


private:
    bool is_multi_ticker = false;
    std::vector<ticker_object_t> tickers_objects = {};
    static uint32_t to_ms(float seconds);

    int get_ticker_object_index(ticker_object_t);
    ticker_object_t find_ticker_object(Ticker::ticker_object_t);
    ticker_object_t find_ticker_object(Ticker::callback_with_arg_t);
    ticker_object_t find_ticker_object(Ticker::callback_t);

    UN bool has_ticker_object(Ticker::ticker_object_t);
    UN bool has_ticker_object(Ticker::callback_with_arg_t);
    UN bool has_ticker_object(Ticker::callback_t);

};

#endif // __ESP332_TICKER_H__
