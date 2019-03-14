<?php

session_start();
$_SESSION['message'] = '';

$servername = "127.0.0.1";
$username = "root";
$password = "";
$dbname = "pong_stats";

$mysqli = new mysqli($servername, $username, $password, $dbname);

if ($_SERVER['REQUEST_METHOD'] == 'POST'){

	if($_POST['password'] == $_POST['confirmpassword']){
		$username = $mysqli->real_escape_string($_POST['username']);
		$email = $mysqli->real_escape_string($_POST['email']);
		$password = password_hash($_POST['password'], PASSWORD_DEFAULT);
		$pw = $_POST['password'];
		
		$_SESSION['username'] = $username;

		if(strlen($username) > 4){

			if(strlen($pw) > 5){
				
				if(preg_match('/[\'^£$%&*()}{@#~?><>,|=_+¬-]/', $pw) && preg_match('/[A-Za-z]/i', $pw) && preg_match("#[0-9]+#", $pw)){

					$sql = "SELECT * FROM users WHERE username = '$username'";
					$result = $mysqli->query($sql);

			        if($result->num_rows < 1){  
				        
						$sql = "INSERT INTO users (username,email,password) "
								. "VALUES ('$username','$email','$password')";

						if($mysqli->query($sql) === true){
							$_SESSION['message'] = "Registration successful! Added $username to the database";
							?> <style type="text/css">#alert{display: block; background-color: #195b0f;color: #47c934;}</style><?php
						}
						else{
							$_SESSION['message'] = "User could not be added to database";
							?> <style type="text/css">#alert{display: block;}</style><?php
						}
					}
					else{
						$_SESSION['message'] = "username exists";
						?> <style type="text/css">#alert{display: block;}</style><?php
					}
				}
				else{
					$_SESSION['message'] = "Password must contain at least one special character and a number";
					?> <style type="text/css">#alert{display: block;}</style><?php	
				}
			}
			else{
				$_SESSION['message'] = "Password must be at least 6 characters long";
			    ?> <style type="text/css">#alert{display: block;}</style><?php	
			}

		}
		else{
			$_SESSION['message'] = "Username must at least 5 characters";
			?> <style type="text/css">#alert{display: block;}</style><?php
		}

	}
	else{
		$_SESSION['message'] = "Passwords do not match";
		//$_SESSION[].document.getElementById("alert").style.display = "block";
	    ?> <style type="text/css">#alert{display: block;}</style><?php
	}

}

?>


<link rel="stylesheet" type="text/css" href="register.css">
<div class="container">
	<h1>Create Account</h1>
	<form class="form-container" action="register.php" method="post" enctype="multipart/form-data" autocomplete="off">
	<div class="alert-error" id="alert"><?= $_SESSION['message'] ?></div>
	<input type="text" name="username" placeholder="User Name" required />
	<input type="email" name="email" placeholder="Email" required />
	<input type="password" name="password" placeholder="Password" required />
	<input type="password" name="confirmpassword" placeholder="Confirm Password" required />
	<input type="submit" value="Register" name="register" class="btn">
	
	</form>
</div>






