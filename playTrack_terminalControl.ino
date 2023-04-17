//Pin connected to latch pin (RCLK pin 12) of 74HC595:  SR pin 12 <-> Arduino Digital 3 
const int latchPin = 3; 
//Pin connected to clock pin (SRCLK pin 11) of 74HC595: SR pin 11 <-> Arduino Digital 4 
const int clockPin = 4; 
//Pin connected to data pin (SER pin 14) of 74HC595:    SR pin 14 <-> Arduino Digital 5 
const int dataPin = 5; 

const int DELAYTIME = 500; //in milliseconds

char noteNum[][4] = {"", "", "", "", "", "", "", "", "", "", "", "", 
                    "C0", "C0#", "D0", "D0#", "E0", "F0", "F0#", "G0", "G0#", "A0", "A0#", "B0", 
                    "C1", "C1#", "D1", "D1#", "E1", "F1", "F1#", "G1", "G1#", "A1", "A1#", "B1", 
                    "C2", "C2#", "D2", "D2#", "E2", "F2", "F2#", "G2", "G2#", "A2", "A2#", "B2", 
                    "C3", "C3#", "D3", "D3#", "E3", "F3", "F3#", "G3", "G3#", "A3", "A3#", "B3", 
                    "C4", "C4#", "D4", "D4#", "E4", "F4", "F4#", "G4", "G4#", "A4", "A4#", "B4", 
                    "C5", "C5#", "D5", "D5#", "E5", "F5", "F5#", "G5", "G5#", "A5", "A5#", "B5", 
                    "C6", "C6#", "D6", "D6#", "E6", "F6", "F6#", "G6", "G6#", "A6", "A6#", "B6", 
                    "C7", "C7#", "D7", "D7#", "E7", "F7", "F7#", "G7", "G7#", "A7", "A7#", "B7", 
                    "C8", "C8#", "D8", "D8#", "E8", "F8", "F8#", "G8", "G8#", "A8", "A8#", "B8", 
                    "C9", "C9#", "D9", "D9#", "E9", "F9", "F9#", "G9"}; 

//NoteToSr: SrId, SrPin
struct NoteToSr{
  int SrId; // Shift register ID (eight registers indexed from 1 to 8)
  int SrPin; // Shift register pin number 
}; 

