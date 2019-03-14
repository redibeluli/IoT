#include <Arduino.h>le
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <string>
#include<QueueList.h>
#include <player.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>



WebSocketsServer webSocket = WebSocketsServer(81);

#define VIBRATION_PLAYER_ONE 5 //D1
#define VIBRATION_PLAYER_TWO 4 //D2
//net is at pin #2     //D4
#define LED_LEFT 12     //D6
#define LED_RIGHT 13    //D7




unsigned long interval = 2000;
unsigned long currentMillis = 0;
long bounce_time = 0;
long delay_ms = 0;
long bounce_delay = 0;
long old_bounce_delay = 0;
long new_bounce = 0;
long old_bounce = 0;
int bounce_counter = 0;
int bounce_timer = 900;

long net_arrival_time = 0;
int reset_time = 3000;
int max_pts = 11;
int sets = 1;
int current_set = 0;
int serve_count = 0;
int flip_serve = 2;
int let = 0;
int int_pts = 0;
int games = 0;
int netPoint_A = 0;
int netPoint_B = 0;
int wonA = 0;
int web_sessions = 0;

int wonB = 0;
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
bool connected_ = false;
bool disconnected = false;
bool game_reset = false;
bool flipped = false;
bool point_changed = false;
bool point_subtrA = false;
bool point_subtrB = false;
bool slaughter = false;
bool sent_mercy = false;
bool autoSwitch = false;
bool logged_in = false;

//******Stat variables******////
float total_pointsA = 0;
float total_pointsB = 0;

float serve_ptsA = 0;
float serve_ptsB = 0;
float lost_serve_ptsA = 0;
float lost_serve_ptsB = 0;
float Snd_ball_ptsA = 0;
float Snd_ball_ptsB = 0;
float Trd_ball_ptsA = 0;
float Trd_ball_ptsB = 0;

int serve_fltA = 0;
int serve_fltB = 0;

int Aces_A = 0;
int Aces_B = 0;
int air_ballA = 0;
int air_ballB = 0;

int game_ptsA_won = 0;
int game_ptsA_lost = 0;
int game_ptsB_won = 0;
int game_ptsB_lost = 0;

WiFiManager wifiManager;
player player1;
player player2;

//*****************

QueueList <double> Rally_Q;

void setup() {


  Serial.begin(115200);
  
  //WiFi.mode(WIFI_AP_STA);
  //wifiManager.resetSettings();
  
  wifiManager.autoConnect("AutoConnectAP");
  //WiFi.begin(ssid, password);

  

 // while (WiFi.status() != WL_CONNECTED) {
   // delay(100);
  //}
  
  Serial.println(WiFi.localIP());
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);


  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);
  attachInterrupt(netPin, net_ISR, RISING);
   

  
  Serial.println(player1.Aces);
  Serial.println(player1.turn);

  player1.turn = true;

}

