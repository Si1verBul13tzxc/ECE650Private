#include "exerciser.h"

void exercise(connection * C) {
  query1(C, 1, 35, 40, 1, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1);
  query2(C, "Orange");
  query2(C, "DarkBlue");
  query2(C, "LightBlue");
  query2(C, "Maroon");
  query3(C, "Duke");
  query3(C, "UNC");
  query4(C, "NC", "DarkBlue");
  query4(C, "NC", "LightBlue");
  query5(C, 13);
}
