void Thread_2(void *pvParameters) //This is a task.
{
  (void) pvParameters;

  char inByte;

  for (;;)
  {
    loopCount++;
    if ( (millis() - startTime) > 3000 ) {
      startTime = millis();
      loopCount = 0;
    }

    // Fills kpd.key[ ] array with up-to 10 active keys.
    // Returns true if there are ANY active keys.
    if (kpd.getKeys())
    {
      for (int i = 0; i < LIST_MAX; i++)  //Scan the whole key list.
      {
        if ( kpd.key[i].stateChanged )    //Only find keys that have changed state.
        {
          switch (kpd.key[i].kstate) {   //Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              msg = " PRESSED";
              break;
            case HOLD:
              msg = " HOLD";
              break;
            case RELEASED:
              msg = " RELEASED";
              break;
            case IDLE:
              msg = " IDLE";
          }

          char my_keypad_temp = kpd.key[i].kchar;
          value_keypad = kpd.key[i].kchar;

          if (msg == " HOLD" && (my_keypad_temp != 'A' && my_keypad_temp != 'B' && my_keypad_temp != 'C' 
          && my_keypad_temp != 'D' && my_keypad_temp != '*' && my_keypad_temp != '#'))
            keypad_temp = my_keypad_temp;

          delay(50);

        }
      }
    }

    if (Serial.available())
    {
     String(inByte) = Serial.readString();
  
      if (inByte == 't') {
        tare_ = true;
      }
    }

    if (tare_ == true)
    {
      tare_ = false;
      value_keypad = "Tare";
      LoadCell_1a.tareNoDelay();
      LoadCell_2a.tareNoDelay();
      LoadCell_3a.tareNoDelay();
      LoadCell_4a.tareNoDelay();
      LoadCell_1b.tareNoDelay();
      LoadCell_2b.tareNoDelay();
      LoadCell_3b.tareNoDelay();
      LoadCell_4b.tareNoDelay();
    }
  }
}
