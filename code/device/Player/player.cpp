// ---------------------------------------------------------------------------
// Created by Redi Beluli - teckel@leethost.com
// Copyright 2016 License: GNU GPL v3 http://www.gnu.org/licenses/gpl.html
//
// See "NewPing.h" for purpose, syntax, version history, links, and more.
// ---------------------------------------------------------------------------

#include "player.h"


player::player() {

	//game control variables
	score = 0;
    turn = false;
    blinked = false;
    bounced = false;
    point_subtr = false;

    


     //STATS
     total_points = 0;
     total_points_lost = 0;
     games_won = 0;
     games_lost = 0;
     games_played = 0;
     netpoints = 0.0; // points obtained after hitting top of net and landing on opponents side
     passivity = 0; //player aggressiveness measured by the time per return the ball has been in the air 
     serve_pts = 0; // points from serve
     lost_serve_pts = 0;	//points lost recieving serve
     Snd_ball_pts = 0;	//second ball points
     Trd_ball_pts = 0;	// third ball points
     serve_flts = 0;	//any kind of serve fault, double bounce on your side flying off table missing opponents side etc.
     Aces = 0; 		// ace serves (return completely misses the table or no return was made)
     air_ball = 0;	//air ball serves
     game_pts_won = 0;	//opponent game points blocked
     game_pts_lost = 0;  //number failed to convert game point

     /*possible stats
	 longest rally
	 average rally




     */


}


player::~player(){

}

bool player::won(){

//checks if won

}




