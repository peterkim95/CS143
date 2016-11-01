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
        <p class="lead">Adding a new actor for a movie</p>
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
          <label>Actor:</label><br>
            <select name="aid">
              <?php
              $db = new mysqli('localhost', 'cs143', '', 'CS143');

              if ($db->connect_errno > 0) {
                die('Unable to connect to database [' . $db->connect_error . ']');
              }

              $rs_actor = $db->query("SELECT * FROM Actor");
              while ($row = $rs_actor->fetch_assoc()) {
                echo '<option value="'.$row['id'].'">'.$row['first'].' '.$row['last'].'</option>';
              }

              ?>
            </select><br><br>
          <label>Role:</label><br><input size="50" type="text" name="role"><br><br>
          <input id="submitButton" type="submit" value="Submit">
        </form>
        <?php
        
        if (isset($_GET['mid']) && isset($_GET['aid']) && isset($_GET['role'])) {
          
          $db = new mysqli('localhost', 'cs143', '', 'CS143');

          if ($db->connect_errno > 0) {
            die('Unable to connect to database [' . $db->connect_error . ']');
          }
          
          $db->query("INSERT INTO MovieActor VALUES (".$_GET['mid'].",".$_GET['aid'].",'".$_GET['role']."')");

          echo '<br><hr><p class="lead">Actor has been successfully added!</p>';
        }

        ?>
      </div>
      <div class="col-md-2"></div>
    </div>
  </body>
</html>