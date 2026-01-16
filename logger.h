#pragma once
#include <Arduino.h>
#include <esp_log.h>

#include <algorithm>
#include <type_traits>
#include <vector>

#include "printableBuffer.h"

#if __cplusplus < 201703L
#warning "Library should be compiled with C++17 or newer."
In platformio.ini set : ( works both with Arduino IDE and ESP - IDF )
build_unflags = -std = gnu++ 11 build_flags = -std = gnu++ 2a
#endif

#ifndef LOGGER_USE_PREFIX
// 1 = [filename.ext::funcName::line]
// 2 = [funcName::line]
// 3 = [fileName.ext::line]
// 4 = [fileName.ext::funcName]
//  no format added
#define LOGGER_USE_PREFIX 1
#endif

namespace logger {
  namespace detail {
    /////////////////////////////////////////////////////////////////////////
    // PRIVATE API, can be accesses by end user, but not recommended
    /////////////////////////////////////////////////////////////////////////

    /* __FILE__ returns full path to file, it's used to cut it into file.ext. Done on primitive string to make it to compilator, less use of run-time resources */
    static constexpr const char* stripPath(const char* path) // Strings are only run time
    {
      const char* p = path;

      for(const char* s = path; *s != '\0'; ++s)
        if(*s == '/' || *s == '\\')
          p = s + 1;

      return p;
    }

    // helper needed because folding doesn't allow it (?? check if true)
    template <typename T>
    void printArg(printableBuffer& buff, const T& arg) {
      buff.print(arg);
      buff.print(' ');
    }

    // Placeholder for all recivers
    inline std::vector<Print*> printers;

    // Implementation of the printing function, calling it directly omits using prefix. Macro log expands to it, needed to unfold __FILE__, __FUNCTION__, __LINE__ values
    template <typename... Args>
    void log_imp(const Args &...args) {
      static printableBuffer buff(1024);
      static SemaphoreHandle_t printMutex = NULL;

      if(!printMutex)
        printMutex = xSemaphoreCreateMutex();

      xSemaphoreTake(printMutex, portMAX_DELAY);

      ( printArg(buff, args), ... );
      for(auto p : printers)
        p->println(buff);

      buff.clear();

      xSemaphoreGive(printMutex);
    }
    /*  re-use of the system log level type, just because using another would be DRY, possible values
    typedef enum {
      ESP_LOG_NONE,   //!< No log output

      ESP_LOG_ERROR,  //!< Critical errors, software module can not recover on its own

      ESP_LOG_WARN,   //!< Error conditions from which recovery measures have been taken

      ESP_LOG_INFO,   //!< Information messages which describe normal flow of events

      ESP_LOG_DEBUG,  //!< Extra information which is not necessary for normal use  (values, pointers, sizes, etc).

      ESP_LOG_VERBOSE //!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output.
    } */

    // Stores non owning pointers. Inline allows header-only usage without multiple definitions
    inline esp_log_level_t level = ESP_LOG_INFO;
  } // namespace detail

  /////////////////////////////////////////////////////////////////////////
  // PUBLIC API
  /////////////////////////////////////////////////////////////////////////

  inline bool logInfo = true;

  /** @brief Remove arbitrary amount of Print objects (or pointers) from the internal list
   * @return false if asked for removal of non-existing one, true when all were removed
   */
  template <typename... Ts>
  inline bool removePrinter(Ts && ...args) {
    // Fold with &&: each lambda returns true only if that argument was successfully removed
    return ( true && ... && ( [ & ]
    {
      using T = std::decay_t<decltype( args )>;
      Print* p = std::is_pointer_v<T> ? args : std::addressof(args);

      if(!p)
        return false; // nullptr fails

      auto it = std::find(detail::printers.begin(), detail::printers.end(), p);

      if(it == detail::printers.end())
        return false; // not found

      detail::printers.erase(it);
      return true; // successfully removed
    }( ) ) );
  }

  /**
   * @brief Add arbitrary amount of Print objects (or pointers) to the list.
   *
   * @return true if **all printers were added, false if any were nullptr or duplicate
   * @details syntax: addPrinter(&Serial0)
   */
  template <typename... Ts>
  inline bool addPrinter(Ts && ...args) {
    // Fold with &&: each lambda returns true only if the argument was newly added
    return ( true && ... && ( [ & ]
    {
      using T = std::decay_t<decltype( args )>;
      Print* p;

      if constexpr(std::is_pointer_v<T>)
        p = args;                 // if pointer, use it directly
      else
        p = std::addressof(args); // if reference, get address

      if(!p)
        return false;

      for(auto x : detail::printers)
        if(x == p)
          return false;

      detail::printers.push_back(p);
      return true;
    }( ) ) );
  }

  inline const esp_log_level_t getLogLevel() {
    return detail::level;
  }

  inline void setLogLevel(const esp_log_level_t newlvl) {
    detail::level = newlvl;
  }

  // overload that automatically cast type
  inline void setLogLevel(const unsigned int lvl) {
    setLogLevel(constrain((esp_log_level_t) lvl, ESP_LOG_NONE, ESP_LOG_VERBOSE));
  }

  /** @brief makes board logs redirect thru logger. Makes sense only when using multiple Printers (like webSerial)*/
  inline void reroute_og_logs() {
    esp_log_set_vprintf([](const char* format, va_list args)
    {
      // calculate needed size
      va_list args_copy;
      va_copy(args_copy, args);
      int len = vsnprintf(nullptr, 0, format, args_copy);
      va_end(args_copy);

      if(len < 0)
        return 0;

      // Allocate exact buffer (+1 for null terminator)
      char* buffer = (char*) malloc(len + 1);
      if(!buffer)
        return 0;

      // Format into buffer
      vsnprintf(buffer, len + 1, format, args);

      detail::log_imp(buffer);

      free(buffer);

      return len; // it must return what og was returning. It's not important how
      // many bytes were printed but how much oryginally should be
    });
  }

} // namespace logger



#if LOGGER_USE_PREFIX == 0
#define log(...) logger::detail::log_imp(__VA_ARGS__)
#elif LOGGER_USE_PREFIX == 1
#define log(...) logger::detail::log_imp("[" + String(logger::detail::stripPath(__FILE__)) + "::" + String(__FUNCTION__) + "::" + String(__LINE__) + "] ", __VA_ARGS__)
#elif LOGGER_USE_PREFIX == 2
#define log(...) logger::detail::log_imp("[" + String(__FUNCTION__) + "::" + String(__LINE__) + "] ", __VA_ARGS__)
#elif LOGGER_USE_PREFIX == 3
#define log(...) logger::detail::log_imp("[" + (String)logger::detail::stripPath(__FILE__) + "::" + String(__LINE__) + "] ", __VA_ARGS__)
#elif LOGGER_USE_PREFIX == 4
#define log(...) logger::detail::log_imp("[" + (String)logger::detail::stripPath(__FILE__) + "::" + String(__FUNCTION__) + "] ", __VA_ARGS__)
#else
#warning "LOGGER_USE_PREFIX MUST BE SET TO 0-4 RANGE"
#endif

#define log_info(...) if (logger::logInfo) log("[I]", __VA_ARGS__)