//25 keys (solenoids) in total. 
//                             C      C#     D      D#     E      F      F#     G      G#     A      A#     B
struct NoteToSr noteToSr[] = {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, //
                              {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, //0
                              {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, //1
                              {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, //2
                              {3,1}, {0,0}, {3,4}, {3,5}, {3,6}, {4,0}, {0,0}, {4,3}, {4,2}, {4,5}, {0,0}, {5,0}, //3
                              {0,0}, {4,7}, {0,0}, {5,1}, {5,4}, {5,6}, {5,3}, {6,0}, {5,5}, {6,3}, {5,7}, {6,1}, //4
                              {6,5}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, //5
                              {7,0}, {7,1}, {7,2}, {7,3}, {7,4}, {7,5}, {7,6}, {7,7}, {0,0}, {0,0}, {0,0}, {0,0}, //6
                              {8,0}, {8,1}, {8,2}, {8,3}, {8,4}, {8,5}, {8,6}, {8,7}, {0,0}, {0,0}, {0,0}, {0,0}, //7
                              {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, //8
                              {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}};                            //9

// store serial port data into data[] 
const uint8_t numChars = 36; 
char data[numChars]; static uint8_t pData = 0; 
// store data that is transmitted to the shift register
// As shift registers are indexed from 1, chordToPlay[0] is meaningless. 
uint8_t chordToPlay[] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; 
//SoftwareSerial mySerial = SoftwareSerial(); 

void readLine(char *s); 
void initChord(); 
void readChord(String data); 
void setChord(int noteNum); 
void outputChord(); 
void testChord(); 
int pitchClass(char note); 
bool cmpString(char* s1, char* s2); 
void MSBShiftOut(int myDataPin, int myClockPin, byte myDataOut); 

void setup() {
  // put your setup code here, to run once:
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin,  OUTPUT); 

  Serial.begin(9600);
  Serial.println("reset");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Reading: "); 
  readLine(); 
  if(cmpString(data, "start")){
    Serial.println("Start reading.\n"); 
    while(!(cmpString(data, "end"))){// enter a chord
      initChord(); 
      readChord(); 
      if(cmpString(data, "end"))break; 
      outputChord(); 
      testChord(); 
    }
    initChord(); 
    outputChord(); 
  }
}

void testChord(){
  Serial.println("chordToPlay array data: "); 
    for(int i = 1; i <= 8; i++){
      Serial.print("Shift register "); Serial.print(i); Serial.print(": "); 
      Serial.println(chordToPlay[i], BIN); 
    }

  Serial.println("<chordToPlay> data: ");
  Serial.println(chordToPlay[1]); 
  Serial.println(chordToPlay[2]); 
  Serial.println(chordToPlay[3]); 
  Serial.println(chordToPlay[4]); 
  Serial.println(chordToPlay[5]); 
  Serial.println(chordToPlay[6]); 
  Serial.println(chordToPlay[7]); 
  Serial.println(chordToPlay[8]); 
  Serial.println(); 
}

// read data from serial port into data[] 
// currently read maximum of 8 notes in a line
void readLine(){ 
  pData = 0; 
  bool endOfLine = false; 
  char endMarker = '\n';
  char rc;

  while (endOfLine == false) {
    while(!Serial.available());
    rc = Serial.read();

    if (rc != endMarker) {
      data[pData++] = rc;
      if (pData >= numChars) {
          pData = numChars - 1;
      }
    } else {
      data[pData] = '\0'; // terminate the string
      endOfLine = true;
    }
  }
}

void initChord(){
  memset(chordToPlay, 0, sizeof(chordToPlay)); 
}

void readChord(){
  readLine();  
  int i = 0, noteNum = 0; 

  for(i = 0; i < pData; i++){
    if(isPitchClass(data[i])){ //whether data[i] == 'C', 'D', ...
      noteNum = getNoteNum(i); 
      setChord(noteNum); 
    }
  }
}

bool isPitchClass(char note){
  if((note == 'C') || (note == 'D') || (note == 'E') || (note == 'F') || (note == 'G') || (note == 'A') || (note == 'B'))return true; 
  return false; 
}

void setChord(int noteNum){
  int SrId = noteToSr[noteNum].SrId; 
  int SrPin = noteToSr[noteNum].SrPin; 
  bitWrite(chordToPlay[SrId], SrPin, HIGH); 

  /*
  Serial.print("Note number: "); Serial.println(noteNum); 
  Serial.print("Shift register ID:"); Serial.println(noteToSr[noteNum].SrId); 
  Serial.print("Shift register pin number:"); Serial.println(noteToSr[noteNum].SrPin); 
  Serial.print("Shift register data:"); Serial.println(chordToPlay[SrId]); 
  */
}

int getNoteNum(int i){
  int j, length = (data[i+2] == ' ')?2:3; 
  char note[4]; 
  strncpy(note, data + i, length); note[length] = '\0'; 

  for(j = 0; j < 125; j++){
    if(cmpString(noteNum[j], note)) return j; 
  }
}

// every line is a chord
void outputChord(){
  digitalWrite(latchPin, LOW); 

  MSBShiftOut(dataPin, clockPin, chordToPlay[1]); 
  MSBShiftOut(dataPin, clockPin, chordToPlay[2]); 
  MSBShiftOut(dataPin, clockPin, chordToPlay[3]); 
  MSBShiftOut(dataPin, clockPin, chordToPlay[4]); 
  MSBShiftOut(dataPin, clockPin, chordToPlay[5]); 
  MSBShiftOut(dataPin, clockPin, chordToPlay[6]); 
  MSBShiftOut(dataPin, clockPin, chordToPlay[7]); 
  MSBShiftOut(dataPin, clockPin, chordToPlay[8]); 

  digitalWrite(latchPin, HIGH); 
}

int getLength(char* s){
  int len = 0; 
  while(s[len] != '\0') len++; 
  return len; 
}

bool cmpString(char* s1, char* s2){
  //Serial.println("In cmpString(char*, char*)"); 
  //Serial.print("Comparing: "); Serial.print(s1); Serial.print(" AND "); Serial.println(s2);  

  int len1 = getLength(s1); 
  int len2 = getLength(s2);  

  if(len1 != len2) return false; 

  int i = 0; 
  for(i = 0; i < len1; i++){
    if(s1[i] != s2[i]) return false; 
  }
  return true; 
}

void MSBShiftOut(int myDataPin, int myClockPin, byte myDataOut){ //MSBFIRST
  int pinState;
  digitalWrite(myDataPin, LOW);
  digitalWrite(myClockPin, LOW);

  for (int i = 7; i >= 0; i--){
    digitalWrite(myClockPin, LOW);
    if(myDataOut & (1<<i)) pinState = 1;
    else pinState = 0; 
    digitalWrite(myDataPin, pinState);
    digitalWrite(myClockPin, HIGH);
    digitalWrite(myDataPin, LOW);
  }
  
  digitalWrite(myClockPin, LOW);
}
