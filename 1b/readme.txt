Scott Shi -- 204422750
Peter Kim -- 204271299

-- Table creation constraints detail(see in comments of 'create.sql')
- Primary keys:
	* 1. Movie's 'id'
	* 2. Actor's 'id'
	* 3. Director's 'id'

- Foreign keys:
	* 1. MovieGenre's 'mid' references Movie's 'id'
	* 2. MovieDirector's 'mid' refs Movie's 'id'
	* 3. MovieDirector's 'did' refs Director's 'id'
	* 4. MovieActor's 'mid' refs Movie's 'id'
	* 5. MovieActor's 'aid' refs Actor's 'id'
	* 6. Review's 'mid' refs Movie's 'id'

- Checks:
	* 1. I looked up online when the first film was. So I set the lower
		bound for film be that year, and made up an upper bound.
	* 2. It was specified that the 'rating' attribute for the Movie table
		was it's MPAA rating, so I set a constraint to check for the
		possible MPAA ratings.
	* 3. In the spec the 'rating' was described as possibly a rating of x/5
		so I put the constraint to have the rating be between 0 and 5
