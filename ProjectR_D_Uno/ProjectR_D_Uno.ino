#include <SPI.h>
#include <Arduino.h>

#define Address_Uno1                    49
#define Address_Mega                    48
//
//#define address_Master                  0x31
//#define address_Slave                   0x30

#define pinLED_connect                  7
#define pinLED_status                   4
#define pinRS485                        6
#define pinSPI                          10

#define LED_connect_blink               2
#define LED_connect_high                1
#define LED_connect_low                 0

#define   SYMBOL_HEADER_Serial          0x40
#define   SYMBOL_HEADER_Ethernet        0x24
#define   SYMBOL_START_DATA             0x3B
#define   SYMBOL_SEPERATE               0x2C
#define   SYMBOL_SLASH                  0x2F
#define   SYMBOL_END                    0x21
#define   SYMBOL_CHECK                  0x3F

#define enable_SPI          digitalWrite(pinSPI, LOW)
#define disable_SPI         digitalWrite(pinSPI, HIGH)
#define RS485_transmiter    digitalWrite(pinRS485, HIGH)
#define RS485_recieve       digitalWrite(pinRS485, LOW) 
#define LED_connect_ON      digitalWrite(pinLED_connect, HIGH) 
#define LED_connect_OFF     digitalWrite(pinLED_connect, LOW)
#define LED_status_ON       digitalWrite(pinLED_status, HIGH) 
#define LED_status_OFF      digitalWrite(pinLED_status, LOW)  

/* Khai báo các biến  */
unsigned char number_7seg_A[11] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0xff};
volatile uint8_t state1 = 0;
volatile uint8_t checkFrame_Status = 0;
volatile byte _state = 0;
volatile uint8_t blinkLED = 0;
byte Mega_Active  = 0;
unsigned char buffer_Frame[15];
uint8_t Size = 11;                                   // biến lưu kích thước bản tin nhận được từ cổng Serial
unsigned int Nhiet_do = 0;
unsigned int Ngoai_quan = 0;
unsigned int countISR = 0;                          // Khai báo biến đếm số giây delay
uint8_t times_errorAgain = 0;
String check = "OK";

/* Khai báo các hàm */
//void sendMega();
//uint8_t waitting();
//uint8_t UART();
//uint8_t waitting(byte times, byte type, int milis, byte mode = 4);

//-----------------------------------------------------------------------------
/* Phần cài đặt ban đầu */
void setup(){
   
  pinMode_begin();
  Display_begin();
  RS485_begin();

  while(Mega_Active == 0){        
    send_Slave2Master(Address_Mega);        
    state1 = waitting(15,1000);    
    if(state1 == 1){
      checkFrame_Status = recieve_check_Master2Slave();
      if (checkFrame_Status == 1){
        LED_connect_ON;
        Mega_Active = 1;
      }
      else {
        LED_connect_OFF;
        state1 = 0;
      }
    }
  }
  Display(Nhiet_do,Ngoai_quan);

  //===============================================================================
  //    Vòng lăp gửi bản tin đến Mega , cứ sau 30s, nếu có bản tin phản hồi
  // và tin đó đúng thì sẽ bật 1 LED báo là có thiết bị trung tâm kết nối. Khi mất
  // kết nối thì cần reset lại thiết bị trên line đó để kết nối lại.
  //===============================================================================
  
  while(Mega_Active == 0){                      // Mega_Active == 0, tức là chưa có kết nối với thiết bị trung tâm
    send_Slave2Master(Address_Mega);                    // truyền bản tin để kết nối  
    state1 = waitting(10,500);                  // hàm waitting sẽ chờ phản hồi trong 1 khoảng thời gian lập trình trước và trả về 1 nếu 
                                                // trong thời gian đó cổng Serial có dữ liệu truyền đến và nếu quá thời gian mà ko có thì trả về 0
                            
    if(state1 == 1){                            // biến state == 1 nếu có phản hồi từ thiết bị trung tâm
      checkFrame_Status = recieve_check_Master2Slave();    // lưu giá trị bản tin của hàm checkFrameUART (trả về 0: tức là cấu trúc bản tin sai, trả vè 1: tức là đúng)
      if(checkFrame_Status == 0){                          // Nếu cấu trúc bản tin là sai - tức là nhiễu lớn
        state1 = 0;
        blinkLED = LED_connect_blink;                   // báo nháy LED nếu nhiễu lớn - nháy LED nhanh
        state1 = waitting(10,500);              // chờ 30s để tiếp tục thử kết nối lại - có thể dùng hàm delay thay thế
      }
      else{                                     // Nếu bản tin có cấu trúc đúng
          Mega_Active = 1;                      // Nếu bản tin nhận được là đúng -->> set biến Mega_Active = 1 báo là đã kết nối thành công!
          blinkLED = LED_connect_high;                  // bật LED sáng để báo đã kết nối
          LED_connect_ON;
      }
    }
    else{                                      // biến state == 0 tức là không có phản hồi từ thiết bị trung tâm 
      state1 = waitting(10,500);               // chờ 30s để tiếp tục thử kết nối lại
    }
  }
  
  state1 = checkFrame_Status = 0;                         // biến state1 = checkFrame_Status = 0 - cài lại giá trị mặc định để sử dụng lưu giá trị trả về của các hàm khác 
  Display(Nhiet_do,Ngoai_quan);
}


