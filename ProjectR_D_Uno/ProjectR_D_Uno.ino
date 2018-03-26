#include <SPI.h>
#include <Arduino.h>
#define Uno        49
#define Mega        1
#define pinSTR      5
#define pinCLK      7
#define pinMOSI     6
#define pinVCC      8
#define slaveSelectPin      10
//comment
#define LED_slow    0
#define LED_speed   0
#define LED_high    0
#define LED_low     0

#define   SYMBOL_HEADER_Serial          0x40
#define   SYMBOL_HEADER_Ethernet        0x24
#define   SYMBOL_START_DATA             0x3B
#define   SYMBOL_SEPERATE               0x2C
#define   SYMBOL_SLASH                  0x2F
#define   SYMBOL_END                    0x21
#define   SYMBOL_CHECK                  0x3F
#define LED_ON    digitalWrite(13, HIGH)  // Data line output high
#define LED_OFF   digitalWrite(13, LOW)   // Data line output low

#define MOSI_HIGH digitalWrite(pinMOSI, HIGH)  // Data line output high
#define MOSI_LOW  digitalWrite(pinMOSI, LOW)   // Data line output low
#define CLK_HIGH  digitalWrite(pinCLK, HIGH)  // CLK output high
#define CLK_LOW   digitalWrite(pinCLK, LOW)   // CLK output low
#define STRB_HIGH digitalWrite(pinSTR, HIGH)  // Latch output high
#define STRB_LOW  digitalWrite(pinSTR, HIGH)  // Latch output low
#define VCC_ON    digitalWrite(pinSTR, HIGH)
#define VCC_OFF   digitalWrite(pinSTR, LOW)

#define MOSI_OUT  pinMode(pinMOSI, OUTPUT)     // Khai báo chân truyền data
#define CLK_OUT   pinMode(pinCLK, OUTPUT)     // Khai báo chân CLK
#define STRB_OUT  pinMode(pinSTR, OUTPUT)     // Khai báo chân cho phép truyền
#define VCC_OUT   pinMode(pinVCC, OUTPUT)     // Khai báo chân cho phép truyền

//Định nghĩa vị trí các phần trong Frame Serial
#define   FRAME_HEADER                  0                         
#define   FRAME_DESTINATION             1
#define   FRAME_START_DATA              2
#define   FRAME_DATA_1_1                3
#define   FRAME_DATA_1_2                4
#define   FRAME_SEPERATE                5
#define   FRAME_DATA_2_1                6
#define   FRAME_DATA_2_2                7
#define   FRAME_END                     8
#define   FRAME_CHECKSUM_1              9
#define   FRAME_CHECKSUM_2              10

/* Khai báo các biến  */
//const int slaveSelectPin = 10;
unsigned char number_7seg_A[11] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0xff};
uint8_t state1 = 0;
uint8_t state2 = 0;
volatile byte _state = 0;
byte Mega_Active  = 0;
unsigned char c[15];
uint8_t Size = 0;    // biến lưu kích thước bản tin nhận được từ cổng Serial
unsigned int Nhiet_do = 0;
unsigned int Ngoai_quan = 0;
unsigned int countISR = 0;                          // Khai báo biến đếm số giây delay
/* Khai báo các hàm */
//void sendMega();
//uint8_t waitting();
//uint8_t UART();
//uint8_t waitting(byte times, byte type, int milis, byte mode = 4);

