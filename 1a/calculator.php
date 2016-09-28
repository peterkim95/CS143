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

      $err = @eval("\$ans=$expr;");
      if (error_get_last()) {
        echo "Error for whatever reason...distinguish them later? Spec doesn't mention";
      } else {
        echo $expr . " = " . $ans;
      }
    }

    ?>

  </body>
</html>
