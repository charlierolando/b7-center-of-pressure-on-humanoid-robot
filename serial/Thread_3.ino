void Thread_3(void *pvParameters)   //This is a task.
{
  (void) pvParameters;

reset_to_mode_1:
  String dx_value = "";
  dx_value_int = 0;
  thread_1_2_stop = true;

  while (1)  //mode 1
  {
    if (value_keypad == "A")
    {
      value_keypad = "Reset LCD";

      //initialize the lcd
      digitalWrite(11, LOW);
      delay(100);
      digitalWrite(11, HIGH);

      lcd.init();

      //Print a message to the LCD.
      lcd.backlight();
    }

    if (value_keypad == "D")
    {
      lcd.setCursor(0, 0);
      lcd.print("                          ");
      lcd.setCursor(0, 1);
      lcd.print("                          ");
      lcd.setCursor(0, 2);
      lcd.print("                          ");
      lcd.setCursor(0, 3);
      lcd.print("                          ");
      keypad_temp = "";
      value_keypad = "Enter";
      dx_value_int = dx_value.toInt();

      break;
    }

    if (value_keypad == "C")
    {
      lcd.setCursor(0, 0);
      lcd.print("                          ");
      lcd.setCursor(0, 1);
      lcd.print("                          ");
      lcd.setCursor(0, 2);
      lcd.print("                          ");
      lcd.setCursor(0, 3);
      lcd.print("                          ");
      keypad_temp = "";
      value_keypad = "Clear";

      goto reset_to_mode_1;
    }

    lcd.setCursor(0, 0);
    lcd.print("(D enter, C clear)");
    lcd.setCursor(0, 1);
    lcd.print("Input dx(mm):");
    lcd.setCursor(0, 2);

    dx_value = dx_value + keypad_temp;
    keypad_temp = "";

    lcd.print(dx_value);

    lcd.setCursor(0, 3);
    if (value_keypad != "")
      lcd.print("Key ");
    lcd.print(value_keypad);
    lcd.print(msg);
    lcd.print("                     ");

    delay(100);
  }

  lcd.setCursor(0, 0);
  lcd.print("Input dx(mm):");
  lcd.setCursor(0, 1);
  lcd.print(dx_value_int);
  lcd.setCursor(0, 2);
  lcd.print("(D enter, C back)");

  while (1)  //mode 2
  {
    if (value_keypad == "A")
    {
      value_keypad = "Reset LCD";

      //initialize the lcd
      digitalWrite(11, LOW);
      delay(100);
      digitalWrite(11, HIGH);

      lcd.init();

      //Print a message to the LCD.
      lcd.backlight();
      lcd.setCursor(0, 1);
      lcd.print(dx_value_int);
      lcd.setCursor(0, 2);
      lcd.print("(D enter, C back)");
    }

    if (value_keypad == "D")
    {
      lcd.setCursor(0, 0);
      lcd.print("                          ");
      lcd.setCursor(0, 1);
      lcd.print("                          ");
      lcd.setCursor(0, 2);
      lcd.print("                          ");
      lcd.setCursor(0, 3);
      lcd.print("                          ");
      keypad_temp = "";
      value_keypad = "Enter";

      break;
    }

    if (value_keypad == "C")
    {
      lcd.setCursor(0, 0);
      lcd.print("                          ");
      lcd.setCursor(0, 1);
      lcd.print("                          ");
      lcd.setCursor(0, 2);
      lcd.print("                          ");
      lcd.setCursor(0, 3);
      lcd.print("                          ");
      keypad_temp = "";
      value_keypad = "Back";

      goto reset_to_mode_1;
    }

    lcd.setCursor(0, 0);
    lcd.print("Input dx(mm):");
    lcd.setCursor(0, 3);
    if (value_keypad != "")
      lcd.print("Key ");
    lcd.print(value_keypad);
    lcd.print(msg);
    lcd.print("                     ");

    delay(100);
  }

  lcd.setCursor(0, 1);
  lcd.print("dx = ");
  lcd.print(dx_value_int);
  lcd.print(" mm");
  lcd.setCursor(0, 2);
  lcd.print("(C back, A rst lcd)");

  while (1)  //mode 3
  {
    if (value_keypad == "C")
    {
      lcd.setCursor(0, 0);
      lcd.print("                          ");
      lcd.setCursor(0, 1);
      lcd.print("                          ");
      lcd.setCursor(0, 2);
      lcd.print("                          ");
      lcd.setCursor(0, 3);
      lcd.print("                          ");
      keypad_temp = "";
      value_keypad = "Back";

      goto reset_to_mode_1;
    }

    thread_1_2_stop = false;

    lcd.setCursor(0, 0);
    lcd.print("X: ");
    lcd.print(xcop_global);
    lcd.setCursor(9, 0);
    lcd.print("Y: ");
    lcd.print(ycop_global);
    lcd.print("    ");
    lcd.setCursor(0, 3);
    if (value_keypad != "")
      lcd.print("Key ");
    lcd.print(value_keypad);
    lcd.print(msg);
    lcd.print("                     ");

    delay(100);

    if (value_keypad == "A")
    {
      value_keypad = "Reset LCD";

      //initialize the lcd
      digitalWrite(11, LOW);
      delay(100);
      digitalWrite(11, HIGH);

      lcd.init();

      //Print a message to the LCD.
      lcd.backlight();

      lcd.setCursor(0, 1);
      lcd.print("dx = ");
      lcd.print(dx_value_int);
      lcd.print(" mm");
      lcd.setCursor(0, 2);
      lcd.print("(C back, A rst lcd)");
    }

    if (value_keypad == "B")
    {
      value_keypad = "Tare";
      tare_ = true;
    }
  }
}
