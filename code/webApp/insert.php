<?php
$servername = "127.0.0.1";
$username = "root";
$password = "";
$dbname = "pong_stats";

$conn = new mysqli($servername, $username, $password, $dbname);


$name = $_POST["name"];

$netpoints = $_POST["netpoints"];
$serve_points = $_POST["serve_pts"];
$lost_serve_points = $_POST["lost_serve_pts"];
$passivity = $_POST["passivity"];
$Second_ball_pts = $_POST["Snd_ball_pts"];
$Third_ball_pts = $_POST["Trd_ball_pts"];
$wins = $_POST["games_won"];
$losses = $_POST["games_lost"];
//$games_played = $_POST["games_played"];
$Aces = $_POST["Aces"];
$air_balls = $_POST["air_balls"];
$serve_faults = $_POST["serve_faults"];
$game_points_won = $_POST["game_points_won"];
$game_points_lost = $_POST["game_points_lost"];
$points = $_POST["total_points"];
$points_lost = $_POST["total_points_lost"];

//echo "$total_points_lost";
//echo "Insert Successful";

// Create connection

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 
//echo "Connected successfully";

$sql = "INSERT INTO player_stats (name, Won, Lost, points,points_lost, aces, air_balls, Serve_Faults, game_pts_lost, netpoints, serve_points, lost_serve_pts, 2nd_ball_points, Third_ball_points, passivity, game_points_won) VALUES ('$name','$wins','$losses','$points','$points_lost','$Aces','$air_balls','$serve_faults','$game_points_lost','$netpoints','$serve_points','$lost_serve_points','$Second_ball_pts','$Third_ball_pts','$passivity','$game_points_won')";

if ($conn->query($sql) === TRUE) {
    echo "Insertion Successful!";
} else {
   echo "Error: " . $sql . "<br>" . $conn->error;
}

$sql = "SELECT name FROM player_stats";

$result = $conn->query($sql);


if ($result->num_rows > 0) {
    //echo "<table><tr><th>ID</th><th>Name</th></tr>";
    // output data of each row
    while($row = $result->fetch_assoc()) {
    //    echo "<tr><td>".$row["player_id"]."</td><td>".$row["name"]." ".$row["lastname"]."</td></tr>";
    }
    //echo "</table>";
} else {
    //echo "0 results";
}

$conn->close();

?>