//-------------------------------------------------------------------------------
//
//                                   Hàm Main
//
//
//-------------------------------------------------------------------------------
void loop(){
  state1 = waitting(10,1000);
  
  /* Kiểm tra giá trị bộ đệm liên tục */
  if(state1 == 1){         // Nếu có dữ liệu truyền đến
    delay(100);
    checkFrame_Status = recieve_data_Master2Slave();          // lưu giá trị bản tin của hàm checkFrameUART (trả về 0: tức là cấu trúc bản tin sai, trả vè 1: tức là đúng)
    if(checkFrame_Status == 5){
      times_errorAgain ++;
      send_Again(Address_Mega);
      blinkLED = LED_connect_blink;
    }
    else 
      blinkLED = LED_connect_high;
    state1 = 0;
    Display(Nhiet_do,Ngoai_quan);
  }
  else {
    times_errorAgain ++;
    if(times_errorAgain == 6){
      LED_connect_OFF;
      blinkLED = LED_connect_low;
    }
  }
}

//---------------------------------------------------------------------------------
void pinMode_begin(){
  // chế độ các chân kết nối
  pinMode(pinLED_connect,   OUTPUT);
  pinMode(pinLED_status,   OUTPUT);
  pinMode(pinRS485, OUTPUT);
  pinMode(pinSPI,   OUTPUT);

  LED_connect_OFF;
  LED_status_OFF;
}
//---------------------------------------------------------------------------------
void RS485_begin(){
  Serial.begin(9600);
  RS485_recieve;
}

//---------------------------------------------------------------------------------
void Display_begin(){
  SPI.begin();
  enable_SPI;
  SPI.transfer(number_7seg_A[8]);
  SPI.transfer(number_7seg_A[8]);
  SPI.transfer(number_7seg_A[8]);
  SPI.transfer(number_7seg_A[8]);
  disable_SPI;
  delay (1000);
  /* Biến */
  Size = 11;
}

