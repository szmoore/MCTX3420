<?php
/*
UserCake Version: 2.0.2
http://usercake.com
*/

require_once("models/config.php");
if (!isUserLoggedIn()) { header("Location: login.php"); die();}

require_once("models/header.php");
startPage();
echo '
  <div class="widget">
    <div class="title centre">Welcome!</div>
    <p>
      This is the administration site for this site. Here, you can manage
      the list of users who has access to this site.
    </p>
    <p>
      <!-- blah blah blah -->
    </p>
  </div>
';

finishPage();

?>
