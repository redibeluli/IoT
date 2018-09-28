#include <Arduino.h>le
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include<QueueList.h>
WebSocketsServer webSocket = WebSocketsServer(81);
#define VIBRATION_PLAYER_ONE 5 //D1
#define VIBRATION_PLAYER_TWO 4 //D2
//net is at pin #2     //D4
#define LED_LEFT 12     //D6
#define LED_RIGHT 13    //D7

const char* ssid     = "Dello-2.4";
const char* password = "Redi1992";


unsigned long interval = 2000;
unsigned long currentMillis = 0;
long delay_ms = 0;
volatile long net_arrival_time = 0;
int reset_time = 3000;
int max_pts = 11;
int sets = 1;
int current_set = 0;
int player1_score = 0;
int player2_score = 0;
int serve_count = 0;
int let = 0;
int games = 0;
int games_won1 = 0;
int games_won2 = 0;
int netPoint_A = 0;
int netPoint_B = 0;
double rally_count = 0;
float passivity_A;
float passivity_B;
float start_count;
float in, out;  //for pulsing LEDs at end of game
int num = 0;           //only update webserver once
int rally_net = 0;
const int netPin = 14;
volatile int netState = 0;
bool player1_turn = true;
bool player2_turn = false;
bool net_hit = false;
bool blinked_A = false;
bool blinked_B = false;
bool startGame = true;
bool printed_stats = false;
bool bounced_A = false;
bool bounced_B = false;
bool temp_net_state = false;

//******Stat variables******////
float total_ptsA = 0;
float total_ptsB = 0;

float serve_ptsA = 0;
float serve_ptsB = 0;
float lost_serve_ptsA = 0;
float lost_serve_ptsB = 0;
float Snd_ball_ptsA = 0;
float Snd_ball_ptsB = 0;
float Trd_ball_ptsA = 0;
float Trd_ball_ptsB = 0;








//*****************

QueueList <double> Rally_Q;

void setup() {


  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println(WiFi.localIP());
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);


  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);
  attachInterrupt(netPin, net_ISR, RISING);

}