//-----------------------------------------------------
uint8_t readUART(uint8_t c[]){
  uint8_t i = 0;
  delay(1000);
  while(Serial.available()){
    c[i] = char (Serial.read());
    i++;
  }
  return i;
}
//----------------------------------------------------
//uint8_t GetImfor(unsigned char data[]){
//  unsigned int error1 = Nhiet_do;                // Biến lưu giá trị lỗi "Nhiệt độ" hiện tại trước khi đọc giá trị mới để so sánh
//  unsigned int error2 = Ngoai_quan;              // Biến lưu giá trị lỗi "Ngoại quan" hiện tại trước khi đọc giá trị mới để so sánh
//  /* đọc giá trị mới */
//  Nhiet_do   = data[FRAME_DATA_1_1]*256 + data[FRAME_DATA_1_2];
//  Ngoai_quan = data[FRAME_DATA_2_1]*256 + data[FRAME_DATA_2_2];
//  /* so sánh các giá trị */
//  if( ( (error1 - Nhiet_do) + (error2 - Ngoai_quan) ) == 1 )      // nếu đúng là 1 trong 2 lỗi tăng lên 1
//    return 1;                                                     
//  else{                                                           // nếu sai tức là có lỗi trong bản tin
//    Nhiet_do    = error1;                                         // đọc lại giá trị Nhiet_do cũ
//    Ngoai_quan  = error2;                                         // đọc lại giá trị Ngoai_quan cũ
//    return 0;                                                     // giá trị trả về 0 - lấy bản tin thất bại 
//   }
//}

//----------------------------------------------------
void Display(unsigned int b, unsigned int a){
  /* Hiển thị lỗi nhiệt độ */
  
  SPI.transfer(number_7seg_A[a%10]);
  SPI.transfer(number_7seg_A[a/10]);
  if(Nhiet_do >= 100){
    SPI.transfer(number_7seg_A[a / 100]);
  }
//  else 
//    SPI.transfer(number_7seg_A[0]);

  /* Hiển thị lỗi Ngoại quan */
  SPI.transfer(number_7seg_A[b % 10]);
  SPI.transfer(number_7seg_A[b / 10]);
  if(Ngoai_quan >= 100){
    SPI.transfer(number_7seg_A[b / 100]);
  }
//  else 
//    SPI.transfer(number_7seg_A[0]);
}

