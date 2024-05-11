#include <SoftwareSerial.h>
#include <string.h>

void (*restFun)(void)=0; // Pointer to Function Points to the start address to rest the Arduino nano 

SoftwareSerial A9GSerial(10, 11); // RX, TX
char RecivedMessage[100];


void setup() {

    Serial.begin(115200);
    A9GSerial.begin(2400);
    delay(5000);
   initializeA9G();
   
    while(attachGPRS()!= true );
    while(checkNetworkRegistration()!= true );
    while(ConnectToSErver()!= true ) {
      sendATCommand("AT+RST"); 
      restFun();
    }
    while(connectToInternet()!= true );

    sendATCommand("AT+GPS=1"); // Activating GPS module 
    delay(20000);
}

////////////////////////////////////////superLoop///////////////////////////////////////////////////
void loop() {


Serial.println("This the gps read data :");
sendATCommand("AT+LOCATION=2");                       //gets lan and long data from the gps 
delay(2000);
              // cheak if the GPS is Atached or not if not dont send Garbage value to Firebasse 
  if(strstr(RecivedMessage ,"GPS NOT FIX NOW") != NULL  || strstr(RecivedMessage ,"+CME ERROR: The command not support") != NULL ) {
  CleartheRecviedMessage();
  }
  //do nothing  
  else  { 
   
  String Location = String(RecivedMessage);
  Location = Location.substring(0,22); 
  
  Serial.println(Location);
  sendDataToFirebase(Location); 
  
  }

  CleartheRecviedMessage();  
  delay(5000);
}

////////////////////////////////////////////Functions///////////////////////////////////////////////
/* this function starts the A9G with full functionalthy*/
/*  takes nothing */
/*returns nothing*/ 
void initializeA9G() {

    sendATCommand("AT");
    delay(27000);
    sendATCommand("AT+CFUN=1");  // Enable full functionality (disable airplane mode)
    delay(5000);
    sendATCommand("AT+IPR=2400");
    //delay(5000);
    A9GSerial.begin(2400);
    sendATCommand("AT+CGSN");
    sendATCommand("ATE0");
}
//clearing the recived buffer  
void CleartheRecviedMessage() {
  for (int j = 0; j<100; j++)
    RecivedMessage[j] = 0;
    delay(5);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
bool checkNetworkRegistration() {
    sendATCommand("AT+CREG?");
    delay(1000);

    // Check the response for network registration status

    if (strstr(RecivedMessage , "+CREG: 1,1")!=NULL || strstr(RecivedMessage ,"+CREG: 0,5")!=NULL || strstr(RecivedMessage ,"OK")!=NULL ) {
        CleartheRecviedMessage();
        return true;
    } else {
      CleartheRecviedMessage();
        return false;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//attaching GPRS Services 
bool attachGPRS() {
    sendATCommand("AT+CGATT=1");
    delay(5000);

    // Check if GPRS is attached
    if ((strstr(RecivedMessage ,"+CGATT:1") !=NULL ) /*|| (strstr(RecivedMessage ,"OK") !=NULL)*/ )  { //A9GSerial.find("+CGATT: 1") || A9GSerial.find("OK")) {
        CleartheRecviedMessage();
        return true;
    } else {
        CleartheRecviedMessage();
        return false;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
bool connectToInternet() {
    delay(2000);
    sendATCommand("AT+CIFSR");
      
    delay(2000);

    sendATCommand("AT+CGACT=1,1");
    delay(5000);

    // Check if PDP activation is successful
    if (strstr(RecivedMessage ,"OK") != NULL) {
        CleartheRecviedMessage();
        return true;
    } else {
        CleartheRecviedMessage();
        return false;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void sendDataToFirebase(String Location) {

    String Location_data = "AT+HTTPPOST=\"https://tracking-app-10503-default-rtdb.firebaseio.com/data.json\",\"text/plain\",""""\""+Location+"\"";
    sendATCommand(String(Location_data));
    delay(10000);
    CleartheRecviedMessage();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////


void sendATCommand(String command) {

    A9GSerial.println(command);
    A9GSerial.flush();
    delay(200);

    Serial.print("Command: ");
    Serial.println(command);
    int i =0;
    while (A9GSerial.available()) {
       RecivedMessage[i] = A9GSerial.read();
        Serial.print(RecivedMessage[i]);
        i++;
        delay(20);
    }
    
 
    Serial.println();
}

bool ConnectToSErver() {    //Conecting to Service Provider  etisalat 

  sendATCommand("AT+CGDCONT=1,\"IP\",\"CITMOI\"");
  delay(2000);
  
  if(strstr(RecivedMessage ,"OK") !=NULL) {
      CleartheRecviedMessage();
      return true ;
}
     else { 
       CleartheRecviedMessage();
       return false ;
     }

}

