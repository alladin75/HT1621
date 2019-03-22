// использованы наработки из http://arduino.ru/forum/programmirovanie/ht1621
// http://arduino.ru/forum/programmirovanie/ht1621#comment-76371

#define sbi(x, y)  (x |= (1 << y))   /*set Register x of y*/
#define cbi(x, y)  (x &= ~(1 <<y ))  /*Clear Register x of y*/       

#define uchar   unsigned char 
#define uint   unsigned int 

//Defined HT1621's command  
#define  ComMode    0x48  //4COM,1/2bias  1000    010 1001  0  
#define  RCosc      0x30  //on-chip RC oscillator(Power-on default)1000 0011 0000 
#define  LCD_on     0x06  //Turn on LCD 
#define  LCD_off    0x04  //Turn off LCD 
#define  Sys_en     0x02  //Turn on system oscillator 1000   0000 0010 
#define  CTRl_cmd   0x80  //Write control cmd 
#define  Data_cmd   0xa0  //Write data cmd   

// //Define port    HT1621 data port
#define CS   2  //Pin 2 as chip selection output
#define WR   3  //Pin 3 as read clock  output
#define DATA 4  //Pin 4 as Serial data output

#define CS1    digitalWrite(CS, HIGH) 
#define CS0    digitalWrite(CS, LOW)
#define WR1    digitalWrite(WR, HIGH) 
#define WR0    digitalWrite(WR, LOW)
#define DATA1  digitalWrite(DATA, HIGH) 
#define DATA0  digitalWrite(DATA, LOW)

#define DelayTime 1000

byte bMask;

// HT1681_Segments 1A 1B 1C 1D 1E 1F 1G	1DP 1Up	..... 8A 8B 8C 8D 8E 8F 8G 8DP 8Up
const byte HT1681_Address[]=
{
  0x04,0x05,0x05,0x04,0x04,0x04,0x04,0x05,0x0E,
  0x06,0x06,0x06,0x06,0x05,0x05,0x06,0x06,0x04,
  0x07,0x08,0x08,0x07,0x07,0x07,0x07,0x08,0x05,
  0x09,0x09,0x09,0x09,0x08,0x08,0x09,0x09,0x07,
  0x0A,0x0B,0x0B,0x0A,0x0A,0x0A,0x0A,0x0B,0x08,
  0x0C,0x0C,0x0C,0x0C,0x0B,0x0B,0x0C,0x0C,0x0A,
  0x0D,0x0E,0x0E,0x0D,0x0D,0x0D,0x0D,0x0E,0x0B,
  0x0F,0x0F,0x0F,0x0F,0x0E,0x0E,0x0F,0x0F,0x0D
};


const byte HT_1681_Value[]=
{
  0x04,0x40,0x20,0x08,0x20,0x40,0x02,0x80,0x08,
  0x40,0x04,0x02,0x80,0x02,0x04,0x20,0x08,0x80,
  0x04,0x40,0x20,0x08,0x20,0x40,0x02,0x80,0x08,
  0x40,0x04,0x02,0x80,0x02,0x04,0x20,0x08,0x80,
  0x04,0x40,0x20,0x08,0x20,0x40,0x02,0x80,0x08,
  0x40,0x04,0x02,0x80,0x02,0x04,0x20,0x08,0x80,
  0x04,0x40,0x20,0x08,0x20,0x40,0x02,0x80,0x08,
  0x40,0x04,0x02,0x80,0x02,0x04,0x20,0x08,0x80
};

// 0-9 7Segment Display Description
// D7 D6 D5 D4 D3 D2 D1 D0
// A  B  C  D  E  F  G  *
//

const byte b7SegDsp[]=
{
  0xFC,0x60,0xDA,0xF2,0x66,0xB6,0xBE,0xE0,0xFE,0xE6
};

// virtual screen 12x6 bits = 72 bits (000X000X)
byte HT1681_Screen[]=
{
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};


// *********************************************************************************************************************************

