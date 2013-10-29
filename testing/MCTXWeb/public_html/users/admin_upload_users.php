<?php

/**
 * This file is not part of the original UserCake system, but uses it.
 */

require_once("models/config.php");
if (!securePage($_SERVER['PHP_SELF'])){die();}

$text_area="# Rows starting with '#' are ignored. Rows are of the form:\n# Username, Full Name, Email[, Title]";


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

    if ($_POST['upload_mode'] === "purge")
    {
      $to_delete = fetchAllUsersWithoutPerm("Administrator");
      if (count($to_delete) > 0)
      {
        if ($deletion_count = deleteUsers($to_delete)) {
          $successes[] = lang("ACCOUNT_DELETIONS_SUCCESSFUL", array($deletion_count));
        }
        else {
          $errors[] = lang("SQL_ERROR");
        }
      }
    }
  
    $text_area = $_POST['userUpload'];

    // Iterate through each row
    $all_rows=preg_split("/((\r?\n)|(\r\n?))/", $_POST['userUpload']);
    
    foreach ($all_rows as $row)
    {
      if (empty($row) or $row[0] === '#')
        continue;

      $fields = preg_split("/,/",  $row);
      if(count($fields) < 3)
      {
        $errors[] = "Warning: Ignoring row not in correct format: ".htmlspecialchars($row);
        continue;
      }
      
      $username = trim($fields[0]);
      $displayname = trim($fields[1]);
      $email = trim($fields[2]);
      $title = trim($fields[3]);
      
      if(!isValidEmail($email))
      {
        $errors[] = "Enter a valid email for row: ".htmlspecialchars($row);
        continue;
      }
      
      // generate the temporary password
      //$password = generatePassword();

      // hey, adrian suggested it
      $password = "mctx".date("MY");

      //Construct a user object
      $user = new User($username,$displayname,$password,$email);
  
      //Checking this flag tells us whether there were any errors such as possible data duplication occured
      if(!$user->status)
      {
        if($user->username_taken) $localerrors[] = lang("ACCOUNT_USERNAME_IN_USE",array($username));
        if($user->displayname_taken) $localerrors[] = lang("ACCOUNT_DISPLAYNAME_IN_USE",array($displayname));
        if($user->email_taken) 	  $localerrors[] = lang("ACCOUNT_EMAIL_IN_USE",array($email));	          
      }
      else
      {
        //Attempt to add the user to the database, carry out finishing  tasks like emailing the user (if required)
        $user->userCakeAddUser(); //This doesn't return anything itself
        
        if($user->mail_failure) $localerrors[] = lang("MAIL_ERROR");
        if($user->sql_failure)  $localerrors[] = lang("SQL_ERROR");
        
        if(strlen($title) >= 50)
        {
          $localerrors[] = "Warning: User".$username." added but failed to set title: ".lang("ACCOUNT_TITLE_CHAR_LIMIT",array(1,50));
        }
        else if (count($localerrors) == 0 && strlen($title) > 0)
        {
          $user_id = fetchUserId($username); //So stupid, when you create a user, it doesn't return the user id
          if (!updateTitle($user_id, $title))
          {
            $localerrors[] = "Warning: User ".$username." added but failed to set title: ". lang("SQL_ERROR");
          }
        }
      }

      if(count($localerrors) == 0)
      {
        $users[] = [$username, $password]; //Push user onto array
    		//$successes[] = ($user->success);
      }
      else
      {
        $errors = array_merge($errors, $localerrors);
      }

    }
 
    if(count($users) > 0)
    {
      $successes[] = (count($users)." users created.");
      $successes[] = "The temporary password is: "."mctx".date("MY");
      $successes[] = "Please change this as soon as possible.";
//      $successes[] = ("The list of usernames and passwords follow. You must save this!");
//      foreach($users as $user)
//      {
//        $successes[] = $user[0].",".$user[1];
//      }
    }
  }


}

require_once("models/header.php");
startPage();

echo notificationBlock($errors,$successes);

echo '<div class="widget"><div class="title">Upload users</div>';

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
  <textarea name=\"userUpload\" rows=\"50\" cols=\"100\" style=\"width: 100%\">".$text_area."</textarea> </p>

  </form>";
}

echo '</div>';
finishPage();
  
?> 

