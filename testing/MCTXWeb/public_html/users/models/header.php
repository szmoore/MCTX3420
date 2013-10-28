<?php
/*
UserCake Version: 2.0.2
http://usercake.com
*/

//Variables to set:
//$custom_header_scripts
//$custom_sidebar

function startPage() {
global $loggedInUser;

echo '
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
  <head>
    <title>MCTX3420 Web Interface</title>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
    <link rel="stylesheet" type="text/css" href="../static/style.css">
    <link rel="stylesheet" type="text/css" href="../static/nav-menu.css">
    
    <script type="text/javascript" src="../static/jquery-1.10.1.min.js"></script>
    
	'.$custom_header_scripts.'
  </head>
  
  <body>
    <div id="header-wrap">
      <div id="header">
        <div id="leftnav">
          <a href="http://www.uwa.edu.au/" target="_blank">
            <img alt = "The University of Western Australia"
            src="../static/uwacrest-text.png">
          </a>
          <span id="title">Site Administration</span>
        </div>
        <div id="rightnav">
 ';
 if (isUserLoggedIn()) {
echo '
            <span id="welcome-container">
              Welcome, '.$loggedInUser->displayname.'
            </span>
';
}
echo '
          <span id="date">'.date("D d M Y").'
          </span>
';

if(isUserLoggedIn()) {
	echo '
          <div id="users-logout-container">
            <form action="logout.php">
              <div>
                <input type="submit" id="logout-users" value="Logout">
              </div>
            </form>
          </div>
       ';
}

echo '
        </div>
        <div class="clear"></div>
      </div>
    </div>
    <!-- End header -->
    
    <div id="content-wrap">
      <noscript>
        <div class="widget centre">
          <div class="title">JavaScript required</div>
          This website requires JavaScript to function correctly.
          Please enable JavaScript to use this site.
        </div>
      </noscript>

      <div id="content">
 ';
 
 require_once("left-nav.php");
 if (isUserLoggedIn()) {
 echo '
        <div id="main">
';
}

} //function startPage()


function finishPage() {
if (isUserLoggedIn()) {
echo '
        </div>
      <!-- End main content -->
';
}

echo '
      </div>
    </div>
  </body>
</html>
';
}


?>
