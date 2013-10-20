<?php

/**
 * This file is not part of the original UserCake system, but uses it.
 */

require_once("models/config.php");
if (!securePage($_SERVER['PHP_SELF'])){die();}

require_once("models/header.php");

$text_area="# Rows starting with '#' are ignored. Rows are of the form:\n# username, Full Name, email";


$show_form = true;
if (!empty($_POST))
{

  $current_users = fetchAllUsers();

  // Check form action
  if ($_POST['action'] === "Download") // Download list of users and populate the text area
  {
  
    foreach ($current_users as $u)
    {
      if ($u['user_name'] !== "admin")
        $text_area=$text_area."\n".$u['user_name'].",".$u['display_name'].",".$u['email'];
    }
  }
  else if ($_POST['action'] === "Upload") // Upload users in the text area
  {

    $text_area = $_POST['userUpload'];


    // Iterate through each row
    $all_rows=preg_split("/((\r?\n)|(\r\n?))/", $_POST['userUpload']);
   
    
    
    
    foreach ($all_rows as $row)
    {
      if (empty($row) or $row[0] === '#')
        continue;

      $errors = array();

      $fields = preg_split("/,/",  $row);
     
      $user_name = trim($fields[0]);
      $display_name = trim($fields[1]);
      $email = trim($fields[2]);
      // generate the temporary password
      $password = generatePassword();


      if(count($errors) == 0)
      {	
        //Construct a user object
    		$user = new User($username,$displayname,$password,$email);
		
    		//Checking this flag tells us whether there were any errors such as possible data duplication occured
    		if(!$user->status)
    		{
    			if($user->username_taken) $errors[] = lang("ACCOUNT_USERNAME_IN_USE",array($username));
    			if($user->displayname_taken) $errors[] = lang("ACCOUNT_DISPLAYNAME_IN_USE",array($displayname));
    			if($user->email_taken) 	  $errors[] = lang("ACCOUNT_EMAIL_IN_USE",array($email));		
    		}
    		else
    		{
    			//Attempt to add the user to the database, carry out finishing  tasks like emailing the user (if required)
    			if(!$user->userCakeAddUser())
    			{
    				if($user->mail_failure) $errors[] = lang("MAIL_ERROR");
    				if($user->sql_failure)  $errors[] = lang("SQL_ERROR");
    			}
    		}
      }
      if(count($errors) == 0)
      {
    		$successes[] = $user->success;
      }

      echo resultBlock($errors,$successes);
    }

    
    
    
  
  }


}

if ($show_form)
{
  /* I can't get fucking file uploads to fucking work with fucking nginx
  echo "<p> Please provide a CSV file of usernames and email addresses. </p>
  <p> Click <a href=\"upload_users_example.csv\">here</a> for an example file. </p>
  <div class=\"title\">Upload</div>
  <form  action=\"".$_SERVER['PHP_SELF']."\" enctype=\"multipart/form-data\" method=\"post\">
  <input type=\"file\" name=\"userUpload\"/>
  <input type=\"submit\" value=\"Upload\"/>
  </form>";
  */
  echo "
 
  <form action=\"".$_SERVER['PHP_SELF']."\" method=\"POST\">
  <p> Action to take on adding users: </p>
  <p> <input type=\"radio\" name=\"upload_mode\" value=\"keep\" checked/>Keep existing users and add these users</p>
  <p> <input type=\"radio\" name=\"upload_mode\" value=\"purge\"/>Purge existing users and add these users</p>
  <input type=\"submit\" name=\"action\" value=\"Upload\"/> 
  <input type=\"submit\" name=\"action\" value=\"Download\"/>
  <input type=\"submit\" name=\"action\" value=\"Reset\"/>
  <p> Enter or copy/paste user information below (resize the text area if necessary): </p>
  <p>
  <textarea name=\"userUpload\" rows=\"50\" cols=\"100\">".$text_area."</textarea> </p>

  </form>";
}
  
?> 

