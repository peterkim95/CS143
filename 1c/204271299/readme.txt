Project 1C
----------

"Ultimate Move Database"

This project completely satisfies the specifications and functionality requirements mentioned in the project description (http://oak.cs.ucla.edu/classes/cs143/project/project1C.html).

* index.php: acts as the main page of the web app where the user can search items or access any of the navigation bar controls
* navbar.php: navigation bar that is embedded using the 'include' function in PHP in every other php files; improves code reuse
* movie.php: shows the movie details where user can also add a review/rating
* actor.php: shows the actor details where user can see a list of movies that the respective actor has starred in
* add_ad.php: allows user to add a new actor or director
* add_ma.php: allows user to associate an existing actor to an existing movie by a given role
* add_md.php: allows user to associate an existing director to an existing movie
* add_movie.php: allows user to add a new movie
* add_review.php: allows user to add a review/rating for a given movie coming from the movie.php page
* bootstrap.min.css: minified CSS files that centralizes design langauge for all PHP pages in the web app

The design is a minimalistic version of the given demo website by the TA. Invalid inputs are handled on the web app layer through the PHP code. Development and testing was done on Chrome, and the selenium test cases were obviously developed on Firefox and double checked on it as well.

My project partner, Scott (204422750), implemented the four input pages (add_ad.php, add_ma.php, add_md.php, add_movie.php). On the other hand, I desgined and implemented index.php, navbar.php, add_review.php, movie.php, actor.php.

Looking back, we wish we could have researched more into different CSS or Javascript libraries that would have allowed our web app to become more interactive and good-looking.