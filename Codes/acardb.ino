#include "Cmd.h"


// July 6 2017
// bad flash chip sample found, symptom : 
// write to one address result two or more place being written in the same chip.
// so, this is good for inclusion in wrtest algo.

// Address : 0000000-3FFFFFF
// PF0..PF7 = A0..A7
// PK0..PK7 = A8..A15
// PA0..PA7 = A16..A23
// PC0 = A24
// PC1 = A25

// Data (16bit): 
// PL0..PL7 = D0..D7
// PB0..PB3 = D8..D11
// PB4..PB7 = D12..D15

// FRONTS addr range
// 1F/5F - 0         ... 3F FF FF
// 2F/6F - 40 00 00  ... 7F FF FF
// 3F/7F - 80 00 00  ... BF FF FF
// 4F/8F - C0 00 00  ... FF FF FF

// BACKS addr range
// 1B/5B - 1 00 00 00  ... 1 3F FF FF
// 2B/6B - 1 40 00 00  ... 1 7F FF FF
// 3B/7B - 1 80 00 00  ... 1 BF FF FF
// 4B/8B - 1 C0 00 00  ... 1 FF FF FF

// nand CS routes
// U2.1 - 7B
// U2.2 - 5B
// U2.3 - 8B
// U2.4 - 6B
// U2.5 - 1B
// U2.6 - 3B
// U2.7 - 2B
// U2.8 - 4B
// U2.53 - 3F
// U2.54 - 1F
// U2.55 - 4F
// U2.56 - 2F
// U2.61 - 5F
// U2.62 - 7F
// U2.63 - 6F
// U2.64 - 8F

// FRONTs even addr -> 1F downward
//        odd  addr -> 5F downward
// BACKs  even addr -> 5B downward
//        odd  addr -> 1B downward

//    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//    @            BACK SIDE            @
//    @  =           =                  @
//    @  =Unpopulated=                  @
//    @  =           =                  @
//    @                                 @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @  @    1 B    @   @    5 B    @  @
//    @  @    odd    @   @   even    @  @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @                                 @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @  @    2 B    @   @    6 B    @  @
//    @  @           @   @           @  @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @                                 @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @  @    3 B    @   @    7 B    @  @
//    @  @           @   @           @  @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @                                 @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @  @    4 B    @   @    8 B    @  @
//    @  @           @   @           @  @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @                                 @
//    @    @@@@@@@                      @
//    @    @  WP @                      @
//    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//          @@@
//  
//    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//    @    @@@@@  FRONT SIDE            @
//    @    @   @        @@@@@@  @@@@@@  @
//    @    @@@@@        @    @  @    @  @
//    @  @@@@@@@@@@     @ U1 @  @ U2 @  @
//    @  @AT28C16 @     @    @  @    @  @
//    @  @  ATTR  @     @@@@@@  @@@@@@  @
//    @  @@@@@@@@@@                     @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @  @    1 F    @   @    5 F    @  @
//    @  @   even    @   @    odd    @  @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @                                 @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @  @    2 F    @   @    6 F    @  @
//    @  @           @   @           @  @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @                                 @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @  @    3 F    @   @    7 F    @  @
//    @  @           @   @           @  @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @                                 @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @  @    4 F    @   @    8 F    @  @
//    @  @           @   @           @  @
//    @  @@@@@@@@@@@@@   @@@@@@@@@@@@@  @
//    @                                 @
//    @                       @@@@@@@   @
//    @                       @  WP @   @
//    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//                               @@@


const int WP = 7; //PH4
const int VS1 = 6; //PH3
const int VS2 = 5; //PE3
const int WAIT = 4; //PG5
const int REG = 3; //PE5

const int CD2 = 14; //PJ1
const int CD1 = 15; //PJ0
const int CRST = 16; //PH1
const int RB = 17; //PH0
const int CE2 = 18; //PD3
const int CE1 = 19; //PD2
const int WE = 20; //PD1
const int OE = 21; //PD0

enum cardaccessmode { BYTEmode, WORDmode, NS };

cardaccessmode accessmode = NS;

unsigned int temp[16];
byte tempc;
uint32_t chksm;

