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
        
        <?php

        if(isset($_GET['id'])) { 
          $db = new mysqli('localhost', 'cs143', '', 'CS143');

          if ($db->connect_errno > 0) {
            die('Unable to connect to database [' . $db->connect_error . ']');
          }

          $query = "SELECT id, CONCAT(first,' ',last) AS name, sex, dob AS born, dod AS died FROM Actor WHERE id='".$_GET['id']."'";
          $rs = $db->query($query);
          echo '<table class="table"><thead>';
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
              if ($k == "died" && is_null($row[$k])) {
                echo '<td>Still Alive</td>';
              } else if (is_null($row[$k])) {
                echo '<td>N/A</td>';
              } else {
                echo '<td>'.$row[$k].'</td>';
              }
            }
            echo "</tr>";
          }
          echo '</tbody></table><h4>Filmography</h4><ul class="list-unstyled">';

          $rs_role = $db->query("SELECT * FROM MovieActor AS ma, Movie AS m WHERE ma.aid='".$_GET['id']."' AND m.id=ma.mid ORDER BY year DESC");
          while($row = $rs_role->fetch_assoc()) {
            echo '<li><a href="movie.php?id='.$row['mid'].'">'.$row['title'].'</a> ('.$row['year'].')<br><i>'.$row['role'].'</i></li>';
          }

          $rs_role->free();
          $rs->free();
          $db->close();
        } else {
          echo '<p class="lead">No Actor ID Given.</p>';
        }

        ?>
      </div>
      <div class="col-md-2"></div>
    </div>
  </body>
</html>
