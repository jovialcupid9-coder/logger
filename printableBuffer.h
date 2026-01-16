// Forward declarations for helper templates
// These are needed because stringify_helper templates rely on array_dims_helper
// They must be in the same (global) namespace

// Base template: for non-array types, returns empty string for dimensions
template <typename T>
struct array_dims_helper {
  static String value() {
    return "";
  }
};

// Forward declarations
template <typename T, size_t N>
struct array_dims_helper<T[ N ]>;

template <size_t N>
String stringify_helper(const char(&arr)[ N ], bool topLevel = true);

template <size_t N>
String stringify_helper(const bool(&arr)[ N ], bool topLevel = true);

template <typename T, size_t N>
String stringify_helper(const T(&arr)[ N ], bool topLevel = true);

template <typename T>
String stringify_helper(const T& val, bool = true);

// String that accepts Printable
class printableBuffer : public Print, public Printable {
  public:
  // preallocates memory to reduce fragmentation, will grow if needed
  explicit printableBuffer(const unsigned int reserve) {
    buf.reserve(reserve);
  }

  void clear() {
    buf.clear();
  }

  // Override Print::write for single byte
  size_t write(uint8_t c) override {
    buf += (char) c;
    return 1;
  }

  // Overload for printing bool values
  size_t print(bool v) {
    return writeString(v ? "true" : "false");
  }

  // Overload for Printable objects
  size_t print(const Printable& p) {
    return p.printTo(*this);
  }

  // Generic template overload for any type
  template <typename T>
  size_t print(const T& v) {
    if constexpr(std::is_array_v<T>)
    {
      // Convert arrays to string representation
      buf += stringify_helper(v);
      return 0;
    }
    else
      return Print::print(v); // Fallback to Print base class for scalars
  }

  // It's not seen as String type, so casting is enabled
  String toString() {
    return buf;
  }

  // Implement Printable interface
  size_t printTo(Print& p) const override {
    return p.print(buf);
  }

  String buf; // internal string buffer

  private:
  // Helper to write C-string into buffer
  size_t writeString(const char* s) {
    size_t len = 0;
    while(*s)
      len += write(*s++);
    return len;
  }
};

// Partial specialization for arrays: recursively build dimension string
template <typename T, size_t N>
struct array_dims_helper<T[ N ]> {
  static String value() {
    return "[" + String(N) + "]" + array_dims_helper<T>::value();
  }
};

// Specialization for char arrays: just convert to String
// otherwise each char array (so basically every "text" would be interpreted as array)
template <size_t N>
String stringify_helper(const char(&arr)[ N ], bool topLevel) {
  return String(arr);
}

// Fallback for scalar types (non-arrays)
template <typename T>
String stringify_helper(const T& val, bool) {
  return String(val);
}

// Specialization for bool arrays: create string like [N] = {true, false, ...}
template <size_t N>
String stringify_helper(const bool(&arr)[ N ], bool topLevel) {
  String prefix;
  if(topLevel)
    prefix = "[" + String(N) + "] = ";
  String result = "{";
  for(size_t i = 0; i < N; ++i)
  {
    result += arr[ i ] ? "true" : "false";
    if(i < N - 1)
      result += ", ";
  }
  result += "}";
  return topLevel ? prefix + result : result;
}

// Generic array handler: recursively stringify arrays of any type
template <typename T, size_t N>
String stringify_helper(const T(&arr)[ N ], bool topLevel) {
  if constexpr(std::is_same_v<T, char>)
    return String(arr); // already handled above

  String prefix;
  if(topLevel)
    prefix = "arr" + array_dims_helper<T[ N ]>::value() + " = ";

  String result = "{";
  for(size_t i = 0; i < N; ++i)
  {
    if constexpr(std::is_array_v<T>)
      result += stringify_helper(arr[ i ], false); // nested array
    else if constexpr(std::is_same_v<T, bool>)
      result += arr[ i ] ? "true" : "false"; // bool element
    else
      result += String(arr[ i ]); // scalar element

    if(i < N - 1)
      result += ", ";
  }
  result += "}";
  return topLevel ? prefix + result : result;
}
