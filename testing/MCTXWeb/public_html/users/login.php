<?php
/*
UserCake Version: 2.0.2
http://usercake.com
*/

require_once("models/config.php");
if (!securePage($_SERVER['PHP_SELF'])){die();}

//Prevent the user visiting the logged in page if he/she is already logged in
if(isUserLoggedIn()) { header("Location: index.php"); die(); }

//Forms posted
if(!empty($_POST))
{
	$errors = array();
	$username = sanitize(trim($_POST["username"]));
	$password = trim($_POST["password"]);
	
	//Perform some validation
	//Feel free to edit / change as required
	if($username == "")
	{
		$errors[] = lang("ACCOUNT_SPECIFY_USERNAME");
	}
	if($password == "")
	{
		$errors[] = lang("ACCOUNT_SPECIFY_PASSWORD");
	}

	if(count($errors) == 0)
	{
		//A security note here, never tell the user which credential was incorrect
		if(!usernameExists($username))
		{
			$errors[] = lang("ACCOUNT_USER_OR_PASS_INVALID");
		}
		else
		{
			$userdetails = fetchUserDetails($username);
			//See if the user's account is activated
			if($userdetails["active"]==0)
			{
				$errors[] = lang("ACCOUNT_INACTIVE");
			}
			else
			{
				//Hash the password and use the salt from the database to compare the password.
				$entered_pass = generateHash($password,$userdetails["password"]);

				//echo "".$userdetails["password"]; //Wut is dis
				
				if($entered_pass != $userdetails["password"])
				{
					//Again, we know the password is at fault here, but lets not give away the combination incase of someone bruteforcing
					$errors[] = lang("ACCOUNT_USER_OR_PASS_INVALID");
				}
				else
				{
					//Passwords match! we're good to go'
					
					//Construct a new logged in user object
					//Transfer some db data to the session object
					$loggedInUser = new loggedInUser();
					$loggedInUser->email = $userdetails["email"];
					$loggedInUser->user_id = $userdetails["id"];
					$loggedInUser->hash_pw = $userdetails["password"];
					$loggedInUser->title = $userdetails["title"];
					$loggedInUser->displayname = $userdetails["display_name"];
					$loggedInUser->username = $userdetails["user_name"];
					
          //Only allow login to admins
          if ($loggedInUser->checkPermission(array(2)))
          {
            //Update last sign in
            $loggedInUser->updateLastSignIn();
            
            $_SESSION["userCakeUser"] = $loggedInUser;
            
            //Redirect to user account page
            header("Location: index.php");
            die();
          }
          else
          {
            $errors[] = ("You are no admin :(");
          }
				}
			}
		}
	}
}

require_once("models/header.php");
startPage();

echo '
      <div id="login-container">
      <div class="widget">
        <div class="title">Notice</div>
        This is the login page for site administration.<br>If you wish to log in
        to the main web-site, see <a href="..">here instead</a>.
      </div>
       <div class="widget">
           <form id="login" name="login" action="'.$_SERVER["PHP_SELF"].'" method="post">
             <p>
               <label>
                 Username<br>
                 <input name="username" type="text">
               </label>
             </p>
             <p>
               <label>
                 Password<br>
                 <input name="password" type="password">
               </label>             
             </p>
             <p style="float:left; margin:0;">
               <a href="forgot-password.php">Forgotten password?</a><br>
               <a href="register.php">Register</a>
             </p>
             <p style="float:right; margin:0;">
               <input type="submit" value="Log In">
             </p>
';
echo resultBlock($errors,$successes);
echo '
            </form>
       </div>
      </div>
 ';

finishPage();

?>
