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
        <p class="lead">Adding a new actor/director</p>
        <form method="get" action="<?php echo $_SERVER['PHP_SELF']; ?>">
          <input type="radio" name="identity" value="actor" checked="true"> Actor &nbsp;&nbsp;
          <input type="radio" name="identity" value="director"> Director <br><br>
          <label>First Name:</label><br><input size="50" type="text" name="first"><br><br>
          <label>Last Name:</label><br><input size="50" type="text" name="last"><br><br>
          <input type="radio" name="gender" value="Male" checked="true"> Male &nbsp;&nbsp;
          <input type="radio" name="gender" value="Female"> Female <br><br>
          <label>Date of Birth:</label><br><input placeholder="1995-12-01" size="50" type="text" name="dob"><br><br>
          <label>Date of Death:</label><br><input placeholder="2087-11-03" size="50" type="text" name="dod"><br><span>Leave blank if actor is still alive</span><br><br>
          <input id="submitButton" type="submit" value="Submit">
        </form>
        <?php
        
        if (isset($_GET['identity']) && isset($_GET['first']) && isset($_GET['last']) && isset($_GET['gender']) && isset($_GET['dob'])) {

          if (empty($_GET['first'])) {
            echo '<br><hr><p class="lead">Please enter a valid first name!</p>';
            exit;
          }

          if (empty($_GET['last'])) {
            echo '<br><hr><p class="lead">Please enter a valid last name!</p>';
            exit;
          }

          function validateDate($date) {
            $d = DateTime::createFromFormat('Y-m-d', $date);
            return $d && $d->format('Y-m-d') === $date;
          }
          
          if (empty($_GET['dob']) || !validateDate($_GET['dob'])) {
            echo '<br><hr><p class="lead">Please enter a valid date of birth!</p>';
            exit;
          }

          if (!empty($_GET['dod'])) {
            if (!validateDate($_GET['dod'])) {
              echo '<br><hr><p class="lead">Please enter a valid date of death!</p>';
              exit;
            }
          }

          
          $db = new mysqli('localhost', 'cs143', '', 'CS143');

          if ($db->connect_errno > 0) {
            die('Unable to connect to database [' . $db->connect_error . ']');
          }

          $db->query("UPDATE MaxPersonID SET id=id+1");
          $rs_maxPersonID = $db->query("SELECT * FROM MaxPersonID");
          $row = $rs_maxPersonID->fetch_assoc();

          // echo $_GET['last'];
          // echo "INSERT INTO Actor VALUES (".$row['id'].",'".$_GET['last']."','".$_GET['first']."','".$_GET['gender']."','".$_GET['dob']."','".$_GET['dod']."')";
          if ($_GET['identity'] == "actor") {
            if (empty($_GET['dod'])) {
              $db->query("INSERT INTO Actor VALUES (".$row['id'].",'".$_GET['last']."','".$_GET['first']."','".$_GET['gender']."','".$_GET['dob']."',NULL)");
            } else {
              $db->query("INSERT INTO Actor VALUES (".$row['id'].",'".$_GET['last']."','".$_GET['first']."','".$_GET['gender']."','".$_GET['dob']."','".$_GET['dod']."')");
            }
          } else {
            if (empty($_GET['dod'])) {
              $db->query("INSERT INTO Director VALUES (".$row['id'].",'".$_GET['last']."','".$_GET['first']."','".$_GET['dob']."',NULL)");
            } else {
              $db->query("INSERT INTO Director VALUES (".$row['id'].",'".$_GET['last']."','".$_GET['first']."','".$_GET['dob']."','".$_GET['dod']."')");
            }
          }

          if ($_GET['identity'] == "actor") {
            echo '<br><hr><p class="lead">Actor has been successfully added!</p>';
          } else {
            echo '<br><hr><p class="lead">Director has been successfully added!</p>';
          }
          
        }

        ?>
      </div>
      <div class="col-md-2"></div>
    </div>
  </body>
</html>