<?php

$servername = "127.0.0.1";
$username = "root";
$password = "";
$dbname = "pong_stats";

$mysqli = new mysqli($servername, $username, $password, $dbname);

$user_name = $_POST["tmp_name"];
$sql = "SELECT * FROM users WHERE username = '$user_name'";
$result = $mysqli->query($sql);


$row = $result->fetch_assoc();


$password = $_POST["pw"];
$hash = $row["password"];

if( password_verify($password, $hash)){
	echo "match";

}
else{
	echo "no match";
}



?>