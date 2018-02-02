
#include <GSM.h>
#include <SPI.h>
#include<string.h>
//#include <Keypad.h>
// Keypad - Version: Latest 
#include <Keypad.h>
#include <MFRC522.h>
#include <Servo.h>
//GSM
#define PINNUMBER ""
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN); 

//keypad
const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad
//keymap defines the key pressed according to the row and columns just as appears on the keypad
char keymap[numRows][numCols]=
{
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};
//Code that shows the the keypad connections to the arduino terminals
byte rowPins[numRows] = {9,8,7,6}; //Rows 0 to 3
byte colPins[numCols]= {5,4,3,2}; //Columns 0 to 3
//initializes an instance of the Keypad class
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

//servo motor
Servo myservo;
int pos = 0;

//GSM
// initialize the library instance
GSM gsmAccess;
GSM_SMS sms;
// Array to hold the number a SMS is retreived from
char senderNumber[20];



void setup() {
 
 //GSM
 Serial.begin(9600);
 while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  

  // connection state
  boolean notConnected = true;

  // Start GSM connection
  while (notConnected) {
    if (gsmAccess.begin(PINNUMBER) == GSM_READY) {
      notConnected = false;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }
  Serial.println("GSM initialized");
  Serial.println("Waiting for messages");
  
  randomSeed(analogRead(2));
  
   //rfid, keypad
 // Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  
   
 //servo motor
  myservo.attach(1);  // attaches the servo on pin 9 to the servo object
  
}
 

void loop() {
    
     //GSM recieve  
  char c;
  int count=0;
  //char senderNumber[10];
  char remoteNumber1[20] = {'+','9','1','9','8','2','3','2','6','6','9','3','6'};

  // If there are any SMSs available()
  if (sms.available()) 
  {
    
    // Get remote number
    sms.remoteNumber(senderNumber, 20);
    Serial.println(senderNumber);
    while (c = sms.read()) {
      Serial.print(c);
    }
    Serial.println();
    sms.flush();
    for(int i=0;i<=12;i++)
    {
      if(senderNumber[i]==remoteNumber1[i])
      {
        count++;
      }
      else
      {
        count--;  
      }
      
    }
    if(count==13)
    {
      Serial.println("Authorized Mobile Number");
      String str="0123456789ABCD";
      //int n = str.length(); 
      // String to hold my OTP
      char OTP[5];
 
      for (int i=0; i<5; i++)
      {
        OTP[i]=(str[random() % 14]);
      }
      char msg[16]={'T','h','e','_','O','T','P','_','i','s',':'};
      for(int i=0,j=11;i<=4,j<=15;i++,j++)
      {
        msg[j]=OTP[i];
      }
      sms.beginSMS(senderNumber);
      sms.print(msg);
      sms.endSMS();
      Serial.println("SENT OTP");
      int k=0;
      while(k<=3)
      {
        Serial.println("Approximate your card to the reader...");
        Serial.println();
        //rfid 
        // Look for new cards
        while ( ! mfrc522.PICC_IsNewCardPresent()) 
        {
          delay(10);
        }
        // Select one of the cards
        if ( ! mfrc522.PICC_ReadCardSerial()) 
        {
          return;
        }
        
        //Show UID on serial monitor
        Serial.print("UID tag :");
        String content= "";
        byte letter;
        for (byte i = 0; i < mfrc522.uid.size; i++) 
        {
          Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(mfrc522.uid.uidByte[i], HEX);
          content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
          content.concat(String(mfrc522.uid.uidByte[i], HEX));
        }
        //Serial.println();
        Serial.println("Message : ");
        content.toUpperCase();
        if (content.substring(1) == "B0 93 17 83") //B0 93 17 83
        {
          Serial.println("Authorized access");
          Serial.println();
          //keypad10FE FB 73
          int check=0;
          int n=0;
          while(n<=3)
          {
            check=keypad(check,OTP);   
            if(check==5)
            {
              //servo motor
              for (pos = 0; pos <= 90; pos += 1)  // goes from 0 degrees to 90 degrees in steps of 1 degree
              {  
                myservo.write(pos);              // tell servo to go to position in variable 'pos'
                delay(15);                       // waits 15ms for the servo to reach the position
              }
              Serial.println("If finished work, close the door and press '#' to lock");
              char keypressed=myKeypad.getKey(); 
              if (keypressed !='#')
              {
                Serial.print(keypressed);
                for (pos = 90; pos >= 0; pos -= 1)  // goes from 90 degrees to 0 degrees
                {
                  myservo.write(pos);              // tell servo to go to position in variable 'pos'
                  delay(15);                       // waits 15ms for the servo to reach the position
                }
              }
            }
            else
            {
              Serial.println("Wrong Password entered, enter OTP again");
              n++;
            }
          }
        }
        else  
        {
          Serial.println(" Access denied");
          delay(3000);
          k++;
         // break;
        }
      }
    }
    else
    {
      Serial.println("Sorry this is not the authenticated user's number!");
    }
  }
}

int keypad(int check,char OTP[])
{
  char pswd[6];
  Serial.print("OTP:");
  for(int i=0;i<=5;i++)
  {
    char keypressed=myKeypad.getKey(); 
    if (keypressed !='#')
    {
      Serial.print(keypressed);
      pswd[i]=keypressed;
    }
    else
    {
      for(int i=0;i<=5;i++)
      {
        if(pswd[i]==OTP[i])
        {
          check++;
        }
       /* else
        {
          Serial.println("Wrong Password entered");
          n++;
          // break;
        }*/
      }
    }  
  }
  return(check);
}
