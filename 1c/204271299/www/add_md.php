<!DOCTYPE html>
<html>
  <head>
    <title>UMD</title>
    <link rel="stylesheet" href="bootstrap.min.css">
    <style type="text/css">
      #brand span {
        text-decoration: underline;
      }

      #navbar a {
        padding-left: 15px;
      }

      form {
        padding-top: 20px;
      }

      li {
        padding-bottom: 10px;
      }

    </style>
    <center>
      <h1>Ultimate Movie Database</h1>
      <?php
        include('navbar.php');
      ?>
    </center>
    <hr>
  </head>
  <body>
    <div class="row">
      <div class="col-md-2"></div>
      <div class="col-md-8">
        <p class="lead">Adding a new director for a movie</p>
        <form method="get" action="<?php echo $_SERVER['PHP_SELF']; ?>">
          <label>Movie Title:</label><br>
            <select name="mid">
              <?php
              $db = new mysqli('localhost', 'cs143', '', 'CS143');

              if ($db->connect_errno > 0) {
                die('Unable to connect to database [' . $db->connect_error . ']');
              }

              $rs_movie = $db->query("SELECT * FROM Movie");
              while ($row = $rs_movie->fetch_assoc()) {
                echo '<option value="'.$row['id'].'">'.$row['title']. ' ('.$row['year'].')</option>';
              }

              ?>
            </select><br><br>
          <label>Director:</label><br>
            <select name="did">
              <?php
              $db = new mysqli('localhost', 'cs143', '', 'CS143');

              if ($db->connect_errno > 0) {
                die('Unable to connect to database [' . $db->connect_error . ']');
              }

              $rs_actor = $db->query("SELECT * FROM Director");
              while ($row = $rs_actor->fetch_assoc()) {
                echo '<option value="'.$row['id'].'">'.$row['first'].' '.$row['last'].'</option>';
              }

              ?>
            </select><br><br>
          <input id="submitButton" type="submit" value="Submit">
        </form>

        <?php
        
        if (isset($_GET['mid']) && isset($_GET['did'])) {
          
          $db = new mysqli('localhost', 'cs143', '', 'CS143');

          if ($db->connect_errno > 0) {
            die('Unable to connect to database [' . $db->connect_error . ']');
          }
          
          $db->query("INSERT INTO MovieDirector VALUES (".$_GET['mid'].",".$_GET['did'].")");

          echo '<br><hr><p class="lead">Director has been successfully added for the movie!</p>';
        }

        ?>

      </div>
      <div class="col-md-2"></div>
    </div>
  </body>
</html>