void loop() {


  int scores[sets][2];

  while (games < sets) {

    if (startGame) {

      start_game();
      /* webSocket.loop();

        String scoreOne = String(player1_score);
        String scoreTwo = String(player2_score);
        webSocket.broadcastTXT("f" + scoreOne);
        delay(5);
        webSocket.broadcastTXT("s" + scoreTwo);
        startGame = false;
      */
    }
    delay(5);                      //avoid Wtd reset (watchdog timer)
    while ((player1_score < max_pts && player2_score < max_pts) || abs(player1_score - player2_score) < 2) {

      //update_scoreboard(client);
      delay(5);                    //watchdog timer

      if (player1_turn) {

        webSocket.loop();
        //Serial.println(net_hit);

        if (!blinked_A) {
          webSocket.broadcastTXT("A"); // l for left, indicates left side to serve
          send_scores();
          //Serial.println("Sending scores...");
          delay(reset_time);
          webSocket.broadcastTXT("noletA");
          blinc_A();
          blinked_A = true;

        }


        if (digitalRead(VIBRATION_PLAYER_ONE) == HIGH) {

          // net_hit = false;
          Serial.println("Bounce A");
          rally_count = 0;
          start_count = millis();
          if((micros() - net_arrival_time)/1000 < 53){
            net_hit = false;
          }
          let = 0;
          serve_count++;
          //delay(60); //let vibrations attenuate
          delay_ms = millis();
          while(millis() - delay_ms < 110){
            //wait
            Serial.println(net_hit);
          }
          Serial.print("net hit");
          Serial.println(net_hit);
          Serial.println((micros() - net_arrival_time)/1000);
          net_hit = false;
          Serial.println(net_hit);

          while (1) {
            currentMillis = millis();

            if (!Detect_B(currentMillis, rally_count)) {
              if (net_hit && rally_count == 0) {
                Serial.println("Let");
                net_hit = false;
                serve_count--;
                let = 1;
                webSocket.broadcastTXT("letA");
                break;
              }
              player2_score++;

              if (rally_count == 2)
                Snd_ball_ptsB++;



              if (rally_net == rally_count - 1 && net_hit == true) {
                Serial.println("netpoint B");
                netPoint_B++;
                net_hit = false;

              }

              Serial.println("rally_count: " + String(rally_count));
              Serial.println("rally_net: " + String(rally_net));
              rally_net = 0;
              net_hit = false;
              Serial.println("\t Point for B!");
              Serial.println("Player1 score: " + String(player1_score));
              Serial.println("Player2 score: " + String(player2_score));
              break;

            }
            passivity_A += (millis() - start_count);
            start_count = millis();

            rally_count++;

            currentMillis = millis();

            if (!Detect_A(currentMillis, rally_count)) {

              player1_score++;

              if (rally_count == 1) {
                serve_ptsA++;
                lost_serve_ptsB++;
                Serial.println("Serve Point A");
              }
              else if (rally_count == 3)
                Trd_ball_ptsA++;

              if (rally_net == rally_count - 1 && net_hit == true) {
                Serial.println("netpoint A");
                netPoint_A++;
                net_hit = false;

              }
              Serial.println("rally_count: " + String(rally_count));
              Serial.println("rally_net: " + String(rally_net));
              rally_net = 0;
              net_hit = false;
              Serial.println("\t Point for A!");
              Serial.println("Player1 score: " + String(player1_score));
              Serial.println("Player2 score: " + String(player2_score));
              break;
            }
            passivity_B += (millis() - start_count);
            start_count = millis();
            rally_count++;


          }

          if (let == 0)
            Rally_Q.push(rally_count);

          blinked_A = false;
          Serial.println(rally_count);
          if (let == 0) {
            if (serve_count % 2 == 0) {    //decide who serves
              player1_turn = false;
              player2_turn = true;

            }
            if (player1_score >= max_pts - 1 && player2_score >= max_pts - 1) {
              player1_turn = false;
              player2_turn = true;
            }
          }
        }
      }

      if (player1_score >= max_pts && player1_score > player2_score + 1) {
        Serial.println();
        Serial.println("***Player1 wins!!***");
        delay(1000);
        break;
      }
      if (player2_score >= max_pts && player2_score > player1_score + 1) {
        Serial.println();
        Serial.println("***Player2 wins!!***");
        delay(1000);
        break;
      }
      //****************************************************PLAYER TWO CODE**************************************************************
      if (player2_turn) {

        if (!blinked_B) {
          webSocket.broadcastTXT("B");
          send_scores();
          delay(reset_time);
          webSocket.broadcastTXT("noletB");
          blinc_B();
          blinked_B = true;

        }

        if (digitalRead(VIBRATION_PLAYER_TWO) == HIGH) {

          Serial.println("Bounce B");        
          rally_count = 0;
          start_count = millis();
          if((micros() - net_arrival_time)/1000 < 53){
            net_hit = false;
          }
          let = 0;
          serve_count++;
          delay_ms = millis();
          while(millis() - delay_ms < 110){
            //wait
          }

          Serial.print("net hit");
          Serial.println(net_hit);


          while (1) {
            currentMillis = millis();

            if (!Detect_A(currentMillis, rally_count)) {
              if (net_hit && rally_count == 0) {
                Serial.println("Let");
                net_hit = false;
                serve_count--;
                let = 1;
                webSocket.broadcastTXT("letB");
                break;
              }
              player1_score++;

              if (rally_count == 2)
                Snd_ball_ptsA++;

              if (rally_net == rally_count - 1 && net_hit == true) {
                Serial.println("netpoint A");
                netPoint_A++;
                net_hit = false;

              }
              Serial.println("rally_count: " + String(rally_count));
              Serial.println("rally_net: " + String(rally_net));
              rally_net = 0;
              net_hit = false;

              Serial.println("\t Point for A!");
              Serial.println("Player1 score: " + String(player1_score));
              Serial.println("Player2 score: " + String(player2_score));
              break;
            }
            passivity_B += (millis() - start_count);
            start_count = millis();
            rally_count++;
            currentMillis = millis();

            if (!Detect_B(currentMillis, rally_count)) {
              player2_score++;

              if (rally_count == 1) {
                serve_ptsB++;
                lost_serve_ptsA++;
                Serial.println("Serve Point B");
              }
              else if (rally_count == 3)
                Trd_ball_ptsB++;

              if (rally_net == rally_count - 1 && net_hit == true) {
                Serial.println("netpoint B!!");
                netPoint_B++;
                net_hit = false;

              }
              Serial.println("rally_count: " + String(rally_count));
              Serial.println("rally_net: " + String(rally_net));
              rally_net = 0;
              net_hit = false;
              Serial.println("\t Point for B!");
              Serial.println("Player1 score: " + String(player1_score));
              Serial.println("Player2 score: " + String(player2_score));
              break;
            }
            passivity_A += (millis() - start_count);
            start_count = millis();
            rally_count++;

          }

          if (let == 0)
            Rally_Q.push(rally_count);

          Serial.println(rally_count);
          blinked_B = false;
          if (let == 0) {
            if (serve_count % 2 == 0) {
              player1_turn = true;
              player2_turn = false;
            }
            if (player2_score >= max_pts - 1 && player1_score >= max_pts - 1) {
              player1_turn = true;
              player2_turn = false;
            }
          }
        }

      }

      if (player1_score >= max_pts && player1_score > player2_score + 1) {
        Serial.println();
        Serial.println("***Player1 wins!!***");
        delay(1000);
        break;
      }
      if (player2_score >= max_pts && player2_score > player1_score + 1) {
        Serial.println();
        Serial.println("***Player2 wins!!***");
        delay(1000);
        break;
      }

    }

    for (int x = current_set; x < current_set + 1; x++) {
      for (int y = 0; y < 2; y++) {
        if (y == 0)
          scores[x][y] = player1_score;
        else if (y == 1)
          scores[x][y] = player2_score;
      }
    }

    if (player1_score > player2_score)
      games_won1++;
    else
      games_won2++;

    send_scores();        //send final scores


    total_ptsA += player1_score;
    total_ptsB += player2_score;
    games++;
    current_set++;
    player1_score = 0;
    player2_score = 0;
    player1_turn = true;
    player2_turn = false;
    serve_count = 0;

  }

  if (!printed_stats) {

    Serial.println();
    print_stats();                  //rallies and net points
    Serial.println("Game Scores:");
    print_scores(scores);
    printed_stats = true;
  }

  end_game();

}
//************************************************FUNCTIONS******************************************************************
boolean Detect_B(long currentMillis, int rally_count) {
  

  for (int x = 0; x < 2000; x++) {
      
    temp_net_state = net_hit;
      //Serial.println(temp_net_state);
      
    if (digitalRead(VIBRATION_PLAYER_ONE) == HIGH) {
      Serial.println("Bounce A");
      delay(110);
      if (net_hit && rally_count < 1)
        net_hit = false;

      return false;
    }
    //Serial.println(x);
    delay(1);

    if (digitalRead(VIBRATION_PLAYER_TWO) == HIGH && (millis() - currentMillis) <= interval) {
      if (net_hit && rally_count == 0){
        //Serial.println("here 1");
        if( (micros() - net_arrival_time)/1000 < 53){
          net_hit = temp_net_state;
          //Serial.println("here2");
        }
        //Serial.print("net_hit: ");
        //Serial.println(net_hit);
        if(net_hit){
          //Serial.println("here3");
          return false;
        }
      }

      Serial.println("Bounce B");
      delay(110);
      return true;
    }
  }
  if (net_hit)
    net_hit = false;

  passivity_A += 550;                   //ball off table
  return false;
}