void SendBit_1621(uchar sdata,uchar cnt) //High bit first
{ 
  uchar i; 
  for(i=0;i<cnt;i++) 
  { 
    WR0;
    delayMicroseconds(1); 
    if(sdata&0x80) DATA1; 
    else DATA0; 
    delayMicroseconds(1);
    WR1;
    delayMicroseconds(1);
    sdata<<=1; 
  } 
  delayMicroseconds(1);
}


void Init_1621(void) 
{
  SendCmd_1621(Sys_en);
  SendCmd_1621(RCosc);    
  SendCmd_1621(ComMode);  
  SendCmd_1621(LCD_on);
} 

void SendCmd_1621(uchar command) 
{ 
  CS0; 
  SendBit_1621(0x80,4);    
  SendBit_1621(command,8);  
  CS1;                    
} 

void Write_1621(uchar addr,uchar sdata) 
{ 
  addr<<=3; 
  CS0; 
  SendBit_1621(0xa0,3);     //Write MODE“101” 
  SendBit_1621(addr,6);     //Write addr high 6 bits
  SendBit_1621(sdata,8);    //Write data  8 bits
  CS1; 
} 

void LCDoff(void) 
{  
  SendCmd_1621(LCD_off);  
} 

void LCDon(void) 
{  
  SendCmd_1621(LCD_on);  
} 

void HT1681_Clear(void)
{
  byte bAddress=0x00;
  for(byte j=0;j<=0x0F;j++)
  {
    bAddress=j;
    HT1681_Screen[bAddress]=0x00;
  };
};

void HT1681_Show(void)
{
  byte i;
  for(i=0;i<=0x0F;i++)
  {
  Write_1621(i, HT1681_Screen[i]);
  };
}


// 0<=bPos<=7 - в какой позиции написать число bNum. 
// Если нужна десятичная точка - то lDecimalPoint=1
// Если нужно верхнее "подчеркивание" - то lUpperLine=1
void HT1681_Display(byte bPos, byte bNum, byte lDecimalPoint, byte lUpperLine)
{
  byte i, bCheckByte, bAddr, bValue;
  
  bCheckByte=b7SegDsp[bNum]; // получаем маску - какие сегменты мы должны включить

  for(i=bPos*9;i<=bPos*9+6;i++) // поскольку сегментов 7 штук 
  {
    if ( (bCheckByte & 0x80) == 0x80) // если старший сегмент =1 то должны 
    {
      bAddr=HT1681_Address[i]; // определить конкретный байт
      bValue=HT_1681_Value[i]; // и определить конкретную маску
      HT1681_Screen[bAddr] = (HT1681_Screen[bAddr] | bValue); // выполним операцию OR для включения соответствующего бита в 1
    };
    bCheckByte=bCheckByte << 0x01; // сдвигаем весь байт влево
  }

  if (lDecimalPoint==1) // отдельно отрабатываем десятичную точку
  {
    bAddr = HT1681_Address[(bPos*9)+7];
    bValue = HT_1681_Value[(bPos*9)+7];
    HT1681_Screen[bAddr] = (HT1681_Screen[bAddr] | bValue);
  }; 

  if (lUpperLine==1) // отдельно отрабатываем верхнее подчеркивание
  {
    bAddr = HT1681_Address[(bPos*9)+8];
    bValue = HT_1681_Value[(bPos*9)+8];
    HT1681_Screen[bAddr] = (HT1681_Screen[bAddr] | bValue);
  };
}

// ***************************************************************************************************************************
void setup()
{
  pinMode(CS, OUTPUT); //Pin 2 
  pinMode(WR, OUTPUT); //Pin 3 
  pinMode(DATA, OUTPUT); //Pin 4
  Init_1621();
  HT1681_Clear();
  HT1681_Show();
  Serial.begin(9600);
}

// ***************************************************************************************************************************

void loop()
{
byte i,j;

  HT1681_Clear();
  HT1681_Show();

  LCDoff(); delay(100);
  for (i=0; i<=7; i++)
  {
    for (j=0; j<=9; j++)
    {
      HT1681_Clear(); 
      LCDoff();
      HT1681_Display(i,j,0,0);
      HT1681_Show();
      LCDon();
      delay(150);
    };
  };
}