//----------------------------------------------------
void send_Again(uint8_t address_Master){
  RS485_transmiter;
  delay(100);
  Serial.print(char(SYMBOL_HEADER_Serial));
  Serial.print(char(Address_Mega));
  Serial.print(",");
  Serial.print(char(Address_Uno1));
  Serial.print(",");
  Serial.print("AGAIN");
  Serial.print(char(SYMBOL_END));
  delay(100);
  RS485_recieve;
}
//----------------------------------------------------
void send_Slave2Master(uint8_t address_Master){
  RS485_transmiter;
  delay(100);
  Serial.print(char(SYMBOL_HEADER_Serial));
  Serial.print(char(Address_Mega));
  Serial.print(",");
  Serial.print(char(Address_Uno1));
  Serial.print(",");
  Serial.print(check);
  Serial.print(char(SYMBOL_END));
  delay(100);
  RS485_recieve;
}
//----------------------------------------------------
uint8_t recieve_check_Master2Slave(){
  /* Kiểm tra cấu trúc (@0,OK!) */
  uint8_t poisition_header = 0;
  uint8_t poisition_end = 0;
  uint8_t poisition_seperate = 0;
  uint8_t check = 0;
  Size = readUART(buffer_Frame);

  /* Kiểm tra có ký tự đầu ko */
  for(uint8_t i =0; i < Size; i++){
    if(buffer_Frame[i] == SYMBOL_HEADER_Serial){
      poisition_header = i;
      check ++;
    }
  }
  
  if(check != 1){
    return 0;
  }  
   /* Kiểm tra có ký tự cuối ko */
  for(uint8_t i = poisition_header; i < Size; i++){
    if(buffer_Frame[i] == SYMBOL_END){
       poisition_end = i;
       check ++;
    }
  }
  
  if(check != 2){
    return 0;
  }
  /* Kiểm tra cấu trúc bên trong */
  for(byte i = (poisition_header+1); i < poisition_end; i++){
    if(buffer_Frame[i] == SYMBOL_SEPERATE){
      if (buffer_Frame[i-1] == Address_Uno1) 
        check ++;
      if ((buffer_Frame[i+1] == 'O') && (buffer_Frame[i+2] == 'K'))
        check ++;
    }
  }
   if(check == 4){
    return 1;
  }
  else 
    return 0;
    //chưa có phần check Sum
}
//----------------------------------------------------
uint8_t recieve_data_Master2Slave(){
  /* Kiểm tra cấu trúc (@1,data1,data2!) */
  uint8_t poisition_header = 0;
  uint8_t poisition_end = 0;
  uint8_t poisition_seperate1 = 0;
  uint8_t poisition_seperate2 = 0;
  uint8_t state = 0;
  unsigned int sum = 0;
  uint8_t sizeData1 = 0;
  uint8_t sizeData2 = 0;
  unsigned int error1 = Nhiet_do;                // Biến lưu giá trị lỗi "Nhiệt độ" hiện tại trước khi đọc giá trị mới để so sánh
  unsigned int error2 = Ngoai_quan;              // Biến lưu giá trị lỗi "Ngoại quan" hiện tại trước khi đọc giá trị mới để so sánh
  Size = readUART(buffer_Frame);
               /* Kiểm tra phần checksum */
//   sum = checkSum(buffer_Frame, Size -2);
//   if( (buffer_Frame[Size -2] != (sum >> 8)) || (buffer_Frame[Size -1] != (sum & 0xff))){
//      return 1;
//   }
//   else 
    state ++;

            /* Kiểm tra có ký tự đầu ko */
  for(uint8_t i = 0; i < Size; i++){
    if(buffer_Frame[i] == SYMBOL_HEADER_Serial){
      poisition_header = i;
      state ++;
    }
  }
  
  if(state != 2){
    return 2;
  }
              /* Kiểm tra có ký tự cuối ko */
  for(uint8_t i = (poisition_header +1); i < Size; i++){
    if(buffer_Frame[i] == SYMBOL_END){
       poisition_end = i;
       state++;
    }
  }
  if(state != 3){
    return 3;
  }    
                  /* Kiểm tra địa chỉ */     
  for(uint8_t i = poisition_header; i < poisition_end; i++){
    if(buffer_Frame[i] == SYMBOL_SEPERATE){
      poisition_seperate1 = i;
      state ++;
      if (buffer_Frame[i-1] == Address_Uno1){
        state ++;
      }
      break;
    }
  }
  if(state != 5){
    return state;
  }
                /* Đọc dữ liệu */
  for(uint8_t i = (poisition_seperate1+1); i < poisition_end; i++){
    if(buffer_Frame[i] == SYMBOL_SEPERATE){
      poisition_seperate2 = i;
      state ++;
      break;
    }
  }
  if(state != 6){
    return 6;
  } 
  sizeData1 = poisition_seperate2 - poisition_seperate1 - 1;
  sizeData2 = poisition_end - poisition_seperate2 - 1;
  /* Đọc giá trị data1 */
  if(sizeData1 == 1)
    Nhiet_do = buffer_Frame[poisition_seperate1 + 1] -48;
  else
    if(sizeData1 == 2)
      Nhiet_do = (buffer_Frame[poisition_seperate1 + 1] -48)*10 + (buffer_Frame[poisition_seperate1 + 2] -48);
    else
      if(sizeData1 == 3)
        Nhiet_do = (buffer_Frame[poisition_seperate1 + 1] -48)*100 + (buffer_Frame[poisition_seperate1 + 2] -48)* 10 + (buffer_Frame[poisition_seperate1 + 3]-48);
  /* Đọc giá trị data2 */     
  if(sizeData2 == 1)
    Ngoai_quan = buffer_Frame[poisition_seperate2 + 1]-48;
  else
    if(sizeData2 == 2)
      Ngoai_quan = (buffer_Frame[poisition_seperate2 + 1]-48)*10 + buffer_Frame[poisition_seperate2 + 2]-48;
    else
      if(sizeData2 == 3)
        Ngoai_quan = (buffer_Frame[poisition_seperate2 + 1]-48)*100 + (buffer_Frame[poisition_seperate2 + 2]-48)* 10 + buffer_Frame[poisition_seperate2 + 3]-48;
   return 0;    
//  /* so sánh các giá trị */
//  
//  Serial.print("Nhiet do: ");
//  Serial.println(Nhiet_do);
//  Serial.print("Ngoai quan: ");
//  Serial.println(Ngoai_quan);
//  Serial.print("error1: ");
//  Serial.println(error1);
//  Serial.print("error2: ");
//  Serial.println(error2);
//  if( ( (Nhiet_do - error1) + (Ngoai_quan - error2) ) == 1 )      // nếu đúng là 1 trong 2 lỗi tăng lên 1
//    return 0;                                                     
//  else{                                                           // nếu sai tức là có lỗi trong bản tin
//    Nhiet_do    = error1;                                         // đọc lại giá trị Nhiet_do cũ
//    Ngoai_quan  = error2;                                         // đọc lại giá trị Ngoai_quan cũ
//    return 7;                                                     // giá trị trả về 0 - lấy bản tin thất bại 
//   }
}