boolean Detect_A(long currentMillis, int rally_count) {
  temp_net_state = net_hit;
  for (int x = 0; x < 2000; x++) {

    if (digitalRead(VIBRATION_PLAYER_TWO) == HIGH) {
      Serial.println("Bounce B");
      delay(110);
      if (net_hit && rally_count < 1)
        net_hit = false;

      return false;
    }
    delay(1);
    if (digitalRead(VIBRATION_PLAYER_ONE) == HIGH && (millis() - currentMillis) <= interval) {
      if (net_hit && rally_count == 0){
          if( (micros() - net_arrival_time)/1000 < 53){
          net_hit = temp_net_state;
          //Serial.println("here2");
          }
        if(net_hit){
          return false;
        }
      }

      Serial.println("Bounce A");
      delay(110);
      return true;
    }
  }
  if (net_hit)
    net_hit = false;

  passivity_B += 550;
  return false;
}

void print_stats() {

  Serial.println("total points A: " + String(total_ptsA));
  Serial.println("total points B: " + String(total_ptsB));

  Serial.println("P1 NetPoints: " + String(netPoint_A));
  Serial.println("P2 NetPoints: " + String(netPoint_B));
  Serial.println("P1 Serve Pts raw: " + String(serve_ptsA));
  Serial.println("P2 Serve Points raw: " + String(serve_ptsB));
  Serial.println("P1 ServePoints: " + String(serve_ptsA / total_ptsA * 100, 1) + "%");
  Serial.println("P2 ServePoints: " + String(serve_ptsB / total_ptsB * 100, 1) + "%");
  Serial.println("P1 failed serve return: " + String(serve_ptsB / total_ptsB * 100, 1) + "%");
  Serial.println("P2 failed serve return: " + String(serve_ptsA / total_ptsA * 100, 1) + "%");
  Serial.println();
  Serial.println("P1 2nd Ball: " + String(Snd_ball_ptsA / total_ptsA * 100, 1) + "%");
  Serial.println("P2 2nd Ball: " + String(Snd_ball_ptsB / total_ptsB * 100, 1) + "%"); 
  Serial.println("P1 2nd Ball lost: " + String(Snd_ball_ptsB / total_ptsB * 100, 1) + "%");
  Serial.println("P2 2nd Ball lost: " + String(Snd_ball_ptsA / total_ptsA * 100, 1) + "%");
  
  Serial.println("P1 3rd Ball: " + String(Trd_ball_ptsA / total_ptsA * 100, 1) + "%");
  Serial.println("P2 3rd Ball: " + String(Trd_ball_ptsB / total_ptsB * 100, 1) + "%");
  Serial.println("P1 3rd Ball lost: " + String(Trd_ball_ptsB / total_ptsB * 100, 1) + "%");
  Serial.println("P2 3rd Ball lost: " + String(Trd_ball_ptsA / total_ptsA * 100, 1) + "%");
  
  

  Serial.println(String(passivity_A / 1000, 3) + " Airtime (S)");
  Serial.println(String(passivity_B / 1000, 3) + " Seconds in air (S)");

  double total = 0;
  double num = 0;
  int maximum = 0;
  double item = 0;

  while (!Rally_Q.isEmpty()) {
    item = Rally_Q.pop();
    total += item;
    if (item > maximum)
      maximum = item;
    num++;
  }
  Serial.println("total hits(sum of rallies): " + String(total));
  Serial.println("Airtime per return A: " + String((passivity_A / 1000) / total, 3));
  Serial.println("Airtime per return B: " + String((passivity_B / 1000) / total, 3));

  Serial.println("Longest rally: " + String(maximum));
  Serial.println("Average Rally: " + String(total / num));

  webSocket.loop();
  webSocket.broadcastTXT("displayStatsA");
  webSocket.broadcastTXT("nA" + String(netPoint_A));
  webSocket.broadcastTXT("sA" + String(serve_ptsA / total_ptsA * 100, 1));
  webSocket.broadcastTXT("fsA" + String(serve_ptsB / total_ptsB * 100, 1));
  webSocket.broadcastTXT("atA" + String((passivity_A / 1000) / total, 3));
  webSocket.broadcastTXT("lRA" + String(maximum));
  webSocket.broadcastTXT("2nA" + String(Snd_ball_ptsA / total_ptsA * 100, 1)); 
  webSocket.broadcastTXT("2lA" + String(Snd_ball_ptsB / total_ptsB * 100, 1));
  webSocket.broadcastTXT("3rA" + String(Trd_ball_ptsA / total_ptsA * 100, 1)); 
  webSocket.broadcastTXT("3lA" + String(Trd_ball_ptsB / total_ptsB * 100, 1));
 
  webSocket.broadcastTXT("displayStatsB");
  webSocket.broadcastTXT("nB" + String(netPoint_B));
  webSocket.broadcastTXT("sB" + String(serve_ptsB / total_ptsB * 100, 1));
  webSocket.broadcastTXT("fsB" + String(serve_ptsA / total_ptsA * 100, 1));
  webSocket.broadcastTXT("atB" + String((passivity_B / 1000) / total, 3));
  webSocket.broadcastTXT("lRB" + String(maximum));
  webSocket.broadcastTXT("2nB" + String(Snd_ball_ptsB / total_ptsB * 100, 1));
  webSocket.broadcastTXT("2lB" + String(Snd_ball_ptsA / total_ptsA * 100, 1));
  webSocket.broadcastTXT("3rB" + String(Trd_ball_ptsB / total_ptsB * 100, 1));
  webSocket.broadcastTXT("3lB" + String(Trd_ball_ptsA / total_ptsA * 100, 1));





}