void loop() {


  int scores[sets][2];

  while (games < sets) {

    if (game_reset)
      break;

    if (startGame) {
      start_game();
    }
    delay(5);                      //avoid Wtd reset (watchdog timer)
    while ((player1.score < max_pts && player2.score < max_pts) || abs(player1.score - player2.score) < 2) {

      //update_scoreboard(client);
      delay(5);                    //watchdog timer

      if (player1.turn) {


        if (game_reset)
          break;

        if (disconnected) {
          delay(5);
          webSocket.loop();
          blink_Disconnected();
          //Serial.println("disconnected");
          delay(5);
        }

        if(connected_){
          digitalWrite(LED_RIGHT,HIGH);
          connected_ = false;
        }
        webSocket.loop();
        //Serial.println(net_hit);

        if (!player1.blinked && !player2.turn) {
          webSocket.broadcastTXT("A"); // l for left, indicates left side to serve

          send_scores();
          Serial.println("Scores sent from Player 1");


          digitalWrite(LED_LEFT, LOW);

          while(bounce_timer > 0){
            delay(1);
            
            if(digitalRead(VIBRATION_PLAYER_ONE) == HIGH || digitalRead(VIBRATION_PLAYER_TWO) == HIGH){
              bounce_timer = 900;
            }           
            bounce_timer -= 1;
            //Serial.println(bounce_timer);
            
          }

           bounce_timer = 900;

          for(int i = 0; i < reset_time; i++){
            webSocket.loop();

            if(flipped || game_reset)
              break;

            if(digitalRead(VIBRATION_PLAYER_ONE) == HIGH){
              while(bounce_timer > 0){
                delay(1);
                
                if(digitalRead(VIBRATION_PLAYER_ONE) == HIGH){
                  bounce_timer = 900;
                }           
                bounce_timer -= 1;
                //Serial.println(bounce_timer);
                
              }
              break;
            }
            delay(1);    
          }

          bounce_timer = 900;
          bounce_counter = 0;
          //delay(reset_time);
          webSocket.broadcastTXT("noletA");
          blinc_A();
          delay(150);
          player1.blinked = true;

        }

        /*
        Serial.println("player 1");
        Serial.print("serve count: ");
        Serial.println(serve_count);
        Serial.print("point changed: ");
        Serial.println(point_changed);
        Serial.print("sub A: ");
        Serial.println(point_subtrA);
        Serial.print("sub B: ");
        Serial.println(point_subtrB);
        */
        /*
        Serial.print("P1 airball: ");
        Serial.println(air_ballA);
        Serial.print("P2 airball: ");
        Serial.println(air_ballB);
        */
        
        
        if (digitalRead(VIBRATION_PLAYER_ONE) == HIGH) {
          bounce_time = micros();
          //cli();
         
          // net_hit = false;
          Serial.println("Bounce A");
          rally_count = 0;
          start_count = millis();
          //if ((micros() - net_arrival_time) / 1000 < 53) {
            //net_hit = false;
          //}
          let = 0;
          serve_count++;
          //delay(60); //let vibrations attenuate
          delay(15);
          net_hit = false;
          delay_ms = millis();
          while (millis() - delay_ms < 95) {
            //wait
            Serial.println(net_hit);
          }
          Serial.print("net hit");
          Serial.println(net_hit);
          //Serial.println((micros() - net_arrival_time) / 1000);
          //net_hit = false;
         // Serial.println(net_hit);
          
          digitalWrite(LED_RIGHT, LOW);
          
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
              player2.score++;

              if(player1.score == max_pts - 1){          
                if(rally_count >= 2){
                  player2.game_pts_won++;
                  player1.game_pts_lost++;
                  Serial.println("Game point A broken");
                }
                else if(rally_count == 0){                
                  Serial.println("Game point A lost");
                  player1.game_pts_lost++;
                }                
              }
             

              if (rally_count == 2)
                player2.Snd_ball_pts++;



              if (rally_net == rally_count - 1 && net_hit == true) {
                Serial.println("netpoint B");
                player2.netpoints++;
                net_hit = false;

              }

              Serial.println("rally_count: " + String(rally_count));
              Serial.println("rally_net: " + String(rally_net));
              rally_net = 0;
              net_hit = false;
              Serial.println("\t Point for B!");
              Serial.println("Player1 score: " + String(player1.score));
              Serial.println("Player2 score: " + String(player2.score));
              break;

            }
            player1.passivity += (millis() - start_count);
            start_count = millis();

            rally_count++;

            currentMillis = millis();

            if (!Detect_A(currentMillis, rally_count)) {

              player1.score++;

              if(player2.score == max_pts - 1){
                Serial.println("Game point B broken");
                player2.game_pts_lost++;
                player1.game_pts_won++;
                               
              }
              

              if (rally_count == 1) {
                player1.serve_pts++;
                player2.lost_serve_pts++;
                Serial.println("Serve Point A");
              }
              else if (rally_count == 3)
                player1.Trd_ball_pts++;

              if (rally_net == rally_count - 1 && net_hit == true) {
                Serial.println("netpoint A");
                player1.netpoints++;
                net_hit = false;

              }
              Serial.println("rally_count: " + String(rally_count));
              Serial.println("rally_net: " + String(rally_net));
              rally_net = 0;
              net_hit = false;
              Serial.println("\t Point for A!");
              Serial.println("Player1 score: " + String(player1.score));
              Serial.println("Player2 score: " + String(player2.score));
              break;
            }
            player2.passivity += (millis() - start_count);
            start_count = millis();
            rally_count++;


          }

          if (let == 0)
            Rally_Q.push(rally_count);

          player1.blinked = false;
          Serial.println(rally_count);

          if (let == 0) {
            if (!flipped) {
              if (serve_count % flip_serve == 0) {    //decide who serves
                player1.turn = false;
                player2.turn = true;
                Serial.println("Switching Serve");
              }
            }
            else {
              flipped = false;
              player2.blinked = false;
              if (serve_count % flip_serve == 0) {    //decide who serves
                player1.turn = true;
                player2.turn = false;
              }
            }


            if (player1.score >= max_pts - 1 && player2.score >= max_pts - 1) {
              player1.turn = false;
              player2.turn = true;
            }
          }
        }
        if (point_changed && serve_count % flip_serve == 0) {    //decide who serves
          if (!(player1.point_subtr || player2.point_subtr)) {
            player2.blinked = false;
            player1.turn = false;
            player2.turn = true;
            point_changed = false;
            Serial.println("Switching Serve");
          }
          else {
            player1.point_subtr = false;
            player2.point_subtr = false;
            point_changed = false;
          }

        }
        else if (point_changed && serve_count % flip_serve != 0) {
          player1.point_subtr = false;
          player2.point_subtr = false;
        }
      }

      if (player1.score >= max_pts && player1.score > player2.score + 1) {
        Serial.println();
        Serial.println("***Player1 wins!!***");
        delay(1000);
        break;
      }
      if (player2.score >= max_pts && player2.score > player1.score + 1) {
        Serial.println();
        Serial.println("***Player2 wins!!***");
        delay(1000);
        break;
      }

      if(slaughter){
        if(((player1.score == 6 && player2.score == 0) || (player2.score == 6 && player1.score == 0)) && !sent_mercy){
          webSocket.loop();
          webSocket.broadcastTXT("mercy");
          sent_mercy = true;
        }
        if((player1.score == 7 && player2.score == 0) || (player2.score == 7 && player1.score == 0)){
          break;
        }
      }
      
      //****************************************************PLAYER TWO CODE**************************************************************
      if (player2.turn) {

        if (game_reset)
          break;

        if (disconnected) {
          webSocket.loop();
          blink_Disconnected();
          // delay(600);
          Serial.println("disconnected");
        }

        if(connected_){
          digitalWrite(LED_LEFT,HIGH);
          connected_ = false;
        }

        webSocket.loop();

        if (!player2.blinked && !player1.turn) {
          webSocket.broadcastTXT("B");
          send_scores();

          digitalWrite(LED_RIGHT, LOW);
          Serial.println("scores sent from player 2");
          
          while(bounce_timer > 0){
            delay(1);
            
            if(digitalRead(VIBRATION_PLAYER_ONE) == HIGH || digitalRead(VIBRATION_PLAYER_TWO) == HIGH){
              bounce_timer = 900;
            }           
            bounce_timer -= 1;
           // Serial.println(bounce_timer);
            
          }
          
          
          bounce_timer = 900;
          
          for(int i = 0; i < reset_time; i++){
            webSocket.loop();

            if(flipped || game_reset)
              break;
              

            if(digitalRead(VIBRATION_PLAYER_TWO) == HIGH){
                while(bounce_timer > 0){
                  delay(1);
                  
                  if(digitalRead(VIBRATION_PLAYER_TWO) == HIGH){
                    bounce_timer = 900;
                  }           
                  bounce_timer -= 1;
                 // Serial.println(bounce_timer);
                  
                }
                break;
            }
            delay(1);    
          }

          bounce_timer = 900;

          webSocket.broadcastTXT("noletB");

          blinc_B();
          delay(150);
          player2.blinked = true;

        }

        /*
        Serial.println("player 2");
        Serial.print("serve count: ");
        Serial.println(serve_count);
        Serial.print("point changed: ");
        Serial.println(point_changed);
        Serial.print("sub A: ");
        Serial.println(point_subtrA);
        Serial.print("sub B: ");
        Serial.println(point_subtrB);
        */

        if (digitalRead(VIBRATION_PLAYER_TWO) == HIGH) {
          bounce_time = micros();
          
          //cli();
          Serial.println("Bounce B");
          rally_count = 0;
          start_count = millis();
          //if ((micros() - net_arrival_time) / 1000 < 53) {
            //net_hit = false;
          //}
          let = 0;
          serve_count++;
          
          delay(15);
          net_hit = false;
          delay_ms = millis();
          while (millis() - delay_ms < 95) {
            //wait
            Serial.println(net_hit);
          }

          Serial.print("net hit");
          Serial.println(net_hit);
          digitalWrite(LED_LEFT, LOW);

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
              player1.score++;

             
              if(player2.score == max_pts - 1){          
                if(rally_count >= 2){
                  player1.game_pts_won++;
                  player2.game_pts_lost++;
                  Serial.println("Game point B broken");
                }
                else if(rally_count == 0){                
                  Serial.println("Game point B lost");
                  player2.game_pts_lost++;
                }                
              }

              if (rally_count == 2)
                player1.Snd_ball_pts++;

              if (rally_net == rally_count - 1 && net_hit == true) {
                Serial.println("netpoint A");
                player1.netpoints++;
                net_hit = false;

              }
              Serial.println("rally_count: " + String(rally_count));
              Serial.println("rally_net: " + String(rally_net));
              rally_net = 0;
              net_hit = false;

              Serial.println("\t Point for A!");
              Serial.println("Player1 score: " + String(player1.score));
              Serial.println("Player2 score: " + String(player2.score));
              break;
            }
            player2.passivity += (millis() - start_count);
            start_count = millis();
            rally_count++;
            currentMillis = millis();

            if (!Detect_B(currentMillis, rally_count)) {
              player2.score++;

              
              if(player1.score == max_pts - 1){
                Serial.println("Game point A broken");
                player1.game_pts_lost++;
                player2.game_pts_won++;
                               
              }
              
             

              if (rally_count == 1) {
                player2.serve_pts++;
                player1.lost_serve_pts++;
                Serial.println("Serve Point B");
                        
              }
              else if (rally_count == 3)
                player2.Trd_ball_pts++;


              if (rally_net == rally_count - 1 && net_hit == true) {
                Serial.println("netpoint B!!");
                player2.netpoints++;
                net_hit = false;

              }
              Serial.println("rally_count: " + String(rally_count));
              Serial.println("rally_net: " + String(rally_net));
              rally_net = 0;
              net_hit = false;
              Serial.println("\t Point for B!");
              Serial.println("Player1 score: " + String(player1.score));
              Serial.println("Player2 score: " + String(player2.score));
              break;
            }
            player1.passivity += (millis() - start_count);
            start_count = millis();
            rally_count++;

          }

          if (let == 0)
            Rally_Q.push(rally_count);

          Serial.println(rally_count);
          player2.blinked = false;

          if (let == 0) {
            if (!flipped) {
              if (serve_count % flip_serve == 0) {
                player1.turn = true;
                player2.turn = false;
                Serial.print("Switching Serve");
              }
            }
            else {
              flipped = false;
              player1.blinked = false;
              if (serve_count % flip_serve == 0) {
                player1.turn = false;
                player2.turn = true;
              }
            }

            if (player2.score >= max_pts - 1 && player1.score >= max_pts - 1) {
              player1.turn = true;
              player2.turn = false;
            }
          }
        }
        
        if (point_changed && serve_count % flip_serve == 0) {
          if (!(player1.point_subtr || player2.point_subtr)) {
            player1.blinked = false;
            player1.turn = true;
            player2.turn = false;
            point_changed = false;

          }
          else {
            player1.point_subtr = false;
            player2.point_subtr = false;
            point_changed = false;
          }

        }
        else if (point_changed && serve_count % flip_serve != 0) {
          player1.point_subtr = false;
          player2.point_subtr = false;
        }
      }

      if (player1.score >= max_pts && player1.score > player2.score + 1) {
        Serial.println();
        Serial.println("***Player1 wins!!***");
        delay(1000);
        break;
      }
      if (player2.score >= max_pts && player2.score > player1.score + 1) {
        Serial.println();
        Serial.println("***Player2 wins!!***");
        delay(1000);
        break;
      }

      if(slaughter){
        if(((player1.score == 6 && player2.score == 0) || (player2.score == 6 && player1.score == 0)) && !sent_mercy){
          webSocket.loop();
          webSocket.broadcastTXT("mercy");
          sent_mercy = true;
        }
        if((player1.score == 7 && player2.score == 0) || (player2.score == 7 && player1.score == 0)){
          break;
        }
      }

    }

    for (int x = current_set; x < current_set + 1; x++) {
      for (int y = 0; y < 2; y++) {
        if (y == 0)
          scores[x][y] = player1.score;
        else if (y == 1)
          scores[x][y] = player2.score;
      }
    }

    if (player1.score > player2.score){
      player1.games_won++;
      wonA++;                 //local variable only used for scoreboard not DB
      player2.games_lost++;
      player1.games_played++;
      player2.games_played++;
    }
    else{
      player2.games_won++;
      wonB++;
      player1.games_lost++;  
      player1.games_played++;
      player2.games_played++;
    }


    send_scores();        //send final scores
    delay(3);
 

    player1.total_points += player1.score;
    player2.total_points += player2.score;    
    print_stats();
    reset_player_stats();   
    games++;
    current_set++;
    player1.score = 0;
    player2.score = 0;
    player1.turn = true;
    player2.turn = false;
    serve_count = 0;
    sent_mercy = false;

    if(games <= sets - 1 && autoSwitch){
          webSocket.loop();
          webSocket.broadcastTXT("switch");   
    }

  }

  if (!printed_stats && !game_reset) {

    Serial.println();
    //print_stats();                  //rallies and net points
    Serial.println("Game Scores:");
    print_scores(scores);
    printed_stats = true;
  }

  if (game_reset) {
    Serial.println("Game reset remotely");
    reset_stats();
    send_scores();
    game_reset = false;
  }
  else {
    webSocket.loop();
    end_game();
    delay(100);
  }

}
//************************************************FUNCTIONS******************************************************************
boolean Detect_B(long currentMillis, int rally_count) {

  temp_net_state = net_hit;
        
  Serial.print("temp_net_state1: ");
  Serial.println(temp_net_state);

  for (int x = 0; x < 1300; x++) {

    
    //Serial.println(temp_net_state);

    if (digitalRead(VIBRATION_PLAYER_ONE) == HIGH) {
      bounce_time = micros();
      delay(15);
      net_hit = temp_net_state;
      Serial.println("Bounce A");
     // if ( (micros() - net_arrival_time) / 1000 < 15) {
        //  net_hit = temp_net_state;
         // Serial.println((micros() - net_arrival_time) / 1000);
        //  Serial.println("here3");
      //  }
      delay(95);
      if (net_hit && rally_count < 1)
        net_hit = false;

      if( rally_count == 0){
        Serial.println("Serve fault A!");
        player1.serve_flts++;
      }
      
      return false;
    }
    //Serial.println(x);
    delay(1);

    if (digitalRead(VIBRATION_PLAYER_TWO) == HIGH && (millis() - currentMillis) <= interval) {
      bounce_time = micros();
      if (net_hit && rally_count == 0) {
        //Serial.println("here 1");
        if ( (micros() - net_arrival_time) / 1000 < 15) {
          net_hit = temp_net_state;
          Serial.println((micros() - net_arrival_time) / 1000);
          Serial.println("here2");
        }
        //Serial.print("net_hit: ");
        //Serial.println(net_hit);
        if (net_hit) {
          //Serial.println("here3");
          return false;
        }
      }

      Serial.println("Bounce Bb");
      Serial.print("temp_net_state2: ");
      Serial.println(temp_net_state);

      delay(110);
      return true;
    }
  }
  if (net_hit)
    net_hit = false;

    if(rally_count == 0){
    Serial.println("Airball A!");
    player1.air_ball++;
   }

  if(rally_count == 1){
    Serial.println("Ace B!");
    player2.Aces++;
  }

  player1.passivity += 550;                   //ball off table
  return false;
}