//------------------------------------------------------
int checkSum (unsigned char checkSum[], byte Size){
  unsigned int sum = 0;
  for (byte c = 0; c < Size; c++){
    sum += checkSum[c];
//    Serial.print("Sum: ");
//    Serial.println(sum);
  }
  return sum;
 }
 //------------------------------------------------------
 ISR (TIMER1_OVF_vect) {
    _state = 1;
    switch(blinkLED){
    //Serial.println("vao phan blinkLED:");
      case LED_connect_low:
        //Serial.println("vao phan LED_connect_low");
        LED_connect_OFF;
        break;
        
      case LED_connect_high:
        //Serial.println("vao phan LED_connect_high");
        LED_connect_ON;
        break;
        
      case LED_connect_blink:
        //Serial.println("vao phan LED_connect_blink");
        if ((countISR % 2) == 0)
          LED_connect_ON;
        else
          LED_connect_OFF;
        break;
        
//      case LED_slow:
//        //Serial.println("vao phan LED_slow");
//        if ((countISR % 3) == 0)
//          LED_connect_ON;
//        else
//          LED_connect_OFF;
//        break;
//        
//      case LED_nhieu:
//        //Serial.println("vao phan LED_nhieu");
//        if (((countISR / 11) == 1) || ((countISR / 12) == 1) || ((countISR / 13) == 1))
//          LED_connect_ON;
//        else
//          LED_connect_OFF;
//        break;
        
      default:
        break;
    }
}
//-------------------------------------------------------
uint8_t waitting(byte times, int ms){
  countISR = 0;
  unsigned int timeISR;
  /* Cài đặt các thông số Timer1 */      
  cli();                                    // Không cho phép ngắt toàn cục
  TCCR1A = 0;                                
  TCCR1B = 0;                               // thanh ghi lựa chọn xung nhịp 
  TIMSK1 = 0;                               // thanh ghi Interrupt Mask
  TCCR1B |= (1 << CS12) | (1 << CS10);      // thời gian tối đa (1024/16)us x 65536 = 4,192s
  timeISR = 65536 - (ms/64)*1000;             

  TCNT1 = timeISR;
  TIMSK1 = (1 << TOIE1);                 // cho phép ngắt tràn (Overflow interrupt enable) 
  sei();                                 // cho phép ngắt toàn cục
  
  while(countISR < times){               // so sánh số lần lỗi
    if (_state == 1){                    // Nếu cờ báo tràn lên 1: _state == 1 (Over time)
      _state = 0;                        // Xóa biến _state = 0
      countISR ++;                       // tăng biến đến số lần ngắt thêm 1 đơn vị
      TCNT1 = timeISR;                   // đặt lại thanh ghi chứa giá trị đếm của Timer_1
      TIMSK1 = (1 << TOIE1);             // cho phép ngắt tràn (Overflow interrupt enable)
      sei(); 
    }

    /* Nếu có dữ liệu đến */
    if(Serial.available()){
      _state = 0;
      TIMSK1 = 0;
      return 1;
    }
  }

  _state = 0;
  TIMSK1 = 0;
  return 0;
}