void setup() {
  // put your setup code here, to run once:
  randomSeed(analogRead(0));
  
  pinMode(WP, INPUT);
  pinMode(VS1, INPUT);
  pinMode(VS2, INPUT);
  pinMode(WAIT, INPUT);
  pinMode(CD1, INPUT_PULLUP);
  pinMode(CD2, INPUT_PULLUP);
  pinMode(RB, INPUT);

  pinMode(REG, OUTPUT);
  pinMode(CRST, OUTPUT);
  pinMode(CE1, OUTPUT);
  pinMode(CE2, OUTPUT);
  pinMode(WE, OUTPUT);  
  pinMode(OE, OUTPUT);
  digitalWrite(REG, HIGH);
  digitalWrite(CRST, HIGH);
  digitalWrite(CE1, HIGH);
  digitalWrite(CE2, HIGH);
  digitalWrite(WE, HIGH);
  digitalWrite(OE, HIGH);
  digitalWrite(CRST, LOW);

  DDRF = 0xFF; // set addr pins as output
  DDRK = 0xFF;
  DDRA = 0xFF;
  DDRC = DDRC | B00000011;
  PORTF = 0;  // set output low
  PORTK = 0;
  PORTA = 0;
  PORTC = 0;

  DDRL = 0; // set data pins as input
  DDRB = 0;
  PORTL = 0xFF; //enable pullups
  PORTB = 0xFF; 

  cmdInit(115200);
  cmdAddDefault("help", cmd_help);
  cmdAdd("attr", cmd_attr);
  cmdAdd("readid", cmd_readid);
  cmdAdd("comn", cmd_comn);
  cmdAdd("blankchk", cmd_blankchk);
  cmdAdd("chiperase", cmd_chiperase);
  cmdAdd("wprog", cmd_wprog);
  //cmdAdd("wrtest", cmd_wrtest);
  //cmdAdd("secterase", cmd_secterase);
  Serial.print("Cardbus Shield v1.0 ready...");  
  cmdPrompt();
}

void loop() {
  // put your main code here, to run repeatedly: 
  cmdPoll();
}

void cmd_help(int arg_cnt, char **args)
{
  Serial.println("Cardbus Shield v1.0");
  Serial.println("usage :");
  Serial.println("help\tshow this help");
  Serial.println("attr\tdump attribute memory");
  Serial.println("readid\tdump id of all chip");
  Serial.println("comn x y\tdump common memory from address y with mode x");
  Serial.println("blankchk x[all]\tblank check of chip x.");
  Serial.println("chiperase x[all]\terase chip x");
  Serial.println("wprog x y\tbyte program addr x with val y.");
}

void cmd_chiperase(int arg_cnt, char **args)
{
  unsigned long start, timeout = 50000, now, elapse;
  unsigned int waitc;
  
  if (!cardready()) return;  

  int tempmin, tempmax;
  uint32_t temp = 0;
  if (arg_cnt < 2) {Serial.println("not enough parameter."); return;}
  if (!strcmp(args[1],"all")) {tempmin = 1; tempmax = 17;}
    else 
    {
      temp = cmdStr2Num(args[1], 10);
      if ((temp < 1) || (temp > 16)) {Serial.println("chip index should be 1..16"); return;}
      tempmin = temp; tempmax = temp + 1;
    }
    
  for (int x = tempmin; x < tempmax; x++)
  {
  temp = x;

  if (temp%2 ==1) {--temp; temp = temp/2; temp = temp * 0x400000; temp++;} else {temp = temp/2; temp--; temp = temp * 0x400000;}
  Serial.print("Erasing "); Serial.println(temp, HEX);

  writeAddr(temp, 0xAA, BYTEmode); 
  writeAddr(temp, 0x55, BYTEmode);  
  writeAddr(temp, 0x80, BYTEmode);  
  writeAddr(temp, 0xAA, BYTEmode); 
  writeAddr(temp, 0x55, BYTEmode);  
  writeAddr(temp, 0x10, BYTEmode);
  
  start = millis();
  waitc = 0;
  do
  {
  ++waitc; if (waitc == 0) Serial.write(0x2E);
  now = millis(); elapse = now - start;
  }
  while ((elapse < timeout) && (digitalRead(RB) == LOW));

  Serial.println();
  if (digitalRead(RB) == HIGH) Serial.println("Erase done."); else Serial.println("Erase Timeout!");
  }
}

