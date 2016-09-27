<!DOCTYPE html>
<html>
  <head>
    <title>PHP Calculator</title>
  </head>
  <body>

    <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="get">
      <input type="text" name="expr">
      <input type="submit" value="Calculate">
    </form>

    <?php

    if(isset($_GET['expr'])) {
      $expr = str_replace(' ', '', $_GET['expr']);
      $valid_chars_only = preg_match("/^[-+*.\/, 0-9]+$/", $expr);
      if ($valid_chars_only) {
        $err = @eval("\$ans=$expr;");
        if (error_get_last()) {
          echo "Error!";
        } else {
          echo $expr . " = " . $ans;
        }
      } else {
        echo "Error! no alphabets pls";
      }
    }

    ?>

  </body>
</html>