boolean Detect_A(long currentMillis, int rally_count) {

  temp_net_state = net_hit;

  Serial.print("temp_net_state3: ");
  Serial.println(temp_net_state);

  
  for (int x = 0; x < 1300; x++) {

    temp_net_state = net_hit;

    if (digitalRead(VIBRATION_PLAYER_TWO) == HIGH) {
      bounce_time = micros();
      delay(15);
      net_hit = temp_net_state;
      
      Serial.println("Bounce B");
      Serial.print("net hit: ");
      Serial.println(net_hit);
      //if ( (micros() - net_arrival_time) / 1000 < 15) {
         // net_hit = temp_net_state;
         // Serial.println((micros() - net_arrival_time) / 1000);
          Serial.println("here4");
     // }
      Serial.println(rally_count);
      delay(95);
      if (net_hit && rally_count < 1)
        net_hit = false;
        
      if( rally_count == 0){
        Serial.println("Serve fault B!");
        player2.serve_flts++;
      }

      return false;
    }
    delay(1);
    if (digitalRead(VIBRATION_PLAYER_ONE) == HIGH && (millis() - currentMillis) <= interval) {
      bounce_time = micros();
      if (net_hit && rally_count == 0) {
        if ( (micros() - net_arrival_time) / 1000 < 15) {
          net_hit = temp_net_state;
          Serial.println((micros() - net_arrival_time) / 1000);
          Serial.println("here2");
        }
        if (net_hit) {
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

   if(rally_count == 0){
    Serial.println("Airball B!");
    player2.air_ball++;
   }

  if(rally_count == 1){
    player1.Aces++;
    Serial.println("Ace A!");
  }
  player2.passivity += 550;
  return false;
}

void print_stats() {
  
 webSocket.loop();


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
  Serial.println("Airtime per return A: " + String((player1.passivity / 1000) / total, 3));
  Serial.println("Airtime per return B: " + String((player2.passivity / 1000) / total, 3));

  Serial.println("Longest rally: " + String(maximum));
  Serial.println("Average Rally: " + String(total / num));
   
  webSocket.loop();
  
  webSocket.broadcastTXT("nA" + String(player1.netpoints));
  Serial.println("nA" + String(player1.netpoints));
  delay(3);
  webSocket.broadcastTXT("sA" + String(player1.serve_pts));
  Serial.println("sA" + String(player1.serve_pts));
  delay(3);
  webSocket.broadcastTXT("fsA" + String(player2.serve_pts));
  Serial.println("fsA" + String(player2.serve_pts));
  delay(3);
  webSocket.broadcastTXT("atA" + String((player1.passivity / 1000) / total, 3));
  Serial.println("atA" + String((player1.passivity / 1000) / total, 3));
  delay(3);
  webSocket.broadcastTXT("lRA" + String(maximum));
  Serial.println("lRA" + String(maximum));
  delay(3);
  
  webSocket.broadcastTXT("2nA" + String(player1.Snd_ball_pts));
  Serial.println("2nA" + String(player1.Snd_ball_pts));
  delay(3);
  webSocket.broadcastTXT("2lA" + String(player2.Snd_ball_pts));
  Serial.println("2lA" + String(player2.Snd_ball_pts));
  delay(3);
  webSocket.broadcastTXT("3rA" + String(player1.Trd_ball_pts));
  Serial.println("3rA: " + String(player1.Trd_ball_pts));
  delay(3);
  webSocket.broadcastTXT("3lA" + String(player2.Trd_ball_pts));
  Serial.println("3lA: " + String(player2.Trd_ball_pts));
  delay(3);
  webSocket.broadcastTXT("tpA" + String(player1.total_points));
  Serial.println("Total PtsA: " + String(player1.total_points));
  delay(3);
  webSocket.broadcastTXT("tplA" + String(player2.total_points));
  Serial.println("Total Points lostA: " + String(player2.total_points));
  delay(3);
  webSocket.broadcastTXT("AcA" + String(player1.Aces));
  Serial.println("Aces A: " + String(player1.Aces));
  delay(3);
  webSocket.broadcastTXT("AbA" + String(player1.air_ball));
  Serial.println("Air balls A: " + String(player1.air_ball));
  delay(3);
  webSocket.broadcastTXT("gpwA" + String(player1.game_pts_won));
  Serial.println("game pts won A: " + String(player1.game_pts_won));
  delay(3);
  webSocket.broadcastTXT("gplA" + String(player1.game_pts_lost));
  Serial.println("game pts lostA: " + String(player1.game_pts_lost));
  delay(3);
  webSocket.broadcastTXT("fltA" + String(player1.serve_flts));
  Serial.println("serve faultsA: " + String(player1.serve_flts));
  delay(3);
  webSocket.broadcastTXT("gwA" + String(player1.games_won));
  Serial.println("gamesWonA: " + String(player1.games_won));
  delay(3);
  webSocket.broadcastTXT("glA" + String(player1.games_lost));
  Serial.println("gamesLostA: " + String(player1.games_lost));
  delay(3);
  webSocket.broadcastTXT("gpA" + String(player1.games_played));
  Serial.println("Games PlayedA:  " + String(player1.games_played));
  delay(3);


//player2

  webSocket.broadcastTXT("nB" + String(player2.netpoints));
  Serial.println("nB" + String(player2.netpoints));
  delay(3);
  webSocket.broadcastTXT("sB" + String(player2.serve_pts));
  Serial.println("sB" + String(player2.serve_pts));
  delay(3);
  webSocket.broadcastTXT("fsB" + String(player1.serve_pts)); ///opposite
  Serial.println("fsB" + String(player1.serve_pts));
  delay(3);
  webSocket.broadcastTXT("atB" + String((player2.passivity / 1000) / total, 3));
  Serial.println("atB" + String((player2.passivity / 1000) / total, 3));
  delay(3);
  webSocket.broadcastTXT("lRB" + String(maximum));
  Serial.println("lRB" + String(maximum));
  delay(3);
  
  webSocket.broadcastTXT("2nB" + String(player2.Snd_ball_pts));
  Serial.println("2nB" + String(player2.Snd_ball_pts));
  delay(3);
  webSocket.broadcastTXT("2lB" + String(player1.Snd_ball_pts));
  Serial.println("2lB" + String(player1.Snd_ball_pts));
  delay(3);
  webSocket.broadcastTXT("3rB" + String(player2.Trd_ball_pts));
  Serial.println("3rB: " + String(player2.Trd_ball_pts));
  delay(3);
  webSocket.broadcastTXT("3lB" + String(player1.Trd_ball_pts));
  Serial.println("3lB: " + String(player1.Trd_ball_pts));
  delay(3);
  webSocket.broadcastTXT("tpB" + String(player2.total_points));
  Serial.println("Total PtsB: " + String(player2.total_points));
  delay(3);
  webSocket.broadcastTXT("tplB" + String(player1.total_points));
  Serial.println("Total Points lostB: " + String(player1.total_points));
  delay(3);
  webSocket.broadcastTXT("BcB" + String(player2.Aces));
  Serial.println("Aces B: " + String(player2.Aces));
  delay(3);
  webSocket.broadcastTXT("BbB" + String(player2.air_ball));
  Serial.println("Air balls B: " + String(player2.air_ball));
  delay(3);
  webSocket.broadcastTXT("gpwB" + String(player2.game_pts_won));
  Serial.println("game pts won B: " + String(player2.game_pts_won));
  delay(3);
  webSocket.broadcastTXT("gplB" + String(player2.game_pts_lost));
  Serial.println("game pts lostB: " + String(player2.game_pts_lost));
  delay(3);
  webSocket.broadcastTXT("fltB" + String(player2.serve_flts));
  Serial.println("serve faultsB: " + String(player2.serve_flts));
  delay(3);
  webSocket.broadcastTXT("gwB" + String(player2.games_won));
  Serial.println("gamesWonB: " + String(player2.games_won));
  delay(3);
  webSocket.broadcastTXT("glB" + String(player2.games_lost));
  Serial.println("gamesLostB: " + String(player2.games_lost));
  delay(3);
  webSocket.broadcastTXT("gpB" + String(player2.games_played));
  Serial.println("Games PlayedB:  " + String(player2.games_played));
  delay(3);
  
  
  

  if(!game_reset)
    webSocket.broadcastTXT("insert");
  
}

void send_scores() {

  webSocket.loop();
  Serial.println("Sending scores...");
  Serial.println(max_pts);
  String game1 = String(wonA);
  String game2 = String(wonB);
  webSocket.broadcastTXT("AA" + game1);
  delay(5);
  webSocket.broadcastTXT("BB" + game2);
  String scoreOne = String(player1.score); //send final scores
  String scoreTwo = String(player2.score);
  webSocket.broadcastTXT("fi" + scoreOne);
  delay(5);
  webSocket.broadcastTXT("se" + scoreTwo);

}

void reset_player_stats() {
  player1.games_won = 0;
  player1.games_lost = 0;
  player1.games_played = 0;
  player1.passivity = 0;
  player1.netpoints = 0;
  player1.serve_pts = 0;
  player1.lost_serve_pts = 0;
  player1.Snd_ball_pts = 0;
  player1.Trd_ball_pts = 0;
  player1.serve_flts = 0;
  player1.Aces = 0;
  player1.air_ball = 0;
  player1.game_pts_won = 0;
  player1.game_pts_lost = 0;
  player1.total_points = 0;

  player2.games_won = 0;
  player2.games_lost = 0;
  player2.games_played = 0;
  player2.passivity = 0;
  player2.netpoints = 0;
  player2.serve_pts = 0;
  player2.lost_serve_pts = 0;
  player2.Snd_ball_pts = 0;
  player2.Trd_ball_pts = 0;
  player2.serve_flts = 0;
  player2.Aces = 0;
  player2.air_ball = 0;
  player2.game_pts_won = 0;
  player2.game_pts_lost = 0;
  player2.total_points = 0;

}



void reset_stats() {

  current_set = 0;
  player1.score = 0;
  player2.score = 0;
  serve_count = 0;
  let = 0;
  int_pts = 0;
  games = 0;
  wonA = 0;
  wonB = 0;
  
  player1.games_won = 0;
  player1.games_lost = 0;
  player1.games_played = 0;
  player1.passivity = 0;
  player1.netpoints = 0;
  player1.serve_pts = 0;
  player1.lost_serve_pts = 0;
  player1.Snd_ball_pts = 0;
  player1.Trd_ball_pts = 0;
  player1.serve_flts = 0;
  player1.Aces = 0;
  player1.air_ball = 0;
  player1.game_pts_won = 0;
  player1.game_pts_lost = 0;
  player1.total_points = 0;

  player2.games_won = 0;
  player2.games_lost = 0;
  player2.games_played = 0;
  player2.passivity = 0;
  player2.netpoints = 0;
  player2.serve_pts = 0;
  player2.lost_serve_pts = 0;
  player2.Snd_ball_pts = 0;
  player2.Trd_ball_pts = 0;
  player2.serve_flts = 0;
  player2.Aces = 0;
  player2.air_ball = 0;
  player2.game_pts_won = 0;
  player2.game_pts_lost = 0;
  player2.total_points = 0;
  rally_count = 0;
  

  //float start_count;
  //float in, out;  //for pulsing LEDs at end of game
  num = 0;           //only update webserver once
  rally_net = 0;

  player1.turn = true;
  player2.turn = false;
  net_hit = false;
  player1.blinked = false;
  player2.blinked = false;
  startGame = true;
  printed_stats = false;
  player1.bounced = false;
  player2.bounced = false;
  temp_net_state = false;
  sent_mercy = false;


}

void print_scores(int scores[][2]) {
  for (int x = 0; x < current_set; x++) {
    for (int y = 0; y < 2; y++) {
      Serial.println(scores[x][y]);
    }
    Serial.println();
  }
  webSocket.loop();
  webSocket.broadcastTXT("displayStatsA");  
}

void net_ISR() {
  net_arrival_time = micros();
  Serial.print("N: ");
  Serial.print(net_arrival_time - bounce_time);


    if(net_arrival_time - bounce_time > 15000){
      net_hit = true;
      rally_net = rally_count;
    }
  

}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  Serial.println("Data recieved from Web");

  switch (type) {
    case WStype_DISCONNECTED:
      {
        
        web_sessions -= 1;
        
        if(web_sessions == 0)
          disconnected = true;
          
        break;
      }
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        webSocket.broadcastTXT("Connected to ESP");

        if(web_sessions > 0){
          webSocket.broadcastTXT("logged in");     
        }
        web_sessions++; 
          
        if(player1.turn)
         webSocket.broadcastTXT("A");
        else
         webSocket.broadcastTXT("B");

        disconnected = false;
        connected_ = true;
        send_scores();
        blink_Connected();

      }
      break;
    case WStype_TEXT:
      {
        //Serial.println("text recieved");
        String text = String((char *) &payload[0]);

        if (text == "flip") {
          flip();
        }

        if (text.startsWith("s")) {

          blink_Connected();
          sets = String((char *) &payload[1]).toInt();
          Serial.println(String((char *) &payload[1]).toInt());

        }

        if (text.startsWith("p")) {

          max_pts = String((char *) &payload[1]).toInt();
          if(max_pts == 25)
            flip_serve = 5;
            
          Serial.println(max_pts);
        }

        if (text.startsWith("t")) {

          reset_time = String((char *) &payload[1]).toInt() * 1000;
          Serial.println(reset_time);
        }

        if (text.startsWith("A")) {

          player1.score += 1;
          point_changed = true;
          serve_count++;
          send_scores();
          Serial.print(player1.score);
        }

        if (text.startsWith("B")) {

          serve_count++;
          point_changed = true;
          player2.score += 1;
          send_scores();
          Serial.println(player2.score);

        }

        if (text.startsWith("C")) {

          if (player1.score > 0) {
            player1.score -= 1;

            if (serve_count % flip_serve == 0) {
              flip();
              serve_count++;
            }
            else
              serve_count -= 1;

            point_changed = true;
            player1.point_subtr = true;
          }
          send_scores();
          Serial.print(player1.score);
        }

        if (text.startsWith("D")) {

          if (player2.score > 0) {

            player2.score -= 1;

            if (serve_count % flip_serve == 0) {
              flip();
              serve_count += 1;
            }
            else
              serve_count -= 1;

            point_changed = true;
            player2.point_subtr = true;
          }
          send_scores();
          Serial.println(player2.score);
        }

        if(text == "login")
          logged_in = true;

        if(text == "yes"){
          Serial.println("Slaughter Rule Enabled");
          slaughter = true;
        }
       
        if(text == "no"){
          slaughter = false;
        }

        if(text == "yesflip")
          autoSwitch = true;

         if(text == "noflip")
          autoSwitch = false;

        if (text == "reset") {
          game_reset = true;
          Serial.println("game reset from web");
        }

      }

      //webSocket.sendTXT(num, payload, lenght);
      webSocket.broadcastTXT(payload, lenght);
      break;

    case WStype_BIN:

      hexdump(payload, lenght);

      // echo data back to browser
      webSocket.sendBIN(num, payload, lenght);
      break;
  }
}

void flip() {

  flipped = true;
  if (player1.turn) {
    player2.turn = true;
    player1.turn = false;
    player1.blinked = false;
    player2.blinked = false;
    Serial.println("Switch to Red Serve");
    digitalWrite(LED_RIGHT, LOW);
  }
  else {
    player1.turn = true;
    player2.turn = false;
    player2.blinked = false;
    player1.blinked = false;
    Serial.println("Switch to Blue Serve");
    digitalWrite(LED_LEFT, LOW);
  }
}


void blinc_A() {
  digitalWrite(LED_LEFT, LOW);
  delay(10);
  digitalWrite(LED_RIGHT, HIGH);
  /*
  delay(150);
  digitalWrite(LED_RIGHT, LOW);
  delay(150);
  digitalWrite(LED_RIGHT, HIGH);
  delay(150);
  digitalWrite(LED_RIGHT, LOW);
  */

}
void blinc_B() {

  digitalWrite(LED_RIGHT, LOW);
  delay(10);
  digitalWrite(LED_LEFT, HIGH);
  /*
  delay(150);
  digitalWrite(LED_LEFT, LOW);
  delay(150);
  digitalWrite(LED_LEFT, HIGH);
  delay(150);
  digitalWrite(LED_LEFT, LOW);
  */
}

void blink_Connected() {
  digitalWrite(LED_LEFT, HIGH);
  digitalWrite(LED_RIGHT, HIGH);
  delay(150);
  digitalWrite(LED_LEFT, LOW);
  digitalWrite(LED_RIGHT, LOW); 
  delay(150);
  digitalWrite(LED_LEFT, HIGH);
  digitalWrite(LED_RIGHT, HIGH);
  delay(150);
  digitalWrite(LED_LEFT, LOW);
  digitalWrite(LED_RIGHT, LOW); 

  delay(5);
  if(player1.turn)
     digitalWrite(LED_RIGHT, HIGH);  
  else
     digitalWrite(LED_LEFT, HIGH);

  
}

void blink_Disconnected() {
  digitalWrite(LED_LEFT, HIGH);
  digitalWrite(LED_RIGHT, HIGH);
  delay(300);
  digitalWrite(LED_RIGHT, LOW);
  digitalWrite(LED_LEFT, LOW);
  delay(300);
  webSocket.loop();
  digitalWrite(LED_LEFT, HIGH);
  digitalWrite(LED_RIGHT, HIGH);
  delay(300);
  digitalWrite(LED_LEFT, LOW);
  digitalWrite(LED_RIGHT, LOW);
  delay(300);

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
  digitalWrite(LED_LEFT, HIGH);
  delay(700);
  digitalWrite(LED_LEFT, LOW);
  webSocket.loop();
  delay(20);
  digitalWrite(LED_RIGHT, HIGH);
  delay(700);
  digitalWrite(LED_RIGHT, LOW);
}



















