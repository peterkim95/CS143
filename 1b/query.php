<!DOCTYPE html>
<html>
  <head>
    <title>IMDB</title>
  </head>
  <body>
    <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="get">
      <textarea name="query" cols="60" rows="8"></textarea>
      <input type="submit" value="Submit">
    </form>

    <?php

    if(isset($_GET['query'])) {
      $db = new mysqli('localhost', 'cs143', '', 'CS143');

      if ($db->connect_errno > 0) {
        die('Unable to connect to database [' . $db->connect_error . ']');
      }

      $query = $_GET['query'];
      $rs = $db->query($query);

      echo "<h3>Results from MySQL:</h3>";
      echo '<table border="1" cellspacing="1" cellpadding="2"><tbody>';

      $keyrow = True;
      while($row = $rs->fetch_assoc()) {
        echo '<tr align="center">';
        $key = array_keys($row);
        if ($keyrow) {
          for ($i = 0; $i < count($key); $i++) {
            echo '<td><b>'.$key[$i].'</b></td>';
          }
          echo '</tr><tr align="center">';
          $keyrow = False;
        }
        for ($i = 0; $i < count($key); $i++) {
          $k = $key[$i];
          if (is_null($row[$k])) {
            echo '<td>N/A</td>';
          } else {
            echo '<td>'.$row[$k].'</td>';
          }
        }
        echo "</tr>";
      }
      echo "</tbody></table>";

      $rs->free();
      $db->close();
    }

    ?>

  </body>
</html>
