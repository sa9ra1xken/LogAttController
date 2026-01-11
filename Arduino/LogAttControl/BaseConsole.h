#ifndef BASE_CONSOLE
#define BASE_CONSOLE

class BaseConsole {
  public:
    enum Mode {
      VOLUME,
      BALANCE
    };

    Mode mode;

    BaseConsole(){

    }
    
    void SayHello(){
      //gfx.setRotation(2);
      //gfx.clearDisplay();
      //gfx.println("issueing display command");
      //gfx.display();
      //gfx.setCursor(0, 0);
      //gfx.setTextSize(2);
      //gfx.print("Hello");
      //gfx.display();
    }

    virtual void knob_change(int delta)=0;
    virtual void button_pressed()=0;
    virtual void button_released(int duration_ms)=0;

  protected:
//    Adafruit_GFX& gfx;

  private:
    int asd;


};

#endif