//-----------------------------------------------------------------------------
/* Phần cài đặt ban đầu */
void setup(){
  digitalMode(13, OUTPUT);
  Serial.begin(9600);
  Value_begin();
  // kiểm tra các thanh LED 7-seg
  SPI.transfer(number_7seg_A[10]);
  SPI.transfer(number_7seg_A[10]);
  SPI.transfer(number_7seg_A[10]);
  SPI.transfer(number_7seg_A[10]);
  
//  //-------------------------------------------------------------------------------
//  //    Vòng lăp gửi bản tin đến Mega , cứ sau 30s, nếu có bản tin phản hồi
//  // và tin đó đúng thì sẽ bật 1 LED báo là có thiết bị trung tâm kết nối. Khi mất
//  // kết nối thì cần reset lại thiết bị trên line đó để kết nối lại.
//  //--------------------------------------------------------------------------------
//  while(Mega_Active == 0){        // Mega_Active == 0, tức là chưa có kết nối với thiết bị trung tâm
//    sendUART(c, Size, Mega,'?','?','?','?');                   // truyền bản tin để kết nối
//    state1 = waitting(2,1000);    // hàm waitting sẽ chờ phản hồi trong 1 khoảng thời gian lập trình trước và trả về 1 nếu 
//                                  // trong thời gian đó cổng Serial có dữ liệu truyền đến và nếu quá thời gian mà ko có thì trả về 0
//                            
//    if(state1 == 1){              // biến state == 1 nếu có phản hồi từ thiết bị trung tâm
//      delay(100);                 // chờ bản tin đến hết
//      Size = readUART(c);         // đọc bản tin truyền đến và lưu kích thước bản tin vào biến Size
//      state2 = checkFrameUART(c,Size,Mega);          // lưu giá trị bản tin của hàm checkFrameUART (trả về 0: tức là cấu trúc bản tin sai, trả vè 1: tức là đúng)
//      if(state2 == 0){            // Nếu cấu trúc bản tin là sai - tức là nhiễu lớn 
//        state1 = 0;
//        blinkLED(LED_speed);      // báo nháy LED nếu nhiễu lớn - nháy LED nhanh
//        state1 = waitting(30,1000);      // chờ 30s để tiếp tục thử kết nối lại - có thể dùng hàm delay thay thế
//      }
//      else{                       // Nếu bản tin có cấu trúc đúng
//        // kiểm tra bản tin nhận được
//        if(c[FRAME_DATA_1_1] == 'O' && c[FRAME_DATA_1_2] == 'K' && c[FRAME_DATA_2_1] == 'O' && c[FRAME_DATA_2_2] == 'K'){
//          Mega_Active = 1;        // Nếu bản tin nhận được là đúng -->> set biến Mega_Active = 1 báo là đã kết nối thành công!
//          blinkLED(LED_high);
//        }
//        else {                    // Nếu bản tin nhận được sai nội dung.
//          state1 = 0;
//          blinkLED(LED_slow);     // báo nháy LED nếu nhiễu nhỏ - tốc độ nháy LED chậm
//          state1 = waitting(30,1000);    // chờ 30s để tiếp tục thử kết nối lại
//        }
//      }
//    }
//    else{                         // biến state == 0 tức là không có phản hồi từ thiết bị trung tâm 
//      blinkLED(LED_low);                 // tắt LED báo kết nối với thiết bị trung tâm
//      state1 = waitting(30,1000);               // chờ 30s để tiếp tục thử kết nối lại
//    }
//  }
}