void cmd_blankchk(int arg_cnt, char **args)
{
  if (!cardready()) return;  

  int tempmin, tempmax;
  uint32_t temp = 0;
  if (!strcmp(args[1],"all")) {tempmin = 1; tempmax = 17;}
    else
    if (arg_cnt > 1) 
    {
      temp = cmdStr2Num(args[1], 10); 
      if ((temp < 1) || (temp > 16)) {Serial.println("chip index should be 1..16"); return;}
      tempmin = temp; tempmax = temp + 1;
    } else {Serial.println("not enough parameter."); return;}

  for (int x = tempmin; x < tempmax; x++)
  {
  temp = x;
  if (temp%2 ==1) {--temp; temp = temp/2; temp = temp * 0x400000; temp++;} else {temp = temp/2; temp--; temp = temp * 0x400000;}

  bool isblank = true;
  Serial.print("Blank check at "); Serial.print(temp, HEX);
  for (uint32_t x = temp; x < (temp+0x400000); x=x+2 ) if (readbAddr(x, BYTEmode) != 0xff) {isblank = false; break;}
  if (isblank) Serial.println(", Chip IS blank."); else Serial.println(", Chip NOT blank.");
  }
}

void cmd_wprog(int arg_cnt, char **args)
{
  if (!cardready()) return;  

  uint32_t temp = 0, val;
  if (arg_cnt < 3) {Serial.println("not enough parameter."); return; }
  temp = cmdStr2Num(args[1], 10);
  val = cmdStr2Num(args[2], 10);

  accessmode = WORDmode;
  writeAddr(temp, 0xAAAA, accessmode);
  writeAddr(temp, 0x5555, accessmode);
  writeAddr(temp, 0xA0A0, accessmode);
  writeAddr(temp, val & 0xffff, accessmode);

  //while (digitalRead(RB) == LOW) {};
}

void cmd_readid(int arg_cnt, char **args)
{
  if (!cardready()) return;
  
  uint32_t temp = 0;
  for (int x = 0; x < 8; x++ )
  {
    temp = x * 0x400000;
    Serial.print("Read ID at "); Serial.print(temp, HEX);  Serial.print(" :");

    //reset
    writeAddr(temp, 0xF0F0, WORDmode); while (digitalRead(RB) == LOW) {};
  
    writeAddr(temp, 0xAAAA, WORDmode);
    writeAddr(temp, 0x5555, WORDmode);
    writeAddr(temp, 0x9090, WORDmode); while (digitalRead(RB) == LOW) {};

    Serial.print(readbAddr(temp, WORDmode), HEX); Serial.print("-"); Serial.println(readbAddr(temp+2, WORDmode), HEX);
  }

  for (int x = 0; x < 8; x++ )
  {
    temp = x * 0x200000;
    writeAddr(temp, 0xF0F0, WORDmode); //reset
    while (digitalRead(RB) == LOW) {};
  }
}

bool cardready()
{
  if ((digitalRead(CD1) == HIGH) || (digitalRead(CD2) == HIGH))
  {
    Serial.println("No card detected or card is not inserted properly.");
    return false;
  }
  
  if (digitalRead(RB) == LOW)
  {
    Serial.println("Card is busy!");
    return false;
  }

  if (digitalRead(WP) == HIGH)
    {Serial.println("Card is write protected."); return false;}
  return true;
}

void writeAddr(unsigned long addr, unsigned int value, cardaccessmode accessmode)
{
  //load addr
  addr = addr & 0x3FFFFFF;
  PORTF = addr & 0xFF;
  PORTK = (addr >> 8) & 0xFF;
  PORTA = (addr >> 16) & 0xFF;
  PORTC = (addr >> 24) & 0xFF;
 
  digitalWrite(CE1, LOW);
  if (accessmode == WORDmode) digitalWrite(CE2, LOW);
  digitalWrite(WE, LOW);

  //set data to output and load data
  DDRL = 0xFF; // set data pins as output
  DDRB = 0xFF;
  PORTL = value & 0xff;
  if (accessmode == WORDmode) PORTB = (value >> 8) & 0xff; 
  
  delayMicroseconds(1);
  digitalWrite(WE, HIGH);
  digitalWrite(CE1, HIGH);
  if (accessmode == WORDmode) digitalWrite(CE2, HIGH);

  //set data back to input
  DDRL = 0; // set data pins as input
  DDRB = 0;
  PORTL = 0xFF; //enable pullups
  PORTB = 0xFF; 
 
}

