class player {
  
  public:
    player();


    ~player();


    bool won();






    bool turn;
    bool blinked;
    bool bounced;
    bool point_subtr;
    int score;

    int netpoints;
    float passivity;
    int total_points; 
    int total_points_lost;  
    int serve_flts;
    int Aces;
    int air_ball;
    int game_pts_won;
    int game_pts_lost;

    int games_won;
    int games_lost;
    int games_played;

    int serve_pts;
    int lost_serve_pts;
    int Snd_ball_pts;
    int Trd_ball_pts;


    private:

 
};