void send_scores() {

  webSocket.loop();
  Serial.println("Sending scores...");
  String game1 = String(games_won1);
  String game2 = String(games_won2);
  webSocket.broadcastTXT("AA" + game1);
  delay(5);
  webSocket.broadcastTXT("BB" + game2);
  String scoreOne = String(player1_score); //send final scores
  String scoreTwo = String(player2_score);
  webSocket.broadcastTXT("fi" + scoreOne);
  delay(5);
  webSocket.broadcastTXT("se" + scoreTwo);

}

void print_scores(int scores[][2]) {
  for (int x = 0; x < current_set; x++) {
    for (int y = 0; y < 2; y++) {
      Serial.println(scores[x][y]);
    }
    Serial.println();
  }
}

void net_ISR() {
   net_arrival_time = micros();
   Serial.println("N");
  delayMicroseconds(47000); //80000 previously
  //if (digitalRead(VIBRATION_PLAYER_ONE) != HIGH && digitalRead(VIBRATION_PLAYER_TWO) != HIGH) {
    net_hit = true;
    rally_net = rally_count;
  //}

}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  switch (type) {
    case WStype_DISCONNECTED:

      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);


      }
      break;
    case WStype_TEXT:
      {

        String text = String((char *) &payload[0]);
        if (text == "LED") {

          webSocket.sendTXT(num, "led just lit", lenght);
        }

        if (text.startsWith("x")) {

          webSocket.sendTXT(num, "red changed", lenght);
        }


        if (text.startsWith("y")) {



          webSocket.sendTXT(num, "green changed", lenght);
        }

        if (text.startsWith("z")) {

          webSocket.sendTXT(num, "blue changed", lenght);
        }
        if (text == "RESET") {


          Serial.println("reset");

        }
      }


      webSocket.sendTXT(num, payload, lenght);
      webSocket.broadcastTXT(payload, lenght);
      break;


    case WStype_BIN:

      hexdump(payload, lenght);

      // echo data back to browser
      webSocket.sendBIN(num, payload, lenght);
      break;
  }
}

