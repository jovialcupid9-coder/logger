# logger
Modern esp32 print implementation. 

Accepts arbitrary amount of args of any Printable type(mixed with one another), multidimensional arrays, correctly handles bool types, 

```cpp
String food[ 2 ][ 3 ] = { {"apple", "banana", "tomato"}, {"paprika", "onion", "garlic"} };
log("food equals to", food);

// [main.cpp::loop::16] food equals to arr[2][3] = {{apple, banana, tomato}, {paprika, onion, garlic}}
```

Adds prefix with file/function name/line. 
```
// 0 // no prefix
// 1 = [filename.ext::funcName::line]  <- default
// 2 = [funcName::line]
// 3 = [fileName.ext::line]
// 4 = [fileName.ext::funcName]
Call #define LOGGER_USE_PREFIX == x before including library
```

 Thread safe, data is put into buffer that grows in need, recommended to keep default buffer to prevent fragmentantion caused constant reallocation. 
```cpp
Call #define LOGGER_USE_PREFIX == x before including library, to change default 1024
 // #define LOGGER_REVERSE_BUFFER_SIZE 200 // d
```

 Part of esp_logs of things on lower level can be rerouted to logger, that enables reading them in webserial, etc
 ```cpp
 logger:reroute_og_logs();
```

// Supports log levels, reused system one esp_log_level_t, default is ESP_LOG_INFO
```cpp
typedef enum {
    ESP_LOG_NONE,       /*! = 0 < No log output */
    ESP_LOG_ERROR,      /*! = 1< Critical errors, software module can not recover on its own */
    ESP_LOG_WARN,       /*! = 2< Error conditions from which recovery measures have been taken */
    ESP_LOG_INFO,       /*! = 3 (< Information messages which describe normal flow of events */ 
    ESP_LOG_DEBUG,      /*! = 4< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    ESP_LOG_VERBOSE     /*! = 5< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} esp_log_level_t;
```

Doesn't support c-string char arrays, otherwise most text will be formatted as array


