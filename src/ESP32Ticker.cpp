#include <string>
#include <esp32-hal-log.h>
#include "ESP32Ticker.h"

static const char *tag = "ESP32Ticker";

Ticker::Ticker(bool useMultiTicker) {
    is_multi_ticker = useMultiTicker;
    if (esp_timer_init() != ESP_OK) {
        ESP_LOGD(tag, "esp_timer_init() failed");
    }
}

Ticker::~Ticker() {
  detach();
}

UN void Ticker::enable_multiticker() {
    is_multi_ticker = true;
}

UN void Ticker::_attach_ms(uint32_t milliseconds, bool repeat, callback_with_arg_t callback, uintptr_t arg) {
    std::string tickerName = tag;
    if (is_multi_ticker) {
        tickerName += "." + std::to_string((unsigned long) (esp_timer_get_time() / 1000ULL));
    }

    auto * to = new ticker_object_t{
            .timer_config = {
                    .callback = callback,
                    .arg = reinterpret_cast<void *>(arg),
                    .dispatch_method = ESP_TIMER_TASK,
                    .name = tickerName.c_str(),
            },
            .timer_handle = is_multi_ticker ? nullptr : _timer_handle,
            .repeatable = repeat,
            .milliseconds = milliseconds,
    };

    detach(to->timer_handle);
    if (esp_timer_create(&to->timer_config, &to->timer_handle) != ESP_OK) {
        ESP_LOGD(tag, "esp_timer_create() failed");
    }

    if (repeat) {
        if (esp_timer_start_periodic(to->timer_handle, milliseconds * 1000) != ESP_OK) {
            ESP_LOGD(tag, "esp_timer_start_periodic() failed");
        }
    } else {
        if (esp_timer_start_once(to->timer_handle, milliseconds * 1000) != ESP_OK) {
            ESP_LOGD(tag, "esp_timer_start_once() failed");
        }
    }

    if (!is_multi_ticker) {
        _timer_handle = to->timer_handle;
        tickers_objects.clear();
    }

    tickers_objects.insert(tickers_objects.end(), *to);
}

uint32_t Ticker::to_ms(float seconds) {
    return (uint32_t)(seconds * 1000);
}

UN void Ticker::attach(float seconds, Ticker::callback_t callback) {
    _attach_ms(to_ms(seconds), true, reinterpret_cast<callback_with_arg_t>(callback), 0);
}

TARG UN void Ticker::attach(float seconds, void (*callback)(arg_t), arg_t arg) {
    static_assert(sizeof(arg_t) <= sizeof(uintptr_t), "attach() callback argument size must be <= 32 bytes");
    _attach_ms(to_ms(seconds), true, reinterpret_cast<callback_with_arg_t>(callback), (uintptr_t)arg);
}

UN void Ticker::attach_ms(uint32_t milliseconds, Ticker::callback_t callback) {
    _attach_ms(milliseconds, true, reinterpret_cast<callback_with_arg_t>(callback), 0);
}

TARG UN void Ticker::attach_ms(uint32_t milliseconds, void (*callback)(arg_t), arg_t arg) {
    static_assert(sizeof(arg_t) <= sizeof(uintptr_t), "attach_ms() callback argument size must be <= 32 bytes");
    _attach_ms(milliseconds, true, reinterpret_cast<callback_with_arg_t>(callback), (uintptr_t)arg);
}

UN void Ticker::once(float seconds, Ticker::callback_t callback) {
    _attach_ms(to_ms(seconds), false, reinterpret_cast<callback_with_arg_t>(callback), 0);
}

UN void Ticker::once_ms(uint32_t milliseconds, Ticker::callback_t callback) {
    _attach_ms(milliseconds, false, reinterpret_cast<callback_with_arg_t>(callback), 0);
}

TARG UN void Ticker::once(float seconds, void (*callback)(arg_t), arg_t arg) {
    static_assert(sizeof(arg_t) <= sizeof(uintptr_t), "attach() callback argument size must be <= 32 bytes");
    _attach_ms(to_ms(seconds), false, reinterpret_cast<callback_with_arg_t>(callback), (uintptr_t)(arg));
}

TARG UN void Ticker::once_ms(uint32_t milliseconds, void (*callback)(arg_t), arg_t arg) {
    static_assert(sizeof(arg_t) <= sizeof(uintptr_t), "attach_ms() callback argument size must be <= 32 bytes");
    _attach_ms(milliseconds, false, reinterpret_cast<callback_with_arg_t>(callback), (uintptr_t)(arg));
}

UN bool Ticker::detach() {
    if (is_multi_ticker) {
        for (auto t : tickers_objects) {
            if (!detach(t)) {
                return false;
            }
        }
        return true;
    }
    return detach(_timer_handle);
}

