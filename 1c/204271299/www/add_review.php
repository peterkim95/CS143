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

      h4 {
        display: inline;
      }

      p {
        display: inline;
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
        
        <?php
        if(isset($_GET['id'])) { 
          $db = new mysqli('localhost', 'cs143', '', 'CS143');

          if ($db->connect_errno > 0) {
            die('Unable to connect to database [' . $db->connect_error . ']');
          }

          $query = "SELECT * FROM Movie WHERE id='".$_GET['id']."'";
          $rs = $db->query($query);
          $row = $rs->fetch_assoc();
          echo '<h4>Adding a new review for </h4><p class="lead"><a href="movie.php?id='.$_GET['id'].'">'.$row['title'].'</a></p>';
          echo '<form method="get" action="'.$_SERVER['PHP_SELF'].'"><input type="hidden" name="id" value="'.$_GET['id'].'"><label>Your Name:</label> <input placeholder="John Abrams" type="text" name="name"><br><label>Rating:</label> <select name="rating"><option value="1">1</option><option value="2">2</option><option value="3">3</option><option value="4">4</option><option value="5">5</option></select><br><label>Comments:</label><br><textarea name="comment" cols="60" rows="8"></textarea><br><input type="submit" value="Submit"></form>';


          if (isset($_GET['name']) && isset($_GET['rating']) && isset($_GET['comment'])) {
            // name, time, mid, rating, comment
            $db->query("INSERT INTO Review VALUES ('".$_GET['name']."','".date("Y-m-d H:i:s", time())."','".$_GET['id']."','".$_GET['rating']."','".$_GET['comment']."')");
            echo '<br><hr><p class="lead">Thanks '.$_GET['name'].', your review has been successfully added! <a href="movie.php?id='.$_GET['id'].'">Go back to movie</a></p>';
          }

          $rs->free();
          $db->close();

        } else {
          echo '<p class="lead">No Movie ID Given.</p>';
        }

        ?>
      </div>
      <div class="col-md-2"></div>
    </div>
  </body>
</html>