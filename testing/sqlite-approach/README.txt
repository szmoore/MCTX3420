Note: Should be discarded in favour of the fastcgi approach

SQLite approach to interfacing with web frontend:

*Main program reads in sensor data and updates the sqlite database 
 with this new data
*When sqlite.php is called, the database is queried, and the values returned
 to the caller (format can be anything you like)
*You could have a main webpage, ie index.html
*Whenever you need to update the page, you use jQuery (or other method)
 to call sqlite.php, which returns the data needed

This works because SQLite allows concurrent access to the database
(or it should).