UN bool Ticker::detach(esp_timer_handle_t &timer_handle) {
    if (esp_timer_stop(timer_handle) == ESP_ERR_INVALID_STATE
        || esp_timer_delete(timer_handle) == ESP_ERR_INVALID_STATE) {
        return false;
    }
    timer_handle = nullptr;
    return true;
}

UN bool Ticker::detach(Ticker::ticker_object_t ticker_object) {
    if (ticker_object.timer_handle == nullptr) {
        return false;
    }

    detach(ticker_object.timer_handle);
    auto tickerIdx = get_ticker_object_index(ticker_object);

    if (tickerIdx < 0) {
        return false;
    }

    tickers_objects.erase(tickers_objects.begin() + tickerIdx);
    return true;
}

UN bool Ticker::detach(Ticker::callback_t callback) {
    return detach(find_ticker_object(callback));
}

UN bool Ticker::active() {
    return is_multi_ticker ? !tickers_objects.empty() : _timer_handle != nullptr && esp_timer_is_active(_timer_handle);
}

UN bool Ticker::active(esp_timer_handle_t timer_handle) {
    if (is_multi_ticker) {
        if (timer_handle == nullptr) {
            return false;
        }
        return esp_timer_is_active(timer_handle);
    } else {
        if (_timer_handle == nullptr) {
            return false;
        }
        return esp_timer_is_active(_timer_handle);
    }
}

UN bool Ticker::active(Ticker::callback_t callback) {
    if (is_multi_ticker) {
        Ticker::ticker_object_t tickerObject = find_ticker_object(callback);
        if (tickerObject.timer_handle == nullptr) {
            return false;
        }
        return esp_timer_is_active(tickerObject.timer_handle);
    }
    return esp_timer_is_active(_timer_handle);
}

UN int Ticker::get_ticker_object_index(Ticker::ticker_object_t ticker_object) {
    int i=0;
    for (auto t : tickers_objects) {
        if (t.timer_handle == nullptr) {
            i++;
            continue;
        }

        if (t.timer_config.callback == ticker_object.timer_config.callback) {
            return i;
        }
        i++;
    }
    return -1;
}

UN Ticker::ticker_object_t Ticker::find_ticker_object(Ticker::ticker_object_t ticker_object) {
    if (tickers_objects.empty() || ticker_object.timer_handle == nullptr){
        return ticker_object_t{};
    }

    for (auto t : tickers_objects) {
        if (t.timer_handle == nullptr) {
            continue;
        }

        if (t.timer_handle == ticker_object.timer_handle) {
            return ticker_object;
        }
    }

    return ticker_object_t{};
}

UN Ticker::ticker_object_t Ticker::find_ticker_object(Ticker::callback_with_arg_t callback) {
    static Ticker::ticker_object_t ticker_object;

    if (tickers_objects.empty()){
        return ticker_object_t{};
    }

    for (auto t : tickers_objects) {
        if (t.timer_handle == nullptr) {
            continue;
        }

        if (t.timer_config.callback == *callback) {
            ticker_object = t;
            break;
        }
    }

    return ticker_object;
}


UN Ticker::ticker_object_t Ticker::find_ticker_object(Ticker::callback_t callback) {
    return find_ticker_object(reinterpret_cast<Ticker::callback_with_arg_t>(callback));
}


UN bool Ticker::has_ticker_object(Ticker::ticker_object_t ticker_object) {
    return find_ticker_object(ticker_object).timer_handle != nullptr;
}

UN bool Ticker::has_ticker_object(Ticker::callback_with_arg_t callback) {
    auto ticker_object = find_ticker_object(callback);
    return ticker_object.timer_handle != nullptr
        && ticker_object.timer_config.callback == callback;
}

UN bool Ticker::has_ticker_object(Ticker::callback_t callback) {
    return has_ticker_object(reinterpret_cast<Ticker::callback_with_arg_t>(callback));
}

UN int Ticker::get_tickers_count() {
    if (!is_multi_ticker) {
        return active(_timer_handle) ? 1 : 0;
    }

    int cnt = 0;
    for (auto t : tickers_objects) {
        if (t.timer_handle == nullptr) {
            continue;
        }
        cnt++;
    }
    return cnt;
}

std::vector<Ticker::ticker_task_object *> Ticker::get_tickers() {
    std::vector<Ticker::ticker_task_object *> ticker_objects;

    for (auto t : tickers_objects) {
        if (t.timer_handle == nullptr) {
            continue;
        }

        ticker_objects.insert(ticker_objects.end(),
                              new ticker_task_object{
                                  .callback = t.timer_config.callback,
                                  .repeatable = t.repeatable,
                                  .milliseconds = t.milliseconds,
                              });
    }
    return ticker_objects;
}
