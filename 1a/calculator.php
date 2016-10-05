<!DOCTYPE html>
<html>
  <head>
    <title>PHP Calculator</title>
  </head>
  <body>
    <h1>Calculator</h1>
    <div>By Peter Kim & Scott Shi</div>
    <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="get">
      <input type="text" name="expr">
      <input type="submit" value="Calculate">
    </form>

    <?php

    if(isset($_GET['expr'])) {
      $expr = str_replace(' ', '', $_GET['expr']);  // eliminate spaces

      $valid_char = preg_match("/^[-+*.\/, 0-9]+$/", $expr);
      $valid_num_beginning = preg_match("/^[0]+[1-9]/", $expr);
      $valid_num = preg_match("/[^1-90][0]+[1-9]/", $expr);
      $decimal_beginning = preg_match("/^\./", $expr);
      $decimal = preg_match("/[^1-90]\./", $expr);
      $div_zero = preg_match("/\/[0]/", $expr);

      $expr = str_replace('--','- -',$expr);

      // echo $expr;
      if (empty($expr)) {
        // output nothing
      } else if (!$valid_char || $valid_num_beginning || $valid_num || $decimal_beginning || $decimal) {
        echo "<h1>Result</h1>";
        echo "Invalid Expression!";
      } else if ($div_zero) {
        echo "<h1>Result</h1>";
        echo "Division by zero error!";
      } else {
        echo "<h1>Result</h1>";
        $err = @eval("\$ans=$expr;");
        if (error_get_last()) {
          echo "Invalid Expression!";
        } else {
          echo $expr . " = " . $ans;
        }
      }


    }

    ?>

  </body>
</html>
