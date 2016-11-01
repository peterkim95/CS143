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
        <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="get">
          <input placeholder="Search for an actor or movie..." size="124" type="text" name="q">
          <input type="submit" value="Search">
        </form>
        <?php

        if(isset($_GET['q'])) { // show search results for q
          $db = new mysqli('localhost', 'cs143', '', 'CS143');

          if ($db->connect_errno > 0) {
            die('Unable to connect to database [' . $db->connect_error . ']');
          }

          $query_actor = "SELECT id, CONCAT(first,' ',last) AS name, sex, dob AS born, dod AS died FROM Actor WHERE ";
          $query_movie = "SELECT * FROM Movie WHERE ";
          $keywords = explode(" ", $_GET['q']);
          foreach ($keywords as $key) {
            $query_actor = $query_actor . "CONCAT(first,' ',last) LIKE '%" . $key . "%' AND ";
            $query_movie = $query_movie . "title LIKE '%" . $key . "%' AND ";
          }
          $query_actor = substr($query_actor, 0, -5);
          $query_movie = substr($query_movie, 0, -5);
          
          // echo $query_movie;
          // echo "<br>";
          // echo $query_actor;

          $rs = $db->query($query_movie);

          echo "<h3>" . $rs->num_rows . " Movie Results</h3>";

          echo '<table class="table table-hover"><thead>';

          $keyrow = True;
          while($row = $rs->fetch_assoc()) {
            echo '<tr>';
            $key = array_keys($row);
            if ($keyrow) {
              for ($i = 0; $i < count($key); $i++) {
                echo '<th>'.$key[$i].'</th>';
              }
              echo '</tr></thead><tbody><tr>';
              $keyrow = False;
            }
            for ($i = 0; $i < count($key); $i++) {
              $k = $key[$i];
              if (is_null($row[$k])) {
                echo '<td>N/A</td>';
              } else if ($k == "title") {
                echo '<td><a href="movie.php?id=' . $row['id'] . '">'.$row[$k].'</a></td>';
              } else {
                echo '<td>'.$row[$k].'</td>';
              }
            }
            echo "</tr>";
          }
          echo "</tbody></table><hr>";

          $rs2 = $db->query($query_actor);

          echo "<h3>" . $rs2->num_rows . " Actor Results</h3>";

          echo '<table class="table table-hover"><thead>';

          $keyrow = True;
          while($row = $rs2->fetch_assoc()) {
            echo '<tr>';
            $key = array_keys($row);
            if ($keyrow) {
              for ($i = 0; $i < count($key); $i++) {
                echo '<th>'.$key[$i].'</th>';
              }
              echo '</tr></thead><tbody><tr>';
              $keyrow = False;
            }
            for ($i = 0; $i < count($key); $i++) {
              $k = $key[$i];
              if (is_null($row[$k])) {
                echo '<td>N/A</td>';
              } else if ($k == "name") {
                echo '<td><a href="actor.php?id=' . $row['id'] . '">'.$row[$k].'</a></td>';
              } else {
                echo '<td>'.$row[$k].'</td>';
              }
            }
            echo "</tr>";
          }
          echo "</tbody></table>";

          $rs->free();
          $rs2->free();
          $db->close();
        }

        ?>
      </div>
      <div class="col-md-2"></div>
    </div>


    

  </body>
</html>
