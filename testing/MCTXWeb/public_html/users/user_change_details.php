<?php
/*
UserCake Version: 2.0.2
http://usercake.com
*/

require_once("models/config.php");
if (!securePage($_SERVER['PHP_SELF'])){die();}

//Forms posted
if(!empty($_POST))
{
	$errors = array();
	$username = sanitize(trim($_POST["username"]));
	$password = trim($_POST["password"]);
  $password_new = trim($_POST["password_new"]);
  $password_confirm = trim($_POST["password_confirm"]);
	
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
					
          if(trim($password_new) == "")
          {
            $errors[] = lang("ACCOUNT_SPECIFY_NEW_PASSWORD");
          }
          else if(trim($password_confirm) == "")
          {
            $errors[] = lang("ACCOUNT_SPECIFY_CONFIRM_PASSWORD");
          }
          else if(minMaxRange(6,50,$password_new))
          {	
            $errors[] = lang("ACCOUNT_NEW_PASSWORD_LENGTH",array(6,50));
          }
          else if($password_new != $password_confirm)
          {
            $errors[] = lang("ACCOUNT_PASS_MISMATCH");
          }
          
          //End data validation
          if(count($errors) == 0)
          {
            //Also prevent updating if someone attempts to update with the same password
            $entered_pass_new = generateHash($password_new,$loggedInUser->hash_pw);
            
            if($entered_pass_new == $loggedInUser->hash_pw)
            {
              //Don't update, this fool is trying to update with the same password Â¬Â¬
              $errors[] = lang("ACCOUNT_PASSWORD_NOTHING_TO_UPDATE");
            }
            else
            {
              //This function will create the new hash and update the hash_pw property.
              $loggedInUser->updatePassword($password_new);
              $successes[] = lang("ACCOUNT_PASSWORD_UPDATED");
            }
          }
				}
			}
		}
	}
}

if (isUserLoggedIn())
{
  //If not admin, log them out after pw change
  if (!$loggedInUser->checkPermission(array(2)))
  {
    $loggedInUser->userLogOut();
  }
}

require_once("models/header.php");
startPage();

echo '
      <div id="login-container">
       <div class="widget">
          <div class="title centre">Change of password</div>
           <form id="login-update" class="clear" name="login-update" action="'.$_SERVER["PHP_SELF"].'" method="post">
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
             <p>
               <label>
                 New password<br>
                 <input name="password_new" type="password">
               </label>             
             </p>
             <p>
               <label>
                 Confirm password<br>
                 <input name="password_confirm" type="password">
               </label>             
             </p>
             <p style="float:left; margin:0;">
               <a href="forgot-password.php">Forgotten password?</a>
             </p>
             <p style="float:right; margin:0;">
               <input type="submit" value="Update">
             </p>
            </form>';
            
echo resultBlock($errors,$successes);            
echo '
       </div>
      </div>
 ';

finishPage();

?>
