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
        
        <?php

        if(isset($_GET['id'])) {
          $db = new mysqli('localhost', 'cs143', '', 'CS143');

          if ($db->connect_errno > 0) {
            die('Unable to connect to database [' . $db->connect_error . ']');
          }

          $query = "SELECT * FROM Movie WHERE id='".$_GET['id']."'";
          $rs = $db->query($query);
          $rs_genre = $db->query("SELECT * FROM MovieGenre WHERE mid='".$_GET['id']."'");
          $genres = "";
          while ($row_genre = $rs_genre->fetch_assoc()) {
            $genres = $genres . $row_genre['genre'] . ", ";
          }
          $genres = substr($genres,0,-2);

          $row = $rs->fetch_assoc();
          echo '<p class="lead"><b>' . $row['title'] . '</b> ('. $row['year'] .')</p>';
          echo '<div>' . $row['rating'] . ' | ' . $genres . ' | ' . $row['company'] . '</div><br>';

          $rs_director = $db->query("SELECT * FROM MovieDirector AS md, Director AS d WHERE md.mid='".$_GET['id']."' AND d.id=md.did");
          if ($rs_director->num_rows == 0) {
            echo "<div><b>No Director</b></div><br>";
          } else {
            $row_director = $rs_director->fetch_assoc();
            echo '<div><b>Director:</b> ' . $row_director['first'] . ' ' . $row_director['last'] . '</div><br>';
          }
          
          $rs_actor = $db->query("SELECT a.id, CONCAT(a.first,' ',a.last) AS name, ma.role FROM MovieActor AS ma, Actor AS a WHERE a.id=ma.aid AND ma.mid='".$_GET['id']."'");
          echo '<div><b>Cast:</b></div><table class="table table-hover"><thead>';
          $keyrow = True;
          while($row = $rs_actor->fetch_assoc()) {
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
          echo "</tbody></table><hr><br><div><b>Reviews:</b></div>";

          $rs_review = $db->query("SELECT * FROM Review WHERE mid=".$_GET['id']." ORDER BY time DESC");
          $rs_avg = $db->query("SELECT AVG(rating) as avg FROM Review WHERE mid=".$_GET['id']);
          $row_avg = $rs_avg->fetch_assoc();
          if ($rs_review->num_rows == 0) {
            echo '<p class="lead">No Reviews Yet. Be the first one to <a href="add_review.php?id='.$_GET['id'].'">add a review.</a></p><br><br>';
          } else {
            echo '<div><label>Average Score: </label> '.$row_avg['avg'].'/5</div>';
            echo '<ul class="list-unstyled">';
            while ($row = $rs_review->fetch_assoc()) {
              echo '<li>' . $row['rating'] . '/5 by <i>' . $row['name'] . '</i> | '. date("j F Y (g:i a)" , strtotime($row['time']));
              echo '<p class="lead">'.$row['comment'].'</p></li>';
            }
            echo '</ul><br><p class="lead"><a href="add_review.php?id='.$_GET['id'].'">Add your review</a></p><br><br>';
          }

          $rs_review->free();
          $rs_genre->free();
          $rs_director->free();
          $rs_actor->free();
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