void blinc_A() {
  digitalWrite(LED_RIGHT, HIGH);
  delay(150);
  digitalWrite(LED_RIGHT, LOW);
  delay(150);
  digitalWrite(LED_RIGHT, HIGH);
  delay(150);
  digitalWrite(LED_RIGHT, LOW);

}
void blinc_B() {

  digitalWrite(LED_LEFT, HIGH);
  delay(150);
  digitalWrite(LED_LEFT, LOW);
  delay(150);
  digitalWrite(LED_LEFT, HIGH);
  delay(150);
  digitalWrite(LED_LEFT, LOW);
}

void start_game() {

  delay(1000);
  digitalWrite(LED_LEFT, HIGH);
  delay(200);
  digitalWrite(LED_LEFT, LOW);
  delay(20);
  digitalWrite(LED_RIGHT, HIGH);
  delay(200);
  digitalWrite(LED_RIGHT, LOW);
  delay(800);
  digitalWrite(LED_RIGHT, HIGH);
  delay(200);
  digitalWrite(LED_RIGHT, LOW);
  delay(20);
  digitalWrite(LED_LEFT, HIGH);
  delay(200);
  digitalWrite(LED_LEFT, LOW);
  delay(800);

  for (int x = 0; x < 3; x++) {
    digitalWrite(LED_LEFT, HIGH);
    delay(150);
    digitalWrite(LED_LEFT, LOW);
    digitalWrite(LED_RIGHT, HIGH);
    delay(150);
    digitalWrite(LED_RIGHT, LOW);
  }

}

void end_game() {

/*  for (in = 0; in < 6.283; in = in + 0.0004)
  {
    out = sin(in) * 127.5 + 127.5;
    analogWrite(LED_LEFT,out);
    analogWrite(LED_RIGHT,out);
    
   // in = in + .001 * out /255;
  }
*/
  digitalWrite(LED_LEFT, HIGH);
  delay(700);
  digitalWrite(LED_LEFT, LOW);
  delay(20);
  digitalWrite(LED_RIGHT, HIGH);
  delay(700);
  digitalWrite(LED_RIGHT, LOW);
  

}



