void cmd_comn(int arg_cnt, char **args)
{
  if (!cardready()) return;
  
  uint32_t temp = 0;
  if (arg_cnt == 3) 
    {
      temp = cmdStr2Num(args[2], 10);
      switch(*args[1])
        {
          case 'b' : accessmode = BYTEmode; break;
          case 'w' : accessmode = WORDmode; break;
          default  : Serial.println("2nd param wrong!"); return; break;
        }
    } else {Serial.println("Not enough param."); return;}
  
  tempc = 0;
  chksm = 0;
  Serial.print("Dump at : "); Serial.println(temp, HEX);

  if (accessmode == BYTEmode)
  for (uint32_t x = temp; x < (temp+600); x++ )
  {
    readAddr(x, accessmode);
  } else
  for (uint32_t x = temp; x < (temp+600); x=x+2 )
  {
    readAddr(x, accessmode);
  }
  
  Serial.print("Chksm : "); Serial.println(chksm, HEX);  
}

void cmd_attr(int arg_cnt, char **args)
{
  if (!cardready()) return;  
  
  tempc = 0;
  chksm = 0;
  //at28c16 address = 000-7FF
  for (uint32_t x = 0; x < 0x7FF; x=x+2 )
  {
    digitalWrite(REG, LOW);
    delayMicroseconds(1);
    accessmode = BYTEmode;
    readAddr(x & 0x7FF, accessmode);
    digitalWrite(REG, HIGH);    
  }
  Serial.print("Chksm : "); Serial.println(chksm, HEX);
}

void readAddr(unsigned long addr, cardaccessmode accessmode)
{
  chksm += readbAddr(addr, accessmode);
  temp[tempc++] = readbAddr(addr, accessmode);
  if (tempc == 16) 
  {
    tempc = 0;
    for (int x = 0; x < 16; x++ )
    {
      Serial.print((temp[x]>>4)&0xf, HEX);
      Serial.print(temp[x]&0xf, HEX);
      if (accessmode == WORDmode) 
      {
        Serial.print((temp[x]>>12)&0xf, HEX);
        Serial.print((temp[x]>>8)&0xf, HEX);
      }
      Serial.print(" ");
    }
    Serial.print("\t");
    for (int x = 0; x < 16; x++ )
    {
      byte tempb = temp[x] & 0xff;
      if ((tempb >= 0x20) && (tempb <= 0x7E)) Serial.write(tempb); else Serial.print(".");
      if (accessmode == WORDmode)
      {
        tempb = (temp[x] >> 8) & 0xff;
        if ((tempb >= 0x20) && (tempb <= 0x7E)) Serial.write(tempb); else Serial.print(".");
      }
    }
    Serial.println();
  }
}

unsigned int readbAddr(unsigned long addr, cardaccessmode accessmode)
{
// Address : 0000000-3FFFFFF
// PF0..PF7 = A0..A7
// PK0..PK7 = A8..A15
// PA0..PA7 = A16..A23
// PC0 = A24
// PC1 = A25

// Data (16bit): 
// PL0..PL7 = D0..D7
// PB0..PB3 = D8..D11
// PB4..PB7 = D12..D15  

  unsigned int val = 0;
  addr = addr & 0x3FFFFFF;
  PORTF = addr & 0xFF;
  PORTK = (addr >> 8) & 0xFF;
  PORTA = (addr >> 16) & 0xFF;
  PORTC = (addr >> 24) & 0xFF;
  
  digitalWrite(CE1, LOW);
  if (accessmode == WORDmode) digitalWrite(CE2, LOW);
  digitalWrite(OE, LOW);
  if (accessmode == WORDmode) {val = PINB; val = val << 8;}
  val = val | PINL;
  digitalWrite(OE, HIGH);
  digitalWrite(CE1, HIGH);
  if (accessmode == WORDmode) digitalWrite(CE2, HIGH);
  return val;
}



