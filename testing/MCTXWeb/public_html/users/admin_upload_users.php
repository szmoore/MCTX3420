<?php

require_once("models/config.php");
if (!securePage($_SERVER['PHP_SELF'])){die();}

require_once("models/header.php");
createPage("User Upload");


if (!empty($_POST))
{
  echo "<p> Uploaded! </p>";
}
else
{
  echo "<p> Please provide a CSV file of usernames and email addresses. </p>
  <div class=\"title\">Upload</div>
  <form name='newUser' action='".$_SERVER['PHP_SELF']."' method='post'>
  <input type=\"file\" name=\"users\"/>
  <input type=\"submit\" value=\"Upload\"/>
  </form>";
}
  
?> 

