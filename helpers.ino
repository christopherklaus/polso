void printDisplay(String firstLine, String secondLine) {
  display.gotoXY(0, 0);
  display.print(firstLine);
  display.print("                      ");
  display.gotoXY(0, 1);
  display.print(secondLine);
  display.print("                      ");
}