//-------------------------------------------------------------------------------
//
//                                   Hàm Main
//
//
//-------------------------------------------------------------------------------
void loop(){
 
  /* Kiểm tra giá trị bộ đệm liên tục */
  if(Serial.available()){         // Nếu có dữ liệu truyền đến
    delay(100);
    state1 = state2 = 0;          // biến state1 = state2 = 0 - cài lại giá trị mặc định để sử dụng lưu giá trị trả về của các hàm khác 
    Size = 0;                     // Cài lại giá trị mặc định của biến lưu kích thước bản tin nhận được từ cổng Serial
    Size = readUART(c);           // đọc bản tin truyền đến và lưu kích thước bản tin vào biến Size
  }
  else                            // Nếu không có dữ liệu truyền đến thì return để bắt đầu lại
    return ;

  /* Khi đã đọc bản tin truyền đến thì -->>> Kiểm tra cấu trúc */
  state1 = checkFrameUART(c,Size,Uno);          // lưu giá trị bản tin của hàm checkFrameUART (trả về 0: tức là cấu trúc bản tin sai, trả vè 1: tức là đúng)
  Serial.println(state1);
//  if(state1 == 0){                // nếu bản tin thất bại (cấu trúc sai)
//    Mega_Active = 2;              // Mega_Active = 2 báo dành đã từng kết nối nhưng có nhiễu lớn bất thường trên đường truyền
//    blinkLED(Mega_Active);        // các kiểu báo hiệu khác nhau cho người dùng biết đang ở lỗi nào
//    return ;                      // vẫn quay lại từ đầu quét cổng Serial
//  }
//  
//// Nếu đã qua được hàm kiểm tra lỗi cấu trúc, nghĩa là cấu trúc bản tin đúng 
//  /* Lấy thông tin để hiển thị */
//  state2 = GetImfor(c);
//  if(state2 == 1){
//    sendUART(c, Size, Mega,'O','K','O','K');
//    Display();
//  }
//  else{
//    sendUART(c, Size, Mega,'E','R','R','!');
//    blinkLED(Mega_Active);                      // báo nháy LED nếu nhiễu nhỏ - tốc độ nháy LED chậm
//    return ;
//  }

  /* Kiểm tra từng hàm */
  
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
void Value_begin(){
  SPI.begin();
  SPI.transfer(number_7seg_A[10]);
  SPI.transfer(number_7seg_A[10]);
  SPI.transfer(number_7seg_A[10]);
  SPI.transfer(number_7seg_A[10]);

  c[FRAME_HEADER]      = SYMBOL_HEADER_Serial;
  c[FRAME_DESTINATION] = Mega;
  c[FRAME_START_DATA]  = SYMBOL_START_DATA;
  c[FRAME_DATA_1_1]    = SYMBOL_CHECK;
  c[FRAME_DATA_1_2]    = SYMBOL_CHECK;
  c[FRAME_SEPERATE]    = SYMBOL_SEPERATE;
  c[FRAME_DATA_2_1]    = SYMBOL_CHECK;
  c[FRAME_DATA_2_2]    = SYMBOL_CHECK;
  c[FRAME_END]         = SYMBOL_END;
  c[FRAME_CHECKSUM_1]  = 0;
  c[FRAME_CHECKSUM_2]  = 0;
}
//-----------------------------------------------------
uint8_t readUART(uint8_t c[]){
  uint8_t i = 0;
  while(Serial.available()){
    c[i] = Serial.read();
    i++;
  }
  return i;
}
//----------------------------------------------------
uint8_t GetImfor(unsigned char data[]){
  unsigned int error1 = Nhiet_do;                // Biến lưu giá trị lỗi "Nhiệt độ" hiện tại trước khi đọc giá trị mới để so sánh
  unsigned int error2 = Ngoai_quan;              // Biến lưu giá trị lỗi "Ngoại quan" hiện tại trước khi đọc giá trị mới để so sánh
  /* đọc giá trị mới */
  Nhiet_do   = data[FRAME_DATA_1_1]*256 + data[FRAME_DATA_1_2];
  Ngoai_quan = data[FRAME_DATA_2_1]*256 + data[FRAME_DATA_2_2];
  /* so sánh các giá trị */
  if( ( (error1 - Nhiet_do) + (error2 - Ngoai_quan) ) == 1 )      // nếu đúng là 1 trong 2 lỗi tăng lên 1
    return 1;                                                     
  else{                                                           // nếu sai tức là có lỗi trong bản tin
    Nhiet_do    = error1;                                         // đọc lại giá trị Nhiet_do cũ
    Ngoai_quan  = error2;                                         // đọc lại giá trị Ngoai_quan cũ
    return 0;                                                     // giá trị trả về 0 - lấy bản tin thất bại 
   }
}

//----------------------------------------------------
void Display(){
  /* Hiển thị lỗi nhiệt độ */
  SPI.transfer(number_7seg_A[Nhiet_do % 10]);
  SPI.transfer(number_7seg_A[Nhiet_do / 10]);
  if(Nhiet_do >= 100){
    SPI.transfer(number_7seg_A[Nhiet_do / 100]);
  }
//  else 
//    SPI.transfer(number_7seg_A[0]);

  /* Hiển thị lỗi Ngoại quan */
  SPI.transfer(number_7seg_A[Ngoai_quan % 10]);
  SPI.transfer(number_7seg_A[Ngoai_quan / 10]);
  if(Nhiet_do >= 100){
    SPI.transfer(number_7seg_A[Ngoai_quan / 100]);
  }
//  else 
//    SPI.transfer(number_7seg_A[0]);
}

//----------------------------------------------------
void sendUART(uint8_t data[], uint8_t Size,uint8_t destination, char data1_1, char data1_2, char data2_1, char data2_2){
  unsigned int check = 0;
  
  data[FRAME_HEADER] = SYMBOL_HEADER_Serial;
  data[FRAME_DESTINATION] = destination;
  data[FRAME_DATA_1_1] = data1_1;
  data[FRAME_DATA_1_2] = data1_2;
  data[FRAME_DATA_2_1] = data2_1;
  data[FRAME_DATA_2_2] = data2_2;
  /* thêm phần checksum*/
  check = checkSum(data, Size -2);
  data[FRAME_CHECKSUM_1] = check >> 8;
  data[FRAME_CHECKSUM_2] = check & 0xff;

  for (byte s = 0; s < Size; s++){
  Serial.print(data[s]);
  }
  /* Phần khác: Cấu trúc truyền 1 loại lỗi với 2 byte */
//  _charCommand[FRAME_DATA_1] = ((data2 >> 12 | data1);
//  _charCommand[FRAME_DATA_2] = (data2 & 0xff );
}

//----------------------------------------------------
uint8_t checkFrameUART(uint8_t data[], uint8_t SizeFrame, uint8_t destination){
  unsigned int sum = 0;
  /* Kiểm tra địa chỉ */
  if( data[FRAME_DESTINATION] != destination)
    return 0;
  Serial.println("check đia chi: ok!"); 
  /* Kiểm tra checkSum */
  sum = checkSum(data, SizeFrame -2);
  if( (data[SizeFrame -2] != (sum >> 8)) || (data[SizeFrame -1] != (sum & 0xff)))
    return 0;
  Serial.println("check sum: ok!"); 
  /* Kiểm tra cấu trúc câu lệnh */
  if(data[FRAME_HEADER] == SYMBOL_HEADER_Serial){
    Serial.println("check header: ok!"); 
    if(( data[FRAME_START_DATA] == SYMBOL_START_DATA) && ( data[FRAME_SEPERATE] == SYMBOL_SEPERATE)&& ( data[FRAME_END] == SYMBOL_END)){
      Serial.println("check cau truc: ok!"); 
      return 1;
    }
    else {
      return 0;
    }
  }
}

//------------------------------------------------------
int checkSum (unsigned char checkSum[], byte Size){
  unsigned int sum = 0;
  for (byte c = 0; c < Size; c++){
    sum += checkSum[c];
    Serial.print("Sum: ");
    Serial.println(sum);
  }
  return sum;
 }
 //------------------------------------------------------
 ISR (TIMER1_OVF_vect) {
    _state = 1;
    switch(blinkLED){
      case LED_low:
        if ((countISR % 100) == 0)
          LED_ON;
        else
          LED_OFF;
        break;
      case LED_high:
          LED_ON;
        break;
      case LED_speed:
        if ((countISR % 100) == 0)
          LED_ON;
        else
          LED_OFF;
        break;
      case LED_slow:
        if ((countISR % 100) == 0)
          LED_ON;
        else
          LED_OFF;
        break;
      case LED_nhieu:
        break;
      case default:
        break;
    }
}
//-------------------------------------------------------
uint8_t waitting(byte times, int ms){

  unsigned int timeISR;
  /* Cài đặt các thông số Timer1 */      
  cli();                                    // Không cho phép ngắt toàn cục
  TCCR1A = 0;                                
  TCCR1B = 0;                               // thanh ghi lựa chọn xung nhịp 
  TIMSK1 = 0;                               // thanh ghi Interrupt Mask
  TCCR1B |= (1 << CS12) | (1 << CS10);      // thời gian tối đa (1024/16)us x 65536 = 4,192s
  timeISR = 65536-(ms/64)*1000;             

  TCNT1 = timeISR;
  TIMSK1 = (1 << TOIE1);                 // cho phép ngắt tràn (Overflow interrupt enable) 
  sei();                                 // cho phép ngắt toàn cục
  while(countISR < times){               // so sánh số lần lỗi
    if (_state == 1){                    // Over time
      countISR ++;
      TCNT1 = timeISR;
      TIMSK1 = (1 << TOIE1);                 // cho phép ngắt tràn (Overflow interrupt enable) 
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
//-------------------------------------------------------
void blinkLED(byte i){
  switch(i){
    case 0:
      LED_low;
      TIMSK0 = (0 << TOIE0);
      break;
    case 1:
      LED_high;
      TIMSK0 = (0 << TOIE0);
      break;
    case 2:
      TCNT0 = 0;
      TIMSK0 = (1 << TOIE0);                  // Overflow interrupt enable 
      break;
    case 3:
      TCNT0 = 0;
      TIMSK0 = (1 << TOIE0);                  // Overflow interrupt enable 
      break;
    case 4:
      TCNT0 = 0;
      TIMSK0 = (1 << TOIE0);                  // Overflow interrupt enable 
      break;
  }
}
//-------------------------------------------------------
void setTimer_0(){
  cli();                                  // tắt ngắt toàn cục  
  /* Reset Timer/Counter1 */
  TCCR0A = 0;
  TCCR0B = 0;
  TIMSK0 = 0;
  
  /* Setup Timer/Counter1 */
  TCCR0B |= (1 << CS02) | (1 << CS00);    // prescale = 1024
  TCNT0 = 0;
  TIMSK0 = (1 << TOIE0);                  // Overflow interrupt enable 
  sei();                                  // cho phép ngắt toàn cục
}
//----------------------------------------------------------
ISR (TIMER1_OVF_vect) 
{
    temp = analogRead(sensor);
    Serial.print(F("Temp:"));
    Serial.println(temp);
}
