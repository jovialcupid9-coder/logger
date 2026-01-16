#include "Arduino.h"
#include "logger.h"

void setup() {
  Serial.begin(115200);
  logger::addPrinter(&Serial);

  // mix any type that is printable
  const char* text = "hello";
  log(text, "world", String("..."), int(3), float(3.14));
  //  hello world ... 3 3.14

  // print multi dymensional arrays
  String food[ 2 ][ 3 ] = { {"apple", "banana", "tomato"}, {"paprika", "onion", "garlic"} };
  log("food equals to", food);
  // food equals to arr[2][3] = {{apple, banana, tomato}, {paprika, onion, garlic}}

// correctly prints bool - it'll be helpful working with arduino json 7^
  bool myBool = false;
  int myInt = 0;
  log("myBool is", myBool, "and myInt is", myInt);
  //  bool is false and myInt is 0

  // it also works for arrays of bool
  bool boolArr[] = { false, true, false };
  log(boolArr);
  // [3] = {false, true, false}
}

void loop() {
}       