void Thread_1(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;)
  {
    while (thread_1_2_stop);

    unsigned long currentTime = millis();

    LoadCell_1a.update();  //Left Pad
    LoadCell_2a.update();
    LoadCell_3a.update();
    LoadCell_4a.update();

    LoadCell_1b.update();   //Right Pad
    LoadCell_2b.update();
    LoadCell_3b.update();
    LoadCell_4b.update();

    t_1a = LoadCell_1a.getData();
    t_2a = LoadCell_2a.getData();
    t_3a = LoadCell_3a.getData();
    t_4a = LoadCell_4a.getData();

    t_1b = LoadCell_1b.getData();
    t_2b = LoadCell_2b.getData();
    t_3b = LoadCell_3b.getData();
    t_4b = LoadCell_4b.getData();

    Smoother();

    if (tChange_1a != t_1a) {
      tChange_1a = t_1a;
      Change_1a = false;
    }
    if (tChange_2a != t_2a) {
      tChange_2a = t_2a;
      Change_2a = false;
    }
    if (tChange_3a != t_3a) {
      tChange_3a = t_3a;
      Change_3a = false;
    }
    if (tChange_4a != t_4a) {
      tChange_4a = t_4a;
      Change_4a = false;
    }
    if (tChange_1b != t_1b) {
      tChange_1b = t_1b;
      Change_1b = false;
    }
    if (tChange_2b != t_2b) {
      tChange_2b = t_2b;
      Change_2b = false;
    }
    if (tChange_3b != t_3b) {
      tChange_3b = t_3b;
      Change_3b = false;
    }
    if (tChange_4b != t_4b) {
      tChange_4b = t_4b;
      Change_4b = false;
    }

    double lsum = (sAverage_1a + sAverage_2a + sAverage_3a + sAverage_4a);
    double rsum = (sAverage_1b + sAverage_2b + sAverage_3b + sAverage_4b);

    lsum = abs(lsum);
    rsum = abs(rsum);

    uint16_t lx, ly, rx, ry;

    int rx_v2;


    lx = (sAverage_1a * lxa + sAverage_2a * lxb + sAverage_3a * lxc + sAverage_4a * lxd) / lsum;
    ly = (sAverage_1a * lya + sAverage_2a * lyb + sAverage_3a * lyc + sAverage_4a * lyd) / lsum;

    rx = (sAverage_1b * rxa + sAverage_2b * rxb + sAverage_3b * rxc + sAverage_4b * rxd) / rsum;
    ry = (sAverage_1b * rya + sAverage_2b * ryb + sAverage_3b * ryc + sAverage_4b * ryd) / rsum;

    lx = map(lx, 1, 99, 0, 110);
    ly = map(ly, 51, 146, 0, 170);

    rx = map(rx, 6, 108, 0, 110);
    ry = map(ry, 44, 154, 0, 170);

    rx_v2 = 110 + rx;

    lx = abs(lx);
    ly = abs(ly);
    rx = abs(rx);
    ry = abs(ry);
    rx_v2 = abs(rx_v2);

    // COP based foots pad
    xCOP = lx * (lsum / (lsum + rsum)) + (rx_v2 + dx_value_int) * (rsum / (lsum + rsum));
    yCOP = ly * (lsum / (lsum + rsum)) + ry * (rsum / (lsum + rsum));

    if (currentTime - prevTime > 150 && lx < 120 && rx < 120  && ly < 180 && ry < 180 && xCOP < 260) {
      Serial.print((String)lx + "/" + ly + "/" + rx + "/" + ry + "/" + xCOP + "/" + yCOP + "/" + "\n");
 
      temp_lx = lx;
      temp_ly = ly;
      temp_rx = rx;
      temp_ry = ry;

      xcop_global = temp_xCOP = xCOP;
      ycop_global = temp_yCOP = yCOP;
    }

    else if (currentTime - prevTime > 150) {
      Serial.print((String)temp_lx + "/" + temp_ly + "/" + temp_rx + "/" + temp_ry + "/" + temp_xCOP + "/" + temp_yCOP + "/" + "\n");

      xcop_global = temp_xCOP;
      ycop_global = temp_yCOP;
    }
  }
}
