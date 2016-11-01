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
        <p class="lead">Adding a new movie</p>
        <form method="get" action="<?php echo $_SERVER['PHP_SELF']; ?>">
          <label>Title:</label><br><input size="50" type="text" name="title"><br><br>
          <label>Company:</label><br><input size="50" type="text" name="company"><br><br>
          <label>Year:</label><br><input size="50" type="text" name="year"><br><br>
          <label>MPAA Rating: </label>
            <select name="rating">
              <option value="G">G</option>
              <option value="PG">PG</option>
              <option value="PG-13">PG-13</option>
              <option value="R">R</option>
              <option value="NC-17">NC-17</option>
            </select>
          <br>
          <label>Genre:</label><br>
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Action"> Action
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Adult"> Adult
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Adventure"> Adventure
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Animation"> Animation
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Comedy"> Comedy
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Crime"> Crime <br>
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Documentary"> Documentary
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Drama"> Drama
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Family"> Family
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Fantasy"> Fantasy
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Horror"> Horror
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Musical"> Musical <br>
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Mystery"> Mystery
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Romance"> Romance
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Sci-Fi"> Sci-Fi
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Short"> Short
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Thriller"> Thriller
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="War"> War
            <input class="genreCheckbox" type="checkbox" name="genre[]" value="Western"> Western
          <br><br>
          <input id="submitButton" type="submit" value="Submit">
        </form>
        <?php
        
        if (isset($_GET['title']) && isset($_GET['company']) && isset($_GET['year']) && isset($_GET['rating'])) {

          if (empty($_GET['title'])) {
            echo '<br><hr><p class="lead">Please enter movie title!</p>';
            exit;
          }

          if (empty($_GET['company'])) {
            echo '<br><hr><p class="lead">Please enter company name!</p>';
            exit;
          }

          if (strpos($_GET['year'],".") !== false || strpos($_GET['year'],"-") !== false || !is_numeric($_GET['year'])) {
            echo '<br><hr><p class="lead">Please enter a valid year!</p>';
            exit;
          }

          $db = new mysqli('localhost', 'cs143', '', 'CS143');

          if ($db->connect_errno > 0) {
            die('Unable to connect to database [' . $db->connect_error . ']');
          }
          $db->query("UPDATE MaxMovieID SET id=id+1");
          $rs_maxMovieID = $db->query("SELECT * FROM MaxMovieID");
          $row = $rs_maxMovieID->fetch_assoc();
          $insert_id = $row['id'];

          // echo "INSERT INTO Movie VALUES (".$insert_id.",'".$_GET['title']."','".$_GET['year']."','".$_GET['rating']."','".$_GET['company']."')";
          
          $db->query("INSERT INTO Movie VALUES (".$insert_id.",'".$_GET['title']."','".$_GET['year']."','".$_GET['rating']."','".$_GET['company']."')");

          if (isset($_GET['genre'])) {
            $query_genre = "INSERT INTO MovieGenre VALUES ";
            foreach ($_GET['genre'] as $genre) {
              $query_genre = $query_genre . "(".$insert_id.",'".$genre."'),";
            }
            $query_genre = substr($query_genre, 0, -1);
            $db->query($query_genre);
          }

          echo '<br><hr><p class="lead"><a href="movie.php?id='.$insert_id.'">'.$_GET['title'].'</a> has been successfully added!</p>';
          

        }

        ?>
      </div>
      <div class="col-md-2"></div>
    </div>
  </body>
</html>